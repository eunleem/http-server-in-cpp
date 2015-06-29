#ifndef _JOURNAL_HPP_
#define _JOURNAL_HPP_
/*
  Name
    Journal

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 11, 2015
  
  History
    April 27, 2015
      Created

  ToDos
    


  Milestones
    1.0
      Write JOURNAL file.
    2.0
      Make it an independent process that runs on its own and receive journals via socket.
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <fstream>
#include <string>


//#include "liolib/DataBlock.hpp"

namespace lio {


class Transaction {

  enum class Table : uint8_t {
    LIVES,
    IDEAS,
    CONTENTS,
    INVITATIONS

  };
  enum class Type : uint8_t {
    INSERT,
    UPDATE,
    DELETE
  };

  Table table;
  Type type;
  uint32_t dataSize;
  unsigned char* data;

};

class Journal {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define JOURNAL_EXCEPTION_MESSAGES \
  "Journal Exception has been thrown."

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

class Config {
public:
  Config() :
    dirPath("./journal/"),
    journalName("lio.journal")
  { }
  ~Config() { }

private:
  std::string dirPath;
  std::string journalName;
};


  Journal(Config config = Config());
  ~Journal();

  ssize_t Record(Transaction transaction);
protected:
  
private:
  Config config_;

  std::fstream journalFile_;

  
};

}

#endif

