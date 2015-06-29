#include "Row.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Row::Exception::exceptionMessages_[] = {
  ROW_EXCEPTION_MESSAGES
};
#undef ROW_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Row::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Row::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Row::ExceptionType
Row::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Row::Row() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

Row::~Row() {
  DEBUG_FUNC_START;

}

void Row::sampleFunc() {
  DEBUG_POINT; // prints out LINE NUMBER and a messaage "POINT REACHED" in GREEN.
  DEBUG_cout << "Prints out Debug message with Pid, FileName, and LineNumber in Blue" << endl;
  DEBUG_cout << "This line is removed when _DEBUG is false." << endl;
  
  DEBUG_cerr << "Prints out error message in Red." << endl; 
  DEBUG_cerr << "This line persists even if DEBUG mode is off. When Debug mode is on, it prints more information with color." << endl; 

  DEBUG_clog << "Prints out log" << endl;
}

//Row::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockRow : public Row {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Row, TESTNAME) {
  MockRow mockRow;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

