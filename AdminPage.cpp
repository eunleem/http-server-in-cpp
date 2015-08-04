#include "AdminPage.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
AdminPage::Exception::exceptionMessages_[] = {
  ADMINPAGE_EXCEPTION_MESSAGES
};
#undef ADMINPAGE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


AdminPage::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
AdminPage::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const AdminPage::ExceptionType
AdminPage::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


AdminPage::AdminPage() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

AdminPage::~AdminPage() {
  DEBUG_FUNC_START;

}


bool AdminPage::Process(HttpConnection* connection, ILioData* data) {
  
  HttpRequest& request = *connection->GetRequest();
  HttpResponse& response = *connection->GetResponse();

  std::string sessionid = request.GetCookie("sessionid").ToString();
  if (sessionid.length() == 0 || sessionid.length() > 32) {
    return false; // returns 404 NOT FOUNT
  } 

  bool isAdmin = data->IsAdmin(sessionid);
  if (isAdmin == false) {
    DEBUG_cerr << "Someone who is not admin tried to access admin page." << endl; 
    return false;
  } 
  
  response.SetResponseCode(http::ResponseCode::OK);

  HttpPostData posted(request.GetBody());

  std::string action = posted.GetData("action").ToString();
  DEBUG_cout << "Action: " << action << endl; 

  if (action == "") {
    DEBUG_cerr << "Action is not specified." << endl; 
    response.SetBody(this->invalid_action, strlen(this->invalid_action), http::ContentType::JSON);
    return true;
  } 


  try {
    if (action == "getinvitations") {
      const std::unordered_map<uint32_t, Invitation>& invits = data->GetInvitations();
      size_t sendSize = 20;
      if (invits.size() < 20) {
        sendSize = invits.size();
      } 

#if 0
      size_t pageNum = 1;
      std::string pageStr = posted.GetData("page").ToString();
      if (pageStr.empty() == false) {
        pageNum = Util::String::ToUInt(pageStr);
      } 

      size_t count = sendSize % (20 * pageNum);
#endif

      std::string responseStr;

      auto itr = invits.begin();
      
      for (size_t i = 0; sendSize > i; i++) {
        const Invitation& invit = itr->second;
        responseStr += std::to_string(invit.id);
        responseStr += "::";
        responseStr += invit.GetCode();
        responseStr += "::";
        responseStr += std::to_string(invit.numRemaining);
        responseStr += "/";
        responseStr += std::to_string(invit.numTickets);
        responseStr += "::";
        responseStr += Util::Time::TimeToString(invit.expiration);
        responseStr += "\n";
        
        itr++;
      } 

      response.SetBody(responseStr, http::ContentType::PLAINTEXT);
      return true;

    } 

    if (action == "addinvitation") {
      std::string numticketsStr = posted.GetData("numtickets").ToString();
      if (numticketsStr.empty() == true) {
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      } 
      std::string expirationHoursStr = posted.GetData("expirationhours").ToString();
      if (expirationHoursStr.empty() == true) {
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      std::string description = posted.GetData("description").ToString();

      size_t numTickets = Util::String::ToUInt(numticketsStr);
      size_t expirationHours = Util::String::ToUInt(expirationHoursStr);

      datetime expiration = std::chrono::system_clock::now() + std::chrono::hours(expirationHours);

      Invitation& invit = data->AddInvitation(description, numTickets, expiration);

      std::string responseStr;
      responseStr += std::to_string(invit.id);
      responseStr += "::";
      responseStr += invit.GetCode();
      responseStr += "::";
      responseStr += std::to_string(invit.numRemaining);
      responseStr += "/";
      responseStr += std::to_string(invit.numTickets);
      responseStr += "::";
      responseStr += Util::Time::TimeToString(invit.expiration);
      responseStr += "\n";

      response.SetBody(responseStr, http::ContentType::PLAINTEXT);
      return true;
    }


    if (action == "getusers") {
      DEBUG_cout << "NOW HERE." << endl; 
      auto& lives = data->GetLives();
      DEBUG_cout << "Size of Lives: " << lives.size() << endl; 
      std::string responseStr;
      for (auto& life : lives) {
        responseStr += std::to_string(life.second.id);
        responseStr += "::";
        responseStr += std::to_string((int)life.second.status);
        responseStr += "::";
        responseStr += std::to_string((int)life.second.group);
        responseStr += "::";
        responseStr += life.second.GetDna();
        responseStr += "::";
        responseStr += Util::Time::TimeToString(life.second.created);
        responseStr += "\n";
      } 

      DEBUG_cout << "reponseStr: " << responseStr << endl; 

      response.SetBody(responseStr, http::ContentType::PLAINTEXT);
      return true;

    } 
  } catch (Invitations::Exception& ex) {
    switch (ex.type()) {
      case Invitations::ExceptionType::GETINVIT:
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        break;
      default:
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        break;
    } 
    return true;
  } 


  return false;
  
}

//AdminPage::
//AdminPage::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockAdminPage : public AdminPage {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(AdminPage, TESTNAME) {
  MockAdminPage mockAdminPage;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

