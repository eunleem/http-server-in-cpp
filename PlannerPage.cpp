#include "PlannerPage.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
PlannerPage::Exception::exceptionMessages_[] = {
  PLANNERPAGE_EXCEPTION_MESSAGES
};
#undef PLANNERPAGE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


PlannerPage::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
PlannerPage::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const PlannerPage::ExceptionType
PlannerPage::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


PlannerPage::PlannerPage() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

PlannerPage::~PlannerPage() {
  DEBUG_FUNC_START;

}

bool PlannerPage::Process(HttpConnection* connection, ILioData* data) {

  HttpRequest& request = *connection->GetRequest();

  HttpResponse& response = *connection->GetResponse();
  response.SetResponseCode(http::ResponseCode::OK);

  std::string sessionid = request.GetCookie("sessionid").ToString();
  if (sessionid.length() == 0 || sessionid.length() > 32) {
    return false; // returns 404 NOT FOUNT
  }

  lifeid_t lifeid = data->GetLifeIdBySessionId(sessionid); // This extends session expiration by 2 hours
  if (lifeid == 0) {
    response.SetBody(this->notloggedin, strlen(this->notloggedin), http::ContentType::JSON);
    return true;
  }

  

  HttpPostData posted(request.GetBody());

  std::string action = posted.GetData("action").ToString();
  DEBUG_cout << "Action: " << action << endl;

  if (action == "") {
    DEBUG_cerr << "Action is not specified." << endl;
    response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
    return true;
  } 


  try {
    if (action == "postidea") {

      std::string content = posted.GetData("content").ToString();
      DEBUG_cout << "RawContent: " << content << "END len: " << content.length() << endl;
      ideaid_t newIdeaId = data->PostIdea(lifeid, content);
      if (newIdeaId == 0) {
        DEBUG_cerr << "newIdeaId is 0" << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      std::string responseStr;
      responseStr =
          "{\"code\": 0, \"ideaid\": " + std::to_string(newIdeaId) + "}";

      response.SetBody(responseStr, http::ContentType::JSON);
      return true;
    } 


    if (action == "getidea") {
      std::string ideaidStr = posted.GetData("ideaid").ToString();
      if (ideaidStr == "") {
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      ideaid_t ideaid = Util::String::To<ideaid_t>(ideaidStr);

      const Idea* idea = data->GetIdeaById(ideaid);
      if (idea == nullptr) {
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      std::string contentStr;

      contentStr = idea->GetTitle();

      if (idea->GetContentId() != 0) {
        const Content* content = data->GetContentById(idea->GetContentId());
        if (content == nullptr) {
          DEBUG_cerr << "ContentNotFound. id:" << idea->GetContentId() << endl;
          response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
          return true;
        }
        contentStr = content->GetContent();
      }

      DEBUG_cout << "ContentStr: " << contentStr << "END" << endl;
      DEBUG_cout << "ContentStrLength: " << contentStr.length() << endl;

      std::string responseStr = "{ \"code\": 0, \"ideaid\": " +
                                std::to_string(idea->GetId()) + ", " +
                                "\"content\": \"" + contentStr.c_str() + "\" }";
      response.SetBody(responseStr, http::ContentType::JSON);
      return true;
    }
    
    if (action == "getideas") {
      std::string grouptype = posted.GetData("type").ToString();
      if (grouptype == "") {
        DEBUG_cout << "getideas.type is not set. Set it to 'latest'." << endl;
        grouptype = "latest";
      }

      if (grouptype != "latest") {
        DEBUG_cerr << "GroupType other than latest is not supported yet." << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      size_t count = 20;
      std::string countStr = posted.GetData("count").ToString();
      if (countStr != "") {
        count = Util::String::To<size_t>(countStr);
      }

      size_t skip = 0;
      std::string skipStr = posted.GetData("skip").ToString();
      if (skipStr != "") {
        skip = Util::String::To<size_t>(skipStr);
      }



      std::vector<Idea*> ideas = data->GetIdeas(count, skip);

      rapidjson::Document jsDoc;
      jsDoc.SetObject();

      rapidjson::Document::AllocatorType& jsAlloc = jsDoc.GetAllocator();

      rapidjson::Value jsvCode;
      jsvCode.SetInt(0);

      jsDoc.AddMember("code", jsvCode, jsAlloc);

      rapidjson::Value jsvIdeas(rapidjson::kArrayType);

      for (const auto& idea : ideas) {
        rapidjson::Value jsvIdea;
        jsvIdea.SetObject();

        rapidjson::Value jsvIdeaId;
        jsvIdeaId.SetUint(idea->GetId());
        jsvIdea.AddMember("ideaid", jsvIdeaId, jsAlloc);

        rapidjson::Value ideaCreatedDate;
        uint64_t milliseconds_since_epoch =
            Util::Time::GetMillisecondsSinceEpoch(idea->GetCreatedTime());
        ideaCreatedDate.SetUint64(milliseconds_since_epoch);
        jsvIdea.AddMember("created", ideaCreatedDate, jsAlloc);

        std::string contentStr;
        if (idea->GetContentId() == 0) {
          contentStr = idea->GetTitle();
        } else {
          const Content* content = data->GetContentById(idea->GetContentId());
          if (content == nullptr) {
            DEBUG_cerr << "ContentNotFound. id:" << idea->GetContentId() << endl;
            response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
            return true;
          }
          contentStr = content->GetContent();
        }

        rapidjson::Value ideaContent;
        ideaContent.SetString(contentStr.c_str(), strlen(contentStr.c_str()));
        jsvIdea.AddMember("content", ideaContent, jsAlloc);


        jsvIdeas.PushBack(jsvIdea, jsAlloc);
      }

      jsDoc.AddMember("ideas", jsvIdeas, jsAlloc);

      response.SetBody(jsDoc);
      return true;
    }

    if (action == "editidea") {
      std::string ideaidStr = posted.GetData("ideaid").ToString();
      if (ideaidStr == "") {
        DEBUG_cerr << "ideaid is required!" << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      ideaid_t ideaid = Util::String::To<ideaid_t>(ideaidStr);

      std::string content = posted.GetData("content").ToString();
      DEBUG_cout << "content: " << content << endl;
      if (content == "") {
        DEBUG_cerr << "content is required!" << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      bool isUpdated = data->UpdateIdea(ideaid, content);
      if (isUpdated == false) {
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      response.SetBody(this->successful, strlen(this->successful), http::ContentType::JSON);
      return true;
    }

    if (action == "gethistory") {
      std::string ideaidStr = posted.GetData("ideaid").ToString();
      if (ideaidStr == "") {
        DEBUG_cerr << "ideaid is required!" << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      ideaid_t ideaid = Util::String::To<ideaid_t>(ideaidStr);
      if (ideaid == 0) {
        DEBUG_cerr << "ideaid cannot be zero!" << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }

      const Idea* idea = data->GetIdeaById(ideaid);
      if (idea == nullptr) {
        DEBUG_cerr << "idea cannot be found!" << endl;
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        return true;
      }
      
      const contentid_t contentId = idea->GetContentId();
      DEBUG_cout << "contentId: " << contentId << endl;
      std::vector<contentid_t> prevContentIds = data->GetAllPrevContents(contentId);
      DEBUG_cout << "prevContentIds.size: " << prevContentIds.size() << endl;

      rapidjson::Document jsDoc;
      jsDoc.SetObject();

      rapidjson::Document::AllocatorType& jsAlloc = jsDoc.GetAllocator();

      rapidjson::Value jsvCode;
      jsvCode.SetInt(0);

      jsDoc.AddMember("code", jsvCode, jsAlloc);

      rapidjson::Value jsvIds(rapidjson::kArrayType);

      for (const auto id : prevContentIds) {
        rapidjson::Value jsvId;
        jsvId.SetUint(id);

        jsvIds.PushBack(jsvId, jsAlloc);
      }

      jsDoc.AddMember("ids", jsvIds, jsAlloc);

      response.SetBody(jsDoc);
      return true;
    }


  } catch (Ideas::Exception& ex) {
    DEBUG_cerr << "Ideas Exception Caught!" << endl;
    switch (ex.type()) {
      case Ideas::ExceptionType::GENERAL:
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        break;
      default:
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        break;
    } 
    return true;

  } catch (Contents::Exception& ex) {
    DEBUG_cerr << "Contents Exception Caught!" << endl;
    switch (ex.type()) {
      case Contents::ExceptionType::GENERAL:
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        break;
      default:
        response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
        break;
    } 
    return true;

  } catch (...) {
    DEBUG_cerr << "Unhandled Exception." << endl;
    response.SetBody(this->invalid, strlen(this->invalid), http::ContentType::JSON);
    return true;
  } 

  return false;
}

//PlannerPage::
//PlannerPage::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockPlannerPage : public PlannerPage {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(PlannerPage, TESTNAME) {
  MockPlannerPage mockPlannerPage;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

