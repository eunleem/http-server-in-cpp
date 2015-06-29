#include "HttpConnection.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpConnection::Exception::exceptionMessages_[] = {
  HTTPCONNECTION_EXCEPTION_MESSAGES
};
#undef HTTPCONNECTION_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpConnection::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpConnection::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpConnection::ExceptionType
HttpConnection::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 

int HttpConnection::KEEP_ALIVE_TIMEOUT = 3;
int HttpConnection::DEFAULT_TIMEOUT = 5;

HttpConnection::HttpConnection() :
  fd(-1),
  isSecureConnection(false),
  ssl(nullptr),
  status(Status::CLOSED),
  last_activity(steady_clock::now()),
  request(nullptr),
  response(nullptr),
  sentSize(0)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpConnection::~HttpConnection() {
  DEBUG_FUNC_START;
  if (this->status != Status::CLOSED) {
    this->Close();
  }
}

HttpConnection::Status HttpConnection::Open(int fd, bool isSecureConnection, SSL_CTX* ssl_ctx) {
  if (this->status == Status::CLOSED) {
    this->fd = fd;
    this->isSecureConnection = isSecureConnection;
    this->UpdateLastActivity();
    if (isSecureConnection == true) {
      DEBUG_cout << "OPENING SECURE CONNECTION!" << endl; 
      DEBUG if (ssl_ctx == nullptr) {
        DEBUG_cerr << "Cannot leave SSL Context empty." << endl; 
        this->status = Status::ERROR;
        return this->status;
      } 

      this->ssl = SSL_new(ssl_ctx);
      if (this->ssl == nullptr || !this->ssl) {
        DEBUG_cerr << "Creating new SSL Connection has failed." << endl; 
        return this->status = Status::ERROR;
      } 
      SSL_set_fd(this->ssl, fd);
      SSL_accept(this->ssl);
      return this->status = Status::OPENING;
    } 

    this->status = Status::OPEN;

  } else if (this->status == Status::OPENING) {
      auto result = SSL_accept(this->ssl);
      if (result == -1) {
        //ERR_print_errors_fp(stderr);
        DEBUG_cerr << "SSL Error." <<
          ERR_error_string(SSL_get_error(this->ssl, result), nullptr) << endl;
        if (SSL_get_error(this->ssl, result) == 2) {
          return this->status = Status::OPENING;
        } 
      } 
      DEBUG_cout << "SSL Connection using " << SSL_get_cipher(this->ssl) << endl; 
      this->status = Status::OPEN;

  } else {
    DEBUG_cerr << "Cannot Open Connection when it's not closed." << endl; 
    return this->status;
  }

  return this->status;
}

HttpConnection::Status HttpConnection::Close() {
  DEBUG_cout << "fd: " << this->fd << endl; 
  if (this->fd >= 0) {
    if (this->isSecureConnection == true) {
      auto result = SSL_shutdown(this->ssl);
      if (result == -1) {
        DEBUG_cerr << "Failed to shutdown SSL." << endl; 
      } 
      close(this->fd);
      SSL_free(this->ssl);
      this->ssl = nullptr;
    } else {
      DEBUG_cout << "syscall close() is called. fd: " << this->fd << endl; 
      close(this->fd);
    }
    this->fd = -1;
  } 

  this->sentSize = 0;

  DEBUG_cout << "About to free Request." << endl; 
  this->FreeRequest();
  DEBUG_cout << "Freed Request." << endl; 
  DEBUG_cout << "About to free Response." << endl; 
  this->FreeResponse();
  DEBUG_cout << "Freed Response." << endl; 

  //this->UpdateLastActivity();

  this->status = Status::CLOSED;
  DEBUG_cout << "Connection Closed." << endl; 

  return this->status;
}

bool HttpConnection::FreeRequest() {
  if (this->request != nullptr) {
    request->Reset();
    this->request = nullptr;
    return true;
  }
  return false;
}

bool HttpConnection::FreeResponse() {
  if (this->response != nullptr) {
    if (this->response->GetStatus() != HttpResponse::Status::CACHED) {
      this->response->Reset();
    } else {
      DEBUG_cout << "Response is cached and will not be reset." << endl; 
    }
    this->response = nullptr;
    this->sentSize = 0;
    return true;
  } 
  return false;
}

void HttpConnection::AcquireReusableRequest() {
  HttpRequestPool& pool = HttpRequestPool::GetInstance();

  this->request = &pool.AcquireReusable();
  this->UpdateLastActivity();
}

void HttpConnection::SetRequest(HttpRequest* request) {
  if (this->request != nullptr) {
    DEBUG_cerr << "Request is already set! " << "ReqId: " << this->request->GetId() << endl;
    DEBUG_cout << "Request Not Replaced!" << endl; 
    return;
  } 

  this->UpdateLastActivity();
  this->request = request;
}

void HttpConnection::AcquireReusableResponse() {
  HttpResponsePool& pool = HttpResponsePool::GetInstance();
  this->response = &pool.AcquireReusable();
  this->UpdateLastActivity();
}

void HttpConnection::SetResponse(HttpResponse* response) {
  if (this->response != nullptr) {
    DEBUG_cerr << "Response is already Set!" << endl; 
  } 

  this->UpdateLastActivity();
  this->response = response;

  this->status = Status::PROCESSED;
}

void HttpConnection::UpdateLastActivity() {
  this->last_activity = steady_clock::now();
}

HttpConnection::Status HttpConnection::CheckTimeout() {
  bool close = false;
  steadytime defaultTimeout =
    this->last_activity + std::chrono::seconds(this->DEFAULT_TIMEOUT);

  steadytime keepaliveTimeout =
    this->last_activity + std::chrono::seconds(this->KEEP_ALIVE_TIMEOUT + 3);

  switch (this->status) {
    case Status::READING:
      if (steady_clock::now() > defaultTimeout) {
        close = true;
      } 
      break;
    case Status::READ:
      if (steady_clock::now() > defaultTimeout) {
        close = true;
      } 
      break;
    case Status::PROCESSING:
      if (steady_clock::now() > defaultTimeout) {
        close = true;
      } 
      break;
    case Status::SENDING:
      if (steady_clock::now() > defaultTimeout) {
        close = true;
      } 
      break;
    case Status::SENT:
      if (steady_clock::now() > keepaliveTimeout) {
        close = true;
      } 
      break;
    default:
      break;
  } 

  if (close == true) {
    this->Close();
  } 

  return this->status;
}

HttpConnection::Status HttpConnection::GetStatus() const {
  return this->status;
}

int HttpConnection::GetFd() const {
  return this->fd;
}

bool HttpConnection::IsKeepAlive() const {
  if (this->response != nullptr) {
    return this->response->IsKeepAlive();
  } 
  DEBUG_cout << "Checking if KeepAlive but Response is not set!" << endl; 
  return false;
}

HttpRequest* HttpConnection::GetRequest() {
  if (this->request == nullptr) {
    DEBUG_cerr << "Request is NULL!" << endl; 
    this->AcquireReusableRequest();
  } 
  return this->request;
}

HttpResponse* HttpConnection::GetResponse() {
  if (this->response == nullptr) {
    DEBUG_cerr << "Response is NULL!" << endl; 
    this->AcquireReusableResponse();
  } 
  return this->response;
}

HttpConnection::Status HttpConnection::ReadRequest() {
  if (this->status == Status::SENT) {
    DEBUG_cout << "Response is sent. Resetting connection to Open.." << endl; 
    this->status = Status::OPEN;

  } else if (this->status != Status::OPEN && this->status != Status::READING) {
    DEBUG_cerr << "Cannot read request. Connection is not even open." << endl; 
    return this->status;
  } 

  if (this->request == nullptr) {
    DEBUG_cerr << "Cannot read request. Request is not set!" << endl; 
    return this->status;
  } 

  HttpRequest::Status status = this->request->ReadRequest(this->fd,
      this->isSecureConnection, this->ssl);
  switch (status) {
    case HttpRequest::Status::DONE_READING:
      DEBUG_cout << "Request Read has returned status DONE_READING." << endl; 
      this->status = Status::READ;
      this->UpdateLastActivity();
      break;
    case HttpRequest::Status::READING_HEADER:
    case HttpRequest::Status::READING_BODY:
      DEBUG_cout << "Request Read has returned status READING." << endl; 
      this->status = Status::READING;
      this->UpdateLastActivity();
      break;
    case HttpRequest::Status::ERROR:
      DEBUG_cout << "Request Read has returned status ERROR." << endl; 
      this->status = Status::ERROR;
      break;
    case HttpRequest::Status::CONNECTION_CLOSED:
      DEBUG_cout << "Request Read has returned status CONNECTION_CLOSED" << endl; 
      this->status = Status::CLOSED;
      break;
    default:
      DEBUG_cerr << "Request Read has returned status: " << (int) status << endl; 
      break;
  } 

  return this->status;
}

bool HttpConnection::MarkProcessed() {
  if (this->response == nullptr) {
    DEBUG_cerr << "Cannot mark as processed. Response Null." << endl; 
    return false;
  } 

  if (this->response->GetStatus() != HttpResponse::Status::PROCESSED) {
    DEBUG_cerr << "Cannot mark as processed. Response is not Processed." << endl; 
    return false;
  } 

  this->status = Status::PROCESSED;
  return true;
}

HttpConnection::Status HttpConnection::SendResponse() {
  if (this->response == nullptr) {
    DEBUG_cerr << "Response is not initialized." << endl; 
    return this->status;
  } 

  if (this->response->GetStatus() == HttpResponse::Status::PROCESSED) {
    this->status = Status::PROCESSED;
  } 

  if (this->status < Status::PROCESSED) {
    DEBUG_cerr << "Request is not processed yet." << endl; 
    return this->status;
  } 


  auto responseStatus = this->response->Send(this->fd, this->sentSize,
      this->isSecureConnection, this->ssl);
  if (responseStatus == HttpResponse::Status::SENT) {
    this->status = Status::SENT;
    this->UpdateLastActivity();

  } else if (responseStatus == HttpResponse::Status::SENDING_BODY ||
      responseStatus == HttpResponse::Status::SENDING_HEADER)
  {
    this->status = Status::SENDING;
    this->UpdateLastActivity();

  } else if (responseStatus == HttpResponse::Status::ERROR) {
    // Gotta make sure connection is closed 
    // when this function returns ERROR status.
    this->Close(); // Probably it can be commented out.
    this->status = Status::ERROR;
  } 

  return this->status;
}

//HttpConnection::
//HttpConnection::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpConnection : public HttpConnection {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpConnection, TESTNAME) {
  MockHttpConnection mockHttpConnection;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

