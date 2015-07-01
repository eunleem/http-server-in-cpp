#ifndef _LIVES_HPP_
#define _LIVES_HPP_
/*
  Name
    Lives

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 30, 2015
  
  History
    December 08, 2014
      Created
    Jun 20, 2015
      Complexity of DNA and Secret Code is Lowered for easier access.

  ToDos

  Milestones
    1.0
      Add Index by DNA
      
  Description
    

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
#include <fstream>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <vector>


#include <cerrno>

#include <openssl/sha.h>

#include "Table.hpp"

#include "liolib/Util.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {

typedef std::chrono::system_clock::time_point datetime;
typedef uint32_t lifeid_t;

class Life : public Row {
public:
  enum class Status : uint8_t {
    ACTIVE,
    SUSPENDED,
    DELETED
  };

  std::string ToString(const Life::Status status) const {
    switch (status) {
      case Status::ACTIVE: return "ACTIVE";
      case Status::SUSPENDED: return "SUSPENDED";
      case Status::DELETED: return "DELETED";
    } 
    DEBUG_cerr << "Invalid Status." << endl; 
    return "UNDEF";
  }


  enum class Group : uint8_t {
    ME,
    MY_FAMILY,
    FRIENDS,
    USERS
  };

  std::string ToString(const Life::Group val) const {
    switch (val) {
      case Group::ME: return "ME";
      case Group::MY_FAMILY: return "MY_FAMILY";
      case Group::FRIENDS: return "FRIENDS";
      case Group::USERS: return "USERS";
    } 
    DEBUG_cerr << "Invalid Group." << endl; 
    return "UNDEF";
  }

  static const int DNA_LENGTH;

  Life() : 
    id(0),
    status(Status::ACTIVE),
    group(Group::USERS)
  {
    std::memset(this->dna, 0, sizeof(this->dna));
    std::memset(this->salt, 0, sizeof(this->salt));
    this->GenerateDna();
    this->generateSalt();
  }

  inline
  std::string GetDna() const {
    return std::string(this->dna, strlen(dna));
  }

  inline
  std::string GetSalt() const {
    return std::string(this->salt, sizeof(this->salt));
  }

  inline
  char* GetHashedPtr() const {
    return (char*)this->hashed;
  }

  inline
  size_t GetHashedSize() const {
    return sizeof(this->hashed);
  }


  static
  const size_t GetRowSize() {
    const size_t size =
      sizeof(Life::id) +
      sizeof(Life::status) +
      sizeof(Life::group) +
      sizeof(Life::dna) +
      sizeof(Life::salt) +
      sizeof(Life::hashed) +
      sizeof(Life::created);
    return size;
  }

  void Print() const {
    DEBUG_cout << "Printing Life..." << endl; 
    DEBUG_cout << "  status: " << ToString(this->status) << endl; 
    DEBUG_cout << "  id: " << this->id << endl; 
    DEBUG_cout << "  group: " << ToString(this->group) << endl; 
    DEBUG_cout << "  dna: " << std::string(this->dna) << endl; 
    DEBUG_cout << "  salt: " << std::string(this->salt, sizeof(this->salt)) << endl; 
    DEBUG_cout << "  created: " << Util::Time::TimeToString(this->created) << endl; 
    DEBUG_cout << "END OF PRINTING LIFE!" << endl; 
  }

  bool SetHashed(std::string secret_code) {
    const std::string salt(this->salt, sizeof(salt));
    secret_code.append(salt);

    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    SHA512_Update(&ctx, secret_code.c_str(), secret_code.length());
    SHA512_Final(this->hashed, &ctx);

    return true;
  }

  bool GenerateDna() {
    const int RANDOM_CHAR_COMPLEXITY = 1; // UpperCase Alphabets only

    Util::String::RandomString((char*)this->dna, Life::DNA_LENGTH, RANDOM_CHAR_COMPLEXITY);
    return true;
  }

  lifeid_t id;
  Status status;
  Group group;
private:
  char dna[12];
  char salt[8];
  unsigned char hashed[SHA512_DIGEST_LENGTH];
public:
  datetime created;


protected:
  std::ostream& Serialize(std::ostream& os) const override {
    if (this->id == 0) {
      DEBUG_cerr << "Life Not Ready to be serialized. Id is 0." << endl; 
      throw::std::invalid_argument("Life Not Ready to be serialized. Id is 0.");
    } 
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->status, sizeof(this->status));
    os.write((char*)&this->group, sizeof(this->group));
    os.write((char*)this->dna, sizeof(this->dna));
    os.write((char*)this->salt, sizeof(this->salt));
    os.write((char*)this->hashed, sizeof(this->hashed));
    os.write((char*)&this->created, sizeof(this->created));
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->id, sizeof(this->id));
    is.read((char*)&this->status, sizeof(this->status));
    is.read((char*)&this->group, sizeof(this->group));
    is.read((char*)this->dna, sizeof(this->dna));
    is.read((char*)this->salt, sizeof(this->salt));
    is.read((char*)this->hashed, sizeof(this->hashed));
    is.read((char*)&this->created, sizeof(this->created));
    return is;
  }

private:
  bool generateSalt() {
    const int RANDOM_CHAR_COMPLEXITY = 4;  
    Util::String::RandomString(this->salt, sizeof(this->salt), RANDOM_CHAR_COMPLEXITY);
    return true;
  }
};

const int Life::DNA_LENGTH = 4;

class LivesSummary : public Summary<lifeid_t> {
public:
  LivesSummary(std::string dirPath) :
    Summary<lifeid_t>("lives.summary", dirPath)
  { }
};


class LivesIndexItemDna : public IndexItem<lifeid_t> {
public:
  char dna[12];

protected:
  std::ostream& Serialize(std::ostream& os) const override {
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->pos, sizeof(this->pos));
    os.write((char*)this->dna, sizeof(this->dna));
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->id, sizeof(this-id));
    is.read((char*)&this->pos, sizeof(this->pos));
    is.read((char*)this->dna, sizeof(this->dna));
    return is;
  }
};

class LivesIndexDna : public Index {
public:
  LivesIndexDna() :
    Index("./lives", "livesdna.idx")
  { }

};

class Lives : public Table {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  NOT_OPEN,
  LIFE_NOT_FOUND
};
#define LIVES_EXCEPTION_MESSAGES \
  "Lives Exception has been thrown.", \
  "Lives table is not open.", \
  "Life not found."

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

  Lives(std::string dirPath = "./data/lives/", std::string dataFileName = "lives.data");
  ~Lives();


  std::pair<const Life*, std::string> CreateLife();

  const std::unordered_map<lifeid_t, Life>& GetLives() const;

  const Life* GetLifeById(lifeid_t id);
  const Life* GetLifeByDnaAndSecretCode(std::string& dna, std::string secret_code) const;

protected:
  ssize_t       SaveAllToStorage() override;
  ssize_t       LoadAllFromStorage() override;

  bool          open() override;
  bool          close() override;

  bool          addLifeToFile(const Life& life);

  ssize_t       recover() override;

private:
  LivesSummary summary_;

  std::string directoryPath_;
  std::string dataFileName_;

  std::fstream dataFile_;


  std::unordered_map<lifeid_t, Life> lifeById_;
  std::unordered_map<std::string, Life*> lifeByDna_;
  
};

}

#endif

