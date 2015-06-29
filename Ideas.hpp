#ifndef _IDEAS_HPP_
#define _IDEAS_HPP_
/*
  Name
    Ideas

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 05, 2015
  
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

using std::string;
using std::chrono::system_clock;

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

  ideaid_t GetId() const;
  datetime GetCreatedTime() const;
  Type GetType() const;
  Status GetStatus() const;
  Permission GetPermission() const;
  std::string GetTitle() const;
  const char* GetTitleChar() const;
  uint32_t GetContentId() const;
  bool IsDeleted() const;
  bool IsSynced() const;

  void Print() const {
    DEBUG_cout << "IDEA" << endl; 
    DEBUG_cout << "id: " << this->id << endl; 
    DEBUG_cout << "lifeid: " << this->lifeId << endl; 
    DEBUG_cout << "type: " << (int) this->type << endl; 
    DEBUG_cout << "perm: " << (int) this->perm << endl; 
    DEBUG_cout << "created: " << Util::TimeToString(this->created) << endl; 
    DEBUG_cout << "title: " << this->title << endl; 
    DEBUG_cout << "contentId: " << this->contentId << endl; 
    DEBUG_cout << "isSynced: " << std::boolalpha << this->isSynced << std::noboolalpha << endl; 
  }

  void SetId(ideaid_t id);
  void SetLifeId(lifeid_t lifeid);
  void SetStatus(Status status);
  void SetPermission(Permission& permission);
  void SetType(Type type);
  bool SetTitle(const string& title);
  void SetContentId(uint32_t contentId);
  void SetIsSynced(bool isSynced);

  //std::streampos WriteIdeaToFile(std::fstream& file);
  //ideaid_t ReadIdeaFromFile(std::fstream& file);

  std::ostream& Serialize(std::ostream& os) const override {
    if (this->isSynced == true) {
      DEBUG_cout << "WARN: writing synced data..." << endl; 
    } 
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->lifeid, sizeof(this->lifeid));
    os.write((char*)&this->status, sizeof(this->status));
    os.write((char*)&this->relatedid, sizeof(this->relatedid));
    os.write((char*)&this->type, sizeof(this->type));
    os.write((char*)&this->perm, sizeof(this->perm));
    os.write((char*)&this->created, sizeof(this->created));
    os.write((char*)this->title, sizeof(this->title));
    os.write((char*)&this->contentId, sizeof(this->contentId));
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->id, sizeof(this->id));
    is.read((char*)&this->lifeid, sizeof(this->lifeid));
    is.read((char*)&this->status, sizeof(this->status));
    is.read((char*)&this->relatedid, sizeof(this->relatedid));
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
  ideaid_t relatedid;
  Type type;
  Permission perm;
  datetime created;
  char title[200];
  uint32_t contentId;
  bool isSynced;
};





class IdeasSummary : public Summary<ideaid_t> {
public:
  IdeasSummary() :
    Summary<ideaid_t>("ideas.summary", "./data/ideas/")
  { }
};




class IdeasIndex : public Index {
public:

  IdeasIndex() :
    Index("ideas.idx", "./data/ideas")
  { }

  IndexItem<ideaid_t>& GetIndexItemById(ideaid_t id) {
    auto it = this->items.find(id);
    if (it == this->items.end()) {
      // not found
      DEBUG_cerr << "Item not found." << endl; 
      throw Exception(ExceptionType::INDEX_ITEM_NOT_FOUND);
    } 

    return it->second;
  }

  bool AddIndex(Idea& idea, std::streampos pos) {
    IndexItem<ideaid_t> newItem;
    newItem.id = idea.GetId();
    newItem.pos = pos;
    //newItem.isSynced = false;

    this->items[newItem.id] = newItem;
    return true;
  }

  ssize_t LoadAllFromStorage() override {

    this->indexFile_.seekg(0, std::ios::beg);

    for (uint64_t i = 0; this->header.numItems > i; i++) {
      IndexItem<ideaid_t> item;
      file.read((char*)&item, sizeof(item));
      //DEBUG_cout << "ReadItem: " << item.id << " " << item.contentSize << " " << item.pos << endl; 
      this->items[item.id] = item;
    } 

    return true;

  }

  ssize_t SaveAllToStorage() override {

    this->indexFile_.seekp(0, std::ios::beg);

    for (auto& item : this->items) {
      //item.second.isSynced = true;
      this->indexFile_.write((char*)&item.second, sizeof(item.second));
    } 

    file.flush();
    DEBUG_cout << std::to_string(this->items.size()) << " index items have been written." << endl; 

    return true;
  }


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


  Ideas(const std::string& dataFileName = "ideas.data",
        const std::string& dirPath = "./data/ideas/");

  virtual
  ~Ideas();

  Idea* GetIdeaById(ideaid_t ideaId);
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
  bool SyncIdea(ideaid_t id);
  bool SyncIdeas();

  bool SyncIndex();

protected:
  bool open() override;
  bool close() override;

  ssize_t LoadAllFromStorage() override;
  ssize_t SaveAllToStorage() override;
  
private:
  IdeasSummary summary_;
  IdeasIndex index_;

  std::string dataFilePath_;

  std::fstream dataFile_;

  std::unordered_map<ideaid_t, Idea> ideas_;
  std::unordered_map<lifeid_t, Idea*> ideasByLifeId_;
  
};

}

#endif

