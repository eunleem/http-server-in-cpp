#ifndef _INDEX_HPP_
#define _INDEX_HPP_
/*
  Name
    Index

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 15, 2015
  
  History
    May 29, 2015
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


#include "liolib/Serializable.hpp"
#include "liolib/Openable.hpp"
//#include "liolib/DataBlock.hpp"

namespace lio {

template<typename ID_T>
class IndexItem : public Serializable {
public:
  IndexItem() :
    id(0),
    pos(0)
  {}
  IndexItem(ID_T id, std::streampos pos) :
    id(id),
    pos(pos)
  { }
  ID_T id;
  std::streampos pos;
protected:
  std::ostream& Serialize(std::ostream& os) const override {
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->pos, sizeof(this->pos));
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->id, sizeof(this-id));
    is.read((char*)&this->pos, sizeof(this->pos));
    return is;
  }
};

class Index : public Openable {
public:
// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  INDEX_ITEM_NOT_FOUND
};
#define INDEX_EXCEPTION_MESSAGES \
  "Index Exception has been thrown.", \
  "IndexItem is not found."

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

  Index(std::string name, std::string dirPath) :
    dirPath_(dirPath),
    indexName_(name)
  { }
  virtual
  ~Index() {}

  virtual
  ssize_t SaveAllToStorage() = 0;

  virtual
  ssize_t LoadAllFromStorage() = 0;

protected:
  virtual
  bool open() {
    const std::string filePath = this->dirPath_ + this->indexName_;
    this->indexFile_.open(this->indexName_,
        std::ios::in | std::ios::out | std::ios::binary);
    if (this->indexFile_.is_open() == false) {
      DEBUG_cerr << "Could not open index file." << endl; 
      return false;
    } 

    const ssize_t loadedCount = this->LoadAllFromStorage();
    DEBUG_cout << "Loaded " << loadedCount << " index items on open." << endl; 
    return loadedCount >= 0;

  }
  virtual
  bool close() {
    if (this->status_ == Index::Status::CLOSE) {
      DEBUG_cout << "File already closed." << endl; 
      return true;
    } 

    const ssize_t savedCount = this->SaveAllToStorage();
    DEBUG_cout << "Saved " << savedCount << " index item(s) on close." << endl; 
    return savedCount >= 0;

    this->indexFile_.flush();
    this->indexFile_.sync();
    this->indexFile_.close();
    return true;
  }
  
  std::string dirPath_;
  std::string indexName_;
  
  std::fstream indexFile_;
private:
};

// ===== Exception Implementation ===== 
const char* const
Index::Exception::exceptionMessages_[] = {
  INDEX_EXCEPTION_MESSAGES
};
#undef INDEX_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.

Index::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Index::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Index::ExceptionType
Index::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


}

#endif

