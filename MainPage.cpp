#include "MainPage.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
MainPage::Exception::exceptionMessages_[] = {
  MAINPAGE_EXCEPTION_MESSAGES
};
#undef MAINPAGE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


MainPage::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
MainPage::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const MainPage::ExceptionType
MainPage::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


MainPage::MainPage() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

MainPage::~MainPage() {
  DEBUG_FUNC_START;

}

bool MainPage::Process(HttpConnection* connection, ILioData* data) {
  HttpRequest& request = *connection->GetRequest();
  HttpResponse& response = *connection->GetResponse();

  std::string uri = request.GetUri();

  return false
}

//MainPage::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockMainPage : public MainPage {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(MainPage, TESTNAME) {
  MockMainPage mockMainPage;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

