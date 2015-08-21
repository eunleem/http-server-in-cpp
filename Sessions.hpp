#ifndef _SESSIONS_HPP_
#define _SESSIONS_HPP_
/*
  Name
    Sessions

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Aug 20, 2015
  
  History
    December 12, 2014
      Created

  ToDos
    Add a function to erase expired sessions
    


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

#include <chrono>
#include <unordered_map>
#include <fstream>
#include <string>

#include <cstdlib>
#include <ctime>

#include "Table.hpp"
#include "Lives.hpp"

#include "liolib/Util.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


typedef std::chrono::system_clock::time_point datetime;
typedef std::chrono::steady_clock::time_point steadytime;

using std::string;

class Session {
public:
  Session() :
    lifeid(0),
    expiration(std::chrono::steady_clock::now() + std::chrono::minutes(30))
  {}

  Session(lifeid_t id,
          steadytime expiration =
            std::chrono::steady_clock::now() + std::chrono::minutes(30)) :
      lifeid(id),
      expiration(expiration)
  {
    if (expiration < std::chrono::steady_clock::now()) {
      DEBUG_cerr << "Expiration cannot be later than NOW." << endl; 
    } 
  }

  Session(lifeid_t id,
          std::chrono::minutes expireAfter = std::chrono::minutes(30)) :
      lifeid(id),
      expiration(std::chrono::steady_clock::now() + expireAfter)
  {
    if (expiration < std::chrono::steady_clock::now()) {
      DEBUG_cerr << "Expiration cannot be later than NOW." << endl; 
    } 
  }

  Session(lifeid_t id,
          std::chrono::hours expireAfter = std::chrono::hours(1)) :
      lifeid(id),
      expiration(std::chrono::steady_clock::now() + expireAfter)
  {
    if (expiration < std::chrono::steady_clock::now()) {
      DEBUG_cerr << "Expiration cannot be later than NOW." << endl; 
    } 
  }

  lifeid_t lifeid;
  steadytime expiration;
};

class Sessions : public Table {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  NOT_FOUNT,
  EXPIRED
};
#define SESSIONS_EXCEPTION_MESSAGES \
  "Sessions Exception has been thrown.", \
  "Session Not Found.", \
  "Session has expired."

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
    Config(std::string dirPath = "./data/sessions/", std::string fileName = "sessions.data") :
      dirPath(dirPath),
      fileName(fileName)
    {
      if (dirPath.back() != '/') {
        dirPath.push_back('/');
      } 
      if (fileName.front() == '/') {
        fileName = fileName.substr(1);
      } 
      if (fileName.back() == '/') {
        fileName.pop_back();
      } 
    }

    std::string GetFilePath() const {
      return this->dirPath + fileName;
    }

    std::string GetDirPath() const {
      return this->dirPath;
    }

    std::string GetFileName() const {
      return this->fileName;
    }
      
  private:
    std::string dirPath;
    std::string fileName;
  };

  Sessions(Config config = Config());
  ~Sessions();

  Session&        GetSession(const std::string& code);

  std::string     AddSession(Session session);

  bool            RemoveSession(const std::string& code);

protected:
  ssize_t         SaveAllToStorage() override;
  ssize_t         LoadAllFromStorage() override;

  bool            open() override;
  bool            close() override;

  
private:
  std::string     generateNotDuplicatingCode();

  Config config_;
  std::unordered_map<std::string, Session> sessionsByCode_;
};

}

#endif
