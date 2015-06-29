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
  dirPath_(dirPath) 
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

ILioData::~ILioData() {
  DEBUG_FUNC_START;

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

TEST(ILioData, TESTNAME) {
  MockILioData mockILioData;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

