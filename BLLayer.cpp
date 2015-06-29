#include "BLLayer.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
BLLayer::Exception::exceptionMessages_[] = {
  BLLAYER_EXCEPTION_MESSAGES
};
#undef BLLAYER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


BLLayer::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
BLLayer::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const BLLayer::ExceptionType
BLLayer::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


BLLayer::BLLayer(Config config) :
  config_(config)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

BLLayer::~BLLayer() {
  DEBUG_FUNC_START;

  if (this->status_ == Status::OPEN) {
    this->Close();
  } 
}


bool BLLayer::Open() {
  if (this->status_ == Status::OPEN) {
    DEBUG_cout << "Already open." << endl; 
    return true;
  } 

  this->lives_.Open();


  this->status_ = Status::OPEN;
  return true;
}

bool BLLayer::Close() {
  if (this->status_ == Status::CLOSED) {
    DEBUG_cout << "Already Closed." << endl; 
    return true;
  }

  this->lives_.Close();

  this->status_ = Status::CLOSED;
  return true;
}

lifeid_t BLLayer::SignUpByInvitationCode(const string& code) {

}


bool BLLayer::IsLoggedIn(const std::string& dna, const std::string& code) {

}


Lives::Life& BLLayer::GetLife() const {


}

//BLLayer::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockBLLayer : public BLLayer {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(BLLayer, TESTNAME) {
  MockBLLayer mockBLLayer;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

