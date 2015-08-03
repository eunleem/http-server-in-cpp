#ifndef _IDEAS_HPP_
#define _IDEAS_HPP_
/*
  Name
    Ideas

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Aug 03, 2015
  
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
    PLAN,   // plan vs goal...
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
      //sizeof(Idea::relatedId) +
      sizeof(Idea::type) +
      sizeof(Idea::perm) +
      sizeof(Idea::created) +
      sizeof(Idea::modified) +
      sizeof(Idea::contentId) +
      Idea::MAX_TITLE_LENGTH;
  }

  static
  const size_t MAX_TITLE_LENGTH;

  static
  const size_t GetTitleSize() {
    DEBUG_cerr << "GetTitleSize() is Deprecated. Use GetMaxTitleLength() instead." << endl;
    return Idea::MAX_TITLE_LENGTH;
  }

  static
  const size_t GetMaxTitleLength() {
    return Idea::MAX_TITLE_LENGTH;
  }

  ideaid_t GetId() const;
  datetime GetCreatedTime() const;
  datetime GetModifiedTime() const;
  Type GetType() const;
  Status GetStatus() const;
  Permission GetPermission() const;
  const std::string& GetTitle() const;
  const char* GetTitleChar() const;
  uint32_t GetContentId() const;

  std::streampos GetPos() const;
  bool IsDeleted() const;
  bool IsSynced() const;

  void Print() const {
    DEBUG_cout << "IDEA" << endl; 
    DEBUG_cout << "id: " << this->id << endl; 
    DEBUG_cout << "lifeid: " << this->lifeId << endl; 
    //DEBUG_cout << "relatedId: " << this->relatedId << endl; 
    DEBUG_cout << "type: " << (int) this->type << endl; 
    DEBUG_cout << "perm: " << (int) this->perm << endl; 
    DEBUG_cout << "created: " << Util::Time::TimeToString(this->created) << endl; 
    DEBUG_cout << "modified: " << Util::Time::TimeToString(this->modified) << endl; 
    DEBUG_cout << "contentId: " << this->contentId << endl; 
    DEBUG_cout << "title: " << this->title << endl; 
    DEBUG_cout << "isSynced: " << std::boolalpha << this->isSynced << std::noboolalpha << endl; 
  }


  virtual
  ssize_t SyncTo(std::fstream& file) override { 
    if (this->isSynced == true) {
      DEBUG_cout << "Already Synced! good!" << endl; 
      return 0;
    }
    if (this->pos == -1) {
      DEBUG_cout << "POS is -1. Meaning it's a new Idea and needs to be added at the end." << endl; 
      file.seekp(0, std::ios::end);
      this->pos = file.tellp();
    } else {
      file.seekp(this->pos, std::ios::beg);
    }
    file.write((char*)&this->id, sizeof(this->id));
    file.write((char*)&this->lifeId, sizeof(this->lifeId));
    file.write((char*)&this->status, sizeof(this->status));
    //file.write((char*)&this->relatedId, sizeof(this->relatedId));
    file.write((char*)&this->type, sizeof(this->type));
    file.write((char*)&this->perm, sizeof(this->perm));
    file.write((char*)&this->created, sizeof(this->created));
    file.write((char*)&this->modified, sizeof(this->modified));
    file.write((char*)&this->contentId, sizeof(this->contentId));

    const size_t orgTitleSize = this->title.length();
    this->title.resize(Idea::MAX_TITLE_LENGTH);
    file.write(this->title.c_str(), this->title.length());
    this->title.resize(orgTitleSize);

    this->isSynced = true;
    return 0;
  }

  virtual
  ssize_t SyncFrom(std::fstream& file) override { 
    if (this->pos != -1) {
      DEBUG_cerr << "WARN: SyncFrom file when it already has pos." << endl;
    }
    this->pos = file.tellg();
    file.read((char*)&this->id, sizeof(this->id));
    file.read((char*)&this->lifeId, sizeof(this->lifeId));
    file.read((char*)&this->status, sizeof(this->status));
    //file.read((char*)&this->relatedId, sizeof(this->relatedId));
    file.read((char*)&this->type, sizeof(this->type));
    file.read((char*)&this->perm, sizeof(this->perm));
    file.read((char*)&this->created, sizeof(this->created));
    file.read((char*)&this->modified, sizeof(this->modified));
    file.read((char*)&this->contentId, sizeof(this->contentId));

    this->title.resize(Idea::MAX_TITLE_LENGTH);
    file.read((char*)this->title.c_str(), Idea::MAX_TITLE_LENGTH);
    this->isSynced = true;

    return 0;
  }

  void SetId(ideaid_t id);
  void SetLifeId(lifeid_t lifeid);
  void SetStatus(Status status);
  void SetPermission(Permission& permission);
  void SetType(Type type);
  bool SetTitle(std::string& title);
  void SetContentId(uint32_t contentId);


  std::ostream& Serialize(std::ostream& os) const override {
    static_assert(true, "DEPRECATED");
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    static_assert(true, "DEPRECATED");
    return is;
  }
protected:

private:
  ideaid_t id;
  lifeid_t lifeId;
  Status status;
  Type type;
  Permission perm;
  datetime created;
  datetime modified;
  uint32_t contentId;
  std::string title;

  std::streampos pos;
  bool isSynced;

};


class IdeasIndex {
public:
  IdeasIndex(const std::string& dirPath,
             const std::string& dataFileName) 
    : dirPath_(dirPath),
      dataFileName_(dataFileName)
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

    std::string dataFilePath = this->dirPath_ + this->dataFileName_;
    
    bool isExisting = Util::File::IsFileExisting(dataFilePath, false); // Do not create file automatically
    if (isExisting == false) {
      DEBUG_cout << "File does not exist. Nothing to load." << endl;
      return 0;
    }

    std::fstream dataFile;
    dataFile.open(this->dirPath_ + this->dataFileName_,
                  std::ios::in | std::ios::binary);
    if (dataFile.is_open() == false) {
      DEBUG_cerr << "Could not open data file. Path: "
                 << this->dirPath_ + this->dataFileName_ << endl;
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
      loadedIdea.SyncFrom(dataFile);
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

  std::string dirPath_;
  std::string dataFileName_;

  std::map<ideaid_t, IndexItem<ideaid_t>> items;
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


  Ideas(const std::string& dirPath = "./data/ideas/",
        const std::string& dataFileName = "ideas.data");

  virtual
  ~Ideas();

  size_t GetNumIdeas() const {
    return this->index_.items.size();
  }

  Idea* GetIdeaById(const ideaid_t ideaId);
  std::vector<Idea*> GetIdeas(size_t count, size_t skip = 0);

  ideaid_t AddIdea(lifeid_t lifeId, std::string& title,
                   uint32_t contentId = 0,
                   Idea::Permission permission = Idea::Permission::PRIVATE,
                   Idea::Type type = Idea::Type::GENERAL);
  // DON'T ADD UPDATE IDEA. Can be done by updating referenced Idea object.
  bool SyncIdea(const ideaid_t id);
  bool SyncIdeas();

  //bool SyncIndex();

protected:
  bool open() override;
  bool close() override;

  ssize_t LoadAllFromStorage() override;
  ssize_t SaveAllToStorage() override;

  Idea* GetIdeaByIndexItem(const IndexItem<ideaid_t>& indexItem);

  
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

