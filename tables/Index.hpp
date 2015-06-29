#ifndef _INDEX_HPP_
#define _INDEX_HPP_
/*
  Name
    Index

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

#include <string>
#include <fstream>

#include "DataFile.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio { namespace db {


template<typename KEY_T>
class BasicIndexItem {
  KEY_T id;
  std::streampos pos;
};

template<class IndexItem>
class Index {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define INDEX_EXCEPTION_MESSAGES \
  "Index Exception has been thrown."

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


  Index();
  ~Index();

  bool Open();
  bool Close();



protected:
  
private:

  
};

} }

#endif

