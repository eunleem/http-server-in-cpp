#ifndef _HTTPRESPONSE_HPP_
#define _HTTPRESPONSE_HPP_
/*
  Name
    HttpResponse

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Mar 04, 2015
  
  History
    October 24, 2014
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

#include <cstring>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "liolib/http/Http.hpp"

#include "liolib/Util.hpp"
//#include "liolib/DataBlock.hpp"

namespace lio {

using http::ResponseCode;
using http::HttpVersion;

using std::chrono::system_clock;
using std::chrono::steady_clock;

class HttpResponse {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPRESPONSE_EXCEPTION_MESSAGES \
  "HttpResponse Exception has been thrown."

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
  NEW,
  IN_PROCESS,
  PROCESSED,
  SENDING_HEADER,
  HEADER_SENT,
  SENDING_BODY,
  SENT,
  CACHED,
  CACHE_EXPIRED,
  TIMEOUT,
  ERROR // Error means something critical. Just close connection.
};

static
int KEEP_ALIVE_TIMEOUT;

  HttpResponse(const ResponseCode code = ResponseCode::UNDEF,
      HttpVersion version = HttpVersion::V1_1);
  ~HttpResponse();

  bool Reset(const ResponseCode code = ResponseCode::UNDEF,
      HttpVersion version = HttpVersion::V1_1);

  bool CheckTimeout(uint32_t seconds = 10);

  Status GetStatus() const;

  bool IsKeepAlive() const;

  void SetStatus(Status status);
  bool SetResponseCode(const ResponseCode responseCode,
      const HttpVersion version = HttpVersion::V1_1);

  bool AddHeaderField(const string& header, const string& value);
  bool SetCookie(const string& name, const string& value,
      datetime exp = system_clock::now(),
      bool isSecure = false, bool isHttpOnly = false);
  bool SetBody(const char* address, size_t size, http::ContentType contentType,
      bool isTempBody = false);
  bool SetBody(string& body, http::ContentType type);

  Status Send(const int& fd, size_t& sentSize,
      bool isSecureConnection = false, SSL* ssl = nullptr);

  bool Cache(int seconds = 0); // 0 is default;
protected:
  
private:
  Status status;
  steadytime lastActivity;
  bool isKeepAlive;
  bool isGzipped;
  http::HttpVersion httpVersion;
  ResponseCode responseCode;
  datetime date;
  datetime lastModified;
  http::ContentType contentType;
  std::map<string, string> cookies;
  char header[4096];
  size_t headerSize;
  char* body;
  size_t bodySize;
  bool isTempBody;

  size_t buildHeader();
};

}

#endif

