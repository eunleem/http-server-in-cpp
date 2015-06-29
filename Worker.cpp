#include "Worker.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Worker::Exception::exceptionMessages_[] = {
  WORKER_EXCEPTION_MESSAGES
};
#undef WORKER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Worker::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Worker::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Worker::ExceptionType
Worker::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


const int Worker::MAX_HTTP_REQUEST = 100;
const int Worker::MAX_HTTP_RESPONSE = 100;

Worker::Worker(Config config) :
  AsyncSocket(Socket::SocketFamily::LOCAL),
  status(Status::STOPPED),
  config_(config),
  fileNoti_(this->AsyncSocket::GetEpollFd()),
  serverAcceptFd_(-2),  // This is CRUCIAL.
  workId_(1),
  currentIndex_(0),
  ssl_ctx(nullptr)

{
  DEBUG_FUNC_START; // Prints out function name in yellow

  //Logger::SetConfig(Logger::Config("worker_log", config.logDirPath));
}

Worker::~Worker() {
  DEBUG_FUNC_START;
  this->Stop();
}

bool Worker::Run() {
  if (this->status != Status::STOPPED) {
    DEBUG_cerr << "Cannot Run Worker when it's not in STOPPED status." << endl; 
    return false;
  } 

  this->initSSL();

  string socketKeyPath = this->config_.keyDirPath + this->config_.serverSocketKeyName;
  this->fileNoti_.AddToWatch("./lifeino", IN_MOVE | IN_MODIFY | IN_DELETE);
  this->status = Status::RUNNING;
  this->AsyncSocket::Listen(socketKeyPath);
  return true;
}

bool Worker::Stop() {
  if (this->ssl_ctx != nullptr) {
    SSL_CTX_free(this->ssl_ctx);
  } 
  this->status = Status::STOPPED;

  return true;
}

bool Worker::initSSL() {
  /* Load encryption & hashing algorithms for the SSL program */
  SSL_library_init();
  /* Load the error strings for SSL & CRYPTO APIs */
  SSL_load_error_strings();

  SSL_METHOD* sslMethod = const_cast<SSL_METHOD*>(TLSv1_method());

  this->ssl_ctx = SSL_CTX_new(sslMethod);
  if (this->ssl_ctx == nullptr) {
    DEBUG_cerr << "Could not initialize SSL Context." << endl; 
    ERR_print_errors_fp(stderr);
    return false;
  } 

  /* Load the server certificate into the SSL_CTX structure */
  if (SSL_CTX_use_certificate_file(this->ssl_ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
    DEBUG_cerr << "Could not load certificate file." << endl; 
    ERR_print_errors_fp(stderr);
    return false;
  }

  /* Load the private-key corresponding to the server certificate */
  if (SSL_CTX_use_PrivateKey_file(this->ssl_ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
    DEBUG_cerr << "Could not load private key." << endl; 
    ERR_print_errors_fp(stderr);
    return false;
  }

  /* Check if the server certificate and private-key matches */
  if (!SSL_CTX_check_private_key(this->ssl_ctx)) {
    DEBUG_cerr << "Private key does not match the certificate public key." << endl; 
    return false;
  }

  return true;
}

void Worker::OnFdEvent(const AsyncSocket::FdEventArgs& event) {
  const int serverSocketFd = this->GetSocketFd();
  DEBUG_cout << "FdEvent. fd: " << event.fd << endl; 

  if (event.fd == this->serverAcceptFd_) {
    // REGULAR REQUESTS
    DEBUG_cout << "     ServerAcceptEvent\n";
    this->OnReceiveFdEvent (event.fd);

  } else if (event.fd == serverSocketFd) {
    DEBUG_cout << "  App: SocketEvent\n";
    this->acceptAndRegisterConnection(event.fd);
    return;

  } else if (event.fd == this->fileNoti_.GetInotifyFd()) {
    this->handleFileChanges(event.fd);
    
  } else {
    if (event.type == FdEventArgs::EventType::EPOLLIN) {
      DEBUG_cout << "IN" << endl; 
      HttpConnection* connection = this->HandleFdInEvent(event.fd);
      if (connection == nullptr) {
        auto it = this->connections_.find(event.fd);
        if (it != this->connections_.end()) {
          this->connections_.erase(it);
        } 
      } else if (
          connection->GetStatus() == HttpConnection::Status::CLOSED ||
          connection->GetStatus() == HttpConnection::Status::ERROR)
      {
        auto it = this->connections_.find(event.fd);
        if (it != this->connections_.end()) {
          this->connections_.erase(it);
        } 
      } 
    } else if (event.type == FdEventArgs::EventType::EPOLLOUT) {
      DEBUG_cout << "OUT fd: " << event.fd << endl; 
      this->HandleFdOutEvent(event.fd);
    } 
  }
}

HttpConnection* Worker::HandleFdInEvent(int fd) {
  AsyncSocket::Connection& conn = this->connections_[fd];

  HttpConnection* connection = nullptr;

  auto it = this->connectionByFd_.find(fd);
  if (it == this->connectionByFd_.end()) {
    // New Connection
    DEBUG_cout << "New Connection!" << endl; 
    if (conn.isSecure == true) {
      this->connectionByFd_[fd].Open(fd, conn.isSecure, this->ssl_ctx);
    } else {
      this->connectionByFd_[fd].Open(fd);
    }
    connection = &this->connectionByFd_[fd];
    connection->AcquireReusableRequest();

  } else {
    // Existing Connection
    DEBUG_cout << "Existing Connection!" << endl; 
    connection = &(it->second);
    if (conn.isSecure == false) {
      int bytes_available = 0;
      ioctl(fd, FIONREAD, &bytes_available);
      DEBUG_cout << "ioctl return value: " << bytes_available << endl; 
      if (bytes_available == 0) {
        // Connection close.
        this->purgeSocket(connection->GetFd());
        connection->Close();
        this->connectionByFd_.erase(it);
        DEBUG_cout << "Connection closed." << endl; 
        return nullptr;
      } 
    } 

    const auto& status = connection->GetStatus();
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

    } else if (status == HttpConnection::Status::OPENING) {
      connection->Open(fd, conn.isSecure, this->ssl_ctx);
    } 
    // #TODO I THINK SOMETHING SHOULD BE DONE HERE!
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

HttpConnection* Worker::HandleFdOutEvent(const int fd) {
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

ssize_t Worker::purgeSocket(const int fd) {
  DEBUG_cout << "PURGING SOCKET." << endl; 
  char tmp[10];
  ssize_t c = read(fd, tmp, 10);
  if (c > 0) {
    DEBUG_cout << "Purged Count: " << c << endl; 
  } 

  return c;
}

void Worker::acceptAndRegisterConnection(const int fd) {
  while (true) {
    struct sockaddr in_addr;
    socklen_t in_len = sizeof (in_addr);

    int acceptFd = accept (fd, &in_addr, &in_len);
    if (acceptFd == ERROR) {
      if ((errno == EAGAIN) ||
          (errno == EWOULDBLOCK)) {
        // All requests have been processed.
        DEBUG_cout << "AppSocketEvent: All Request have been processed." << endl; 
        return;
      } else {
        DEBUG_cerr << "AsyncSocket Accept Error" << endl;
        return;
      }
    }
    DEBUG_cout << "    AcceptEvent: " << acceptFd << endl;
    if (this->serverAcceptFd_ > 0) {
      // if connection is already established, close the connection
      // and make the new connection nonblocking
      DEBUG_cout << "Closing serverAcceptFd: " << this->serverAcceptFd_ << endl; 
      close (this->serverAcceptFd_);
    }

    this->serverAcceptFd_ = acceptFd;
    this->setNonBlocking (acceptFd);
    this->addFdToEpoll (this->epollFd_, acceptFd);
  } 


  DEBUG_cerr << "Returned with Error." << endl; 
}

void Worker::handleFileChanges(const int fd) {
  char buffer[1024 * (sizeof(struct inotify_event) + 16) ];
  ssize_t readCount = 0;
  while (true) {
    readCount = read(fd, buffer, sizeof(buffer));
    if (readCount < 0) {
      if (errno != EAGAIN) {
        DEBUG_cerr << "read Error. errno: " << errno << endl; 
      } 
      break;
    } 
    DEBUG_cout << "FE readCount: " << readCount << endl; 

    ssize_t index = 0;
    while (index < readCount) {
      struct inotify_event *ievent = (struct inotify_event *) &buffer[index];
      DEBUG_cout << "ievent->len: " << ievent->len << endl; 
      if (ievent->len) {
        DEBUG_cout << "ievent Name: " << ievent->name << endl; 
        DEBUG_cout << "ievent Wd: " << ievent->wd << endl; 

        const std::map<int, string>& dirs = this->fileNoti_.GetWatchingDirs();

        auto it = dirs.find(ievent->wd);
        if (it == dirs.end()) {
          DEBUG_cerr << "NOTI ERROR. Continuing." << endl; 
          goto NOTISKIP;
        } 

        string fileName = (*it).second;
        DEBUG_cout << "fileName: " << fileName << endl; 
        if (fileName[fileName.length() - 1] != '/') {
          fileName += "/";
        } 
        fileName += ievent->name;
        if (fileName.find(".swp", fileName.length() - strlen(".swp")) != string::npos) {
          DEBUG_cout << ".swp files are ignored." << endl; 
          goto NOTISKIP;
        } 
        if (ievent->mask & IN_DELETE) {
          if (ievent->mask & IN_ISDIR) {
            // Directory Deleted.
          } else {
            DEBUG_cout << "File Deleted" << endl; 
            this->removeCache(fileName);

          }
        } else if (ievent->mask & IN_MODIFY) {
          if (ievent->mask & IN_ISDIR) {
            // Directory Modified.

          } else {
            DEBUG_cout << "File modified." << endl; 
            this->removeCache(fileName);
          }
        } else if (ievent->mask & IN_MOVE) {
          if (ievent->mask & IN_ISDIR) {
            
          } else {
            DEBUG_cout << "File moved or renamed." << endl; 
            this->removeCache(fileName);

          }
        } 
      } 
NOTISKIP:
      index += sizeof(struct inotify_event);
    }
  } 
}

void Worker::OnReceiveFdEvent(int fd) {

  const int ERROR = -1;
  const int SUCCESS = 0;

  int httpRequestFd = 0;
  AsyncSocket::Connection connection;

  while (true) {
    
    ssize_t size = this->getFdFromServer(fd, httpRequestFd, connection);
    if (size == ERROR) {
      if (errno == EAGAIN) {
        DEBUG_cout << "App: Done reading from server." << endl;
      } else {
        DEBUG_cerr << "App: getFdFromServer() failed." << endl;
      }
      return;
    } else if (size == 0) {
      // Connection CLOSED. DIE!
      DEBUG_cout << "Connection Closed by Server. Dying! Ugh!" << endl; 
      this->AsyncSocket::StopGracefully();
      //this->Stop();
      break;
    } else if(size == 3) {
      DEBUG_cout << "Connection Closed by Server. Dying! Ugh!" << endl; 
      this->AsyncSocket::StopGracefully();
      break;

    }

    if (httpRequestFd <= 0) {
      DEBUG_cerr << "App: Invalid fd has been received. fd: " << httpRequestFd << endl;
      return;
    }

    this->connections_[httpRequestFd] = connection;
    DEBUG {
      char hostBuf[NI_MAXHOST], portBuf[NI_MAXSERV];
      int result = ERROR;
      result = getnameinfo (&(connection.in_addr), connection.in_len,
                            hostBuf, sizeof hostBuf,
                            portBuf, sizeof portBuf,
                            NI_NUMERICHOST | NI_NUMERICSERV);
      if (result == SUCCESS) {
        // #TODO: Convert it to LOGGING.
        DEBUG_cout << "AppTest Connection Accepted. Fd: " << httpRequestFd <<
                                        " Host: " << hostBuf <<
                                        " Port: " << portBuf << endl;
      } else {
        DEBUG_cerr << "connection info error... errno: " << errno << endl;
      }
    }
    
    this->addFdToEpoll (this->epollFd_, httpRequestFd, EPOLLOUT);
  }
}

ssize_t Worker::getFdFromServer(int fd, int& receivedFd, AsyncSocket::Connection& connection) {

  const ssize_t ERROR = -1;

  struct msghdr   msg;
  struct iovec    iov[1];
  ssize_t         readCount = -1;

  union {
    struct cmsghdr    cm;
    char              control[CMSG_SPACE(sizeof(int))];
  } control_un;
  struct cmsghdr  *control_message = NULL;

  memset(&msg, 0, sizeof(struct msghdr));
  memset(&control_un.control, 0, sizeof(control_un.control));

  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof(control_un.control);
  //DEBUG_cout << "msg_controllen: " << msg.msg_controllen << endl; 

  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  // #WARN: This section must match withe section in httpserver sendFdToWorker.
  // OMG. This freaking thing wasted so many of my precious hours.
  iov[0].iov_base = &connection;
  iov[0].iov_len = sizeof(connection);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

#if 0
  bool result = fcntl(0, F_GETFD) != -1 || errno != EBADF;
  DEBUG_cout << "IS 0 VALID FD? " << result << endl; 
  result = fcntl(1, F_GETFD) != -1 || errno != EBADF;
  DEBUG_cout << "IS 1 VALID FD? " << result << endl; 
  result = fcntl(2, F_GETFD) != -1 || errno != EBADF;
  DEBUG_cout << "IS 2 VALID FD? " << result << endl; 
#endif 

  readCount = recvmsg(fd, &msg, 0);
  if (readCount == 3) {
    return 3;
  } 
  if (readCount > 0) {
    DEBUG_cout << "getFdFromServer ReadCount: " << readCount << endl; 
    //DEBUG_cout << "msg_flags: " << msg.msg_flags << endl; 
    //break;
  } else if (readCount == -1) {
    if (errno == EAGAIN) {
      DEBUG_cout << "read_fd recvmsg done reading." << endl;
      //break;
    } else {
      DEBUG_cerr << "read_fd recvmsg Error! errno: " << errno << " readCount: " << readCount << endl;
    }
    return ERROR;
  } else if (readCount == 0) {
    DEBUG_cout << "read_fd recvmsg. read 0. DO NOTHING." << endl;
    return 0;
  }

  for (control_message = CMSG_FIRSTHDR(&msg);
       control_message != NULL;
       control_message = CMSG_NXTHDR(&msg, control_message))
  {
    if ( (control_message->cmsg_level == SOL_SOCKET) &&
         (control_message->cmsg_type == SCM_RIGHTS))
    {
      receivedFd = *((int *) CMSG_DATA(control_message));

      DEBUG_cout << "CHILD: Message has been Received!" << " receivedFd: " << receivedFd << endl;
      DEBUG_cout << "connection portNumber: " << connection.portNumber << endl; 
      return readCount;
    }
  }

  return ERROR;
}


bool Worker::removeCache(string fileName) {
  return this->app_.RemoveCache(fileName);
}

//Worker::
//Worker::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockWorker : public Worker {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Worker, TESTNAME) {
  MockWorker mockWorker;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.

#include <sys/prctl.h> // prctl();

lio::Worker* worker = nullptr;

void sighandler(int signum) {
  DEBUG_cout << "CHILD Signal Receive. signum: " << signum << endl; 
  switch (signum) {
    case SIGUSR1:
      DEBUG_cout << "SIGUSR1 Received!" << endl; 
      // #TODO: SAVE DATA
      break;
    case SIGHUP:
      DEBUG_cout << "SIGHUP Received!" << endl; 
      if (worker != nullptr) {
        delete worker;
        worker = nullptr;
      } 
      break;
    case SIGINT:
      DEBUG_cout << "SIGINT Received!" << endl; 
      if (worker != nullptr) {
        delete worker;
        worker = nullptr;
      } 
      break;
    default:
      if (worker != nullptr) {
        delete worker;
        worker = nullptr;
      } 
      break;
  } 

}
int main(int argc, char** argv) {
  //http://stackoverflow.com/a/284443
  prctl(PR_SET_PDEATHSIG, SIGHUP);

  signal(SIGPIPE, SIG_IGN);

  signal(SIGINT, sighandler);
  signal(SIGHUP, sighandler);
  signal(SIGUSR1, sighandler);


  string socketFileName(argv[1]);
  DEBUG_cout << "socketFileName argv1: " << argv[1] << endl; 
  lio::Worker::Config config(socketFileName);
  worker = new lio::Worker(config);
  try {
    worker->Run();

  } catch (...) {
    delete worker;
    worker = nullptr;

  } 

  if (worker != nullptr) {
    delete worker;
    worker = nullptr;
  } 

  DEBUG_cout << "WORKER HAS BEEN TERMINATED GRACEFULLY!" << endl; 

  return 0;
}

#endif

#undef _UNIT_TEST

