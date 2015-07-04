#include "Sessions.hpp"

#define _UNIT_TEST false

namespace lio {

// ===== Exception Implementation ===== 
const char* const
Sessions::Exception::exceptionMessages_[] = {
  SESSIONS_EXCEPTION_MESSAGES
};
#undef SESSIONS_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Sessions::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Sessions::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Sessions::ExceptionType
Sessions::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Sessions::Sessions(Config config) : 
  config_(config)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
  std::srand(std::time(0)); 

  this->baseConfig_.loadAllOnOpen = true;
}

Sessions::~Sessions() {
}

Session& Sessions::GetSession(const std::string& code) {
  auto it = this->sessionsByCode_.find(code);
  if (it == this->sessionsByCode_.end()) {
    // Not Found
    DEBUG_cout << "Session not found." << endl; 
    throw Sessions::Exception(ExceptionType::NOT_FOUNT);
  } 

  if (it->second.expiration < std::chrono::steady_clock::now()) {
    DEBUG_cout << "Session expired!" << endl; 
    this->RemoveSession(code);
    throw Sessions::Exception(ExceptionType::EXPIRED);
  } 

  return it->second;
}

std::string Sessions::AddSession(Session sessionData) {
  std::string code = this->generateNotDuplicatingCode();

  this->sessionsByCode_[code] = sessionData; 
  DEBUG_cout << "code: " << code << endl; 

  return code;
}

std::string Sessions::generateNotDuplicatingCode() {
  std::string code(16, 0);

  while (true) {
    code = Util::String::RandomString(16);
    if (this->sessionsByCode_.find(code) == this->sessionsByCode_.end()) {
      break;
    } 
  } 

  return code;
}

#if 0
bool Sessions::RemoveSession(const lifeid_t& lifeid) {
  string code;
  for (auto& item : this->sessionsByCode_) {
    if (item.second.lifeid == lifeid) {
      code = item.first;
      break;
    } 
  } 

  if (code.empty() == true) {
    DEBUG_cerr << "Could not find session by lifeid." << endl; 
    return false;
  } 

  return this->RemoveSession(code);
}
#endif

bool Sessions::RemoveSession(const std::string& code) {
  auto it = this->sessionsByCode_.find(code);
  if (it == this->sessionsByCode_.end()) {
    DEBUG_cerr << "Could not find session by Id." << endl; 
    return false;
  }
  this->sessionsByCode_.erase(it);
  DEBUG_cout << "Successfully removed session." << endl; 
  return true;
}

ssize_t Sessions::SaveAllToStorage() {
  DEBUG_FUNC_START;

  std::fstream file;

  // Deletes the file overwrites the file.
  file.open(this->config_.GetFilePath(), std::ios::out | std::ios::binary);
  if (file.is_open() == false) {
    DEBUG_cerr << "Could not open file." << endl; 
    return -1;
  } 

  ssize_t count = 0;
  for (auto& session : this->sessionsByCode_) {
    const std::string& sessionKey = session.first;
    const Session& sessionData = session.second;

    if (sessionData.expiration < std::chrono::steady_clock::now()) {
      // Skip expired ones
      DEBUG_cout << "Skipping session because it has expired." << endl; 
      continue;
    } 

    Util::File::WriteString<uint8_t>(file, sessionKey);
    file.write((char*)&sessionData.lifeid, sizeof(sessionData.lifeid));
    file.write((char*)&sessionData.expiration, sizeof(sessionData.expiration));
    count += 1;
  } 
  file.flush();
  file.close();

  DEBUG_cout << "Saved " << count << " sessions." << endl; 

  return count;
}

ssize_t Sessions::LoadAllFromStorage() {
  DEBUG_FUNC_START;
  bool isExistent = Util::File::IsFileExisting(this->config_.GetFilePath());
  if (isExistent == false) {
    DEBUG_cout << "File does not exist." << endl; 
    return -1;
  } 

  std::fstream file;
  file.open(this->config_.GetFilePath(), std::ios::in | std::ios::binary);
  if (file.is_open() == false) {
    DEBUG_cerr << "Could not open file." << endl; 
    return -1;
  } 

  ssize_t count = 0;
  while (file.good() == true) {

    std::string key;
    ssize_t readCount = Util::File::ReadString<uint8_t>(file, key);
    if (readCount == 0) {
      break;
    } 
    Session sessionData;
    file.read((char*)&sessionData.lifeid, sizeof(sessionData.lifeid));
    file.read((char*)&sessionData.expiration, sizeof(sessionData.expiration));

    if (sessionData.expiration < std::chrono::steady_clock::now()) {
      // Skip expired ones
      DEBUG_cout << "Skipping session because it has expired." << endl; 
      continue;
    } 

    DEBUG_cout << "Loaded code: " << key << endl; 

    this->sessionsByCode_[key] = sessionData;
    count += 1;
  } 

  file.close();

  DEBUG_cout << "There were " << count << " sessions in file." << endl; 
  DEBUG_cout << "Restored " << this->sessionsByCode_.size() << " sessions into memory." << endl; 

  return count;
}

bool Sessions::open() {
  bool isDirExistent = Util::File::IsDirectoryExisting(this->config_.GetDirPath());
  if (isDirExistent == false) {
    DEBUG_cerr << "Directory does not exist. dir: " << this->config_.GetDirPath() << endl; 
    return false;
  } 

  ssize_t numLoaded = this->LoadAllFromStorage();
  if (numLoaded == -1) {
    return false;
  } 
  return true;
}

bool Sessions::close() {
  this->SaveAllToStorage();
  return true;
}

}

#if _UNIT_TEST

#include <chrono>
#include <string>

#include "Sessions.hpp"
#include "Lives.hpp"

using namespace lio;


int main() {

  std::string sid;

    Sessions::Config config("./testdata/sessions/", "sessions.data");
  {
    Sessions session(config);

    auto time = std::chrono::system_clock::now();

    sid = session.AddSession(1, time + std::chrono::minutes(30));
    session.AddSession(2, time + std::chrono::minutes(10));
    session.AddSession(3, time + std::chrono::minutes(10));
    session.AddSession(4, time + std::chrono::minutes(0));
    session.AddSession(5, time + std::chrono::minutes(1));
  }


  {
    Sessions session(config);
    lifeid_t lifeId = session.GetSession(sid);
    DEBUG_cout << "LifeId: " << lifeId << endl; 
  }


  return 0;
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

