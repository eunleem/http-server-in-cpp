#ifndef _HTTPREQUESTPOOL_HPP_
#define _HTTPREQUESTPOOL_HPP_
/*
  Name
    HttpRequestPool

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

#include <vector>


#include "HttpRequest.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


class HttpRequestPool {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  ACQUIRE_REQUEST
};
#define HTTPREQUESTPOOL_EXCEPTION_MESSAGES \
  "HttpRequestPool Exception has been thrown.", \
  "Failed to acquire reusable request."

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

  static HttpRequestPool& GetInstance();

  HttpRequest& AcquireReusable();
  bool ReleaseReusable(HttpRequest& request);

  int SetMaxPoolSize(size_t sizeMax);

protected:
  HttpRequestPool();
  ~HttpRequestPool();

  std::vector<HttpRequest> requests_;

  
private:
  static HttpRequestPool* instance;
  static size_t currentIndex;

  
};

}

#endif

