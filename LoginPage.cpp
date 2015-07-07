#include "LoginPage.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

LoginPage::LoginPage() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

LoginPage::~LoginPage() {
  DEBUG_FUNC_START;

}

bool LoginPage::Process(HttpConnection* connection, ILioData* data) {

  HttpRequest& request = *connection->GetRequest();

  HttpResponse& response = *connection->GetResponse();
  response.SetResponseCode(http::ResponseCode::OK);

  HttpPostData posted(request.GetBody());
  std::string sessionid = request.GetCookie("sessionid").ToString();
  if (sessionid.length() > 0) {
    DEBUG_cout << "sessionid: " << sessionid << endl; 
    lifeid_t lifeid = data->GetLifeIdBySessionId(sessionid);
    DEBUG_cout << "lifeid: " << lifeid << endl; 
    if (lifeid > 0) {
      response.SetBody(this->already_logged_in, strlen(this->already_logged_in), http::ContentType::JSON);
      return true;
    } 
  } 

  std::string code = posted.GetData("code").ToString();
  size_t totalCodeLength = static_cast<size_t>(Life::DNA_LENGTH + Life::SECRET_CODE_LENGTH);
  if (code.length() < totalCodeLength) {
    DEBUG_cerr << "Login Code is too short." << endl; 

    response.SetBody(this->login_failed, strlen(this->login_failed), http::ContentType::JSON);
    return true;
  } 


  try {
    std::string dna = code.substr(0, Life::DNA_LENGTH);
    std::string pwd = code.substr(Life::DNA_LENGTH, Life::SECRET_CODE_LENGTH);
    std::pair<std::string, Session&> session = data->Login(dna, pwd);
    std::string& sessionCode = session.first;
    response.SetCookie("sessionid", sessionCode,
        std::chrono::system_clock::now() + std::chrono::minutes(120));
    response.SetBody(this->successful_login, strlen(this->successful_login), http::ContentType::JSON);
    return true;

  } catch (Lives::Exception& ex) {
    response.SetBody(this->login_failed, strlen(this->login_failed), http::ContentType::JSON);
    return true;

  } catch (...) {
    response.SetBody(this->error, strlen(this->error), http::ContentType::JSON);
    return true;

  }

  return false;
}



//LoginPage::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockLoginPage : public LoginPage {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(LoginPage, TESTNAME) {
  MockLoginPage mockLoginPage;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

