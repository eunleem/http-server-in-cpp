#ifndef _HTTPCONNECTION_HPP_
#define _HTTPCONNECTION_HPP_
/*
  Name
    HttpConnection

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 20, 2015
  
  History
    November 09, 2014
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

#include <functional> // std::function

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#include "HttpRequestPool.hpp"
#include "HttpResponsePool.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


class HttpConnection {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPCONNECTION_EXCEPTION_MESSAGES \
  "HttpConnection Exception has been thrown."

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

typedef std::chrono::system_clock::time_point datetime;
typedef std::chrono::steady_clock::time_point steadytime;

enum class Status : uint8_t {
  CLOSED,
  OPENING,
  OPEN,
  READING,
  READ,
  PROCESSING,
  PROCESSED,
  SENDING,
  SENT,
  ERROR,
  TIMEOUT
};

  HttpConnection();
  ~HttpConnection();

  HttpRequest* GetRequest();
  HttpResponse* GetResponse();

  Status Open(int fd, bool isSecureConnection = false, SSL_CTX* ssl_ctx = nullptr);
  void AcquireReusableRequest();
  void SetRequest(HttpRequest* request);
  Status ReadRequest();

  void SetResponse(HttpResponse* response);

  void AcquireReusableResponse();


  bool MarkProcessed();

  Status SendResponse();
  Status Close();

  bool FreeRequest();
  bool FreeResponse();

  inline
  void UpdateLastActivity();

  Status CheckTimeout();
  Status GetStatus() const;
  int GetFd() const;
  bool IsKeepAlive() const;

  static
  int KEEP_ALIVE_TIMEOUT;

  static
  int DEFAULT_TIMEOUT;

protected:
  
private:
  int fd;
  bool isSecureConnection;
  SSL* ssl;
  Status status;
  steadytime last_activity;
  HttpRequest* request;
  HttpResponse* response;
  size_t sentSize;



};

}

#endif

