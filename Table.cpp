#include "Table.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Table::Exception::exceptionMessages_[] = {
  TABLE_EXCEPTION_MESSAGES
};
#undef TABLE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.

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



//Table::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockTable : public Table {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Table, TESTNAME) {
  MockTable mockTable;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

