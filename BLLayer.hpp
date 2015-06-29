#pragma once
/*
  Name
    BLLayer

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    May 28, 2015
  
  History
    April 26, 2015
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
#include <string>


#include "Sessions.hpp"

#include "Lives.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {


class BLLayer {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define BLLAYER_EXCEPTION_MESSAGES \
  "BLLayer Exception has been thrown."

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


enum class Status : uint8_t {
  CLOSED,
  OPEN,
};


class Config {
  public:
    Config(std::string dirPath = "./data/") :
      dirPath(dirPath)
  {
    if (this->dirPath.back() != '/') {
      this->dirPath.push_back('/');
    } 
  }
  private:
    std::string dirPath;
};

/*
 * Responsible for Opening and Closing related TABLES.
 * Provide simpler interface for Users.
 * */
  BLLayer(Config config = Config());
  ~BLLayer();

  bool Open();
  bool Close();

  bool GetTopicByTopicId();


  uint32_t GetInvitationByCode(const std::string& invitationCode);

  // Sign up
  // 1. Call RedeemTicket from Invitation Class
  // 2. Create a new LIFE from Lives class
  // 3. Return LIFE ID upon successful signup.
  std::pair<lifeid_t, std::string>
    SignUpByInvitationCode(const std::string& invitationCode);


  Life* Login(const std::string& dna, const std::string& key);

  bool IsLoggedIn(const std::string& sid);

  Life& GetLife(const lifeid_t lifeid) const;
protected:
  
private:
  Status status_;
  Config config_;

  // Journal journal;

  Sessions sessions_;

  Lives lives_;

  
};

}
