#ifndef _TABLE_HPP_
#define _TABLE_HPP_
/*
  Name
    Table

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 22, 2015
  
  History
    April 27, 2015
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

#include <iostream>
#include <cstdio> // std::remove

#include "Summary.hpp"
#include "Index.hpp"

#include "liolib/Serializable.hpp"
#include "liolib/Openable.hpp"
#include "liolib/CustomExceptions.hpp"

//#include "liolib/DataBlock.hpp"
namespace lio {



class Row : public Serializable { };

class TableConfig {
public:
  TableConfig() :
    loadAllOnOpen(false)
  { }

  std::string tableDir;
  bool loadAllOnOpen;
};


class Table : public Openable {
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


  Table(TableConfig config = TableConfig()) :
    baseConfig_(config)
  { }

  virtual
  ~Table() {
    DEBUG_FUNC_START;
    if (this->status_ == Status::OPEN) {
      DEBUG_cerr << "FORGOT TO CLOSE TABLE!!!" << endl; 
    } 
  }

  virtual
  ssize_t LoadAllFromStorage() = 0;

  virtual
  ssize_t SaveAllToStorage() = 0;


  virtual
  ssize_t CreateBackup() {
    DEBUG_cerr << "Not yet implemented." << endl; 
    return -1;
  }

protected:

  virtual
  bool open() override {
    DEBUG_cout << "BASE TABLE open() has been called!" << endl; 
    if (this->baseConfig_.loadAllOnOpen == true) {
      const ssize_t loadedCount = this->LoadAllFromStorage();
      DEBUG_cout << "Loaded " << loadedCount << " row(s) on open." << endl; 
    } 
    return true;
  }

  virtual
  bool close() override {
    DEBUG_cout << "BASE TABLE close() has been called!" << endl; 
    //this->SaveAllToStorage();
    return true;
  }


  bool isLockFileExisting(const std::string& dirPath) {
    std::string lockFilePath = dirPath + "table.lock";
    return Util::File::IsFileExisting(lockFilePath);
  }


  bool createLockFile(const std::string& dirPath) {
    std::fstream lockFile;
    std::string lockFilePath = dirPath + "table.lock";
    lockFile.open(lockFilePath, std::ios::out);
    if (lockFile.is_open() == false) {
      DEBUG_cerr << "Failed to create a lock file." << endl; 
      return false;
    } 

    lockFile.close();
    DEBUG_cout << "Lock file has been created." << endl; 
    return true;
  }

  bool deleteLockFile(const std::string& dirPath) {
    std::string lockFilePath = dirPath + "table.lock";
    int result = std::remove(lockFilePath.c_str());
    if (result > 0) {
      DEBUG_cerr << "Could not delete lock file." << endl; 
      return false;
    } 

    DEBUG_cout << "Lock file has been deleted." << endl; 
    return true;
  }

  virtual
  ssize_t recover() {
    DEBUG_cerr << "Recovery function is not yet implemented." << endl; 
    return -1;
  }

  // Get rid of rows marked as deleted by rebuilding data file.
  // Index file is also rebuilt.
  virtual
  ssize_t cleanUp() {
    DEBUG_cerr << "Clean up function is not yet implemented." << endl; 
    return -1;
  }
  
  TableConfig baseConfig_;

private:
  
};

}

#endif

