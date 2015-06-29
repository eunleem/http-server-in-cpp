#ifndef _WORKER_HPP_
#define _WORKER_HPP_
/*
  Name
    Worker

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 27, 2015
  
  History
    October 22, 2014
      Created

  ToDos
    


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

#include <chrono>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <functional>

#include <cstring>

#include <signal.h>
#include <sys/prctl.h> // prctl()
#include <sys/ioctl.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "liolib/Socket.hpp"
#include "liolib/AsyncSocket.hpp"
#include "liolib/Inotify.hpp"


#include "Logger.hpp"

//#include "WorkManager.hpp"
#include "HttpConnection.hpp"
#include "HttpRequestPool.hpp"
#include "HttpResponsePool.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include "AppCore.hpp"


#include "liolib/DataBlock.hpp"

namespace lio {

using std::string;

class Worker : protected AsyncSocket {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define WORKER_EXCEPTION_MESSAGES \
  "Worker Exception has been thrown."

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

enum class Status : uint8_t {
  STOPPED,
  STOPPING,
  RUNNING
};

struct Config {
  Config(string serverSocketKeyName = "ServerSocketKey",
         string keyDirPath = "./key/",
         string logDirPath = "./log/") :
    serverSocketKeyName(serverSocketKeyName),
    keyDirPath(keyDirPath),
    logDirPath(logDirPath)
  { }

  string serverSocketKeyName;
  string keyDirPath;
  string logDirPath;
};

  Worker(Config config);
  ~Worker();

  bool Run();
  bool Stop();
protected:

  void OnFdEvent(const AsyncSocket::FdEventArgs& event) override;

  void OnReceiveFdEvent(const int fd);

  HttpConnection* HandleFdInEvent(const int fd);
  HttpConnection* HandleFdOutEvent(const int fd);



  static
  ssize_t getFdFromServer(const int fd, int& receivedFd, AsyncSocket::Connection& connection);

  void acceptAndRegisterConnection(const int fd);

  void handleFileChanges(const int fd);

  bool removeCache(string fileName);


  ssize_t purgeSocket(const int fd);
  
private:
  Status status;

  Config config_;
  
//WorkManager work;

  Inotify fileNoti_;


  int serverAcceptFd_;


  uint32_t workId_;
  uint32_t currentIndex_;

  std::unordered_map<int, AsyncSocket::Connection> connections_;
  std::unordered_map<int, HttpConnection> connectionByFd_;

  SSL_CTX* ssl_ctx;

  AppCore app_;


  bool initSSL();

  static
  const int MAX_HTTP_REQUEST;
  static
  const int MAX_HTTP_RESPONSE;
};

}

#endif

