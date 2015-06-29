#ifndef _ROW_HPP_
#define _ROW_HPP_
/*
  Name
    Row

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    April 15, 2015
  
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


class Row {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define ROW_EXCEPTION_MESSAGES \
  "Row Exception has been thrown."

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


  Row();
  ~Row();
protected:
  
private:

/** FUNC_NAME 
 *    DESCRIPTION
 *  Input
 *    DESCRIPTION_ABOUT_PARAMETERS
 *  Returns
 *    void NOTHING
 */
  void sampleFunc();
  
};

}

#endif

