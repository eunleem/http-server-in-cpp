#ifndef _HTTPRESPONSEPOOL_HPP_
#define _HTTPRESPONSEPOOL_HPP_
/*
  Name
    HttpResponsePool

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 06, 2015
  
  History
    April 06, 2015
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

#include "HttpResponse.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


class HttpResponsePool {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  ACQUIRE_RESPONSE
};
#define HTTPRESPONSEPOOL_EXCEPTION_MESSAGES \
  "HttpResponsePool Exception has been thrown.", \
  "Failed to acquire response."

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
  static HttpResponsePool& GetInstance();
  
  HttpResponse& AcquireReusable();
  bool ReleaseReusable(HttpResponse& obj);

  int SetMaxPoolSize(size_t sizeMax);

protected:
  HttpResponsePool();
  ~HttpResponsePool();
  
  std::vector<HttpResponse> responses_;

private:
  static HttpResponsePool* instance;
  static size_t currentIndex;
};

}

#endif

