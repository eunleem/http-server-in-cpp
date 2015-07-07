#ifndef _INVITATIONPAGE_HPP_
#define _INVITATIONPAGE_HPP_
/*
  Name
    InvitationPage

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 05, 2015
  
  History
    July 05, 2015
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


class InvitationPage : public Page {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define INVITATIONPAGE_EXCEPTION_MESSAGES \
  "InvitationPage Exception has been thrown."

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


  InvitationPage();
  ~InvitationPage();

  
  virtual
  bool Process(HttpConnection* connection, ILioData* data) override;

protected:
  
private:
  
  const char* successful = "{\"code\": 0}";
  const char* invalid = "{\"code\": -1}";
  const char* expired = "{\"code\": -2}";
  const char* nomore = "{\"code\": -3}";
  
};

}

#endif

