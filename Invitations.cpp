#include "Invitations.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Invitations::Exception::exceptionMessages_[] = {
  INVITATIONS_EXCEPTION_MESSAGES
};
#undef INVITATIONS_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Invitations::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Invitations::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Invitations::ExceptionType
Invitations::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 

Invitations::Invitations(Config config) :
  config_(config),
  summary_(config.dirPath)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
}

Invitations::~Invitations() {
}

bool Invitations::open() {
  srand(time(NULL));

  const std::string dataFilePath = this->config_.GetDataFilePath();
  DEBUG_cout << "Data File Path: " << dataFilePath << endl; 

  const bool IF_NOT_CREATE = true;
  bool isExisting = Util::IsFileExisting(dataFilePath, IF_NOT_CREATE);
  if (isExisting == false) {
    DEBUG_cerr << "Could not create file. Critical Error." << endl; 
    return false;
  } 

  this->summary_.Open();


  this->dataFile_.open(dataFilePath, 
      std::ios::in | std::ios::out | std::ios::binary);
  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "Could not open dataFile." << endl; 
    return false;
  } 

  this->LoadAllFromStorage();

  return true;
}

bool Invitations::close() {
  this->summary_.Close();

  if (this->dataFile_.is_open() == true) {
    this->dataFile_.flush();
    this->dataFile_.close();

  } else {
    DEBUG_cerr << "Data file is not open which is abnormal!" << endl; 
    return true;
  }

  return true;
}

ssize_t Invitations::LoadAllFromStorage() {
  ssize_t fileSize = Util::File::GetSize(this->dataFile_);

  this->dataFile_.seekg(0, std::ios_base::beg);

  size_t count = 0;

  // #TODO: Redesign this logic.
  while (this->dataFile_.good() == true) {
    if (this->dataFile_.tellg() == fileSize) {
      break;
    } 
    Invitation invitation;
    this->dataFile_ >> invitation;
    if (invitation.id == 0) {
      // InvitationId == 0 meaning it's been deleted.
      // Skipping on deleted invitation.
      continue;
    } 

    this->invitationById[invitation.id] = invitation;
    const std::string code(invitation.code, sizeof(invitation.code));
    this->invitationByCode[code] = &this->invitationById[invitation.id];

    count += 1;
  } 

  this->dataFile_.seekg(0);
  this->dataFile_.clear();
  DEBUG_cout << "Loaded " << count << " invitations." << endl; 

  return count;

}

ssize_t Invitations::SaveAllToStorage() {
  DEBUG_cerr << "This function is not suitable for this table." << endl; 
  return -1;
}

const std::unordered_map<uint32_t, Invitation>& Invitations::GetInvitations() const {
  return this->invitationById;
}


Invitation& Invitations::GetInvitationById(const uint32_t invitId) {
  auto it = this->invitationById.find(invitId);

  if (it == this->invitationById.end()) {
    DEBUG_cerr << "Could not find invitation by id " << invitId << endl; 
    throw Exception(ExceptionType::GETINVIT);
  } 

  return it->second;
}

Invitation& Invitations::GetInvitationByCode(const std::string& code) {
  if (code.length() < 5) {
    DEBUG_cerr << "Code is too short." << endl; 
    throw Exception(ExceptionType::GETINVIT);
  } 
  auto it = this->invitationByCode.find(code);
  if (it == this->invitationByCode.end()) {
    DEBUG_cerr << "Could not find invitation by code. Code: " << code << endl; 
    throw Exception(ExceptionType::GETINVIT);
  } 

  return *(it->second);
}

Invitation& Invitations::CreateNew(
    uint32_t topicId,
    //const std::string& description,
    uint16_t numTickets,
    datetime expiration)
{
#if 0
  if (topicId == 0) {
    DEBUG_cerr << "TopicId cannot be 0 or NULL." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 
#endif

  if (system_clock::now() >= expiration) {
    DEBUG_cerr << "WARNING! Expiration cannot be in the past." << endl; 
    //throw Exception(ExceptionType::CREATE);
  } 

  if (numTickets == 0) {
    DEBUG_cerr << "NumTickets available cannot be 0." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 

  this->summary_.lastid += 1;

  Invitation invit;

  invit.id = this->summary_.lastid;
  invit.topicId = topicId;
  invit.numTickets = numTickets;
  invit.numRemaining = numTickets;
  invit.expiration = expiration;
  invit.created = system_clock::now();

  const int COMPLEXITY = 4;

REGEN:
  Util::String::RandomString(invit.code, sizeof(invit.code), COMPLEXITY);
  std::string code(invit.code, sizeof(invit.code));
  if (this->invitationByCode.find(code) != this->invitationByCode.end()) {
    DEBUG_cout << "Regen..." << endl; 
    goto REGEN;
  } 

  this->dataFile_.seekp(0, std::ios_base::end);
  invit.pos = this->dataFile_.tellp();

  this->invitationById[invit.id] = invit;
  this->invitationByCode[code] = &this->invitationById[invit.id];

  bool isSaved = this->addInvitationToFile(invit);
  if (isSaved == false) {
    DEBUG_cerr << "Could not save invitation." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 

  DEBUG_cout << "Created new Invitation. Id: " << invit.id << endl; 

  return this->invitationById[invit.id];
}


size_t Invitations::RedeemTicket(const std::string& code) {
  Invitation& invit = this->GetInvitationByCode(code);
  if (invit.numRemaining <= 0) {
    DEBUG_cerr << "No tickets available anymore." << endl; 
    throw Exception(ExceptionType::INVIT_OUT);
  } 

  if (invit.expiration <= system_clock::now()) {
    DEBUG_cerr << "Ticket Expired." << endl; 
    throw Exception(ExceptionType::INVIT_EXP);
  } 

  invit.numRemaining -= 1;

  this->dataFile_.seekp(invit.pos, std::ios::beg);
  this->dataFile_ << invit;

  return static_cast<ssize_t>(invit.numRemaining);
}

bool Invitations::Delete(const uint32_t id) {

  auto itr = this->invitationById.find(id);
  if (itr == this->invitationById.end()) {
    DEBUG_cerr << "Delete failed. Invit does not exist." << endl; 
    return false;
  } 

  itr->second.id = 0;
  this->dataFile_ << itr->second;

  auto invitcodeitr = this->invitationByCode.find(std::string(itr->second.code, sizeof(itr->second.code)));

  this->invitationByCode.erase(invitcodeitr);
  this->invitationById.erase(itr);

  return true;


#if 0
  auto it = this->invitationById.find(id);
  if (it == this->invitationById.end()) {
    DEBUG_cerr << "Invalid Id." << endl; 
    return false;
  } 
  this->invitationById.erase(it);
  return true;
#endif
}


bool Invitations::addInvitationToFile(Invitation& invitation) {
  DEBUG_FUNC_START;
  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "Datafile is not open and cannot be added.." << endl; 
    return 0;
  } 

  this->dataFile_.seekp(0, std::ios_base::end);
  this->dataFile_ << invitation;

  if (this->dataFile_.bad() == true) {
    DEBUG_cerr << "DATAFILE WRITE BAD." << endl; 
  } 

  this->dataFile_.flush();

  return true;
}

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockInvitations : public Invitations {
public:
  MockInvitations(Invitations::Config config) : 
    Invitations(config) {} 
};

using ::testing::AtLeast;

Invitations::Config config("./testdata/invitations/", "invitations.data");

TEST(InvitationsTest, OpenAndClose) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());
  ASSERT_TRUE(invitations.Close());
}

TEST(InvitationsTest, GetInvitByWrongIdAndCode) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  EXPECT_THROW(invitations.GetInvitationById(100000), Invitations::Exception);
  EXPECT_THROW(invitations.GetInvitationByCode("3993FD9VKD30dfddf123"), Invitations::Exception);

  EXPECT_TRUE(invitations.Close());
}


TEST(InvitationsTest, CreateNew) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  datetime expTime = std::chrono::system_clock::now() + std::chrono::hours(24);
  Invitation& invit = invitations.CreateNew(0, 10, expTime);
  EXPECT_GT(invit.id, 0);
  EXPECT_EQ(0, invit.topicId);

  EXPECT_TRUE(invitations.Close());
}

TEST(InvitationsTest, CreateNewExpired) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  datetime expTime = std::chrono::system_clock::now();
  Invitation& invit = invitations.CreateNew(0, 3, expTime);

  std::string code(invit.code, sizeof(invit.code));
  EXPECT_THROW(invitations.RedeemTicket(code), Invitations::Exception);

  EXPECT_TRUE(invitations.Close());
}



std::string code;

TEST(InvitationsTest, GetInvitById) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  Invitation& invit = invitations.GetInvitationById(1);
  EXPECT_EQ(1, invit.id);
  EXPECT_EQ(0, invit.topicId);
  code = std::string(invit.code, sizeof(invit.code));
  DEBUG_cout << "code: " << code << endl; 

  EXPECT_TRUE(invitations.Close());
}

TEST(InvitationsTest, GetInvitByCode) {
  ASSERT_GT(code.length(), 6);

  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  Invitation& invit = invitations.GetInvitationByCode(code);
  DEBUG_cout << "code: " << code << endl; 
  DEBUG_cout << "invit.code: " << std::string(invit.code, sizeof(invit.code)) << endl; 
  EXPECT_EQ(1, invit.id);
  EXPECT_EQ(0, invit.topicId);

  EXPECT_TRUE(invitations.Close());
}

TEST(InvitationsTest, RedeemTicket) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  try {
    while (true) {
      size_t numRemaining = invitations.RedeemTicket(code);
      DEBUG_cout << "numRemaining: " << numRemaining << endl; 
      EXPECT_GT(10, numRemaining);
    } 
  } catch (Invitations::Exception& ex) {
    DEBUG_cerr << ex.what() << endl; 
  } 
  EXPECT_TRUE(invitations.Close());
}

TEST(InvitationsTest, Delete) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());


  bool deleted = invitations.Delete(1);
  EXPECT_TRUE(deleted);

  EXPECT_TRUE(invitations.Close());
}

TEST(InvitationsTest, GetDeletedInvitById) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  EXPECT_THROW(invitations.GetInvitationById(1), Invitations::Exception); 

  EXPECT_TRUE(invitations.Close());
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

