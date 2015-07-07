#include "Page.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Page::Exception::exceptionMessages_[] = {
  PAGE_EXCEPTION_MESSAGES
};
#undef PAGE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Page::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Page::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Page::ExceptionType
Page::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Page::Page() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

Page::~Page() {
  DEBUG_FUNC_START;

}

//Page::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockPage : public Page {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Page, TESTNAME) {
  MockPage mockPage;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

