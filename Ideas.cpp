#include "Ideas.hpp"

namespace lio {

// ===== Exception Implementation ===== 
const char* const
Ideas::Exception::exceptionMessages_[] = {
  IDEAS_EXCEPTION_MESSAGES
};
#undef IDEAS_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Ideas::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Ideas::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Ideas::ExceptionType
Ideas::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Ideas::Ideas(const std::string& dataFileName,
             const std::string& dirPath) :
  dataFilePath_(dirPath + dataFileName)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
}


Ideas::~Ideas() {
  DEBUG_FUNC_START;

}


bool Ideas::open () {
  this->summary_.Open();
  this->index_.Open();

  bool isExistent = Util::IsFileExistent(this->dataFilePath_);
  if (isExistent == false) {
    this->dataFile_.open(this->dataFilePath_,
        std::ios::out | std::ios::binary);
    this->dataFile_.close();
  }

  this->dataFile_.open(this->dataFilePath_, std::ios::in | std::ios::out | std::ios::binary);
  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "Could not open DataFile." << endl; 
    return false;
  } 

  return true;
}

bool Ideas::close () {
  this->index_.SaveAllToStorage();
  this->index_.Close();

  this->dataFile_.flush();
  this->dataFile_.close();

  this->summary_.Close();

  return true;
}


Idea* Ideas::GetIdeaById(ideaid_t id) {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  auto it = this->ideas_.find(id);
  if (it == this->ideas_.end()) {
    // not found
    try {
      IndexItem<ideaid_t>& item = this->index_.GetIndexItemById(id);
      DEBUG_cout << "ItemPos: " << item.pos << endl; 
      if (this->dataFile_.is_open() == false) {
        DEBUG_cerr << "DataFile is Not Open!" << endl; 
        return nullptr;
      } 

      this->dataFile_.seekg(item.pos, std::ios::beg);

      Idea loadedIdea;
      this->dataFile_ >> loadedIdea;
      //loadedIdea.ReadIdeaFromFile(this->dataFile_);

      this->ideas_[loadedIdea.GetId()] = loadedIdea;
      return &this->ideas_[loadedIdea.GetId()];

    } catch (Ideas::Exception& ex) {
      DEBUG_cerr << "Idea does not exist." << endl; 
      return nullptr;
    } 
  } 

  return &it->second;
}

std::vector<Idea*> Ideas::GetIdeas(size_t from, size_t count) {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  DEBUG_cout << "from: " << from << " count: " << count << endl; 
  std::vector<Idea*> result;
  if (count > 200) {
    // number too big
    DEBUG_cerr << "Count can't be too big. Returning empty vector." << endl; 
    return result;
  } 
  result.reserve(count);

  if (from == 0) {
    result = this->GetLastIdeas(count);
  } else {
    auto indexRit = this->index_.items.find(from);
    if (indexRit == this->index_.items.end()) {
      DEBUG_cout << "Invalid from. from: " << from << endl; 
      return result;
    } 
    for (uint32_t i = 0; count > i && this->index_.items.begin() != indexRit; i++, --indexRit) {
      result.push_back(this->GetIdeaById(indexRit->first));
    }
  }

  DEBUG_cout << "Ideas are returned! n: " << result.size() << endl; 

  return result;
}

std::vector<Idea*> Ideas::GetLastIdeas(size_t count) {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 
  std::vector<Idea*> result;
  if (count > 200) {
    // number too big
    DEBUG_cerr << "Count can't be too big. Returning empty vector." << endl; 
    return result;
  } 
  result.reserve(count);

  auto indexRit = this->index_.items.crbegin();
  for (uint32_t i = 0; count > i && this->index_.items.crend() != indexRit; i++, ++indexRit) {
    result.push_back(this->GetIdeaById(indexRit->first));
  }

  return result;
}


ideaid_t Ideas::AddIdea(lifeid_t lifeId,
                        const string& title, 
                        uint32_t contentId,
                        Idea::Permission permission,
                        Idea::Type type)
{
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  this->index_.header.lastId += 1;
  this->index_.header.numItems += 1;

  Idea newIdea;
  newIdea.SetId(this->index_.header.lastId);
  newIdea.SetLifeId(lifeId);
  newIdea.SetTitle(title);
  newIdea.SetType(type);
  newIdea.SetPermission(permission);
  newIdea.SetStatus(Idea::Status::ACTIVE);
  newIdea.SetContentId(contentId);


  this->dataFile_.seekp(0, std::ios::end);
  const std::streampos pos = this->dataFile_.tellp();
  this->dataFile_ << newIdea;
  
  this->index_.AddIndex(newIdea, pos);
  this->ideas_[newIdea.GetId()] = newIdea;

  return newIdea.GetId();
}


bool Ideas::SyncIdea(ideaid_t id) {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  auto it = this->ideas_.find(id);
  if (it == this->ideas_.end()) {
    // not found;
    DEBUG_cout << "Idea not found. Id: " << id << endl; 
    return false;
  } 
  Idea& idea = it->second ;

  if (idea.IsSynced() == true) {
    DEBUG_cout << "Already synced! Good" << endl; 
    return true;
  } 

  auto idx = this->index_.GetIndexItemById(id);

  this->dataFile_.seekp(idx.pos);
  this->dataFile_ << it->second;
  it->second.SetIsSynced(true);
  DEBUG_cout << "Idea " << id << " has been synced." << endl; 

  return true;
}

bool Ideas::SyncIdeas() {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  for (auto& idea : this->ideas_) {
    if (idea.second.IsSynced() == false) {
      auto idx = this->index_.GetIndexItemById(idea.second.GetId());
      this->dataFile_.seekp(idx.pos);
      this->dataFile_ << idea.second;
      //idea.second.WriteIdeaToFile(this->dataFile_);
      DEBUG_cout << "Idea " << idea.second.GetId() << " has been synced." << endl; 
    } 
  } 

  return true;
}

bool Ideas::SyncIndex() {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  this->index_.SaveAllToStorage();
  return true;
}

ssize_t Ideas::LoadAllFromStorage() {
  

  return -1;
}

ssize_t Ideas::SaveAllToStorage() {

  return -1;
}

//Ideas::
//Ideas::

// =================================================== IDEA
Idea::Idea() :
  id(0),
  lifeId(0),
  status(Status::ACTIVE),
  type(Type::GENERAL),
  perm(Permission::PRIVATE),
  created(system_clock::now()),
  contentId(0),
  isSynced(false)
{ 
  memset(this->title, 0, sizeof(this->title));
}

Idea::~Idea() {
  if (this->isSynced == false) {
    DEBUG_cout << "Unsynced Idea has been destroyed." << endl; 
  } 
}


ideaid_t Idea::GetId() const {
  return this->id;
}

system_clock::time_point Idea::GetCreatedTime() const {
  return this->created;
}

Idea::Type Idea::GetType() const {
  return this->type;
}

Idea::Permission Idea::GetPermission() const {
  return this->perm;
}

Idea::Status Idea::GetStatus() const {
  return this->status;
}

std::string Idea::GetTitle() const {
  return string(this->title);
}

const char* Idea::GetTitleChar() const {
  return this->title;
}

uint32_t Idea::GetContentId() const {
  return this->contentId;
}

bool Idea::IsSynced() const {
  return this->isSynced;
}

void Idea::SetId(ideaid_t id) {
  this->id = id;
  this->isSynced = false;
}

void Idea::SetLifeId(lifeid_t lifeid) {
  this->lifeId = lifeid;
  this->isSynced = false;
}

void Idea::SetStatus(Idea::Status status) {
  this->status = status;
  this->isSynced = false;
}

void Idea::SetType(Idea::Type type) {
  this->type = type;
  this->isSynced = false;
}

void Idea::SetPermission(Idea::Permission& perm) {
  this->perm = perm;
  this->isSynced = false;
}

bool Idea::SetTitle(const std::string& title) {
  size_t length = title.length();
  if (length > 200) {
    DEBUG_cerr << "Title length is too long. Trimming it and continue." << endl; 
    length = 200;
  } 
  memset(this->title, 0, sizeof(this->title));
  memcpy(this->title, title.c_str(), length);
  this->isSynced = false;
  return true;
}

void Idea::SetContentId(uint32_t contentId) {
  this->contentId = contentId;
  this->isSynced = false;
}

void Idea::SetIsSynced(bool isSynced) {
  this->isSynced = isSynced;
}


}

#define _UNIT_TEST false
//#include "liolib/Test.hpp"
#if _UNIT_TEST

//#include "gmock/gmock.h"
//#include "gtest/gtest.h"
#include <string>
using namespace std;
using namespace lio;

int main() {
  Ideas idea;
  string content = "Hello First Idea";
  
  Ideas::ideaid_t newId = idea.AddIdea(content);
  DEBUG_cout << "newId: " << newId << endl; 
  content = "Hello SEcond Idea";
  newId = idea.AddIdea(content);
  DEBUG_cout << "newId: " << newId << endl; 

  vector<Ideas::Idea*> ideas = idea.GetIdeas(0, 9);
  DEBUG_cout << "count: " << ideas.size() << endl; 


  DEBUG_cout << "ID: " << ideas.front()->GetId() << endl;
  ideas.front()->SetTitle("Hello world Updated!");
  idea.SyncIdeas();

  return 0;
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

