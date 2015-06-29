#include "AppCore.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
AppCore::Exception::exceptionMessages_[] = {
  APPCORE_EXCEPTION_MESSAGES
};
#undef APPCORE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


AppCore::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
AppCore::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const AppCore::ExceptionType
AppCore::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


AppCore::AppCore()
{
  DEBUG_FUNC_START; // Prints out function name in yellow
  //this->lives_.Open();

}

AppCore::~AppCore() {
  DEBUG_FUNC_START;

  this->clearFileCache();
}

bool AppCore::RemoveCache(const string& cacheName) {
  const auto it = this->fileCache_.find(cacheName);
  if (it == this->fileCache_.end()) {
    DEBUG_cout << "Failed to remove cache. Cache does not exist. Name: " << cacheName << endl; 
    return false;
  } 

  delete[] static_cast<char*>(it->second.GetObject());
  this->fileCache_.erase(it);
  
  return true;
}

bool AppCore::securityCheck(HttpConnection* connection) {
  // Do something here later.
  return true;
}

bool AppCore::Process(HttpConnection* connection) {
  DEBUG_FUNC_START;

  bool isProcessed = false;

  connection->AcquireReusableResponse();

  bool isSafe = securityCheck(connection);
  if (isSafe == true) {
    HttpRequest* request = connection->GetRequest();

    string host = request->GetHost();
    Util::String::ToLowerFly(host);

    if (host.substr(0, strlen("localhost")) == "localhost" ||
               host == "lifeino.com:8080") {
      DEBUG_cout << "LOCALHOST" << endl; 
      isProcessed = this->ProcessLocalhost(connection);
    }
  } 

  if (isProcessed == false) {
    HttpResponse* response = connection->GetResponse();
    response->SetResponseCode(http::ResponseCode::NOT_FOUND);
    response->SetBody(this->not_found, strlen(this->not_found),
        http::ContentType::PLAINTEXT);
    return true;
  } 

  return isProcessed;
}


bool AppCore::ProcessLocalhost(HttpConnection* connection) {
  HttpRequest* request = connection->GetRequest();
  string uri = request->GetUri();

  DEBUG_cout << "Uri: " << uri << endl; 

  if (uri == "/") {
    uri = "/index.html";
  } 

  if (uri.rfind(".") != string::npos) {
    // #TODO Temporary security code.
    if (uri == "/admin.html") {
      return false;
    } 

    // File Request.
    bool fileFound = this->CreateFileResponse(connection, "./lifeino" + uri);
    if (fileFound == false) {
      // maybe we can do something like this, but unnecessary for now.
      //this->CreateFileResponse(connection, "./lifeino/not_found.html");
      return false;
    } 
    return true;

  } else {
    if (request->GetRequestMethod() == http::RequestMethod::GET) {

    } else if (request->GetRequestMethod() == http::RequestMethod::POST) {

    } else {
      DEBUG_cerr << "Unsupported RequestMethod!" << (int) request->GetRequestMethod() << endl; 

      connection->GetResponse()->SetResponseCode(http::ResponseCode::METHOD_NOT_ALLOWED);
      connection->GetResponse()->SetBody(this->server_error, strlen(this->server_error),
          http::ContentType::PLAINTEXT);
      return true;
    }
    return false;
  }

  return true;
}

#if 0
bool AppCore::CacheResponse(string cacheName, HttpResponse* response) {
  DEBUG_cerr << "WARNING: Dont use it for now." << endl; 

  bool result = response->Cache();
  if (result == true) {
    this->cachedResponses_[cacheName] = response;
  } 

  return result;
}
#endif


bool AppCore::CreateFileResponse(HttpConnection* connection, const string& filePath) {

  DataBlock<> file;

  auto it = this->fileCache_.find(filePath);
  if (it == this->fileCache_.end()) {
    // NOT CACHED
    file = this->loadFile(filePath);
    if (file.IsNull() == true) {
      // FILE DOESNT EXIST
      DEBUG_cout << "File does not exist." << endl; 
      return false;
    } else {
      DEBUG_cout << "File Cached. Name: " << filePath << endl; 
      this->fileCache_[filePath] = file;
    }

  } else {
    file = it->second;

  }

  http::ContentType contentType = http::ContentType::UNDEF;
  string fileExt = this->getFileExt(filePath);
  DEBUG_cout << "FileExt: " << fileExt << endl;
  if (fileExt == "jpg") {
    contentType = http::ContentType::IMG_ICON;
  } else if (fileExt == "gif") {
    contentType = http::ContentType::IMG_GIF;
  } else if (fileExt == "png") {
    contentType = http::ContentType::IMG_PNG;
  } else if (fileExt == "ico") {
    contentType = http::ContentType::IMG_ICON;
  } else if (fileExt == "js") {
    contentType = http::ContentType::JAVASCRIPT;
  } else if (fileExt == "css") {
    contentType = http::ContentType::CSS;
  } else if (fileExt == "html") {
    contentType = http::ContentType::HTML;
  } else if (fileExt == "zip") {
    contentType = http::ContentType::ZIP;
  } else if (fileExt == "xml") {
    contentType = http::ContentType::XML;
  } else if (fileExt == "mp3") {
    contentType = http::ContentType::AUDIO_MP3;
  } else {
    DEBUG_cerr << "Invalid file extension." <<
      " ext: " << fileExt <<
      " fileName: " << filePath << endl; 
    return false;
  }

  HttpResponse* response = connection->GetResponse();
  response->SetResponseCode(http::ResponseCode::OK);
  response->SetBody((char*)file.GetObject(), file.GetLength(), contentType);

  return true;
}

DataBlock<> AppCore::loadFile (const string& filePath) {
  if (filePath[0] == '/') {
    DEBUG_cerr << "filePath starts with slash (/) which is dangerous!" << endl; 
    return DataBlock<>(); // Null DataBlock
  } 

  if (filePath.find("..") != string::npos) {
    DEBUG_cerr << "filePath contains two dots (..) which is dangerous!" << endl; 
    return DataBlock<>(); // Null DataBlock
  } 

  std::ifstream fileStream;

  fileStream.open(filePath.c_str());
  if(fileStream.is_open()) {
    DEBUG_cout << "File Opened!" << endl; 
    fileStream.seekg(0, std::ios::end);
    size_t fileSize = fileStream.tellg();
    
    DEBUG_cout << "FileSize: " << fileSize << endl; 

    char* memblock = new char[fileSize];

    fileStream.seekg(0, std::ios::beg);
    fileStream.read(memblock, fileSize);
    DEBUG_cout << "File Loaded to memory." << endl; 
    fileStream.close();

    return DataBlock<>(memblock, 0, fileSize);

  } else {
    DEBUG_cerr << "Could not open file. Errno: " << errno << endl; 
  }

  return DataBlock<>(); // Null DataBlock
}

size_t AppCore::clearFileCache() {
  auto clearedItems = this->fileCache_.size();
  for (auto& cache : this->fileCache_) {
    delete[] (char*)cache.second.GetObject();
  } 

  this->fileCache_.clear();

  return clearedItems;
}

string AppCore::getFileExt(const string& fileName) {
  size_t extStartLocation = fileName.rfind(".");
  if (extStartLocation == string::npos) {
    return "";
  }
  
  string fileExt = fileName.substr(extStartLocation + 1);
  if (fileExt.length() > 4) {
    DEBUG_cerr << "File Ext is too long." << endl;
    return "";
  }
  Util::String::ToLowerFly(fileExt);
  return fileExt;
}


//AppCore::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockAppCore : public AppCore {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(AppCore, TESTNAME) {
  MockAppCore mockAppCore;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

