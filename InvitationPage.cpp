#include "InvitationPage.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
InvitationPage::Exception::exceptionMessages_[] = {
  INVITATIONPAGE_EXCEPTION_MESSAGES
};
#undef INVITATIONPAGE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


InvitationPage::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
InvitationPage::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const InvitationPage::ExceptionType
InvitationPage::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


InvitationPage::InvitationPage() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

InvitationPage::~InvitationPage() {
  DEBUG_FUNC_START;

}

bool InvitationPage::Process(HttpConnection* connection, ILioData* data) {

  HttpRequest& request = *connection->GetRequest();

  HttpResponse& response = *connection->GetResponse();
  response.SetResponseCode(http::ResponseCode::OK);

  HttpPostData posted(request.GetBody());

  std::string action = posted.GetData("action").ToString();

  if (action == "") {
    action = "open";
  } 

  if (action == "open" ||
      action == "signup")
  {
    std::string code = posted.GetData("code").ToString();
    if (code.length() < 5) {
      DEBUG_cerr << "Code length is too short." << endl; 
      response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
      return true;
    } 

    try {
      if (action == "open") {

        Invitation invit;
        std::string description;
        //std::pair<Invitation&, std::string> invitAndDescription = data->GetInvitationByCode(code);
        std::tie(invit, description) = data->GetInvitationByCode(code);
        DEBUG_cout << "Description:" << description << endl; 
        if (description.length() == 0 ) {
          response.SetBody(this->successful, strlen(this->successful), http::ContentType::JSON);
          return true;
        } 

        std::string responseJson = "{ \"code\": 0, \"description\": \"" +
          description + "\"}";

        response.SetBody(responseJson, http::ContentType::JSON);
        return true;

      } else if (action == "signup") {
        std::pair<const Life*, std::string> lifeAndCode = data->SignUp(code);
        DEBUG_cout << "GetDNA: " << lifeAndCode.first->GetDna() << endl; 
        DEBUG_cout << "second: " << lifeAndCode.second << endl; 

        std::string dna = lifeAndCode.first->GetDna();
        std::string scode = lifeAndCode.second.substr(0, Life::SECRET_CODE_LENGTH);
        DEBUG_cout << "length: " << dna.length() << endl; 
        DEBUG_cout << "scodeleng: " << scode.length() << endl; 

        std::string responseJson = "{\"code\": 0, \"account_code\": \"" + dna + scode + "\"}";
        DEBUG_cout << "responseJson: " << responseJson << endl; 

        std::pair<std::string, Session&> session = data->Login(dna, scode);

        response.SetCookie("sessionid", session.first);
        response.SetBody(responseJson, http::ContentType::JSON);

        return true;
      }
    } catch (Invitations::Exception& ex) {
      switch (ex.type()) {
        case Invitations::ExceptionType::GETINVIT:
          response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
          break;
        case Invitations::ExceptionType::INVIT_EXP:
          response.SetBody(this->expired, strlen(this->expired), http::ContentType::JSON);
          break;
        case Invitations::ExceptionType::INVIT_OUT:
          response.SetBody(this->nomore, strlen(this->nomore), http::ContentType::JSON);
          break;
        default:
          response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
          break;
      } 
      return true;
    } 

  } else {
    response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
    return false;
  }

  return true;
}

//InvitationPage::
//InvitationPage::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockInvitationPage : public InvitationPage {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(InvitationPage, TESTNAME) {
  MockInvitationPage mockInvitationPage;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

