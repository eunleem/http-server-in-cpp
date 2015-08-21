#ifndef _HTTPSERVER_HPP_
#define _HTTPSERVER_HPP_
/*
  Name
    HttpServer

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description
    One Master One Worker architecture Http Server

  Last Modified Date
    Aug 20, 2015
  
  History
    October 22, 2014
      Created

  ToDos
    Basic Anti DoS feature by adding whitelist, blacklist.
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <algorithm> // for_each
#include <functional> //std::function
#include <list> // list
#include <string>
#include <thread>
#include <vector>

#include <cstdlib> // abort()
#include <cstdint> // uint16_t
#include <cstring> // memset()
#include <cerrno> // errno
//#include <ctime> // timer

#include <unistd.h> // fcntl(), read(), execl(), sleep();
#include <signal.h> // kill(), SIGUSR1
#include <wait.h> // waitpid()
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include <sys/prctl.h> // prctl
#include <sys/types.h> //getaddrinfo()
#include <sys/stat.h> //
#include <sys/file.h>
//#include <sys/socket.h> // getaddrinfo() getnameinfo()
//#include <netdb.h> // getaddrinfo() getnameinfo()
//#include <sys/epoll.h>


#include "Logger.hpp"


#include "liolib/Socket.hpp"
#include "liolib/AsyncSockets.hpp"

#include "liolib/Inotify.hpp"

#include "liolib/Util.hpp"


//#include "liolib/DataBlock.hpp"


namespace lio {

using std::string;
using std::cout;
using std::endl;

using std::list;


class HttpServer : protected AsyncSockets {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  NOT_INITIALIZED,
  ENV_NOT_MET,
  WORKER_EXEC_FAILED
};
#define HTTPSERVER_EXCEPTION_MESSAGES \
  "HttpServer Exception has been thrown.", \
  "Socket has not been initialized.", \
  "Required Environment has not met.", \
  "Failed to start workers."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********

struct Config {
  Config() :
    portNumber(80),
    portNumberSecure(443),
    logDirPath("./log/"),
    keyDirPath("./key/"),
    workerDirPath("./workers/")
  {
    this->formatDirPath(this->logDirPath);
    this->formatDirPath(this->keyDirPath);
    this->formatDirPath(this->workerDirPath);
  }

  uint16_t portNumber;
  uint16_t portNumberSecure;
  string logDirPath;
  string keyDirPath;
  string workerDirPath;

  bool formatDirPath(string& path) {
    if (path.back() != '/') {
      path.append("/");
    } 
    return true;
  }
};


enum class ProcStatus : uint8_t {
  INIT,
  RUNNING,
  STOPPED,
  KILLING,
  KILLED,
  ERROR
};

struct Worker {
  Worker() :
    pid(0),
    status(ProcStatus::INIT),
    socketKeyFile("./key/worker.key"),
    socket(nullptr),
    sockFd(0) {}
  pid_t         pid;
  ProcStatus    status;
  string        socketKeyFile;
  Socket*       socket;
  int           sockFd;
};

  HttpServer(Config config);
  virtual
  ~HttpServer();

  void      Run();
  void      Stop(int signum = 0);

  void      OnChildDeathSignal();

  void      ReportStatus();

  bool      CheckEnvironment();


protected:
  void      OnFdEvent(const AsyncSockets::FdEventArgs& event) override;
  
private:
  Config config_;
  ProcStatus status_;

  Inotify inoti_;

  int workerId_;

  list<Worker> workers_;
  Worker* curWorker_;


  ssize_t   transferFd(const int fd, int fdToTransfer, AsyncSockets::Connection& connection);

  bool      runNewWorker();

  Worker    startWorker(const string& workerFileName);
  bool      connectToWorker(Worker* worker);
  int       killWorker(Worker* worker);
  
};

}

#endif

