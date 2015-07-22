#ifndef _CONTENTS_HPP_
#define _CONTENTS_HPP_
/*
  Name
    Contents

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 22, 2015
  
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
#include <vector>
#include <list>

#include <chrono>

#include "Table.hpp"
#include "Index.hpp"
#include "Lives.hpp"
#include "Ideas.hpp"

#include "liolib/Util.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


typedef std::chrono::system_clock::time_point datetime;

typedef uint32_t contentid_t;







class Content : public Row {
public:
  enum class Status : uint8_t {
    ACTIVE,
    DELETED,
    ARCHIVED,
    UPDATED
  };

  enum class Type : uint8_t {
    GENERAL, // Unspecified
    JOURNAL,
    ACTION,
    IDEA,
    INFO
  };

  Content();
  ~Content();

  contentid_t GetId() const;
  contentid_t GetPrevId() const;
  datetime GetCreatedTime() const;
  Type GetType() const;
  const std::string& GetContent() const;
  bool IsDeleted() const;
  bool IsSynced() const;

  std::streampos GetPos() const;

  inline void Print() const {
    DEBUG_cout << "Content" << endl; 
    DEBUG_cout << "id: " << this->id << endl; 
    DEBUG_cout << "status: " << (int) this->status << endl; 
    DEBUG_cout << "type: " << (int) this->type << endl; 
    DEBUG_cout << "created: " << Util::Time::ToString(this->created) << endl; 
    DEBUG_cout << "content: " << this->content.substr(0, 100) << endl; 
    DEBUG_cout << "isSynced: " << std::boolalpha << this->isSynced << std::noboolalpha << endl; 
  }

  void SetId(contentid_t id);
  void SetPrevId(contentid_t prevId);
  void SetStatus(Status status);
  void SetType(Type type);
  bool SetContent(const std::string& content);

  std::ostream& Serialize(std::ostream& os) const override;
  std::istream& Deserialize(std::istream& is) override;

private:
  contentid_t id;
  contentid_t prevId;
  Status status;
  Type type;
  datetime created;
  std::string content;

// In-Memory only fields
  mutable std::streampos pos;
  mutable bool isSynced;
};










class ContentsIndex : public Index {
public:
  ContentsIndex(const std::string& dirPath,
                const std::string& indexFileName,
                const std::string& dataFileName);

  contentid_t GetLastId() const;

  IndexItem<contentid_t>* GetIndexItemById(contentid_t id);

  bool AddIndex(Content& content, std::streampos pos);

  ssize_t Rebuild(const std::string& dataFilePath);

  virtual
  ssize_t LoadAllFromStorage() override;
  virtual
  ssize_t SaveAllToStorage() override;

protected:
  std::string dataFileName_;

  std::map<contentid_t, IndexItem<contentid_t>> items;

private:
};










class Contents : public Table {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define CONTENTS_EXCEPTION_MESSAGES \
  "Contents Exception has been thrown."

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


  Contents(const std::string& dirPath,
           const std::string& dataFileName = "contents.data");




  const Content*                GetContentById(contentid_t contentId);
  std::vector<const Content*>   GetContents(std::vector<contentid_t>& contentIds);

  contentid_t                   AddContent(Content& newContent);
  contentid_t                   UpdateContent(contentid_t id, const std::string& content);
  
  bool SyncContent(contentid_t id);
  bool SyncContents();

  ssize_t LoadAllFromStorage() override {
    DEBUG_cout << "Not Used." << endl; 
    return 0;
  }
  ssize_t SaveAllToStorage() override {
    DEBUG_cout << "Not Used." << endl; 
    return 0;
  }

protected:
  bool open() override;
  bool close() override;

  //ssize_t recover() override;
  //ssize_t cleanup() override;




private:

  std::string dirPath_;
  std::string dataFileName_;
  std::string indexFileName_;

  ContentsIndex index_;

  std::fstream dataFile_;


  std::map<contentid_t, Content> contents_;
  
};

}

#endif

