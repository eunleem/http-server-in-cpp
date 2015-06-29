#include "WorkManager.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
WorkManager::Exception::exceptionMessages_[] = {
  WORKMANAGER_EXCEPTION_MESSAGES
};
#undef WORKMANAGER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


WorkManager::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
WorkManager::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const WorkManager::ExceptionType
WorkManager::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


const int WorkManager::MAX_HTTP_REQUEST = 100;
const int WorkManager::MAX_HTTP_RESPONSE = 100;

WorkManager::WorkManager() : 
  workId_(1),
  currentIndex_(0),
  app_(),
  ssl_ctx(nullptr)
{
  DEBUG_FUNC_START;

  /* Load encryption & hashing algorithms for the SSL program */
  SSL_library_init();
  /* Load the error strings for SSL & CRYPTO APIs */
  SSL_load_error_strings();

  SSL_METHOD* sslMethod = const_cast<SSL_METHOD*>(TLSv1_method());

  this->ssl_ctx = SSL_CTX_new(sslMethod);
  if (this->ssl_ctx == nullptr) {
    DEBUG_cerr << "Could not initialize SSL Context." << endl; 
    ERR_print_errors_fp(stderr);
    return;
  } 

  /* Load the server certificate into the SSL_CTX structure */
  if (SSL_CTX_use_certificate_file(this->ssl_ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
    DEBUG_cerr << "Could not load certificate file." << endl; 
    ERR_print_errors_fp(stderr);
    return;
  }

  /* Load the private-key corresponding to the server certificate */
  if (SSL_CTX_use_PrivateKey_file(this->ssl_ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
    DEBUG_cerr << "Could not load private key." << endl; 
    ERR_print_errors_fp(stderr);
    return;
  }

  /* Check if the server certificate and private-key matches */
  if (!SSL_CTX_check_private_key(this->ssl_ctx)) {
    DEBUG_cerr << "Private key does not match the certificate public key." << endl; 
  }
}

WorkManager::~WorkManager() {
  DEBUG_FUNC_START;
  DEBUG_cout << "Freeing SSL Context." << endl; 
  if (this->ssl_ctx != nullptr) {
    SSL_CTX_free(this->ssl_ctx);
  } 
}

bool WorkManager::RemoveCache(const string& cacheName) {
  return this->app_.RemoveCache(cacheName);
}

HttpConnection* WorkManager::HandleFdInEvent(int fd, AsyncSocket::Connection& conn) {
  HttpConnection* connection = nullptr;

  auto it = this->connectionByFd_.find(fd);
  if (it != this->connectionByFd_.end()) {
    // Existing Connection
    DEBUG_cout << "Existing Connection!" << endl; 
    connection = &(it->second);
    if (conn.isSecure == false) {
      int bytes_available = 0;
      ioctl(fd, FIONREAD, &bytes_available);
      DEBUG_cout << "ioctl return value: " << bytes_available << endl; 
      if (bytes_available == 0) {
        // Connection close.
        char tmp[10];
        auto c = read(connection->GetFd(), tmp, 10);
        DEBUG_cout << "ReadCount: " << c << endl; 
        connection->Close();
        this->connectionByFd_.erase(it);
        DEBUG_cout << "Connection closed." << endl; 
        return nullptr;
      } 
    } 

    auto status = connection->GetStatus();
    if (status == HttpConnection::Status::CLOSED) {
      DEBUG_cout << "Connection is Closed! Opening it!" << endl; 
      if (conn.isSecure == true) {
        connection->Open(fd, conn.isSecure, this->ssl_ctx);
      } else {
        connection->Open(fd);
      }
      connection->AcquireReusableRequest();

    } else if(status == HttpConnection::Status::SENT) {
      connection->AcquireReusableRequest();
      //connection->SetRequest(this->appointRequest());

    } else if (status == HttpConnection::Status::OPENING) {
      connection->Open(fd, conn.isSecure, this->ssl_ctx);
    } 
    // #TODO I THINK SOMETHING SHOULD BE DONE HERE!

  } else {
    // New Connection
    DEBUG_cout << "New Connection!" << endl; 
    if (conn.isSecure == true) {
      this->connectionByFd_[fd].Open(fd, conn.isSecure, this->ssl_ctx);
    } else {
      this->connectionByFd_[fd].Open(fd);
    }
    connection = &this->connectionByFd_[fd];
    connection->AcquireReusableRequest();
  }

  DEBUG_cout << "Connection Status: " << (int) connection->GetStatus() << endl; 
  if (connection->GetStatus() == HttpConnection::Status::OPEN ||
      connection->GetStatus() == HttpConnection::Status::READING ||
      connection->GetStatus() == HttpConnection::Status::SENT) {
    connection->ReadRequest();
    if (connection->GetStatus() == HttpConnection::Status::READ) {
      bool isProcessed = this->app_.Process(connection);
      if (isProcessed == true) {
        connection->MarkProcessed();
      } 
    } 
  } 

  if (connection->GetStatus() == HttpConnection::Status::PROCESSED) {
    DEBUG_cout << "PROCESSED." << endl; 
    connection->FreeRequest();
    connection->SendResponse();
  } 

  if (connection->GetStatus() == HttpConnection::Status::SENT) {
    DEBUG_cout << "SENT." << endl; 
    if (connection->IsKeepAlive() == false) {
      connection->CheckTimeout();
    } 
    connection->FreeResponse();
  } 


  if (connection->GetStatus() == HttpConnection::Status::ERROR) {
    DEBUG_cerr << "ERROR." << endl; 
    //connection->FreeRequest();
    //connection->FreeResponse();
    connection->Close();
  } 

  if (connection->GetStatus() == HttpConnection::Status::CLOSED) {
    //connection->FreeRequest();
    //connection->FreeResponse();
    connection->Close();
  } 

  return connection;
}

HttpConnection* WorkManager::HandleFdOutEvent(int fd) {
  auto it = this->connectionByFd_.find(fd);
  if (it != this->connectionByFd_.end()) {
    DEBUG_cout << "FdOutEvent. Found connection." << endl; 

    auto& connection = it->second;

    if (connection.GetStatus() == HttpConnection::Status::SENDING) {
      DEBUG_cout << "Continuing Sending. fd: " << connection.GetFd() << endl; 
      connection.SendResponse();
    } else {
      DEBUG_cerr << "Epoll Out event for non-sending response. Status:" <<
        (int) connection.GetStatus() << endl; 
      if (connection.GetStatus() == HttpConnection::Status::CLOSED) {
        //connection.Close();
      } 
    }

    if (connection.GetStatus() == HttpConnection::Status::SENT) {
      connection.FreeResponse();
    } 

    if (connection.GetStatus() == HttpConnection::Status::ERROR ||
        connection.GetStatus() == HttpConnection::Status::TIMEOUT) {
      connection.FreeRequest();
      connection.FreeResponse();
      connection.Close();
    } 

    return &connection;
  } 
  return nullptr;
}

int WorkManager::CheckTimeout() {
  int count = 0;
  for (auto& connection : this->connectionByFd_) {
    const auto& status = connection.second.CheckTimeout();
    if (status == HttpConnection::Status::CLOSED) {
      count += 1;
    } 
  } 
  return count;
}



//WorkManager::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockWorkManager : public WorkManager {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(WorkManager, TESTNAME) {
  MockWorkManager mockWorkManager;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

