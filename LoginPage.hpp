#ifndef _LOGINPAGE_HPP_
#define _LOGINPAGE_HPP_
/*
  Name
    LoginPage

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 07, 2015
  
  History
    July 02, 2015
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

#include "Page.hpp"

#include "HttpPostData.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {


class LoginPage : public Page {
public:


  LoginPage();
  ~LoginPage();

  virtual
  bool Process(HttpConnection* connection, ILioData* data) override;
protected:
  
private:

  const char* already_logged_in = "{\"code\": 1, \"message\": \"Already Logged In.\"}";
  const char* successful_login = "{\"code\": 0}";
  const char* login_failed = "{\"code\": -1, \"message\": \"Could not find matching Information.\"}";
  const char* error = "{\"code\": -2}";

  
};

}

#endif

