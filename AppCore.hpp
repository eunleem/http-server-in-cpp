#ifndef _APPCORE_HPP_
#define _APPCORE_HPP_
/*
  Name
    AppCore

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 30, 2015
  
  History
    November 18, 2014
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

#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <sys/ioctl.h>

#include "HttpRequestPool.hpp"
#include "HttpResponsePool.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include "HttpConnection.hpp"

#include "HttpPostData.hpp"


#include "ILioData.hpp"


#include "liolib/JsonString.hpp"
#include "liolib/DataBlock.hpp"

namespace lio {

using std::vector;

class AppCore {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define APPCORE_EXCEPTION_MESSAGES \
  "AppCore Exception has been thrown."

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

  AppCore();
  ~AppCore();

  bool Process(HttpConnection* connection);

  bool RemoveCache(const string& cacheName);

protected:
  bool ProcessLocalhost(HttpConnection*);


  //bool RequireLogin(HttpConnection*);

  //bool NotFound(HttpConnection*);
  //bool ServerError(HttpConnection*);
  //
  bool securityCheck(HttpConnection* connection);

  const char* test_page = "This is Test Page.";
  const char* not_found = "Page Not Found";
  const char* server_error = "Server Error";
  //const char* not_authorized = "Login Required";
  
  
  const char* code_m4 = "{\"code\": -4}";
  const char* code_m3 = "{\"code\": -3}";
  const char* code_m2 = "{\"code\": -2}";
  const char* code_m1 = "{\"code\": -1}";
  const char* code_0 = "{\"code\": 0}";
  
private:
  uint32_t lastWorkId;
  int currentIndex;

  //std::map<string, HttpResponse*> cachedResponses_;


  ILioData data;


  std::map<string, DataBlock<>> fileCache_;
  bool CreateFileResponse(HttpConnection* connection, const string& filePath);
  DataBlock<> loadFile(const string& filePath);
  size_t clearFileCache();

  inline
  string getFileExt(const string& filaName);

  
};

}

#endif

