#pragma once

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false


#include "liolib/Debug.hpp"
#include "liolib/Consts.hpp" // Default Constant Values

#include <iostream> 
#include <string> 
#include <fstream> //ofstream
#include <chrono>

#include <ctime> // time_t time() gmtime() strftime() struct tm
#include <cstring> // memset();

#include <sys/stat.h> //mkdir()
#include <sys/types.h> //mkdir()

#include "liolib/Util.hpp"

namespace lio { 

using std::string;
using std::chrono::system_clock;
//using std::chrono::system_clock::time_point;
//
using std::ofstream;
using std::ios;

using consts::OK;
using consts::ERROR;


// #TODO: Logger Module deals with paths of Directories and Files. All the Paths need to be checked so security is not compromised. e.g. ../../../ attach "Allow *"
class Logger {
public:
// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  FILE_OPEN_FAILED,
  INVALID_CONFIG
};
#define LOGGER_EXCEPTION_MESSAGES \
  "Logger Exception has been thrown.", \
  "Failed to create or open File for Logger.", \
  "Invalid Configuration value has been entered."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;

  virtual const               ExceptionType type() const noexcept;
  
private:
  ExceptionType      exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********

  struct Log {
    string      body;
    system_clock::time_point  timestamp;
  };

  struct Config {
    Config(string logName, string logDirPath = ".",
           size_t logMaxSize = 0, size_t logMaxNumLogs = 0) :
      logName(logName),
      logDirPath(logDirPath),
      logMaxSize(logMaxSize),
      logMaxNumLogs(logMaxNumLogs)
    {}
    string logName;
    string logDirPath;
    size_t logMaxSize; // 0 means not set and is unlimited. in KB
    size_t logMaxNumLogs; // 0 means not set and is unlimited.
  };

  static
  bool SetConfig(Logger::Config config);
  static
  Logger& GetInstance();


  //int Log(LogType logType, string logTitle, string logMessage);
  virtual
  int    LogEvent(const string& logMessage);
  virtual
  int    LogError(const string& logMessage);
  //void   ReadFromLogFile(const string logPath);
  //
  std::ostream& LogEventStream() {
    return this->logFile_;
  }
  
 protected:
  Logger();
  virtual
  ~Logger();

  //int       setDeveloperEmail (string developerEmail);
  //int        setTimestampFormat (string format);
  
  
  /* #ADD: To reduce number of requests to the disk, it will use memory as a buffer.
      To prevent any logs getting lost due to failure of the process or the server,
      it will also send logs over Network to dedicted LogServer (ManagerServer)
      
      Hmm Maybe just need to create another class named "NetworkLogger" and inherit this class?
      Nah... One Module is good?
      Nah... Let's make it inherited to add functionalities 
      
      
  int       sendToLogServer();
  
  */
  
  
  
private:

  static
  Logger*   instance;

  static
  Config      config;
  //string      m_developerEmail;
  
  std::ofstream logFile_;


  int       setLogDirectoryPath (string& logDirectoryPath);
  int       setLogName (string& logName);

  bool      openLogFile();

  int       writeToLogFile (const Log& log);
  int       writeToLogFile (const string& rawString);

  //int notifyDeveloper(LogType logType, string logMessage);
  
};

}
