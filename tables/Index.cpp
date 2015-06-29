#include "Index.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio { namespace db {

// ===== Exception Implementation ===== 
const char* const
Index::Exception::exceptionMessages_[] = {
  INDEX_EXCEPTION_MESSAGES
};
#undef INDEX_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Index::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Index::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Index::ExceptionType
Index::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Index::Index() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

Index::~Index() {
  DEBUG_FUNC_START;

}


//Index::

} }

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockIndex : public Index {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Index, TESTNAME) {
  MockIndex mockIndex;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

