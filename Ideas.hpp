#ifndef _IDEAS_HPP_
#define _IDEAS_HPP_
/*
  Name
    Ideas

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 21, 2015
  
  History
    September 23, 2014
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
#include <vector>
#include <list>

#include <chrono>

#include "Table.hpp"
#include "Lives.hpp"

#include "liolib/Util.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {

typedef std::chrono::system_clock::time_point datetime;

typedef uint32_t ideaid_t;

class Idea : public Row {
public:
  enum class Status : uint8_t {
    ACTIVE,
    DELETED,
    ARCHIVED
  };

  enum class Type : uint8_t {
    GENERAL, // Unspecified
    JOURNAL,
    PLAN, // plan vs goal...
    ACTION, // ToDo or Task
    IDEA,
    INFO,
    COMMENT
  };

  enum class Permission : uint8_t {
    PRIVATE,
    TEAM_VIEW_ONLY,
    TEAM_COMMENTABLE,
    PUBLIC_VIEW_ONLY,
    PUBLIC_COMMENTABLE
  };

  Idea();
  ~Idea();

  static
  size_t GetRowSize() {
    return 
      sizeof(Idea::id) +
      sizeof(Idea::lifeId) +
      sizeof(Idea::status) +
      sizeof(Idea::relatedId) +
      sizeof(Idea::type) +
      sizeof(Idea::perm) +
      sizeof(Idea::created) +
      sizeof(Idea::title) +
      sizeof(Idea::contentId);
  }

  ideaid_t GetId() const;
  datetime GetCreatedTime() const;
  Type GetType() const;
  Status GetStatus() const;
  Permission GetPermission() const;
  std::string GetTitle() const;
  const char* GetTitleChar() const;
  uint32_t GetContentId() const;

  std::streampos GetPos() const;
  bool IsDeleted() const;
  bool IsSynced() const;

  void Print() const {
    DEBUG_cout << "IDEA" << endl; 
    DEBUG_cout << "id: " << this->id << endl; 
    DEBUG_cout << "lifeid: " << this->lifeId << endl; 
    DEBUG_cout << "relatedId: " << this->relatedId << endl; 
    DEBUG_cout << "type: " << (int) this->type << endl; 
    DEBUG_cout << "perm: " << (int) this->perm << endl; 
    DEBUG_cout << "created: " << Util::Time::TimeToString(this->created) << endl; 
    DEBUG_cout << "title: " << this->title << endl; 
    DEBUG_cout << "contentId: " << this->contentId << endl; 
    DEBUG_cout << "isSynced: " << std::boolalpha << this->isSynced << std::noboolalpha << endl; 
  }


  bool Sync(std::fstream& file) { 
    file << this;
    return true;
  }

  void SetId(ideaid_t id);
  void SetLifeId(lifeid_t lifeid);
  void SetStatus(Status status);
  void SetPermission(Permission& permission);
  void SetType(Type type);
  bool SetTitle(const string& title);
  void SetContentId(uint32_t contentId);


  std::ostream& Serialize(std::ostream& os) const override {
    if (this->isSynced == true) {
      DEBUG_cout << "WARN: writing synced data..." << endl; 
    } 
    if (this->pos == -1) {
      DEBUG_cout << "POS is -1. Meaning it's a new Idea and needs to be added at the end." << endl; 
      os.seekp(0, std::ios::end);
      this->pos = os.tellp();
    } else {
      os.seekp(this->pos, std::ios::beg);
    }
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->lifeId, sizeof(this->lifeId));
    os.write((char*)&this->status, sizeof(this->status));
    os.write((char*)&this->relatedId, sizeof(this->relatedId));
    os.write((char*)&this->type, sizeof(this->type));
    os.write((char*)&this->perm, sizeof(this->perm));
    os.write((char*)&this->created, sizeof(this->created));
    os.write((char*)this->title, sizeof(this->title));
    os.write((char*)&this->contentId, sizeof(this->contentId));
    this->isSynced = true;
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    this->pos = is.tellg();
    is.read((char*)&this->id, sizeof(this->id));
    is.read((char*)&this->lifeId, sizeof(this->lifeId));
    is.read((char*)&this->status, sizeof(this->status));
    is.read((char*)&this->relatedId, sizeof(this->relatedId));
    is.read((char*)&this->type, sizeof(this->type));
    is.read((char*)&this->perm, sizeof(this->perm));
    is.read((char*)&this->created, sizeof(this->created));
    is.read((char*)this->title, sizeof(this->title));
    is.read((char*)&this->contentId, sizeof(this->contentId));
    this->isSynced = true;
    return is;
  }
protected:

private:
  ideaid_t id;
  lifeid_t lifeId;
  Status status;
  ideaid_t relatedId;
  Type type;
  Permission perm;
  datetime created;
  char title[200];
  uint32_t contentId;

  mutable std::streampos pos;
  mutable bool isSynced;

};





class IdeasSummary : public Summary<ideaid_t> {
public:
  IdeasSummary() :
    Summary<ideaid_t>("ideas.summary", "./data/ideas/")
  { }
};




class IdeasIndex {
public:
  IdeasIndex(const std::string& dataFileName,
             const std::string& dirPath) 
    : dataFileName_(dataFileName),
      dirPath_(dirPath)
  {
    if (this->dirPath_.back() != '/') {
      this->dirPath_.push_back('/');
    } 
  }

  IndexItem<ideaid_t>& GetIndexItemById(ideaid_t id) {
    auto itr = this->items.find(id);
    if (itr == this->items.end()) {
      // not found
      DEBUG_cerr << "Item not found." << endl; 
      throw std::exception();
    } 

    return itr->second;
  }

  bool AddIndex(Idea& idea, std::streampos pos) {
    IndexItem<ideaid_t> newItem;
    newItem.id = idea.GetId();
    newItem.pos = pos;
    //newItem.isSynced = false;

    this->items[newItem.id] = newItem;
    return true;
  }

  ssize_t LoadAllFromStorage() {

    std::fstream dataFile;
    dataFile.open(this->dirPath_ + this->dataFileName_, std::ios::in | std::ios::binary);
    if (dataFile.is_open() == false) {
      DEBUG_cerr << "Could not open data file. Path: " << this->dirPath_ + this->dataFileName_ << endl; 
      return -1;
    } 

    const ssize_t fileSize = Util::File::GetSize(dataFile);
    const ssize_t rowSize = Idea::GetRowSize();
    
    if (fileSize % rowSize != 0) {
      DEBUG_cerr << "FileSize % rowSize != 0 !!! Datafile might be corrupted!!!" << endl; 
    } 

    const size_t numRows = fileSize / rowSize;
    DEBUG_cout << "numRows: " << numRows << endl; 

    dataFile.seekg(0, std::ios::beg);
    for (size_t i = 0; numRows > i; ++i) {
      IndexItem<ideaid_t> item;
      Idea loadedIdea;

      item.pos = dataFile.tellg();
      dataFile >> loadedIdea;
      item.id = loadedIdea.GetId();

      DEBUG_cout << "ReadItem: " << item.id << " " << " " << item.pos << endl; 
      
      this->items[item.id] = item;
    } 

    return true;
  }

  ssize_t SaveAllToStorage() {
    DEBUG_cout << "Nothing" << endl; 

    return true;
  }


  std::map<ideaid_t, IndexItem<ideaid_t>> items;

  std::string dataFileName_;
  std::string dirPath_;
};

class Ideas : public Table {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  INDEX_ITEM_NOT_FOUND
};
#define IDEAS_EXCEPTION_MESSAGES \
  "Ideas Exception has been thrown.", \
  "Index Item Not Found."

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


  Ideas(const std::string& dataFileName = "ideas.data",
        const std::string& dirPath = "./data/ideas/");

  virtual
  ~Ideas();

  Idea* GetIdeaById(const ideaid_t ideaId);
  std::vector<Idea*> GetIdeas(size_t from, size_t count);
  std::vector<Idea*> GetLastIdeas(size_t count);
  //vector<Idea*> GetIdeasByLifeId(lifeid_t lifeId, size_t from, size_t count);
  ideaid_t AddIdea(
      lifeid_t lifeId,
      const std::string& title,
      uint32_t contentId = 0,
      Idea::Permission permission = Idea::Permission::PRIVATE,
      Idea::Type type = Idea::Type::GENERAL
    );
  // DON'T ADD UPDATE IDEA. Can be done by updating referenced Idea object.
  bool SyncIdea(const ideaid_t id);
  bool SyncIdeas();

  //bool SyncIndex();

protected:
  bool open() override;
  bool close() override;

  ssize_t LoadAllFromStorage() override;
  ssize_t SaveAllToStorage() override;

  
private:
  std::string dirPath_;
  std::string dataFileName_;

  std::string dataFilePath_;

  //IdeasSummary summary_;
  IdeasIndex index_;




  std::fstream dataFile_;

  std::unordered_map<ideaid_t, Idea> ideas_;
  //std::unordered_map<lifeid_t, Idea*> ideasByLifeId_;
  
};

}

#endif

