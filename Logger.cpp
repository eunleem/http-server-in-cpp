#include "Logger.hpp"


#define _UNIT_TEST false
#include "liolib/Test.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
Logger::Exception::exceptionMessages_[] = {
  LOGGER_EXCEPTION_MESSAGES
};
#undef LOGGER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Logger::Exception::Exception(Logger::ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Logger::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Logger::ExceptionType
Logger::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


Logger* Logger::instance = nullptr;
Logger::Config Logger::config = Logger::Config("logs");

Logger& Logger::GetInstance() {
  if (instance == nullptr) {
    instance = new Logger();
  } else {
    DEBUG_cout << "Getting Existing Logger Instance." << endl; 
  }

  return (*instance);
}


bool Logger::SetConfig(Logger::Config config) {
  config = config;
  return true;
}

Logger::Logger() {
  DEBUG_FUNC_START;

  this->setLogDirectoryPath(config.logDirPath);
  this->setLogName(config.logName);
  
  // Time to Formatted String
  //    http://www.cplusplus.com/reference/ctime/strftime/
  //m_timestampFormat = "%F %T %Z";
  
  openLogFile();
}

Logger::~Logger() {
  DEBUG_FUNC_START;

  if (this->logFile_.is_open()) {
    DEBUG_cout << "Closed Log file. " << config.logName << endl; 
    this->logFile_.close();
  } 
}

int Logger::LogEvent(const string& logBody) {
  Logger::Log log;

  log.body = logBody;
  log.timestamp = system_clock::now();
  
  return this->writeToLogFile(log);
}

int Logger::LogError(const string& logBody) {
  Logger::Log log;

  log.body = "ERR " + logBody;
  log.timestamp = system_clock::now();
  
  return this->writeToLogFile(log);
}

bool Logger::openLogFile() {
  // Open file for writing only for append op.
  string fullPath = config.logDirPath + config.logName;
  DEBUG_cout << "Log path: " << fullPath << endl;
  this->logFile_.open(fullPath.c_str(), ios::out | ios::app | ios::binary);
  if (this->logFile_.is_open() == false) {
    DEBUG_cerr << "Could not create or open log file." << endl; 
    throw Exception(ExceptionType::FILE_OPEN_FAILED);
  } 

  return true;
}

int Logger::writeToLogFile (const Log& log) {
 
  if (this->logFile_.is_open() == false) {
    this->openLogFile();
  } 

  string time = Util::Time::TimeToString(log.timestamp);
  this->logFile_ << time << " " << log.body << endl;
  
  return OK;
}


int Logger::setLogDirectoryPath (string& logDirectoryPath) {

  int result = ERROR;
  
  if (logDirectoryPath.empty()) {
    DEBUG_cerr << "Invalid Configuration Value." << endl; 
    throw Exception(ExceptionType::INVALID_CONFIG);
  } 

  if (logDirectoryPath.back() != '/') {
    logDirectoryPath.append("/");
  } 
  
  if (Util::IsDirectoryExisting(logDirectoryPath) == false) {
    result = mkdir(logDirectoryPath.c_str(), 0770);
    if (result != OK) {
      // Creating Directory Failed.
      DEBUG_cerr << "Creating " << logDirectoryPath << " has failed..." << endl;
      throw Exception(ExceptionType::INVALID_CONFIG);
    }
  }

  config.logDirPath = logDirectoryPath;

  return OK;
}

int Logger::setLogName (string& logName) {
  if (logName.empty()) {
    DEBUG_cerr << "Log name cannot be empty." << endl; 
    throw Exception(ExceptionType::INVALID_CONFIG);
  }

  // #THINK: I don't really know why I check for last char here lol
  //  I don't think it's necessary here.
  if (logName.back() == '/') {
    // ERROR
    DEBUG_cerr << "Log name cannot end with a slash." << endl; 
    throw Exception(ExceptionType::INVALID_CONFIG);
  }

  if (logName.find("..") != string::npos) {
    DEBUG_cerr << "Log name cannot contain two-dots." << endl; 
    throw Exception(ExceptionType::INVALID_CONFIG);
  }

 
  string date = Util::Time::Timestamp("%F");
  config.logName = logName + "-" + date + ".log";
  return OK;
}
/*
int Logger::setDeveloperEmail (string developerEmail) {
  //#TODO: Check Email format.
  m_developerEmail = developerEmail;
  return 0;
}
*/

/*
void Logger::ReadFromLogFile(const string logPath) {
  using std::ifstream;
  using std::ios;

  ifstream logFile;
  logFile.open(logPath.c_str(), ios::in | ios::binary);
  if(logFile.is_open()) {
    //logFile.seekg(0, std::ios::end);
    //size_t fileSize = logFile.tellg();
    //logFile.seekg(0, std::ios::beg);

    string line;
    while ( std::getline(logFile, line) ) {
      Log* log = (Log *) line.c_str();
      switch (log->type) {
       case Logger::LogType::ERROR:
        DEBUG_cerr << "Log Type Error ";
        break;
       default:
        DEBUG_cerr << "Log Type Unknown ";
        break;
      }
      DEBUG_cout << log->body << endl;
    }
    
    logFile.close();

  }
}
*/

}

#if _UNIT_TEST

#include <iostream>
#include <string>

using namespace lio;

int main () {
  Logger::Config config("testlog.txt");
  Logger::SetConfig(config);
  Logger& logger = Logger::GetInstance();
  logger.LogEvent("Hello");

  return 0;
}



#endif
