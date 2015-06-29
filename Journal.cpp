#include "Journal.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Journal::Exception::exceptionMessages_[] = {
  JOURNAL_EXCEPTION_MESSAGES
};
#undef JOURNAL_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Journal::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Journal::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Journal::ExceptionType
Journal::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Journal::Journal() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

Journal::~Journal() {
  DEBUG_FUNC_START;

}


//Journal::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockJournal : public Journal {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Journal, TESTNAME) {
  MockJournal mockJournal;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

