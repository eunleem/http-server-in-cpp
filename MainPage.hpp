#ifndef _MAINPAGE_HPP_
#define _MAINPAGE_HPP_
/*
  Name
    MainPage

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 08, 2015
  
  History
    July 08, 2015
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


class MainPage : public Page {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define MAINPAGE_EXCEPTION_MESSAGES \
  "MainPage Exception has been thrown."

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


  MainPage();
  ~MainPage();

  virtual
  bool Process(HttpConnection* connection, ILioData* data) override;

protected:
  
private:
  
};

}

#endif

