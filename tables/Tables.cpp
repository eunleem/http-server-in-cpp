#include "Tables.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Table::Exception::exceptionMessages_[] = {
  TABLE_EXCEPTION_MESSAGES
};
#undef TABLES_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Table::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Table::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Table::ExceptionType
Table::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 

Table::Table() {
  DEBUG_FUNC_START; // Prints out function name in yellow
}

Table::~Table() {
  DEBUG_FUNC_START;
}




//Tables::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockTables : public Tables {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Tables, TESTNAME) {
  MockTables mockTables;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

