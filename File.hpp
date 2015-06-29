#ifndef _FILE_HPP_
#define _FILE_HPP_
/*
  Name
    File

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 30, 2015
  
  History
    April 30, 2015
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

//#include "liolib/DataBlock.hpp"

namespace lio {


class File {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define FILE_EXCEPTION_MESSAGES \
  "File Exception has been thrown."

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

  enum class Status : uint8_t {
    CLOSE,
    OPEN
  };

  File(std::string filePath);
  ~File();

  bool OpenFile(std::ios_base::openmode openFlags = std::ios::in | std::ios::out) {
    if (this->status_ == Status::OPEN) {
      DEBUG_cout << "Already Open." << endl; 
      return true;
    } 

    this->file_.open(this->filePath_, openFlags);
    if (this->file_.is_open() == false) {
      DEBUG_cerr << "Could not open file." << endl; 
      return false;
    } 
    DEBUG_cout << "Opened file " << this->filePath << endl; 
    return true;
  }

  bool CloseFile();

  bool Rename(std::string);

protected:
  
private:

  Status status_;

  std::string filePath_;
  std::fstream file_;

  
};

}

#endif

