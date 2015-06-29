#pragma once
/*
  Name
    DataFile

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

#include "liolib/Util.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio { namespace db {


class DataFile {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL, 
  FILE_NOT_OPEN
};
#define DATAFILE_EXCEPTION_MESSAGES \
  "DataFile Exception has been thrown.", \
  "DataFile is not open."

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


  DataFile();
  ~DataFile();


  bool Open();
  bool Close();

  std::fstream& GetFile();

  virtual
  std::streampos WriteRow() = 0;
  virtual
  std::streampos ReadRow() = 0;

protected:
  std::string filePath;
  std::fstream file;
  
private:

  
};

} }
