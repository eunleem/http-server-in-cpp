#include "HttpServer.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpServer::Exception::exceptionMessages_[] = {
  HTTPSERVER_EXCEPTION_MESSAGES
};
#undef HTTPSERVER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpServer::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpServer::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpServer::ExceptionType
HttpServer::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


HttpServer::HttpServer(Config config) :
  config_(config),
  status_(ProcStatus::INIT),
  inoti_(this->AsyncSockets::GetEpollFd()),
  workerId_(0)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

  Logger::SetConfig(Logger::Config("logs", config.logDirPath));
  Logger::GetInstance().LogEvent("HttpServer Object has been created.");
}

HttpServer::~HttpServer() {
  DEBUG_FUNC_START;
  if (this->status_ != ProcStatus::STOPPED) {
    this->Stop();
  } 

  Logger::GetInstance().LogEvent("HttpServer Object bas been destroyed.");
}

void HttpServer::Run() {
  bool isEnvGood = this->CheckEnvironment();
  if (isEnvGood == false) {
    return;
  } 

  DEBUG_cout << "HttpServer is Running." << endl; 
  Logger::GetInstance().LogEvent("HttpServer is now running.");


  bool result = this->runNewWorker();
  if (result == false) {
    return;
  } 

  this->inoti_.AddToWatch(this->config_.workerDirPath, IN_MOVE | IN_MODIFY);
  this->status_ = ProcStatus::RUNNING;
  this->AsyncSockets::SetNumMaxEvent(100);
  this->AsyncSockets::AddSocket(this->config_.portNumber);
  this->AsyncSockets::AddSocket(this->config_.portNumberSecure);
  this->AsyncSockets::Listen(this->config_.portNumber);
  this->AsyncSockets::Listen(this->config_.portNumberSecure);
  this->AsyncSockets::Wait();
}

void HttpServer::Stop(int signum) {
  DEBUG_FUNC_START;
  this->AsyncSockets::Close(this->config_.portNumber);
  this->AsyncSockets::Close(this->config_.portNumberSecure);
  this->killWorker(this->curWorker_);
  this->status_ = ProcStatus::STOPPED;
}

bool HttpServer::runNewWorker() {
  string workerPath = this->config_.workerDirPath + "HttpWorker.exe";
  Worker newWorker = this->startWorker(workerPath);
  if (newWorker.pid <= 0) {
    DEBUG_cerr << "Failed to start Worker." << endl; 
    return false;
  } 
  this->workers_.push_back(newWorker);
  bool isWorkerRunning = this->connectToWorker(&this->workers_.back());
  if (isWorkerRunning == false) {
    DEBUG_cerr << "Could not connect to Worker." << endl; 
    return false;
  } 

  this->curWorker_ = &this->workers_.back();
  return true;
}

// ============================================================================

void HttpServer::OnFdEvent(const AsyncSockets::FdEventArgs& event) {
  const int socketFd = this->GetSocketFd(this->config_.portNumber);
  const int secureSocketFd = this->GetSocketFd(this->config_.portNumberSecure);

  if (event.fd == socketFd || event.fd == secureSocketFd) {
    if (event.type == FdEventArgs::EventType::EPOLLIN) {
      // Socket Event,
      while (true) {
        // On Socket Event
        struct sockaddr in_addr;
        socklen_t in_len = sizeof (in_addr);

        int acceptFd = accept (event.fd, &in_addr, &in_len);
        if (acceptFd == consts::ERROR) {
          if ((errno == EAGAIN) ||
              (errno == EWOULDBLOCK)) {
            // All requests have been processed.
            break;
          } 
          
          DEBUG_cerr << "AsyncSocket Accept Error." << endl;
          Logger::GetInstance().LogError("Failed to accept a connection.");
          close(event.fd);
          continue;
        } 

        this->setNonBlocking (acceptFd);

        // On Connection Accept
        uint16_t portNumber = this->config_.portNumber;
        bool isSecure = false;
        if (event.fd == secureSocketFd) {
          portNumber = this->config_.portNumberSecure;
          isSecure = true;
        } 

        AsyncSockets::Connection connection(in_addr, in_len, portNumber, isSecure);
        // Maybe I can use MOD % for better performance.

        Worker* worker = this->curWorker_;

        ssize_t sentCount = this->transferFd(worker->sockFd,
                                             acceptFd,
                                             connection);
        if (sentCount == -1) {
          DEBUG_cerr << "Server: Send Error. errno: " << errno << " sentCount: " << sentCount << endl;
          if (errno == EBADF) { // Bad File Descriptor (fdTobeSent is bad)
            // This error happens when client closes the connection before it gets processed.
            close(acceptFd);
          } else{
            DEBUG_cerr << "Failed to send a request to worker. errno: " << errno << endl;
            Logger::GetInstance().LogError("Failed to send a request to worker. errno: " +
                                    std::to_string(errno));
            close(acceptFd);
          }
        } else {
          DEBUG_cout << "Request Sent to Worker." << endl;
        }

        // Close the connection on Server side.
        // Since Fd is already passed to child, it's not closed on child's side. :)
        close(acceptFd);
        DEBUG_cout << "Successfully transferred connection to Worker." << endl;
      }
    }
  
  } else if (event.fd == this->inoti_.GetInotifyFd()) {
    DEBUG_cout << "INOTIFY EVENT" << endl; 
    char buffer[1024 * (sizeof(struct inotify_event) + 16) ];
    ssize_t readCount = 0;
    while (true) {
      readCount = read(event.fd, buffer, sizeof(buffer));
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
        if (ievent->len) {
          DEBUG_cout << "ievent Name: " << ievent->name << endl; 
          string fileName(ievent->name);
          DEBUG_cout << "fileName: " << fileName << endl; 
          if (ievent->mask & IN_CREATE) {
            if (ievent->mask & IN_ISDIR) {
              // Directory Created.
              DEBUG_cout << "Dir Created" << endl; 
            } else {
              DEBUG_cout << "File Created" << endl; 

            }
          } else if (ievent->mask & IN_DELETE) {
            if (ievent->mask & IN_ISDIR) {
              // Directory Deleted.
            } else {
              DEBUG_cout << "File Deleted" << endl; 
              // ************************************************

            }
          } else if (ievent->mask & IN_MODIFY) {
            if (ievent->mask & IN_ISDIR) {
              // Directory Modified.

            } else {
              // ************************************************

            }
          } else if (ievent->mask & IN_MOVE) {
            if (ievent->mask & IN_ISDIR) {
              
            } else {
              DEBUG_cout << "File moved or renamed." << endl; 
              // ************************************************
              if (fileName == "HttpWorker.exe") {
                Worker& oldWorker = this->workers_.front();
                this->killWorker(&oldWorker);
                bool result = this->runNewWorker();
                if (result == true) {
                  DEBUG_cout << "Running New Worker!" << endl; 
                  this->workers_.pop_front();
                } else {
                  DEBUG_cerr << "Failed to run new worker." << endl; 
                }
                
              } 
            }
          } 
        } 
        index += sizeof(struct inotify_event);
      }
    } 

  } else {
    DEBUG_cerr << "Unexpected epoll event." << endl; 
  }
}

HttpServer::Worker HttpServer::startWorker(const string& workerFileName) {
  DEBUG_cout << "Starting Worker..." << endl; 
  Logger::GetInstance().LogEvent("Starting Worker...");

  string socketKeyfileName = "ServerSocketKey" + std::to_string(this->workerId_);
    
  Worker newWorker;

  pid_t worker_pid;
  worker_pid = fork();
  if (worker_pid == 0) {
    execl(workerFileName.c_str(),
          workerFileName.c_str(),
          socketKeyfileName.c_str(),
          (char*) 0);
    DEBUG_cerr << "Failed to start Worker." << endl; 
    Logger::GetInstance().LogError("Failed to start Worker. THIS LINE SHOULD NEVER GET EXEXCUTED.");
    //this->worker_.status = ProcStatus::ERROR;

  } else if (worker_pid > 0) {
    // Parent
    DEBUG_cout << "Worker has started. PID: " << worker_pid << endl; 
    Logger::GetInstance().LogEvent("Worker has started.");
    newWorker.pid = worker_pid;
    newWorker.socketKeyFile = socketKeyfileName;
    newWorker.status = ProcStatus::INIT;

  } else {
    DEBUG_cerr << "Failed to fork a child process to start Worker." << endl; 
    Logger::GetInstance().LogError("Failed to fork child process for Worker.");
    //this->worker_.status = ProcStatus::ERROR;
  }

  this->workerId_ += 1;
  if (this->workerId_ > 10) {
    this->workerId_ = 0;
  } 

  return newWorker;
}

bool HttpServer::connectToWorker(Worker* worker) {
  DEBUG_FUNC_START;
  Socket* newSocket = new Socket(Socket::SocketFamily::LOCAL);
  bool connectResult =
    newSocket->Connect( this->config_.keyDirPath + worker->socketKeyFile);
  if (connectResult == false) {
    DEBUG_cerr << "Server: Could not connt to Worker." << endl; 
    Logger::GetInstance().LogError("Failed to connect to Worker");
    return false;
  } 

  worker->socket = newSocket;
  worker->sockFd = newSocket->GetSocketFd();
  worker->status = ProcStatus::RUNNING;
  return true;
}

ssize_t HttpServer::transferFd (const int fd, int fdToTransfer,
    AsyncSockets::Connection& connection) 
{
  // #LEARN: UNIX NETWORK PROGRAMMING book.
  //  Pg 363 for cmshdr explanation.
  //  Pg 381 for Passing fd to another process.
  struct msghdr   msg; // Size is 56
  struct iovec    iov[1]; // Size of each iovec is 16

  union {
    struct cmsghdr    cm; // Size is 16
    char              control[CMSG_SPACE(sizeof(int))]; // Size is 24
  } control_un;

  struct cmsghdr  *cmptr; // cmsghdr size is 16

  msg.msg_control = control_un.control; // Size 24
  msg.msg_controllen = sizeof(control_un.control); // Size 24

  cmptr = CMSG_FIRSTHDR(&msg);
  cmptr->cmsg_len = CMSG_LEN(sizeof(int)); // length in byte. // Size 8
  cmptr->cmsg_level = SOL_SOCKET; // originating protocol // Size 4
  cmptr->cmsg_type = SCM_RIGHTS; // Specifies that it's for fd transfer. // Size 4
  *((int *) CMSG_DATA(cmptr)) = fdToTransfer;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  //data.requestId = 1;
  iov[0].iov_base = &connection;
  iov[0].iov_len = sizeof(connection);
  //iov[0].iov_base = &connection;
  //iov[0].iov_len = sizeof(connection);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  DEBUG_cout << "  Sent FD from server. fdSent: " << fdToTransfer << endl;
  
  if (fcntl (fdToTransfer, F_GETFD) == -1) {
    DEBUG_cerr << "fd To be Sent is BAD!!" << endl;
    return -1;
  }

  ssize_t sentCount = sendmsg(fd, &msg, 0);
  if (sentCount == -1) {
    DEBUG_cerr << "sendmsg error. Errno: " << errno << endl; 
  } 
  return sentCount;
}

bool HttpServer::CheckEnvironment() {
  bool result = Util::File::IsDirectoryExisting(this->config_.keyDirPath);
  if (result == false) {
    DEBUG_cerr << "Key Directory " << this->config_.keyDirPath << " does not exist." << endl; 
    return false;
  } 
  result = Util::File::IsDirectoryExisting(this->config_.logDirPath);
  if (result == false) {
    DEBUG_cerr << "Log Directory " << this->config_.logDirPath << " does not exist." << endl; 
    return false;
  } 
  result = Util::File::IsDirectoryExisting(this->config_.workerDirPath);
  if (result == false) {
    DEBUG_cerr << "Worker Directory " << this->config_.workerDirPath << " does not exist." << endl; 
    return false;
  } 

  return true;
}


int HttpServer::killWorker(Worker* worker) {
  DEBUG_cout << "KLLING WORKER!" << endl; 
  write(worker->sockFd, "DIE", 3);
  char buf[3];
  auto readCount = read(worker->sockFd, buf, 3);
  if (readCount == -1) {
    DEBUG_cerr << "Failed to get response from worker." << endl; 
  } 
  close(worker->sockFd);
  delete worker->socket;
  worker->status = ProcStatus::KILLING;
  return 0;
}

void HttpServer::OnChildDeathSignal() {
  DEBUG_cout << "Do Nothing!" << endl; 
  return;

  Worker& worker = this->workers_.front();
  if (worker.status != ProcStatus::KILLING) {
    // Unexpected termination.
    close(worker.sockFd);
    delete worker.socket;

    this->workers_.pop_front();
    bool result = this->runNewWorker();
    if (result == true) {
      DEBUG_cout << "Running New Worker!" << endl; 
    } else {
      DEBUG_cerr << "Failed to run new worker." << endl; 
    }
  } 


}

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpServer : public HttpServer {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpServer, TESTNAME) {
  MockHttpServer mockHttpServer;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.

lio::HttpServer* server = nullptr;

void childdeath(int sig) {
  std::cout << "ChildDied. Sig: " << sig << std::endl;
  server->OnChildDeathSignal();

}

void sighandler(int signum) {
  DEBUG_cout << "Signal Receive. signum: " << signum << endl; 
  switch (signum) {
    case SIGUSR1:
      break;
    case SIGINT:
      DEBUG_cout << "Stopping Server..." << endl; 
      if (server != nullptr) {
        server->Stop();
        delete server;
        server = nullptr;
      } 
      break;
    case SIGHUP:
      DEBUG_cout << "SIGHUP caught!" << endl; 
      if (server != nullptr) {
        server->Stop();
        delete server;
        server = nullptr;
      } 
    default:
      DEBUG_cout << "Received signal but nothing's done." << endl; 
      break;
  } 
}


int main() {
  
  //http://stackoverflow.com/a/284443
  prctl(PR_SET_PDEATHSIG, SIGHUP);

  struct sigaction act;
  act.sa_handler = childdeath;
  act.sa_flags = SA_NOCLDWAIT;
  sigaction (SIGCHLD, &act, NULL);

  signal(SIGINT, sighandler);
  signal(SIGHUP, sighandler);
  signal(SIGUSR1, sighandler);

  lio::HttpServer::Config config;
  config.portNumber = 8080;
  config.portNumberSecure = 8088;

  DEBUG_cout << "Port: " << config.portNumber << ", " << config.portNumberSecure << endl; 

  server = new lio::HttpServer(config);
  server->Run();

  if (server != nullptr) {
    delete server;
  } 

  DEBUG_cout << "SERVER HAS BEEN TERMINATED GRACEFULLY!" << endl; 

  return 0;
}

#endif

#undef _UNIT_TEST

