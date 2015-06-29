#include "HttpRequest.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpRequest::Exception::exceptionMessages_[] = {
  HTTPREQUEST_EXCEPTION_MESSAGES
};
#undef HTTPREQUEST_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpRequest::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpRequest::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpRequest::ExceptionType
HttpRequest::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


const size_t HttpRequest::MAX_URI_SIZE = 256;
const size_t HttpRequest::MAX_HEADER_SIZE = 1024 * 8;
const size_t HttpRequest::MAX_FIELD_SIZE = 1024 * 2;

HttpRequest::HttpRequest(uint32_t id) :
  status(Status::NEW),
  timestamp(system_clock::now()),
  id(id),
  freeSize(sizeof(this->buffer)),
  usedSize(0),
  body(nullptr),
  secondBuffer(nullptr),
  secondBufferSize(0),
  secondBufferUsedSize(0),
  httpVersion(HttpVersion::V1_0),
  httpMethod(RequestMethod::GET),
  isGzipSupported(false),
  isKeepAliveSupported(false),
  uri(nullptr), uriLength(0),
  host(nullptr), hostLength(0),
  contentType(lio::http::ContentType::UNDEF),
  contentLength(0),
  contentTypeValue(nullptr), contentTypeValueLength(0)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpRequest::~HttpRequest() {
  //DEBUG_FUNC_START;
  if (this->secondBuffer != nullptr) {
    delete[] this->secondBuffer;
  } 

}

void HttpRequest::Reset(uint32_t id) {
  this->status = Status::NEW;
  this->timestamp = system_clock::now();
  this->id = id;
  this->freeSize = sizeof(this->buffer);
  this->usedSize = 0;
  if (this->secondBuffer != nullptr) {
    DEBUG_cout << "Deleting Second Buffer." << endl; 
    delete[] this->secondBuffer;
    this->secondBuffer = nullptr;
  } 
  this->secondBufferSize = 0;
  this->secondBufferUsedSize = 0;
  this->firstLineEndPos = 0;
  this->httpVersion = http::HttpVersion::V1_1;
  this->httpMethod = http::RequestMethod::UNDEF;
  this->isGzipSupported = false;
  this->isKeepAliveSupported = true;
  this->uri = nullptr;
  this->uriLength = 0;
  this->host = nullptr;
  this->hostLength = 0;
  this->contentLength = 0;
  this->contentTypeValue = nullptr;
  this->contentTypeValueLength = 0;
  this->contentType = http::ContentType::UNDEF;

  this->userAgent = nullptr;
  this->userAgentLength = 0;
  this->cookies = nullptr;
  this->cookiesLength = 0;
  this->referer = nullptr;
  this->refererLength = 0;
  this->acceptLanguage = nullptr;
  this->acceptLanguageLength = 0;

  memset(this->buffer, 0, sizeof(this->buffer));

  DEBUG_cout << "RESET REQUEST." << endl; 
}

uint32_t HttpRequest::GetId() const {
  return this->id;
}

HttpRequest::Status HttpRequest::GetStatus() const {
  return this->status;
}

http::RequestMethod HttpRequest::GetRequestMethod() const {
  if (this->httpMethod == http::RequestMethod::UNDEF) {
    DEBUG_cerr << "Http Request Method is Undefined" << endl; 
  } 

  return this->httpMethod;
}

string HttpRequest::GetUri() const {
  if (this->uri == nullptr || this->uriLength == 0) {
    DEBUG_cerr << "URI not found." << endl; 
    return "";
  } 

  return string(this->uri, this->uriLength);
}

string HttpRequest::GetHost() const {
  if (this->host == nullptr || this->hostLength == 0) {
    DEBUG_cerr << "Host Not Set." << endl; 
    return "";
  } 

  return string(this->host, this->hostLength);
}

bool HttpRequest::IsKeepAlive() const {
  return this->isKeepAliveSupported;
}

bool HttpRequest::IsGzipSupported() const {
  return this->isGzipSupported;
}

http::ContentType HttpRequest::GetContentType() const {
  if (this->contentType == http::ContentType::UNDEF) {
    DEBUG_cerr << "Content Type is not found. Request without body." << endl; 
  } 
  return this->contentType;
}

DataBlock<char*> HttpRequest::GetBody() const {
  if (this->body != nullptr) {
    return DataBlock<char*>(this->body, 0, this->contentLength);
  }

  DEBUG_cerr << "No Body Found." << endl; 
  return DataBlock<char*>();
}

DataBlock<char*> HttpRequest::GetCookie(const string& cookieName) {
  if (this->cookies == nullptr) {
    DEBUG_cout << "Cookie field not found." << endl; 
    return cookies;
  } 

  ssize_t cookiePos = Util::String::Find(cookieName + "=", this->cookies, this->cookiesLength);
  if (cookiePos == -1) {
    DEBUG_cout << "cookies: " << std::hex << this->cookies << endl; 
    DEBUG_cout << "cookiesLength: " << this->cookiesLength << endl; 
    DEBUG_cerr << "Cookie " << cookieName << " not found." << endl; 
    return DataBlock<char*>();
  } 

  cookiePos += cookieName.length() + strlen("=");
  size_t cookieMaxLength = this->cookiesLength - cookiePos;
  DEBUG_cout << "cookieMaxLength: " << cookieMaxLength << endl; 
  DEBUG_cout << "cookiePos: " << cookiePos << endl; 

  size_t cookieLength = 0;

  ssize_t cookieEnd = Util::String::Find(";", this->cookies + cookiePos, cookieMaxLength);
  if (cookieEnd == -1) {
    cookieEnd = Util::String::Find("\r\n", this->cookies + cookiePos, cookieMaxLength + 2);
    if (cookieEnd == -1) {
      DEBUG_cerr << "Could not find end of cookie value." << endl; 
      return DataBlock<char*>();
    } 
  } 
  cookieLength = cookieEnd;

  DEBUG_cout << "Cookie: " << string(this->cookies + cookiePos, cookieLength) << endl; 

  return DataBlock<char*>(this->cookies + cookiePos, 0, cookieLength);
}

bool HttpRequest::CheckTimedout() {
  auto now = std::chrono::system_clock::now();
  if (this->status == Status::NEW ||
      this->status == Status::CONNECTION_CLOSED ||
      this->status == Status::ERROR)
  {
    return false;

  } else if (this->status <= Status::READING_BODY) { 
    if (this->timestamp + std::chrono::seconds(5) < now) {
      this->SetStatus(Status::READ_TIMEOUT);
      return true;
    } 

  } else if (this->status == Status::IN_PROCESS ||
             this->status == Status::DONE_READING)
  {
    if (this->timestamp + std::chrono::seconds(15) < now) {
      this->SetStatus(Status::PROCESS_TIMEOUT);
      return true;
    } 

  }

  return false;
}

HttpRequest::Status HttpRequest::SetStatus(HttpRequest::Status status) {
  this->status = status;
  this->timestamp = system_clock::now();

  if (status == Status::CONNECTION_CLOSED || 
      status == Status::READ_TIMEOUT)
  {
    //close(this->fd);
    this->freeSecondBuffer();

  } else if (status == Status::ERROR) {
    //close(this->fd);
    this->freeSecondBuffer();

  } 

  return this->status;
}

HttpRequest::Status HttpRequest::ReadRequest(int fd, bool isSecureConnection, SSL* ssl) {
  if (this->status == Status::PROCESSED) {
    this->Reset();
  } 

  if (this->status == Status::NEW) {
    this->SetStatus(Status::READING_HEADER);
  } 

  if (this->status == Status::READING_HEADER) {
    uintptr_t readPos = (uintptr_t) this->buffer + this->usedSize;
    size_t toReadSize = this->freeSize;

    while (true) {
      DEBUG if (toReadSize == 0) {
        // It means it read all the necessary stuff.
        // Try to flush the socket..
        char empty[100];
        while (true) {
          ssize_t emptiedSize = -1;
          if (isSecureConnection == true) {
            emptiedSize = SSL_read(ssl, (void*) empty, sizeof(empty));
          } else {
            emptiedSize = read(fd, (void*) empty, sizeof(empty));
          }
          if (emptiedSize == -1) {
            if (errno == EAGAIN) {
              // Best Case. It is supposed to return EAGAIN
              break;
            } 
          } else {
            DEBUG_cerr << "Emptied count: " << emptiedSize << endl; 
          }
        } 
      } 

      ssize_t readSize = -1;
      if (isSecureConnection == true) {
        DEBUG_cout << "SSL READ." << endl; 
        readSize = SSL_read(ssl, (void*) readPos, toReadSize);
      } else {
        readSize = read(fd, (void*) readPos, toReadSize);
      }

      DEBUG_cout << "ReadCount: " << readSize << endl; 
      if (readSize == lio::consts::ERROR) {
        if (errno == EAGAIN) {
          DEBUG_cout << "EAGAIN. This is usually a normal case." << endl; 
          break;
        } 
        DEBUG_cerr << "Read Error. errno: " << errno << " fd: " << fd << endl; 
        return this->SetStatus(Status::ERROR);

      } else if (readSize == 0) {
        DEBUG_cout << "Client has closed the connection." << endl; 
        return this->SetStatus(Status::CONNECTION_CLOSED);

      }

      readPos += readSize;
      toReadSize -= readSize;
      this->usedSize += readSize;  
      this->freeSize -= readSize;

      if (this->freeSize == 0) {
        // Request filled first level buffer.
        DEBUG_cerr << "First level buffer has been filled." << endl; 
        break;
      } 
    } 

    if (this->usedSize < 6) {
      this->SetStatus(Status::READING_HEADER);
    } else {
      ssize_t headerSize = this->findHeaderSize();
      if (headerSize == consts::NOT_FOUND) {
        // Header Size too big
        return this->SetStatus(Status::ERROR);
      }
      DEBUG_cout << "Request: " << endl << this->buffer << endl; 

      this->SetStatus(Status::READ_HEADER);

    }
  } 

  if (this->status == Status::READ_HEADER) {
    DEBUG_cout << "HEADER IS READ finding Body." << endl; 

    ssize_t contentLength = this->findContentLength();
    //DEBUG_cout << "Request ContentLength: " << contentLength << endl; 
    if (contentLength > 0) {
      DEBUG_cout << "BODY FOUND." << endl; 
      //DEBUG_cout << "usedSize: " << this->usedSize << endl; 
      if (this->usedSize >= this->headerSize + contentLength) {
        this->body = this->buffer + this->headerSize;
        this->SetStatus(Status::DONE_READING);

      } else {
        this->SetStatus(Status::READING_BODY);
        if (this->contentLength + this->headerSize > sizeof(this->buffer)) {
          // If first buffer alone is not enough.
          this->createSecondBuffer();
          this->body = this->secondBuffer;
        } else {
          this->body = this->buffer + this->headerSize;
        }
      }

      bool isExpectContinue = this->isClientExpectContinue();
      if (isExpectContinue == true) {
        // TODO if ExpectContinue, send Continue and read.
        DEBUG_cerr << "EXPECT CONTINUE IS NOT HANDLED YET." << endl; 
        this->SetStatus(Status::ERROR);
      } 


    } else {
      DEBUG_cout << "Content Length not found" << endl; 
      this->SetStatus(Status::DONE_READING);
    }
  } 

  if (this->status == Status::READING_BODY) {
    uintptr_t readPos = (uintptr_t) this->buffer + this->usedSize;
    size_t toReadLength = this->contentLength;
    if (this->secondBuffer != nullptr) {
      // if secondBuffer is not nullptr then it means it's using Second Buffer.
      readPos = (uintptr_t) this->secondBuffer + this->secondBufferUsedSize;
      toReadLength = this->secondBufferSize - this->secondBufferUsedSize;
    } 


    while (true) {
      DEBUG if (toReadLength == 0) {
        // It means it read all the necessary stuff.
        // Try to flush the socket..
        char empty[100];
        while (true) {
          ssize_t emptiedSize = -1;
          if (isSecureConnection == true) {
            emptiedSize = SSL_read(ssl, (void*) empty, sizeof(empty));
          } else {
            emptiedSize = read(fd, (void*) empty, sizeof(empty));
          }
          if (emptiedSize == -1) {
            if (errno == EAGAIN) {
              // Best Case. It is supposed to return EAGAIN
              break;
            } 
          } else {
            DEBUG_cerr << "Emptied count: " << emptiedSize << endl; 
          }
        } 
      } 

      ssize_t readSize = -1;
      if (isSecureConnection == true) {
        readSize = SSL_read(ssl, (void*) readPos, toReadLength);
      } else {
        readSize = read(fd, (void*) readPos, toReadLength);
      }
      DEBUG_cout << "ReadCount: " << readSize << endl; 
      if (readSize == lio::consts::ERROR) {
        if (errno == EAGAIN) {
          DEBUG_cout << "EAGAIN." << endl; 
          break;
        } 
        DEBUG_cerr << "Read Error." << endl; 
        // TODO call a function that totally ignores this.
        return this->SetStatus(Status::ERROR);
        

      } else if (readSize == 0) {
        DEBUG_cout << "Client has closed the connection." << endl; 
        return this->SetStatus(Status::CONNECTION_CLOSED);

      }

      readPos += readSize;
      toReadLength -= readSize;
      
      if (this->secondBuffer != nullptr) {
        this->secondBufferUsedSize += readSize;

      } else {
        this->usedSize += readSize;  
        this->freeSize -= readSize;

      }

      if (this->secondBufferUsedSize >= this->contentLength ||
          this->usedSize >= this->headerSize + this->contentLength)
      {
        this->SetStatus(Status::DONE_READING);
        break;
      } 
    } 
    this->timestamp = system_clock::now();
  } 
  
  if (this->status == Status::DONE_READING) {
    DEBUG_cout << "Received all the data." << endl; 
    DEBUG_cout << "Request: " << endl << this->buffer << endl; 
    this->Preprocess();
    //DEBUG_cout << "Preprocessed Request: " << endl << this->buffer << endl; 
  } 

  return this->status;
}

bool HttpRequest::Preprocess() {
  DEBUG if (this->status != Status::DONE_READING) {
    DEBUG_cerr << "Cannot preprocess when status is not DONE READING." << endl; 
    return false;
  }

  this->firstLineEndPos = Util::String::Find("\r\n", this->buffer, this->MAX_URI_SIZE + 25);
  //DEBUG_cout << "FirstLineEndPos: " << this->firstLineEndPos << endl; 
  if (this->firstLineEndPos == lio::consts::NOT_FOUND) {
    DEBUG_cerr << "Invalid Http Request. Uri may be too long." << endl; 
    this->SetStatus(Status::ERROR);
    return false;
  }

  this->findHttpVersion();

  ssize_t methodEndPos = this->findRequestMethod();
  if (methodEndPos == lio::consts::NOT_FOUND) {
    this->SetStatus(Status::ERROR);
    return false;
  } 

  ssize_t uriEndPos = this->findUri(methodEndPos);
  if (uriEndPos == lio::consts::NOT_FOUND) {
    this->SetStatus(Status::ERROR);
    return false;
  } 


  char* searchBeg = this->buffer + this->firstLineEndPos + strlen("\r\n");
  //DEBUG_cout << "buffer: " << std::hex << static_cast<void*>(this->buffer) << endl; 
  //DEBUG_cout << "searchBeg: " << std::hex << static_cast<void*>(searchBeg) << std::dec << endl; 
  //DEBUG_cout << "MAX_FIELD_SIZE:" << this->MAX_FIELD_SIZE << endl; 

  // NOW START PARSING HEADER FIELDS
  while (true) {
    //DEBUG_cout << "searchBeg: " << searchBeg << endl; 
    ssize_t lineEndPos = Util::String::Find("\r\n", searchBeg,
        this->MAX_FIELD_SIZE); 
    if (lineEndPos == consts::NOT_FOUND) {
      DEBUG_cerr << "LineEnd Not Found or too long." << endl; 
      return consts::ERROR;

    } else if (lineEndPos < 1) {
      DEBUG_cout << "lineEndPos: " << lineEndPos << endl; 
      // End of header.
      return true;
    }

    ssize_t fieldNameEndPos = Util::String::Find(": ", searchBeg, 32);
    if (fieldNameEndPos == consts::NOT_FOUND) {
      // NOT FOUND
      DEBUG_cout << "Could not retrive header." << endl; 
      searchBeg += lineEndPos + strlen("\r\n");
      continue;
    } 

    //DEBUG_cout << "fieldName: " << string(searchBeg, fieldNameEndPos) << endl; 

    char* fieldValueStart = searchBeg + fieldNameEndPos + strlen(": ");
    size_t fieldValueLength = lineEndPos - fieldNameEndPos - strlen(": ");
    if (this->host == nullptr && strncmp(searchBeg, "Host", strlen("Host")) == 0) {
      this->host = fieldValueStart;
      this->hostLength = fieldValueLength;


    } else if (strncmp(searchBeg, "Accept-Encoding", strlen("Accept-Encoding")) == 0) {
      Util::String::ToLowerFly(fieldValueStart, fieldValueLength);
      ssize_t foundPos = Util::String::Find("gzip", fieldValueStart, fieldValueLength);
      this->isGzipSupported = foundPos != consts::NOT_FOUND;

    } else if (strncmp(searchBeg, "Connection", strlen("Connection")) == 0) {
      Util::String::ToLowerFly(fieldValueStart, fieldValueLength);
      ssize_t foundPos = Util::String::Find("keep-alive", fieldValueStart, fieldValueLength);
      this->isKeepAliveSupported = foundPos != consts::NOT_FOUND;

    } else if (this->userAgent == nullptr &&
        strncmp(searchBeg, "User-Agent", strlen("User-Agent")) == 0) {
      this->userAgent = fieldValueStart;
      this->userAgentLength = fieldValueLength;

    } else if (this->acceptLanguage == nullptr &&
        strncmp(searchBeg, "Accept-Language", strlen("Accept-Language")) == 0) {
      this->acceptLanguage = fieldValueStart;
      this->acceptLanguageLength = fieldValueLength;

    } else if (this->referer == nullptr &&
        strncmp(searchBeg, "Referer", strlen("Referer")) == 0) {
      this->referer = fieldValueStart;
      this->refererLength = fieldValueLength;

    } else if (this->cookies == nullptr &&
        strncmp(searchBeg, "Cookie", strlen("Cookie")) == 0) {
      this->cookies = fieldValueStart;
      this->cookiesLength = fieldValueLength;

    } else if (strncmp(searchBeg, "Content-Type", strlen("Content-Type")) == 0) {
      this->contentTypeValue = fieldValueStart;
      this->contentTypeValueLength = fieldValueLength;

      string value(fieldValueStart, fieldValueLength);
      Util::String::ToLowerFly(value);
      if (value.find(http::ContentTypeString[(int)http::ContentType::FORMDATA]) != string::npos) {
        this->contentType = http::ContentType::FORMDATA;
      } else if (value.find(
            http::ContentTypeString[(int)http::ContentType::FORMDATA_MULTIPART]) != string::npos) {
        this->contentType = http::ContentType::FORMDATA_MULTIPART;
      } 
    } 

    DEBUG {
      //string fieldContent(fieldValueStart, fieldValueLength);
      //DEBUG_cout << "fieldContent: " << fieldContent << endl; 
    }

    searchBeg += lineEndPos + strlen("\r\n");
  } 

  this->SetStatus(Status::PREPROCESSED);
  DEBUG_cout << "Request has been preprocessed." << endl; 
  return true;
}

char* HttpRequest::GetHeaderField(const string& fieldName) {
  ssize_t result = Util::String::Find(fieldName + ": ", this->buffer, this->headerSize);
  if (result == -1) {
    DEBUG_cout << "Header you are looking for is not found." << endl; 
    return nullptr;
  }

  return this->buffer + result + fieldName.length() + strlen(":");
}

lio::http::ContentType HttpRequest::findContentType(char* fieldValueStart,
    size_t fieldValueLength)
{
  string fieldValue(fieldValueStart, fieldValueLength);

  http::ContentType type = http::ContentType::UNDEF;

  if (strncmp(fieldValueStart,
        http::ContentTypeString[(int) http::ContentType::FORMDATA].c_str(),
        http::ContentTypeString[(int) http::ContentType::FORMDATA].length() ) == 0)
  {
    type = http::ContentType::FORMDATA;
  } else if (strncmp(fieldValueStart,
        http::ContentTypeString[(int) http::ContentType::FORMDATA_MULTIPART].c_str(),
        http::ContentTypeString[(int) http::ContentType::FORMDATA_MULTIPART].length() ) == 0)
  {
    type = http::ContentType::FORMDATA_MULTIPART;
  } 

  if (type == http::ContentType::UNDEF) {
    DEBUG_cerr << "Content-Type is undefined." << endl; 
    return type;
  }

  this->contentType = type;

  if (this->contentType == http::ContentType::FORMDATA_MULTIPART) {
    DEBUG_cerr << "Form multipart is not supported yet." << endl; 
    return this->contentType;

    size_t pos = fieldValue.find("boundary=");
    if (pos != string::npos) {

    } else {
      DEBUG_cerr << "WARN: Boundary value might be required. but it's not provided." << endl; 
    }
  } 

  return this->contentType;
}


HttpVersion HttpRequest::findHttpVersion() {
  ssize_t found = Util::String::Find(" HTTP/1.1\r\n", this->buffer, this->firstLineEndPos);
  if (found == lio::consts::NOT_FOUND) {
    this->httpVersion = HttpVersion::V1_0;
    this->isKeepAliveSupported = false; // 1.0's default connection is NOT keep-alive.

  } else {
    this->httpVersion = HttpVersion::V1_1;

  }

  return httpVersion;
}

ssize_t HttpRequest::findRequestMethod() {
  ssize_t methodEndPos = Util::String::Find(" ", this->buffer, 20);
  if (methodEndPos == lio::consts::NOT_FOUND) {
    DEBUG_cerr << "Request Method not found." << endl; 
    return -1;
  } 

  string methodStr = string(this->buffer, methodEndPos);
  Util::String::Trim(methodStr);
  Util::String::ToUpperFly(methodStr);
  if (methodStr == "GET") {
    this->httpMethod = RequestMethod::GET;

  } else if (methodStr == "POST") {
    this->httpMethod = RequestMethod::POST;

  } else if (methodStr == "HEAD") {
    this->httpMethod = RequestMethod::HEAD;

  } else if (methodStr == "CONNECT") {
    this->httpMethod = RequestMethod::CONNECT;

  } else {
    DEBUG_cerr << "Receive request method that is not supported." << endl; 
    return -1;
  }

  return methodEndPos;
}

ssize_t HttpRequest::findUri(size_t methodEndPos) {
  ssize_t uriEndPos = Util::String::Find(" ",
      (char*)((uintptr_t)this->buffer + methodEndPos + 1),
      this->MAX_URI_SIZE); // MAX URI LENGTH.
  if (uriEndPos == lio::consts::NOT_FOUND) {
    DEBUG_cerr << "URI not found or may be too long." << endl; 
    return -1;
  } 
  //DEBUG_cout << "uriEndPos: " << uriEndPos << endl; 

  this->uri = this->buffer + methodEndPos + 1;
  this->uriLength = uriEndPos;
  DEBUG_cout << "uriLength: " << this->uriLength << endl; 

  ssize_t length = Util::String::UriDecodeFly(this->uri, this->uriLength);
  this->uriLength = length;
  DEBUG_cout << "decodedUriLength: " << this->uriLength << endl; 


  return uriEndPos;
}

ssize_t HttpRequest::findHeaderSize() {
  ssize_t headerEndPos = Util::String::Find("\r\n\r\n",
                                            this->buffer,
                                            sizeof(this->buffer));
  if (headerEndPos == consts::NOT_FOUND) {
    // Header Size too big TODO
    DEBUG_cerr << "Header Too long or not found." << endl; 
    return -1;
  }

  this->headerSize = headerEndPos + strlen("\r\n\r\n");
  DEBUG_cout << "HeaderSize: " << this->headerSize << endl; 

  return this->headerSize;
}


ssize_t HttpRequest::findContentLength() {
  ssize_t contentLengthPos = Util::String::Find("\r\nContent-Length: ",
      this->buffer,
      sizeof(this->buffer));

  if (contentLengthPos > 0) {
    string contentLengthStr = Util::String::RetrieveBetween(
        (char*)((uintptr_t)this->buffer + contentLengthPos + strlen("\r\nContent-Length: ")),
        "\r\n", 1024);
    this->contentLength = Util::String::ToUInt(contentLengthStr);
    if (this->contentLength > 1024 * 1024 * 3) {
      DEBUG_cerr << "Content length is too big. More than 3 MB." << endl; 
      this->SetStatus(Status::ERROR);
      return -1;
    } 
  }

  return this->contentLength;
}

size_t HttpRequest::createSecondBuffer() {
  if (this->contentLength == 0) {
    DEBUG_cout << "No need to create second buffer for request w/o content length." << endl; 
    return 0;
  } 

  this->secondBuffer = new char[this->contentLength + 1];
  this->secondBufferSize = this->contentLength;
  
  // Move partial
  memcpy(this->secondBuffer, (const void*)((uintptr_t)this->buffer + this->headerSize),
      this->usedSize - this->headerSize);
  this->secondBufferUsedSize = this->usedSize - this->headerSize;

  memset(this->buffer + this->headerSize, '\0',
      sizeof(this->buffer) - this->headerSize);

  this->usedSize = this->headerSize;
  this->freeSize = sizeof(this->buffer) - this->headerSize;

  DEBUG_cout << "Created Second Buffer. Size: " << this->secondBufferSize << endl; 

  return this->secondBufferSize;
}

bool HttpRequest::isClientExpectContinue() {
  ssize_t expectContinuePos =
    Util::String::Find("\r\nExpect: 100-continue", this->buffer, sizeof(this->buffer));
  if (expectContinuePos > 0) {
    this->SetStatus(Status::CONTINUE);
    return true;
  }

  return false;
}



bool HttpRequest::freeSecondBuffer() {
  if (this->secondBuffer != nullptr) {
    delete[] this->secondBuffer;
    this->secondBuffer = nullptr;
    this->secondBufferSize = 0;
    this->secondBufferUsedSize = 0;
    return true;
  } 
  return false;
}


}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpRequest : public HttpRequest {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpRequest, TESTNAME) {
  MockHttpRequest mockHttpRequest;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

