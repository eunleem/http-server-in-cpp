#include "HttpResponse.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpResponse::Exception::exceptionMessages_[] = {
  HTTPRESPONSE_EXCEPTION_MESSAGES
};
#undef HTTPRESPONSE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpResponse::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpResponse::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpResponse::ExceptionType
HttpResponse::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 

int HttpResponse::KEEP_ALIVE_TIMEOUT = 3;

HttpResponse::HttpResponse(const ResponseCode code, HttpVersion version) :
  status(Status::NEW),
  lastActivity(steady_clock::now()),
  isKeepAlive(true),
  isGzipped(false),
  responseCode(ResponseCode::UNDEF),
  contentType(http::ContentType::UNDEF),
  headerSize(0),
  body(nullptr),
  bodySize(0),
  isTempBody(false)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

  this->SetResponseCode(code, version);
}

HttpResponse::~HttpResponse() {
  //DEBUG_FUNC_START;

  if (this->body != nullptr) {
    if (this->isTempBody == true) {
      delete[] this->body;
    } 
  } 

}

bool HttpResponse::Reset(const ResponseCode code, HttpVersion version) {
  if (this->status == Status::NEW) {
    return true;
  } 

  if (this->status == Status::IN_PROCESS ||
      this->status == Status::PROCESSED ||
      this->status == Status::SENDING_HEADER || 
      this->status == Status::HEADER_SENT || 
      this->status == Status::SENDING_BODY || 
      this->status == Status::CACHED)
  {
    DEBUG_cerr << "Resetting Response that is still in use." <<
      " Status: " << (int)this->status << endl; 
  } 

  this->status = Status::NEW;
  this->lastActivity = steady_clock::now();
  this->isKeepAlive = true;
  this->isGzipped = false;
  this->responseCode = http::ResponseCode::UNDEF; // Prevents SetResponseCode from spitting out error.
  this->SetResponseCode(code, version);
  this->contentType = http::ContentType::UNDEF;
  cookies.clear();
  memset(this->header, 0, sizeof(this->header));
  this->headerSize = 0;
  if (this->body != nullptr) {
    if (this->isTempBody == true) {
      delete[] this->body;
    } 
    this->body = nullptr;
  } 
  this->bodySize = 0;
  this->isTempBody = false;

  DEBUG_cout << "RESET RESPONSE." << endl; 
  return true;
}

bool HttpResponse::CheckTimeout(uint32_t seconds) {
  if (this->lastActivity + std::chrono::seconds(seconds) < steady_clock::now()) {
    // Timeout
    this->status = Status::TIMEOUT;
    return true;
  } 
  
  return false;
}

HttpResponse::Status HttpResponse::GetStatus() const {
  return this->status;
}

bool HttpResponse::IsKeepAlive() const {
  return this->isKeepAlive;
}

void HttpResponse::SetStatus(Status status) {
  if (status != Status::IN_PROCESS) {
    DEBUG_cerr << "This function is originally designed to set status to IN_PROCESS only." << endl; 
  } 
  this->lastActivity = std::chrono::steady_clock::now();
  this->status = status;
}

bool HttpResponse::SetResponseCode(const ResponseCode responseCode,
    const HttpVersion version)
{
  if (this->responseCode != ResponseCode::UNDEF) {
    // if not empty, then WARN
    DEBUG_cerr << "ResponseHeader is not empty. Continuing... code:" << (int)this->responseCode << endl;
  }
  this->httpVersion = version;
  this->responseCode = responseCode;
  this->lastActivity = std::chrono::steady_clock::now();
  return true;
}

bool HttpResponse::AddHeaderField(const string& name, const string& value) {
  string field = name + ": " + value + "\r\n";
  if (sizeof(this->header) < this->headerSize + field.length()) {
    DEBUG_cerr << "HEADER TOO BIG." << endl; 
    return false;
  } 

  ssize_t found = Util::String::Find(name + ": ", this->header, this->headerSize);
  if (found != -1) {
    DEBUG_cerr << "WARNING: Header with name " << header << " already exists." << endl; 
  } 

  DEBUG_cout << "AddHeaderField. headersize: " << this->headerSize << endl; 

  strncpy(this->header + this->headerSize, field.c_str(), field.length());
  this->headerSize += field.length();

  return true;
}

bool HttpResponse::SetCookie(const string& name, const string& value,
    datetime exp, bool isSecure, bool isHttpOnly)
{
  // #TODO expiration, isSecure, isHttpOnly
  this->cookies[name] = value;

  return true;
}

bool HttpResponse::SetBody(string& body, http::ContentType type) {
  if (this->body != nullptr) {
    DEBUG_cerr << "Body is already Set. Cannot overwrite." << endl; 
    return false;
  } 

  if (this->status != Status::IN_PROCESS) {
    DEBUG_cerr << "WARNING: SetBody is meant to be called when the status is IN_PROCESS!" << endl; 
    //return false;
  } 

  this->body = new char[body.length()];
  body.copy(this->body, body.length());
  //strncpy(this->body, body.c_str(), body.length());
  this->bodySize = body.length();
  this->isTempBody = true;
  this->contentType = type;

  this->status = Status::PROCESSED;
  return true;
}


bool HttpResponse::SetBody(const char* address, size_t size, http::ContentType contentType, bool isTempBody) {
  if (this->body != nullptr) {
    DEBUG_cerr << "Body is already Set. Cannot overwrite." << endl; 
    return false;
  } 

  if (this->status != Status::IN_PROCESS) {
    DEBUG_cerr << "WARNING: SetBody is meant to be called when the status is IN_PROCESS!" << endl; 
    //return false;
  } 

  this->isTempBody = isTempBody;
  this->body = const_cast<char*>(address);
  this->bodySize = size;
  this->contentType = contentType;

  this->status = Status::PROCESSED;
  return true;
}

bool HttpResponse::SetBody(const rapidjson::Document& jsdoc) {

  if (this->status != Status::IN_PROCESS) {
    DEBUG_cerr << "WARNING: SetBody is meant to be called when the status is IN_PROCESS!" << endl; 
    //return false;
  } 

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  jsdoc.Accept(writer);

  size_t length = strlen(buffer.GetString());

  this->body = new char[length];
  strncpy(this->body, buffer.GetString(), length);
  this->bodySize = length;
  this->isTempBody = true;
  this->contentType = http::ContentType::JSON;

  this->status = Status::PROCESSED;
  return true;
}

size_t HttpResponse::buildHeader() {
  if (this->responseCode == ResponseCode::UNDEF) {
    DEBUG_cerr << "Cannot build header. ResponseCode not set." << endl; 
    return 0;
  } 

  if (this->headerSize != 0) {
    DEBUG_cerr << "WARN: Header size is not 0. Something is suspicious." << endl; 
    this->headerSize = 0;
  } 

  memset(this->header, 0, sizeof(this->header));

  switch (this->httpVersion) {
    case HttpVersion::V1_0:
      strncpy(this->header, "HTTP/1.0 ", strlen("HTTP/1.0 "));
      this->headerSize = strlen("HTTP/1.0 ");
      break;
    case HttpVersion::V1_1:
      strncpy(this->header, "HTTP/1.1 ", strlen("HTTP/1.1 "));
      this->headerSize = strlen("HTTP/1.1 ");
      break;
    case HttpVersion::V2_0:
      DEBUG_cerr << "Http2.0 is not supported yet." << endl; 
      return 0;
      break;
    default:
      break;
  } 

  strncpy(this->header + this->headerSize,
      http::ResponseCodeString[(int)responseCode].c_str(),
      http::ResponseCodeString[(int)responseCode].length());
  this->headerSize += http::ResponseCodeString[(int)responseCode].length();

  strncpy(this->header + this->headerSize, "\r\n", strlen("\r\n"));
  this->headerSize += strlen("\r\n");

  for (auto& cookie : this->cookies) {
    this->AddHeaderField("Set-Cookie", cookie.first + "=" + cookie.second);
  } 

  if (this->isKeepAlive == true) {
    this->AddHeaderField("Connection", "keep-alive");
    this->AddHeaderField("Keep-Alive", "timeout=2, max=10");
  } else {
    this->AddHeaderField("Connection", "close");
  }

  if (this->isGzipped == true) {
    this->AddHeaderField("Content-Encoding", "gzip");
  } 

  if (this->body != nullptr && this->bodySize > 0) {

    string charset = "";
    switch (this->contentType) {
      case http::ContentType::HTML:
      case http::ContentType::CSS:
      case http::ContentType::PLAINTEXT:
      case http::ContentType::XML:
      case http::ContentType::JAVASCRIPT:
      case http::ContentType::JSON:
        charset = "; charset=UTF-8";
      default:
        break;
    } 
    this->AddHeaderField("Content-Type",
        http::ContentTypeString[(int) this->contentType] + charset);
    this->AddHeaderField("Content-Length", std::to_string(this->bodySize));
  }

  strncpy(this->header + this->headerSize, "\r\n", strlen("\r\n"));
  this->headerSize += strlen("\r\n");

  return this->headerSize;
}

HttpResponse::Status HttpResponse::Send(const int& fd, size_t& sentSize,
    bool isSecureConnection, SSL* ssl) {
  DEBUG_cout << "Response Send Called!. fd: " << fd << " sentSize: " << sentSize << endl; 

  this->lastActivity = std::chrono::steady_clock::now();

  char* sendPtr = nullptr;
  size_t sendSize = 0;

  if (this->status == Status::PROCESSED) {
    DEBUG_cout << "Send. Status is PROCESSED." << endl; 
    this->buildHeader();
    sendPtr = this->header;
    sendSize = this->headerSize;
    this->status = Status::SENDING_HEADER;
  } 

  if (this->status == Status::SENDING_HEADER) {
    DEBUG_cout << "Sending Header." << endl; 
    while (true) {
      sendPtr = (char*)((uintptr_t)this->header + sentSize);
      sendSize = this->headerSize - sentSize;

      if (sendSize == 0) {
        this->status = Status::HEADER_SENT;
        break;
      } 
      ssize_t writeCount = -1;
      if (isSecureConnection == true) {
        writeCount = SSL_write(ssl, sendPtr, sendSize);
      } else {
        writeCount = write(fd, sendPtr, sendSize);
      }
      DEBUG_cout << "Sending Header... Count: " << writeCount << endl; 
      if (writeCount == -1) {
        if (errno == EAGAIN) {
          // GOOD
          break;
        } else {
          DEBUG_cerr << "Write Error. errno: " << errno << endl; 
          this->status = Status::ERROR;
          break;

        }

      } else if (writeCount == 0) {
        DEBUG_cerr << "Write returned 0 which is unusual." << endl; 

      } else {
        sentSize += writeCount;

      }
    } 
    //this->timestamp = system_clock::now();
  } 
  
  if (this->status == Status::HEADER_SENT) {
    DEBUG_cout << "Header Sent!" << endl; 
    if (this->bodySize == 0) {
      // Header Only Response
      this->status = Status::SENT;
    } else {
      sendPtr = this->body;
      sendSize = this->bodySize;

      this->status = Status::SENDING_BODY;
    }
  } 

  if (this->status == Status::SENDING_BODY) {
    DEBUG_cout << "Sending Body..." << endl; 
    while (true) {
      sendPtr = this->body + (sentSize - this->headerSize);
      sendSize = this->bodySize - (sentSize - this->headerSize);

      if (sendSize == 0) {
        this->status = Status::SENT;
        break;
      } 
      ssize_t writeCount = -1;
      if (isSecureConnection == true) {
        writeCount = SSL_write(ssl, sendPtr, sendSize);
      } else {
        writeCount = write(fd, sendPtr, sendSize);
      }
      DEBUG_cout << "Sending Body... Count: " << writeCount << endl; 
      if (writeCount == -1) {
        if (errno == EAGAIN) {
          // GOOD
          break;
        } else {
          DEBUG_cerr << "Write Error." << endl; 
          this->status = Status::ERROR;
          break;

        }

      } else if (writeCount == 0) {
        DEBUG_cerr << "Write returned 0 which is unusual." << endl; 

      } else {
        sentSize += writeCount;

      }
    } 
    //this->timestamp = system_clock::now();
  } 

  if (this->status == Status::SENT) {
    DEBUG_cout << "Response SENT! sentSize: " << sentSize << endl; 
  }

  return this->status;
}

bool HttpResponse::Cache(int seconds) {
  if (this->status == Status::PROCESSED ||
      this->status == Status::SENT)
  {
    this->status = Status::CACHED;
    return true;
  } 

  DEBUG_cout << "Could not Cache Response that is not PROCESSED." << endl; 
  return false;
}
//HttpResponse::
//HttpResponse::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpResponse : public HttpResponse {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpResponse, TESTNAME) {
  MockHttpResponse mockHttpResponse;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

