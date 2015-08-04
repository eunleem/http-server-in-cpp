#ifndef _HTTPPOSTDATA_HPP_
#define _HTTPPOSTDATA_HPP_
/*
  Name
    HttpPostData

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Aug 04, 2015
  
  History
    January 01, 2015
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

#include <unordered_map>

#include "liolib/DataBlock.hpp"
#include "liolib/Util.hpp"

namespace lio {


class HttpPostData {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPPOSTDATA_EXCEPTION_MESSAGES \
  "HttpPostData Exception has been thrown."

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


  HttpPostData(DataBlock<char*> data);
  HttpPostData(char* data, size_t length);
  ~HttpPostData();

  DataBlock<char*> GetData(const std::string& fieldName);
protected:
  
private:
  const DataBlock<char*> content_;
  //std::unordered_map<std::string, DataBlock<char*>> cached_;
};

}

#endif

