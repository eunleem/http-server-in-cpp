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
  DEBUG_FUNC_START;
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
  while (this->dataFile_.eof() == false) {
    Invitation invitation;
    this->dataFile_ >> invitation;
    this->invitationById[invitation.id] = invitation;
    const std::string code(invitation.code, sizeof(invitation.code));
    this->invitationByCode[code] = &this->invitationById[invitation.id];

    count += 1;
    if (this->dataFile_.tellg() == fileSize) {
      break;
    } 
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


const Invitation& Invitations::GetInvitationById(const uint32_t invitId) {
  auto it = this->invitationById.find(invitId);

  if (it == this->invitationById.end()) {
    DEBUG_cerr << "Could not find invitation by id " << invitId << endl; 
    throw Exception(ExceptionType::GETINVIT);
  } 

  return it->second;
}

const Invitation& Invitations::GetInvitationByCode(const string& code) {
  auto it = this->invitationByCode.find(code);
  if (it == this->invitationByCode.end()) {
    DEBUG_cerr << "Could not find invitation by code. Code: " << code << endl; 
    throw Exception(ExceptionType::GETINVIT);
  } 

  return *(it->second);
}

const Invitation& Invitations::CreateNew(
    uint32_t topicId,
    const std::string& description,
    uint16_t numTickets,
    steadytime expiration)
{
#if 0
  if (topicId == 0) {
    DEBUG_cerr << "TopicId cannot be 0 or NULL." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 
#endif

  if (steady_clock::now() >= expiration) {
    DEBUG_cerr << "Expiration cannot be in the past." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 

  if (numTickets == 0) {
    DEBUG_cerr << "NumTickets available cannot be 0." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 

  this->summary_.lastid += 1;

  Invitation& invit = this->invitationById[this->summary_.lastid];

  invit.id = this->summary_.lastid;
  invit.topicId = topicId;
  invit.description = description;
  invit.numTickets = numTickets;
  invit.numRemaining = numTickets;
  invit.expiration = expiration;
  invit.created = steady_clock::now();

  std::string code = this->generateUniqueCode();
  code.copy(invit.code, sizeof(invit.code));

  this->invitationByCode[code] = &invit;

  bool isSaved = this->addInvitationToFile(invit);
  if (isSaved == false) {
    DEBUG_cerr << "Could not save invitation." << endl; 
    throw Exception(ExceptionType::CREATE);
  } 

  DEBUG_cout << "Created new Invitation. Id: " << invit.id << endl; 

  return this->invitationById[invit.id];
}

std::string Invitations::generateUniqueCode() {
  std::string code;
  for (int i = 0; i < 100; i++) {
    std::string code = Util::String::RandomString(sizeof(Invitation::code));
    auto itr = this->invitationByCode.find(code);
    if (itr == this->invitationByCode.end()) {
      // Successful
      return code;
    } 
    DEBUG_cout << "Duplicate Invitation Code has been generated. Regenerating..." << endl; 
  } 

  DEBUG_cerr << "Could not generate Unique Code." << endl; 
  return "";
}

ssize_t Invitations::RedeemTicket(const std::string& code) {
  auto invit = this->GetInvitationByCode(code);
  if (invit.numRemaining <= 0) {
    DEBUG_cerr << "No tickets available anymore." << endl; 
    return -1;
  } 

  bool result = this->UpdateNumRemaining(invit.id, invit.numRemaining - 1);
  if (result == false) {
    return -1;
  } 

  return static_cast<ssize_t>(invit.numRemaining);
}

bool Invitations::UpdateNumRemaining(const uint32_t id, uint16_t numRemaining) {
  auto it = this->invitationById.find(id);
  if (it == this->invitationById.end()) {
    DEBUG_cerr << "Invaild invitation Id. " << id << endl; 
    throw Exception(ExceptionType::UPDATE);
    return false; // Fallback command.
  } 
  this->invitationById[id].numRemaining = numRemaining;
  return true;
}

bool Invitations::UpdateDescription(const uint32_t id, std::string& description) {
  DEBUG_cerr << "Not Yet Implemented." << endl; 
  throw NotYetImplementedException();

  auto it = this->invitationById.find(id);
  if (it == this->invitationById.end()) {
    DEBUG_cerr << "Invaild invitation Id. " << id << endl; 
    throw Exception(ExceptionType::UPDATE);
    return false; // Fallback command.
  } 
  this->invitationById[id].description = description;
  return true;
}

bool Invitations::Delete(const uint32_t id) {
  DEBUG_cerr << "Not Yet Implemented." << endl; 
  throw NotYetImplementedException();

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


bool Invitations::addInvitationToFile(const Invitation& invitation) {
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


TEST(InvitationsTest, CreateNew) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  steadytime expTime = std::chrono::steady_clock::now() + std::chrono::hours(24);
  const Invitation& invit = invitations.CreateNew(0, "TestDescription", 10, expTime);
  EXPECT_GT(invit.id, 0);
  EXPECT_EQ(0, invit.topicId);
  EXPECT_EQ(std::strcmp("TestDescription", invit.description.c_str()), 0);


  EXPECT_TRUE(invitations.Close());
}

std::string code;

TEST(InvitationsTest, GetInvitById) {
  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  const Invitation& invit = invitations.GetInvitationById(1);
  EXPECT_EQ(1, invit.id);
  EXPECT_EQ(0, invit.topicId);
  ASSERT_EQ(std::strcmp("TestDescription", invit.description.c_str()), 0);
  code = std::string(invit.code, sizeof(invit.code));

  EXPECT_TRUE(invitations.Close());
}

TEST(InvitationsTest, GetInvitByCode) {
  ASSERT_GT(code.size(), 5);

  Invitations invitations(config);

  ASSERT_TRUE(invitations.Open());

  const Invitation& invit = invitations.GetInvitationByCode(code);
  EXPECT_EQ(1, invit.id);
  EXPECT_EQ(0, invit.topicId);
  EXPECT_EQ(std::strcmp("TestDescription", invit.description.c_str()), 0);

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

