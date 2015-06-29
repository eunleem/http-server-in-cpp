#ifndef _TESTTABLE_HPP_
#define _TESTTABLE_HPP_
/*
  Name
    TestTable

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    May 07, 2015
  
  History
    April 27, 2015
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

#include "Table.hpp"

#include "liolib/Serializable.hpp"

#include "liolib/Util.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


class TestIndex : public Index {
public:
  TestIndex() : 
    Index("test.idx", "./data/")
  {}

  bool AddIndexItem(IndexItem<uint32_t> item) {
    auto itr = this->indexItems_.find(item.id);
    if (itr != this->indexItems_.end()) {
      DEBUG_cout << "DUPLICATING INDEX ITEM. id: " << item.id << endl; 
      return false;
    } 

    this->indexItems_[item.id] = item;

    return true;
  }



  IndexItem<uint32_t>& GetIndexItem(uint32_t id) {
    auto itr = this->indexItems_.find(id);
    if (itr == this->indexItems_.end()) {
      DEBUG_cout << "Item Not Found. id: " << id << endl; 
      throw Exception();
    } 

    return itr.second;
  }

  ssize_t SaveAllToStorage() override {
    this->indexFile_.seekp(0, std::ios::beg);

    for (auto& item : this->indexItems_) {
      this->indexFile_ << item;
    } 

    this->indexFile_.flush();
    this->indexFile_.sync();

    this->indexFile_.seekp(0, std::ios::beg);

    DEBUG_cout << "Saved " << this->indexItems_.size() << " index items." << endl; 
    return this->indexItems_.size();
  }

  ssize_t LoadAllFromStorage() override {
    const size_t fileSize = Util::File::GetFileSize(this->indexFile_);
    const size_t itemSize = sizeof(uint32_t) + sizeof(std::streampos);
    const size_t numLoops = fileSize / itemSize;
    DEBUG_cout << "NumLoops: " << numLoops << endl; 

    this->indexFile_.seekg(0, std::ios::beg);
    for (size_t i = 0; numLoops > i; i++) {
      IndexItem<uint32_t> indexItem;
      this->indexFile_ >> indexItem;
      DEBUG_cout << "indexItem.id: " << indexItem.id << endl; 
      this->indexItems_[indexItem.id] = indexItem;
    } 

    this->indexFile_.seekg(0, std::ios::beg);

    DEBUG_cout << "Loaded " << this->indexItems_.size() << " IndexItems." << endl; 
    return this->indexItems_.size();
  }

protected:
  std::map<uint32_t, IndexItem<uint32_t>> indexItems_;

private:
};

class TestRow : public Row {
public:
  TestRow() : 
    id(0),
    name("EMPTY"),
    num(0) { }

  uint32_t id;
  std::string name;
  uint8_t num;

protected:
  std::ostream& Serialize(std::ostream& os) const override {
    os.write((char*)&this->id, sizeof(this->id));
    ssize_t nWrote = Util::File::WriteString(os, this->name);
    DEBUG_cout << "nWrote: " << nWrote << endl; 
    os.write((char*)&this->num, sizeof(this->num));

    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->id, sizeof(this-id));
    ssize_t nRead = Util::File::ReadString(is, this->name);
    DEBUG_cout << "nRead: " << nRead << endl; 
    is.read((char*)&this->num, sizeof(this->num));

    return is;
  }


private:
};

class TestTable : public Table {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define TESTTABLE_EXCEPTION_MESSAGES \
  "TestTable Exception has been thrown."

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


  TestTable() {} 
  ~TestTable() {
    DEBUG_FUNC_START;
    if (this->status_ == Status::OPEN) {
      this->Close();
    } 
  }

  TestRow* GetRowById(uint32_t id) {
    auto it = this->rows.find(id);
    if (it != this->rows.end()) {
      DEBUG_cout << "Row Found. id: " << it->second.id << endl; 
      return &(it->second);
    } else {
      DEBUG_cerr << "Row Not Found." << endl; 
      return nullptr;
    }
  }

  bool AddRow(TestRow row) {
    if (this->status_ != Status::OPEN) {
      DEBUG_cerr << "Table is NOT open." << endl; 
      return false;
    } 
    this->rows[row.id] = row;

    return true;
  }

protected:
  ssize_t SaveAllToStorage() override {
    DEBUG_FUNC_START;
    this->file.seekp(0, std::ios::beg);
    for (auto& row : this->rows) {
      this->file << row.second;
    } 

    return this->rows.size();
  }

  ssize_t LoadAllFromStorage() override {
    DEBUG_FUNC_START;
    while (file.eof() == false) {
      TestRow row;
      file >> row;
      if (row.id == 0) {
        break;
      } 
      rows[row.id] = row;
    } 

    file.seekg(0);
    file.clear();

    return 0;
  }

  bool open() override {

    this->openIndex();
    this->file.open("./TestTable.data",
        std::ios::out | std::ios::in | std::ios::binary);
    if (this->file.is_open() == false) {
      DEBUG_cerr << "Could not open File." << endl; 
      return false;
    } 
    return true;
  }

  bool close() override {
    this->file.flush();
    this->file.sync();
    this->file.close();
    return true;
  }

  bool openIndex() {
    index.Open();

  }
  
private:

  std::map<uint32_t, TestRow> rows;
  std::fstream file;

  TestIndex index;

  
};

}

#endif

