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

const size_t Idea::MAX_TITLE_LENGTH = 256;


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
  this->SyncIdeas();
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

  auto itr = this->ideas_.find(id);
  if (itr == this->ideas_.end()) {
    // not found
    try {
      IndexItem<ideaid_t>& item = this->index_.GetIndexItemById(id);
      return this->GetIdeaByIndexItem(item);

    } catch (Ideas::Exception& ex) {
      DEBUG_cerr << "Idea does not exist. " << ex.what() << endl; 
      return nullptr;
    } 
  } 

  return &itr->second;
}

Idea* Ideas::GetIdeaByIndexItem(const IndexItem<ideaid_t>& indexItem) {

  auto itr = this->ideas_.find(indexItem.id);
  if (itr == this->ideas_.end()) {
    // not found
    try {
      const IndexItem<ideaid_t>& item = indexItem;
      DEBUG_cout << "ItemPos: " << item.pos << endl;

      this->dataFile_.seekg(item.pos, std::ios::beg);

      Idea loadedIdea;
      loadedIdea.SyncFrom(this->dataFile_);
      const ideaid_t loadedid = loadedIdea.GetId();
      if (indexItem.id != loadedid) {
        DEBUG_cerr << "READ WRONG ROW. SERIOUSLY WRONG. id: " << indexItem.id
                   << " loadedid: " << loadedid << endl;
        return nullptr;
      } 
      this->ideas_[loadedid] = std::move(loadedIdea);
      return &this->ideas_[loadedid];

    } catch (Ideas::Exception& ex) {
      DEBUG_cerr << "Idea does not exist. " << ex.what() << endl; 
      return nullptr;
    } 
  } 

  return &itr->second;
}

std::vector<Idea*> Ideas::GetIdeas(size_t count, size_t skip) {

  std::vector<Idea*> result;

  if (count == 0) {
    DEBUG_cerr << "Count can't be 0. Returning empty vector." << endl; 
    return result;
  }

  if (count > 200) {
    // number too big
    DEBUG_cerr << "Count can't be too big. Returning empty vector." << endl; 
    return result;
  } 

  if (this->index_.items.size() <= skip) {
    DEBUG_cerr << "Skip cannot be larger than numItems." << endl; 
    return result;
  }

  if (this->index_.items.size() <= (count + skip)) {
    count = this->index_.items.size() - skip; 
  }

  result.reserve(count);


  auto indexRit = this->index_.items.crbegin();
  for (size_t i = 0; i < skip; ++i) {
    ++indexRit;
  }
  for (size_t i = 0; i < count; ++i) {
    result.push_back(this->GetIdeaByIndexItem(indexRit->second));
    ++indexRit;
  }


  DEBUG_cout << "Ideas are returned! n: " << result.size() << endl; 
  return result;
}

ideaid_t Ideas::AddIdea(lifeid_t lifeId, std::string& title,
                        uint32_t contentId, Idea::Permission permission,
                        Idea::Type type) {
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

  newIdea.SyncTo(this->dataFile_);
  
  const std::streampos pos = newIdea.GetPos();
  
  this->index_.AddIndex(newIdea, pos);
  this->ideas_[newIdea.GetId()] = std::move(newIdea);

  return newIdea.GetId();
}


bool Ideas::SyncIdea(ideaid_t id) {
  auto itr = this->ideas_.find(id);
  if (itr == this->ideas_.end()) {
    // not found;
    DEBUG_cout << "Idea not found. Id: " << id << endl; 
    return false;
  } 

  Idea& idea = itr->second ;
  idea.SyncTo(this->dataFile_);

  return true;
}

bool Ideas::SyncIdeas() {
  size_t count = 0;
  for (auto& idea : this->ideas_) {
    if (idea.second.IsSynced() == false) {
      idea.second.SyncTo(this->dataFile_);
      count += 1;
    } 
  } 
  DEBUG_cout << "Synced " << count << " idea(s)." << endl;
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
  modified(std::chrono::system_clock::now()),
  contentId(0),
  pos(-1),
  isSynced(false)
{ }

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

const std::string& Idea::GetTitle() const {
  return this->title;
}

const char* Idea::GetTitleChar() const {
  DEBUG_cerr
      << "GetTitleChar() is DEPRECATED. Use std::string GetTitle() instead."
      << endl;
  return this->title.c_str();
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
    this->modified = std::chrono::system_clock::now();
    this->isSynced = false;
  } 
}

void Idea::SetType(Idea::Type type) {
  if (this->type != type) {
    this->type = type;
    this->modified = std::chrono::system_clock::now();
    this->isSynced = false;
  } 
}

void Idea::SetPermission(Idea::Permission& perm) {
  if (this->perm != perm) {
    this->perm = perm;
    this->modified = std::chrono::system_clock::now();
    this->isSynced = false;
  }
}

bool Idea::SetTitle(std::string& title) {
  if (title.length() > Idea::MAX_TITLE_LENGTH) {
    DEBUG_cerr << "Title is too long. Trimmed and cont..." << endl;
    title.resize(Idea::MAX_TITLE_LENGTH);
  } 

  auto trimmedBytes = Util::String::TrimIncompleteUTF8(title);
  if (trimmedBytes > 0) {
    DEBUG_cout << "Trimmed " << trimmedBytes << " bytes from title." << endl;
  }

  //this->title = std::move(title);
  this->title = title;

  this->modified = std::chrono::system_clock::now();
  this->isSynced = false;
  return true;
}

void Idea::SetContentId(uint32_t contentId) {
  if (this->contentId != contentId) {
    this->contentId = contentId;
    this->modified = std::chrono::system_clock::now();
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
    Ideas table("./testdata/ideas/");
    ASSERT_TRUE(table.Open());
    ASSERT_TRUE(table.Close());
  });
}



ideaid_t firstId = 0;
TEST(IdeasTest, AddIdea) {
  Ideas table("./testdata/ideas/");
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
  Ideas table("./testdata/ideas/");
  EXPECT_TRUE(table.Open());

  std::string content = "Hello First Idea";

  ASSERT_NO_THROW({
    Idea* idea = table.GetIdeaById(firstId);
    ASSERT_FALSE(idea == nullptr);
    EXPECT_EQ(firstId, idea->GetId());
    EXPECT_STREQ(content.c_str(), idea->GetTitle().c_str());
    EXPECT_TRUE(idea->IsSynced() == true);
    EXPECT_EQ(0, idea->GetContentId());
  });

  EXPECT_TRUE(table.Close());
}

size_t numRowsToAdd = 20;
TEST(IdeasTest, AddIdeas) {
  Ideas table("./testdata/ideas/");
  srand(time(NULL));

  EXPECT_TRUE(table.Open());

  for (size_t i = 0; numRowsToAdd > i; ++i) {
    size_t randomLength = rand() % (Idea::MAX_TITLE_LENGTH + 100);
    uint32_t randomLifeId = rand() % 2000;

    std::string randomContent = Util::String::RandomString(randomLength);

    ASSERT_NO_THROW({
      std::string trimmed((randomContent.length() > Idea::MAX_TITLE_LENGTH)
                              ? randomContent.substr(0, Idea::MAX_TITLE_LENGTH)
                              : randomContent);

      Util::String::TrimIncompleteUTF8(trimmed);

      ideaid_t newId = table.AddIdea(randomLifeId, randomContent);
      EXPECT_GT(newId, 0);
      DEBUG_cout << "newId: " << newId << endl; 

      Idea* idea = table.GetIdeaById(newId);
      ASSERT_FALSE(idea == nullptr);
      EXPECT_EQ(newId, idea->GetId());
      //EXPECT_EQ(randomLifeId, idea->Get());

      EXPECT_LT(idea->GetTitle().length(), Idea::MAX_TITLE_LENGTH + 1);

      EXPECT_STREQ(trimmed.c_str(), idea->GetTitle().c_str());
      EXPECT_TRUE(idea->IsSynced() == true);
      EXPECT_EQ(0, idea->GetContentId());
      idea->Print();
    });
  } 

  EXPECT_TRUE(table.Close());
}

TEST(IdeasTest, OpenAfterAdding) {
  Ideas table("./testdata/ideas/");
  EXPECT_TRUE(table.Open());
  //table.GetIdea();
  EXPECT_TRUE(table.Close());
}


TEST(IdeasTest, GetAddedIdeaa) {
  Ideas table("./testdata/ideas/");
  EXPECT_TRUE(table.Open());

  for (size_t i = 0; numRowsToAdd > i; ++i) {
    const ideaid_t randomId = rand() % numRowsToAdd + 1; 

    ASSERT_NO_THROW({
      const Idea* item = table.GetIdeaById(randomId);
      ASSERT_FALSE(item == nullptr);
      EXPECT_TRUE(item->IsSynced() == true);
      item->Print();
    });
  } 

  EXPECT_TRUE(table.Close());
}

TEST(IdeasTest, GetIdeasA) {
  Ideas table("./testdata/ideas/");
  EXPECT_TRUE(table.Open());

  // THIS USES MANY LARGE COUNT AND SKIP TO TEST UNACCEPTABLE INPUT
  const size_t numItems = table.GetNumIdeas();
  for (int i = 0; i < 20; ++i) {
    const size_t randomCount = rand() % (numItems * 2); 
    const size_t randomSkip = rand() % (numItems * 2); 

    ASSERT_NO_THROW({
      DEBUG_cout << "GetIdeas. TotalItemsInIdeas: " << numItems << endl;
      DEBUG_cout << "GetIdeas. count: " << randomCount << " skip: " << randomSkip << endl;
      auto items = table.GetIdeas(randomCount, randomSkip);
      DEBUG_cout << "ReturnedCount: " << items.size() << endl;
      if (randomCount == 0 ||
          randomCount > 200)
      {
        EXPECT_EQ(0, items.size());
        continue;
      }

      if (numItems <= randomSkip) {
        EXPECT_EQ(0, items.size());
        continue;
      }

      if (numItems <= (randomCount + randomSkip)) {
        if (numItems < randomSkip) {
          EXPECT_EQ(0, items.size());
          continue;
        }
        EXPECT_EQ(numItems - randomSkip, items.size());
      }

      if (numItems >= (randomCount + randomSkip)) {
        EXPECT_EQ(randomCount, items.size());
      }
    });
  }

  EXPECT_TRUE(table.Close());
}

TEST(IdeasTest, GetIdeasB) {
  Ideas table("./testdata/ideas/");
  EXPECT_TRUE(table.Open());

  const size_t numItems = table.GetNumIdeas();
  for (int i = 0; i < 200; ++i) {
    const size_t randomCount = rand() % (50); 
    const size_t randomSkip = rand() % (numItems); 

    ASSERT_NO_THROW({
      DEBUG_cout << "GetIdeas. TotalItemsInIdeas: " << numItems << endl;
      DEBUG_cout << "GetIdeas. count: " << randomCount << " skip: " << randomSkip << endl;
      auto items = table.GetIdeas(randomCount, randomSkip);
      DEBUG_cout << "ReturnedCount: " << items.size() << endl;
      if (randomCount == 0 ||
          randomCount > 200)
      {
        EXPECT_EQ(0, items.size());
        continue;
      }

      if (numItems <= randomSkip) {
        EXPECT_EQ(0, items.size());
        continue;
      }

      if (numItems <= (randomCount + randomSkip)) {
        if (numItems < randomSkip) {
          EXPECT_EQ(0, items.size());
          continue;
        }
        EXPECT_EQ(numItems - randomSkip, items.size());
      }

      if (numItems >= (randomCount + randomSkip)) {
        EXPECT_EQ(randomCount, items.size());
      }
    });
  }

  EXPECT_TRUE(table.Close());
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

