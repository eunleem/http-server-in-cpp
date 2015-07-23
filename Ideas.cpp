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


Ideas::Ideas(const std::string& dirPath,
             const std::string& dataFileName) :
  dirPath_(dirPath),
  dataFileName_(dataFileName),
  dataFilePath_(dirPath + dataFileName),
  index_(dirPath, dataFileName)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
}


Ideas::~Ideas() {
  DEBUG_FUNC_START;

}


bool Ideas::open() {

  bool isLockFileExisting = this->isLockFileExisting(this->dirPath_);
  if (isLockFileExisting == true) {
    // Table was closed abnormaly or is still running.
    // For now, only assume table was closed abnormaly.
    // JUST WARNING FOR NOW.
    DEBUG_cerr << "WHILE OPENING IDEA TABLE, LOCK FILE WAS DETECTED!" << endl; 
  } 


  
  //this->summary_.Open();
  //this->index_.Open();
  this->index_.LoadAllFromStorage();
  
  const bool CREATE_IF_NOT_EXISTING = true;

  bool isExisting = Util::File::IsFileExisting(this->dataFilePath_, CREATE_IF_NOT_EXISTING);
  if (isExisting == false) {
    DEBUG_cerr << "Could not create data file!!" << endl; 
    return false;
  } 

  this->dataFile_.open(this->dataFilePath_,
      std::ios::in | std::ios::out | std::ios::binary);
  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "Could not open DataFile." << endl; 
    return false;
  } 

  this->createLockFile(this->dirPath_);

  return true;
}

bool Ideas::close() {
  //this->index_.SaveAllToStorage();
  //this->index_.Close();

  this->dataFile_.flush();
  this->dataFile_.close();

  //this->summary_.Close();

  this->deleteLockFile(this->dirPath_);

  return true;
}


Idea* Ideas::GetIdeaById(const ideaid_t id) {
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Ideas table is not open!" << endl; 
    throw Exception();
  } 

  auto itr = this->ideas_.find(id);
  if (itr == this->ideas_.end()) {
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
      const ideaid_t loadedid = loadedIdea.GetId();
      if (id != loadedid) {
        DEBUG_cerr << "READ WRONG ROW. SERIOUSLY WRONG. id: " << id << " loadedid: " << loadedid << endl; 
        return nullptr;
      } 
      this->ideas_[loadedid] = std::move(loadedIdea);
      return &this->ideas_[loadedid];

    } catch (Ideas::Exception& ex) {
      DEBUG_cerr << "Idea does not exist." << endl; 
      return nullptr;
    } 
  } 

  return &itr->second;
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
  result.reserve(count * sizeof(Idea*));

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

  //this->index_.header.lastId += 1;
  //this->index_.header.numItems += 1;

  const ideaid_t newId = this->index_.items.size() + 1;

  Idea newIdea;
  newIdea.SetId(newId);
  newIdea.SetLifeId(lifeId);
  newIdea.SetTitle(title);
  newIdea.SetType(type);
  newIdea.SetPermission(permission);
  newIdea.SetStatus(Idea::Status::ACTIVE);
  newIdea.SetContentId(contentId);

  this->dataFile_ << newIdea;
  
  const std::streampos pos = newIdea.GetPos();
  
  this->index_.AddIndex(newIdea, pos);
  this->ideas_[newIdea.GetId()] = std::move(newIdea);

  return newIdea.GetId();
}


bool Ideas::SyncIdea(ideaid_t id) {
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

  this->dataFile_ << idea;
  DEBUG_cout << "Idea " << id << " has been synced." << endl; 

  return true;
}

bool Ideas::SyncIdeas() {

  for (auto& idea : this->ideas_) {
    if (idea.second.IsSynced() == false) {
      this->dataFile_ << idea.second;
      DEBUG_cout << "Idea " << idea.second.GetId() << " has been synced." << endl; 
    } 
  } 

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
  created(std::chrono::system_clock::now()),
  contentId(0),
  pos(-1),
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

datetime Idea::GetCreatedTime() const {
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
  return std::string(this->title);
}

const char* Idea::GetTitleChar() const {
  return this->title;
}

uint32_t Idea::GetContentId() const {
  return this->contentId;
}

std::streampos Idea::GetPos() const {
  return this->pos;
}

bool Idea::IsDeleted() const {
  return this->status == Status::DELETED;
}

bool Idea::IsSynced() const {
  return this->isSynced;
}

void Idea::SetId(ideaid_t id) {
  if (this->id != id) {
    this->isSynced = false;
    this->id = id;
  } 
}

void Idea::SetLifeId(lifeid_t lifeid) {
  if (this->lifeId != lifeid) {
    this->isSynced = false;
    this->lifeId = lifeid;
  } 
}

void Idea::SetStatus(Idea::Status status) {
  if (this->status != status) {
    this->status = status;
    this->isSynced = false;
  } 
}

void Idea::SetType(Idea::Type type) {
  if (this->type != type) {
    this->type = type;
    this->isSynced = false;
  } 
}

void Idea::SetPermission(Idea::Permission& perm) {
  if (this->perm != perm) {
    this->perm = perm;
    this->isSynced = false;
  }
}

bool Idea::SetTitle(const std::string& title) {
  size_t length = title.length();
  if (length > 200) {
    DEBUG_cerr << "Title length is too long. Trimming it and continue." << endl; 
    length = 200;
  } 
  memset(this->title, 0, sizeof(this->title));
  title.copy(this->title, length);
  //memcpy(this->title, title.c_str(), length);
  this->isSynced = false;
  return true;
}

void Idea::SetContentId(uint32_t contentId) {
  if (this->contentId != contentId) {
    this->contentId = contentId;
    this->isSynced = false;
  }
}


}

#define _UNIT_TEST false
//#include "liolib/Test.hpp"
#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

#include <string>


TEST(IdeasTest, OpenAndClose) {
  ASSERT_NO_THROW({
    Ideas table("ideas.data", "./testdata/ideas/");
    ASSERT_TRUE(table.Open());
    ASSERT_TRUE(table.Close());
  });
}



ideaid_t firstId = 0;
TEST(IdeasTest, AddIdea) {
  Ideas table("ideas.data", "./testdata/ideas/");
  EXPECT_TRUE(table.Open());

  std::string content = "Hello First Idea";

  ASSERT_NO_THROW({
    ideaid_t newId = table.AddIdea(1, content);
    firstId = newId;
    EXPECT_GT(newId, 0);
    DEBUG_cout << "newId: " << newId << endl; 
    Idea* idea = table.GetIdeaById(newId);
    ASSERT_FALSE(idea == nullptr);
    EXPECT_TRUE(idea->IsSynced() == true);
    EXPECT_EQ(0, idea->GetContentId());
  });

  EXPECT_TRUE(table.Close());
}


TEST(IdeasTest, GetAddedIdea) {
  Ideas table("ideas.data", "./testdata/ideas/");
  EXPECT_TRUE(table.Open());

  std::string content = "Hello First Idea";

  ASSERT_NO_THROW({
    Idea* idea = table.GetIdeaById(firstId);
    ASSERT_FALSE(idea == nullptr);
    EXPECT_EQ(firstId, idea->GetId());
    EXPECT_STREQ(content.c_str(), idea->GetTitleChar());
    EXPECT_STREQ(content.c_str(), idea->GetTitle().c_str());
    EXPECT_TRUE(idea->IsSynced() == true);
    EXPECT_EQ(0, idea->GetContentId());
  });

  EXPECT_TRUE(table.Close());
}

TEST(IdeasTest, AddIdeas) {
  Ideas table("ideas.data", "./testdata/ideas/");
  srand(time(NULL));

  EXPECT_TRUE(table.Open());

  for (int i = 0; 200 > i; ++i) {
    size_t randomLength = rand() % (200 + 100);
    uint32_t randomLifeId = rand() % 2000;

    std::string randomContent = Util::String::RandomString(randomLength);

    ASSERT_NO_THROW({
      ideaid_t newId = table.AddIdea(randomLifeId, randomContent);
      EXPECT_GT(newId, 0);
      DEBUG_cout << "newId: " << newId << endl; 

      Idea* idea = table.GetIdeaById(newId);
      ASSERT_FALSE(idea == nullptr);
      EXPECT_EQ(newId, idea->GetId());
      //EXPECT_EQ(randomLifeId, idea->Get());

      EXPECT_LT(idea->GetTitle().length(), 201);
      std::string trimmed(
        (randomContent.length() > 200) ? randomContent.substr(0, 200) : std::move(randomContent)
      );

      EXPECT_STREQ(trimmed.c_str(), idea->GetTitle().c_str());
      EXPECT_STREQ(trimmed.c_str(), idea->GetTitleChar());
      EXPECT_TRUE(idea->IsSynced() == true);
      EXPECT_EQ(0, idea->GetContentId());
      idea->Print();
    });
  } 

  EXPECT_TRUE(table.Close());
}

TEST(IdeasTest, OpenAfterAdding) {
  Ideas table("ideas.data", "./testdata/ideas/");
  EXPECT_TRUE(table.Open());
  //table.GetIdea();
  EXPECT_TRUE(table.Close());
}

TEST(IdeasTest, UpdateIdea) {
}

//TEST(IdeasTest, ) {
//}


int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
#if 0
int main() {
  Ideas table("ideas.data", "./testdata/ideas/");

  table.Open();
  std::string content = "Hello First Idea";
  
  ideaid_t newId = table.AddIdea(1, content);
  content = "Hello SEcond Idea";
  newId = table.AddIdea(1, content);
  DEBUG_cout << "newId: " << newId << endl; 

  std::vector<Idea*> ideas = table.GetIdeas(0, 9);
  DEBUG_cout << "count: " << ideas.size() << endl; 


  DEBUG_cout << "ID: " << ideas.front()->GetId() << endl;
  ideas.front()->SetTitle("Hello world Updated!");
  table.SyncIdeas();

  table.Close();

  return 0;
}
#endif

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

