#ifndef _PLANNERPAGE_HPP_
#define _PLANNERPAGE_HPP_
/*
  Name
    PlannerPage

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 31, 2015
  
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
#include "include/rapidjson/document.h"

#include "Page.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {


class PlannerPage : public Page {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define PLANNERPAGE_EXCEPTION_MESSAGES \
  "PlannerPage Exception has been thrown."

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


  PlannerPage();
  ~PlannerPage();

  
  virtual
  bool Process(HttpConnection* connection, ILioData* data) override;


protected:
  
private:

  //std::string jsonifyIdea(ideaid_t id);
  
  const char* successful = "{\"code\": 0}";
  const char* invalid = "{\"code\": -1}";
  const char* notloggedin = "{\"code\": -2}";
  const char* nomore = "{\"code\": -3}";
  
};

}

#endif

