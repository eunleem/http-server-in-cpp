#ifndef _PAGE_HPP_
#define _PAGE_HPP_
/*
  Name
    Page

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 02, 2015
  
  History
    July 02, 2015
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


#include "HttpConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include "HttpPostData.hpp"

#include "ILioData.hpp"

#include "liolib/DataBlock.hpp"

namespace lio {


class Page {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  FAILED
};
#define PAGE_EXCEPTION_MESSAGES \
  "Page Exception has been thrown.", \
  "Failed."

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


  Page();
  ~Page();

  virtual
  bool Process(HttpConnection* connection, ILioData* data) = 0;
protected:
  
private:

  
};

}

#endif

