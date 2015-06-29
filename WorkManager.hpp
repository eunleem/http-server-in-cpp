#ifndef _WORKMANAGER_HPP_
#define _WORKMANAGER_HPP_
/*
  Name
    WorkManager

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 06, 2015
  
  History
    November 11, 2014
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

#include <unordered_map>
#include <vector>

#include <functional>
#include <sys/ioctl.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "liolib/AsyncSocket.hpp" // AsyncSocket::Connection

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpConnection.hpp"

#include "AppCore.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {


class WorkManager {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define WORKMANAGER_EXCEPTION_MESSAGES \
  "WorkManager Exception has been thrown."

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

  WorkManager();
  ~WorkManager();

  bool RemoveCache(const string& cacheName);

  HttpConnection* HandleFdInEvent(int fd,
      AsyncSocket::Connection& connection);
  HttpConnection* HandleFdOutEvent(int fd);

  int CheckTimeout();

protected:
  
private:
  uint32_t workId_;
  uint32_t currentIndex_;

  std::unordered_map<int, HttpConnection> connectionByFd_;

  AppCore app_;


  SSL_CTX* ssl_ctx;
  static
  const int MAX_HTTP_REQUEST;
  static
  const int MAX_HTTP_RESPONSE;
  
};

}

#endif

