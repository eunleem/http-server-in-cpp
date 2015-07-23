#include "ILioData.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
ILioData::Exception::exceptionMessages_[] = {
  ILIODATA_EXCEPTION_MESSAGES
};
#undef ILIODATA_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


ILioData::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
ILioData::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const ILioData::ExceptionType
ILioData::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


ILioData::ILioData(std::string dirPath) :
  dirPath_(dirPath),
  sessions_(Sessions::Config(dirPath + "sessions/")),
  lives_(dirPath + "lives/"),
  invitations_(Invitations::Config(dirPath + "invitations/")),
  ideas_(dirPath + "ideas/"),
  contents_(dirPath + "contents/")
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

ILioData::~ILioData() {
  DEBUG_FUNC_START;

}

bool ILioData::open() {
  this->sessions_.Open();
  this->lives_.Open();
  this->invitations_.Open();

  this->ideas_.Open();
  this->contents_.Open();
  return true;
}

bool ILioData::close() {

  this->sessions_.Close();
  this->lives_.Close();
  this->invitations_.Close();

  this->ideas_.Close();
  this->contents_.Close();

  return true;
}

Invitation& ILioData::AddInvitation(
    const std::string& description,
    size_t numTickets,
    datetime expiration)
{
  Invitation& invit = this->invitations_.CreateNew(0, description, numTickets, expiration);
  return invit;
}

std::pair<Invitation&, std::string> ILioData::GetInvitationByCode(const std::string& invitcode) {
  Invitation& invit =  this->invitations_.GetInvitationByCode(invitcode); // THROWS
  
  if (invit.expiration <= std::chrono::system_clock::now()) {
    throw Invitations::Exception(Invitations::ExceptionType::INVIT_EXP);
  } 
  if (invit.numRemaining == 0) {
    throw Invitations::Exception(Invitations::ExceptionType::INVIT_OUT);
  } 

  std::string description = this->invitations_.GetInvitationDescriptionByCode(invitcode);
  return std::pair<Invitation&, std::string>(invit, description);
}

std::pair<const Life*, std::string> ILioData::SignUp(std::string invitcode) {
  this->invitations_.RedeemTicket(invitcode); // THROWS 

  std::pair<const Life*, std::string> newLife = this->lives_.CreateLife();
  return newLife;
}

std::pair<std::string, Session&> ILioData::Login(std::string dna, std::string code) {
  const Life* life = this->lives_.GetLifeByDnaAndSecretCode(dna, code);
  if (life == nullptr) {
    throw Exception(ExceptionType::LOGIN);
  } 

  Session session(life->id, std::chrono::minutes(120));
  std::string sessionId = this->sessions_.AddSession(session);

  return std::pair<std::string, Session&>(sessionId, session);
}

lifeid_t ILioData::GetLifeIdBySessionId(std::string sid) {
  if (sid.length() < 6) {
    DEBUG_cout << "SessionId is too short or empty. Assuming not Logged In." << endl; 
    return 0;
  } 

  try {
    Session& session = this->sessions_.GetSession(sid);
    return session.lifeid;

  } catch (Sessions::Exception& ex) {
    DEBUG_cout << "ex.what(): " << ex.what() << endl; 
    return 0;
  }

  return 0;
}

Life& ILioData::GetLifeById(lifeid_t lifeid) {
  // THROWS Lives::Exception on failed look up.
  const Life* life = this->lives_.GetLifeById(lifeid);
  return const_cast<Life&>(*life);
}


ideaid_t ILioData::PostIdea(lifeid_t lifeId, std::string& content,
    Idea::Type type, Idea::Permission perm) {

  // Check if lifeId is non-zero
  if (lifeId == 0) {
    DEBUG_cerr << "lifeId is 0!!! Allowing it for now!" << endl;
  }

  if (content.length() > 50000) {
    DEBUG_cerr << "Content is TOO long." << endl;
    return 0;
  }

  bool isSafeUserInput = Util::String::IsUserInputSafe(content);
  if (isSafeUserInput == false) {
    DEBUG_cerr << "Content is not safe." << endl;
    return 0;
  }

  // Check content for unsafe or invalid chars for json/html
  //std::string encoded = Util::String::JsonEncode(content);
  bool isSafeJson = Util::String::IsSafeForJson(content);
  if (isSafeJson == false) {
    DEBUG_cerr << "NOT SAFE FOR JSON!" << endl;
    return 0;
  }

  contentid_t contentId = 0;

  std::string ideatitle;

  if (content.length() > Idea::GetTitleSize()) {
    ideatitle = content.substr(0, Idea::GetTitleSize());
    auto numBytesTrimmed = Util::String::TrimIncompleteUTF8(ideatitle);
    DEBUG_cout << "Trimmed " << numBytesTrimmed << " bytes for UTF-8 string!"<< endl;

    Content newContent;
    newContent.SetContent(content);
    newContent.SetType(Content::Type::GENERAL);

    contentId = this->contents_.AddContent(newContent);

  } else {
    ideatitle = content;
  }

  ideaid_t newIdeaId = this->ideas_.AddIdea(lifeId, ideatitle, contentId);

  if (newIdeaId == 0) {
    DEBUG_cerr << "Could not Add Idea!" << endl;
  }

  return newIdeaId;
}

//ILioData::
//ILioData::




bool ILioData::IsAdmin(const std::string& sid) {
  try {
    lifeid_t lifeid = this->GetLifeIdBySessionId(sid);
    if (lifeid == 0) {
      DEBUG_cout << "LifeId is O" << endl; 
      return false;
    } 
    if (lifeid == 1) {
      DEBUG_cout << "LifeId is 1. ADMIN!" << endl; 
      return true;
    } 
#if 0
    Life& life = this->GetLifeById(lifeid);
    if (life.group == Life::Group::ME ||
        life.id == 1) {
      return true;
    } 
#endif
    DEBUG_cerr << "WHAT??? LifeId: " << lifeid << endl; 
  } catch (...) {
    DEBUG_cerr << "ERROR CAUGHT! NO ADMIN." << endl; 
    return false;
  } 

  return false;
}

const std::unordered_map<uint32_t, Invitation>& ILioData::GetInvitations() const {
  return this->invitations_.GetInvitations();
}

const std::unordered_map<lifeid_t, Life>& ILioData::GetLives() const {
  return this->lives_.GetLives();
}
//ILioData::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockILioData : public ILioData {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(ILioData, OpenAndClose) {

}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

