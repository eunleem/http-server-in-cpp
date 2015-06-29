#ifndef _HTTPREQUEST_HPP_
#define _HTTPREQUEST_HPP_
/*
  Name
    HttpRequest

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Feb 23, 2015
  
  History
    October 23, 2014
      Created

  ToDos
    Preprocess
      Find ContentType and Parse boundry and all that.
    


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

#include <cstddef> // ptrdiff_t
#include <cstring> // memcpy()

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "liolib/http/Http.hpp"

#include "liolib/Consts.hpp"
#include "liolib/Util.hpp"

#include "liolib/DataBlock.hpp"


namespace lio {


using std::chrono::system_clock;
using lio::http::HttpVersion;
using lio::http::RequestMethod;

class HttpRequest {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPREQUEST_EXCEPTION_MESSAGES \
  "HttpRequest Exception has been thrown."

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


enum class Status : uint8_t {
  NEW,
  READING_HEADER,
  READ_HEADER,
  CONTINUE,
  READING_BODY,
  DONE_READING,
  PREPROCESSED,
  IN_PROCESS,
  PROCESSED,
  // ==================
  CONNECTION_CLOSED,
  ERROR,
  READ_TIMEOUT,
  PROCESS_TIMEOUT
};

  HttpRequest(uint32_t id = 0);
  ~HttpRequest();

  void Reset(uint32_t id = 0);

  bool CheckTimedout();

  uint32_t GetId() const;
  Status GetStatus() const;

  char*             GetHeaderField(const string& fieldName) ;
  //DataBlock<char*>  GetHeaderField(const string& fieldName);

  RequestMethod     GetRequestMethod() const;
  std::string       GetUri() const;
  std::string       GetHost() const;
  bool              IsKeepAlive() const;
  bool              IsGzipSupported() const;

  http::ContentType GetContentType() const;
  DataBlock<char*>  GetBody() const;

  DataBlock<char*>  GetCookie(const string& cookieName);


  Status SetStatus(Status status);

  Status ReadRequest(int fd = -1, bool isSecureConnection = false, SSL* ssl = nullptr);

  bool Preprocess(); // #THINK: Should it Public func or Private func?


  static const size_t MAX_HEADER_SIZE;
  static const size_t MAX_FIELD_SIZE;
  static const size_t MAX_URI_SIZE;

protected:
  
private:
  Status status;
  datetime timestamp;
  uint32_t id;
  uint16_t freeSize;
  uint16_t usedSize;
  uint16_t headerSize;
  char* body; 
  char* secondBuffer;
  uint32_t secondBufferSize;
  uint32_t secondBufferUsedSize;
  // ==============================
  int16_t firstLineEndPos;
  HttpVersion httpVersion; 
  RequestMethod httpMethod;
  bool isGzipSupported;
  bool isKeepAliveSupported;
  char* uri;
  uint16_t uriLength;
  char* host;
  uint16_t hostLength;
  http::ContentType contentType;
  size_t contentLength;
  char* contentTypeValue;
  uint16_t contentTypeValueLength;
  //char* contentBoundary;
  //uint8_t contentBoundaryLength;

  char* userAgent;
  uint16_t userAgentLength;
  char* cookies;
  uint16_t cookiesLength;
  char* referer;
  uint16_t refererLength;
  char* acceptLanguage;
  uint16_t acceptLanguageLength;

  char buffer[8192];

  // =============================== 
  ssize_t       findHeaderSize();

  ssize_t       findContentLength();
  size_t        createSecondBuffer();
  bool          isClientExpectContinue();

  inline
  ssize_t       findRequestMethod();
  inline
  ssize_t       findUri(size_t methodEndPos);
  inline 
  HttpVersion   findHttpVersion();
  inline
  bool          findIfKeepAliveSupported();


  http::ContentType findContentType(char* fieldValueStart, size_t fieldValueLength);


  inline
  bool freeSecondBuffer();
  
};

}

#endif

