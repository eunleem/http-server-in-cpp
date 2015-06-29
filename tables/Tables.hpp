#ifndef _TABLES_HPP_
#define _TABLES_HPP_
/*
  Name
    Tables

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 15, 2015
  
  History
    April 15, 2015
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


//#include "liolib/DataBlock.hpp"

namespace lio {


class Table {
public:

  enum class Status : uint8_t {
    CLOSE,
    OPEN
  };

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define TABLE_EXCEPTION_MESSAGES \
  "Table Exception has been thrown."

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


  Table();
  virtual
  ~Table();

  virtual bool open() = 0;
  virtual bool close() = 0;

protected:
  
private:


  
};

}

#endif

