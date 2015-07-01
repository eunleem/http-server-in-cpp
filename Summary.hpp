#ifndef _SUMMARY_HPP_
#define _SUMMARY_HPP_
/*
  Name
    Summary

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 01, 2015
  
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

#include "liolib/Util.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {


template<typename ID_T>
class Summary : public Serializable, public Openable {
public:
  Summary(std::string summaryFileName, std::string dirPath) 
  {
    if (dirPath.back() != '/') {
      dirPath.push_back('/');
    } 
    this->filePath = dirPath + summaryFileName;
  } 

  virtual
  ~Summary() {
  }

  bool SaveToStorage() {
    this->summaryFile.open(this->filePath, std::ios::out | std::ios::binary);
    if (this->summaryFile.is_open() == true) {
      this->summaryFile << *this;
      this->summaryFile.flush();
      this->summaryFile.close();
      return true;
    } 
    return false;
  }

  bool LoadFromStorage() {
    bool isExisting = Util::File::IsFileExisting(this->filePath, false);
    if (isExisting == false) {
      this->lastid = 0;
      this->SaveToStorage();
    } 
    
    this->summaryFile.open(this->filePath, std::ios::in | std::ios::binary);
    if (this->summaryFile.is_open() == true) {
      this->summaryFile >> *this;
      this->summaryFile.close();
      return true;
    } 
    return false;
  }

  ID_T lastid;
  // Possible Extra fields
  //  1. Total Size
  //  2. Data Integrity check using MD5 or etc.

protected:
  virtual
  bool open() override {
    return this->LoadFromStorage();
  }

  virtual
  bool close() override {
    return this->SaveToStorage();
  }

  std::ostream& Serialize(std::ostream& os) const override {
    os.write((char*)&this->lastid, sizeof(this->lastid));
    return os;
  }

  std::istream& Deserialize(std::istream& is) override {
    is.read((char*)&this->lastid, sizeof(this-lastid));
    return is;
  }

  std::string filePath;
  std::fstream summaryFile;
};

}

#endif

