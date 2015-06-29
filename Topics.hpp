#ifndef _TOPICS_HPP_
#define _TOPICS_HPP_
/*
  Name
    Topics

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 27, 2015
  
  History
    June 13, 2014
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

#include <map>
#include <list>
#include <vector>

#include <chrono>

#include "liolib/Util.hpp"
#include "liolib/DataBlock.hpp"

namespace lio {

using std::string;

using std::fstream;

using std::vector;
using std::list;
using std::map;

using std::chrono::system_clock;

class Topics {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  NOT_FOUND
};
#define TOPICS_EXCEPTION_MESSAGES \
  "Topics Exception has been thrown.", \
  "Topic not found."

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


  enum class Status {
    INIT,
    OPEN,
    CLOSE
  };

  struct Topic {
    uint32_t id;
    string name;
    string description;
    system_clock::time_point created;
  };

  Topics();
  ~Topics();



  const Topic& GetTopicById(uint32_t id);

  const Topic& CreateNew(string name, string description);
protected:
  
private:

  struct Summary {
    Summary() :
      lastId(0)
    { }
    uint32_t lastId;
  };

  Status status_;

  Summary summary_;

  map<uint32_t, Topic> topicById;
  
};

}

#endif

