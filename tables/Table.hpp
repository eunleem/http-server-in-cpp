#ifndef _TABLE_HPP_
#define _TABLE_HPP_
/*
  Name
    Table

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
#include <map>
#include <fstream>


#include "DataFile.hpp"
#include "Index.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio { namespace db {


struct TableConfig {
  std::string directoryPath;

};

class Table {
public:

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

  enum class Status : uint8_t {
    CLOSE,
    OPEN
  };


  Table();
  virtual
  ~Table();

  bool open();
  bool close();

protected:

  bool saveToStorage();
  bool loadFromStorage();


  Status status;

  std::string directoryPath;
  std::map<std::string, DataFile> files;

  bool checkDirectory();
  bool checkFiles();

  bool openFiles();
  bool closeFiles();
  bool openIndex();
  bool closeIndex();
  
private:


  
};

}
}

#endif

