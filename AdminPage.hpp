#ifndef _ADMINPAGE_HPP_
#define _ADMINPAGE_HPP_
/*
  Name
    AdminPage

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 07, 2015
  
  History
    July 07, 2015
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

#include "Page.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {

class AdminPage : public Page {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define ADMINPAGE_EXCEPTION_MESSAGES \
  "AdminPage Exception has been thrown."

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


  AdminPage();
  ~AdminPage();

  virtual
  bool Process(HttpConnection* connection, ILioData* data) override;

  const char* successful = "{\"code\": 0}";
  const char* invalid = "{\"code\": -1}";
  const char* invalid_action = "{\"code\": -1, \"message\":\"Invalid action\"}";
  const char* invalid_query = "{\"code\": -1, \"message\":\"Invalid query\"}";
protected:
  
private:

  
};

}

#endif
