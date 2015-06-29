#ifndef _INVITATIONS_HPP_
#define _INVITATIONS_HPP_
/*
  Name
    Invitations

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 29, 2015
  
  History
    June 12, 2014
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
#include <unordered_map>
#include <list>
#include <vector>

#include <chrono>

#include "Table.hpp"

#include "liolib/Util.hpp"
#include "liolib/DataBlock.hpp"
#include "liolib/CustomExceptions.hpp"

namespace lio {

using std::chrono::system_clock;
using std::chrono::steady_clock;

typedef std::chrono::system_clock::time_point datetime;
typedef std::chrono::steady_clock::time_point steadytime;

typedef uint32_t invitid_t;




class Invitation : public Row {
public:
  Invitation() :
    id(0),
    topicId(0),
    numTickets(0),
    numRemaining(0)
  {
    memset(this->code, 0, sizeof(this->code));
  }
  invitid_t       id;
  std::streampos  pos;
  uint32_t        topicId;
  char            code[12];
  uint16_t        numTickets;
  uint16_t        numRemaining;
  datetime        created;
  datetime        expiration;

  void Print() const {
    DEBUG_cout << "Printing Invitation Row..." << endl; 
    DEBUG_cout << "  id: " << id << endl; 
    DEBUG_cout << "  pos: " << pos << endl; 
    DEBUG_cout << "  topicId: " << topicId << endl; 
    DEBUG_cout << "  code: " << std::string(this->code, sizeof(code)) << endl; 
    DEBUG_cout << "  numTickets: " << (int)this->numTickets << endl; 
    DEBUG_cout << "  numRemaining: " << (int)this->numRemaining << endl; 
    DEBUG_cout << "  cretead: " << Util::Time::TimeToString(this->created) << endl; 
    DEBUG_cout << "  expiration: " << Util::Time::TimeToString(this->expiration) << endl; 
  }
protected:
  std::ostream& Serialize(std::ostream& os) const override {
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->pos, sizeof(this->pos));
    os.write((char*)&this->topicId, sizeof(this->topicId));
    os.write((char*)this->code, sizeof(this->code));
    os.write((char*)&this->numTickets, sizeof(this->numTickets));
    os.write((char*)&this->numRemaining, sizeof(this->numRemaining));
    os.write((char*)&this->created, sizeof(this->created));
    os.write((char*)&this->expiration, sizeof(this->expiration));

    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->id, sizeof(this->id));
    is.read((char*)&this->pos, sizeof(this->pos));
    is.read((char*)&this->topicId, sizeof(this->topicId));
    is.read((char*)this->code, sizeof(this->code));
    is.read((char*)&this->numTickets, sizeof(this->numTickets));
    is.read((char*)&this->numRemaining, sizeof(this->numRemaining));
    is.read((char*)&this->created, sizeof(this->created));
    is.read((char*)&this->expiration, sizeof(this->expiration));

    return is;
  }
};

class InvitationsSummary : public Summary<invitid_t> {
public:
  InvitationsSummary(const std::string& dirPath = "./data/invitations/",
      const std::string& fileName = "invitations.summary") :
    Summary<invitid_t>(fileName, dirPath)
  {
    DEBUG_cout << "SummaryFilePath: " << dirPath + fileName << endl; 
  }
};

class Invitations : public Table {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  CREATE,
  UPDATE,
  GETINVIT,
  INVIT_OUT,
  INVIT_EXP
};
#define INVITATIONS_EXCEPTION_MESSAGES \
  "Invitations Exception has been thrown.", \
  "Failed to create invitation.", \
  "Failed to update invitation.", \
  "Failed to Get Invitiation by Id or code.", \
  "All the tickets have been used.", \
  "Invitation has been expired."

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

  struct Config { 
    Config(
        std::string dirPath = "./data/invitations/",
        std::string dataFileName = "invitations.data") : 
      dirPath(dirPath),
      dataFileName(dataFileName)
    {
      if (this->dirPath.back() != '/') {
        this->dirPath.push_back('/');
      } 
    }

    std::string GetDataFilePath() const {
      return this->dirPath + this->dataFileName;
    }
    std::string dirPath;
    std::string dataFileName;
  };

  Invitations(Config config = Config());
  ~Invitations();
  
  const std::unordered_map<uint32_t, Invitation>& GetInvitations() const;

  Invitation& GetInvitationById(const uint32_t id);
  Invitation& GetInvitationByCode(const std::string& code);

  Invitation& CreateNew(
      const uint32_t topicId,
      //const std::string& description,
      uint16_t numTickets,
      datetime expiration);

  size_t RedeemTicket(const std::string& code);

  bool Delete(const uint32_t id);

protected:
  bool        open() override;
  bool        close() override;

  ssize_t     LoadAllFromStorage() override;
  ssize_t     SaveAllToStorage() override;

  bool        addInvitationToFile(Invitation& invitation);

private:
  Config config_;
  InvitationsSummary summary_;

  std::fstream dataFile_;

  std::unordered_map<invitid_t, Invitation> invitationById;
  std::map<std::string, Invitation*> invitationByCode;

  
};

}

#endif
