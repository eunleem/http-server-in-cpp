#include "Contents.hpp"

namespace lio {

// ===== Exception Implementation ===== 
const char* const
Contents::Exception::exceptionMessages_[] = {
  CONTENTS_EXCEPTION_MESSAGES
};
#undef CONTENTS_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Contents::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Contents::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Contents::ExceptionType
Contents::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 



// =================================================== CONTENT

Content::Content() :
  id(0),
  prevId(0),
  status(Status::ACTIVE),
  type(Type::GENERAL),
  created(std::chrono::system_clock::now()),
  content(),
  pos(-1),
  isSynced(false)
{ }

Content::~Content() {
  if (this->IsSynced() == false) {
    DEBUG_cerr << "Destroying Unsynced content. id: " << this->GetId() << endl; 
  } 
}

contentid_t Content::GetId() const {
  return this->id;
}

contentid_t Content::GetPrevId() const {
  return this->prevId;
}

const std::string& Content::GetContent() const {
  return this->content;
}

datetime Content::GetCreatedTime() const {
  return this->created;
}

Content::Type Content::GetType() const {
  return this->type;
}

bool Content::IsDeleted() const {
  return this->status == Status::DELETED;
}

bool Content::IsSynced() const {
  return this->isSynced;
}

std::streampos Content::GetPos() const {
  return this->pos;
}

void Content::SetId(contentid_t id) {
  this->id = id;
  this->isSynced = false;
}

void Content::SetPrevId(contentid_t id) {
  this->prevId = id;
  this->isSynced = false;
}

void Content::SetStatus(Content::Status status) {
  this->status = status;
  this->isSynced = false;
}

void Content::SetType(Content::Type type) {
  this->type = type;
  this->isSynced = false;
}

bool Content::SetContent(const std::string& content) {
  this->content = content;
  this->isSynced = false;
  return true;
}

std::ostream& Content::Serialize(std::ostream& os) const {
  if (this->isSynced == true) {
    DEBUG_cout << "WARN: writing synced data..." << endl; 
  } 

  if (this->pos == -1) {
    DEBUG_cout << "POS is -1. Meaning it's a new Idea and needs to be added at the end." << endl; 
    os.seekp(0, std::ios::end);
    this->pos = os.tellp();
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->prevId, sizeof(this->prevId));
    os.write((char*)&this->status, sizeof(this->status));
    os.write((char*)&this->type, sizeof(this->type));
    os.write((char*)&this->created, sizeof(this->created));
    Util::File::WriteString<uint32_t>(os, this->content);

  } else {
    // Updates fixed size fields only!!! Does not update Content.
    os.seekp(this->pos, std::ios::beg);
    os.write((char*)&this->id, sizeof(this->id));
    os.write((char*)&this->prevId, sizeof(this->prevId));
    os.write((char*)&this->status, sizeof(this->status));
    os.write((char*)&this->type, sizeof(this->type));
    //os.write((char*)&this->created, sizeof(this->created));
    //Util::File::WriteString<uint32_t>(os, this->content);
  }
  this->isSynced = true;
  return os;
}

std::istream& Content::Deserialize(std::istream& is) {
  this->pos = is.tellg();
  is.read((char*)&this->id, sizeof(this->id));
  is.read((char*)&this->prevId, sizeof(this->prevId));
  is.read((char*)&this->status, sizeof(this->status));
  is.read((char*)&this->type, sizeof(this->type));
  is.read((char*)&this->created, sizeof(this->created));
  Util::File::ReadString<uint32_t>(is, this->content);
  this->isSynced = true;
  return is;
}


// =================================================== INDEX

ContentsIndex::ContentsIndex(const std::string& dirPath,
                             const std::string& indexFileName,
                             const std::string& dataFileName)
   : Index(indexFileName, dirPath),
     dataFileName_(dataFileName)
{
  if (this->dirPath_.back() != '/') {
    DEBUG_cout << "WARN: DirPath needs to end with a slash. "
               << "Automatically putting one for you but FIX IT!" << endl; 
    this->dirPath_.push_back('/');
  }
}


contentid_t ContentsIndex::GetLastId() const {
  return static_cast<contentid_t>(this->items.size());
}

IndexItem<contentid_t>* ContentsIndex::GetIndexItemById(contentid_t id) {
  auto itr = this->items.find(id);
  if (itr == this->items.end()) {
    DEBUG_cerr << "Index Item Not Found." << endl; 
    return nullptr;
  } 
  return &itr->second;
}

bool ContentsIndex::AddIndex(Content& content, std::streampos pos) {
  if (this->items.find(content.GetId()) != this->items.end()) {
    DEBUG_cerr << "Duplicating Index!" << endl; 
    return false;
  }

  IndexItem<contentid_t> newIndexItem(content.GetId(), pos);

  this->indexFile_.seekp(0, std::ios::end);
  this->indexFile_ << newIndexItem;
  this->indexFile_.sync();

  DEBUG_cout << "Added Index! id: " << newIndexItem.id << " pos:" << pos << endl; 

  this->items[newIndexItem.id] = std::move(newIndexItem);
  return true;
}

ssize_t ContentsIndex::Rebuild(const std::string& dataFilePath) {
  DEBUG_FUNC_START;

  if (this->status_ == Status::OPEN) {
    DEBUG_cerr << "Cannot perform Rebuild() while it's open." << endl; 
    return -1;
  }


  std::fstream dataFile;
  dataFile.open(dataFilePath, std::ios::in | std::ios::binary);

  if (dataFile.is_open() == false) {
    DEBUG_cerr << "Could not open data file." << endl; 
    return -1;
  } 

  std::string indexFilePath = this->dirPath_ + this->indexName_;
  this->indexFile_.open(indexFilePath, std::ios::out | std::ios::binary);
  if (this->indexFile_.is_open() == false) {
    DEBUG_cerr << "Failed to open Index File. path: " << indexFilePath << endl; 
    return -1;
  }

  size_t count = 0;

  dataFile.seekg(0);

  while (dataFile.eof() == false) {
    Content newContent;
    dataFile >> newContent;
    //DEBUG_cout << "tellg " << dataFile.tellg() << endl; 

    contentid_t contentid = newContent.GetId();
    std::streampos pos = newContent.GetPos();
    if (contentid == 0) {
      DEBUG_cerr << "Read row and content id is 0. Assuming end of dataFile." << endl; 
      break;
    } 

    DEBUG_cout << "id: " << contentid << " pos: " << pos << endl; 

    IndexItem<contentid_t> newIndexItem(contentid, pos);

    this->indexFile_ << newIndexItem;

    count += 1;
  }

  dataFile.close();

  this->indexFile_.clear();
  this->indexFile_.close();

  return count;
}


ssize_t ContentsIndex::SaveAllToStorage() {

  DEBUG_cout << "SAVE ALL TO STORAGE is not needed! Done Nothing." << endl; 
  return 0;
}

ssize_t ContentsIndex::LoadAllFromStorage() {

  const size_t fileSize = Util::File::GetSize(this->indexFile_);
  const size_t rowSize = sizeof(contentid_t) + sizeof(std::streampos);

  const size_t numLoops = fileSize / rowSize;


  this->indexFile_.seekg(0);

  for (unsigned int i = 0; numLoops > i; ++i) {
    IndexItem<contentid_t> newIndexItem;

    this->indexFile_ >> newIndexItem;

    if (newIndexItem.id == 0) {
      continue;
    } 
    DEBUG_cout << "Loaded index. id: " << newIndexItem.id << " pos: " << newIndexItem.pos << endl; 
    this->items[newIndexItem.id] = std::move(newIndexItem);
  } 

  this->indexFile_.seekg(0);
  this->indexFile_.clear();
  
  return numLoops;
}







// ========================================= CONTENTS

Contents::Contents(const std::string& dirPath,
                   const std::string& dataFileName)
  : dirPath_(dirPath),
    dataFileName_(dataFileName),
    indexFileName_("contents.idx"),
    index_(this->dirPath_, this->indexFileName_, this->dataFileName_)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}


bool Contents::open() {
  DEBUG_FUNC_START;

  const bool CREATE_IF_NOT_EXIST = true;

  const std::string dataFilePath = this->dirPath_ + this->dataFileName_;


  bool isExisting = Util::File::IsFileExisting(dataFilePath, CREATE_IF_NOT_EXIST);
  if (isExisting == false) {
    DEBUG_cerr << "Could not create data file." << endl; 
    return false;
  } 

  bool isLockFileExisting = this->isLockFileExisting(this->dirPath_);
  if (isLockFileExisting == true) {
    DEBUG_cerr << "WHILE OPENING CONTENTS TABLE, LOCK FILE WAS DETECTED!" << endl; 
    // REBUILD INDEX

    const ssize_t rebuildCount = this->index_.Rebuild(dataFilePath);
    if (rebuildCount == -1) {
      DEBUG_cerr << "Index Rebuild Failed." << endl; 
      return false;
    }
    DEBUG_cout << "Rebuilt Index. NumItems: " << rebuildCount << endl; 

    this->deleteLockFile(this->dirPath_);
  } 

  bool isOpened = this->index_.Open();
  if (isOpened == false) {
    DEBUG_cerr << "Could not open index file." << endl; 
    return false;
  } 


  this->dataFile_.open(dataFilePath, std::ios::in | std::ios::out | std::ios::binary);

  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "Could not open data file." << endl; 
    return false;
  } 

  this->createLockFile(this->dirPath_);

  return true;
}

bool Contents::close() {
  DEBUG_FUNC_START;

  this->index_.Close();

  this->dataFile_.flush();
  this->dataFile_.close();

  this->deleteLockFile(this->dirPath_);

  return true;
}


const Content* Contents::GetContentById(contentid_t id) {
  DEBUG_cout << "GettingContentById. Id: " << id << endl; 

  auto contentItr = this->contents_.find(id);
  if (contentItr == this->contents_.end()) {
    // not found
    IndexItem<contentid_t>* item = this->index_.GetIndexItemById(id);
    if (item == nullptr) {
      DEBUG_cerr << "Could not find Index Item. id: " << id << endl; 
      return nullptr;
    } 
    DEBUG_cout << "ItemPos: " << item->pos << endl; 

    Content loadedContent;

    this->dataFile_.seekg(item->pos, std::ios::beg);

    this->dataFile_ >> loadedContent;

    if (id != loadedContent.GetId()) {
      DEBUG_cerr << "LOADED CONTENT has different ID! " << id << "vs" << loadedContent.GetId() << endl; 
      return nullptr;
    } 

    this->contents_[loadedContent.GetId()] = std::move(loadedContent);

    return &this->contents_[loadedContent.GetId()];
  } 

  return &contentItr->second;
}

std::vector<const Content*> Contents::GetContents(std::vector<contentid_t>& contentIds) {
  std::vector<const Content*> result;

  for (const auto& id : contentIds) {
    const Content* content = this->GetContentById(id);
    if (content != nullptr) {
      result.push_back(content);
    } 
  } 
  DEBUG_cout << result.size() << " contents are returned!" << endl; 

  return result;
}

contentid_t Contents::AddContent(Content& newContent) {
  DEBUG_FUNC_START;

  if (newContent.GetContent().length() > 50000) {
    DEBUG_cerr << "Content is hmm too long..." << endl; 
    return 0;
  } 

  newContent.SetId(this->index_.GetLastId() + 1);
  newContent.SetStatus(Content::Status::ACTIVE);

  this->dataFile_.seekp(0, std::ios::end);
  std::streampos pos = this->dataFile_.tellp();

  this->dataFile_ << newContent;

  this->index_.AddIndex(newContent, pos);
  this->contents_[newContent.GetId()] = std::move(newContent);

  return newContent.GetId();
}

contentid_t Contents::UpdateContent(contentid_t id, const std::string& content) {
  DEBUG_FUNC_START;

  if (content.length() > 50000) {
    DEBUG_cerr << "Content is hmm too long..." << endl; 
    return 0;
  } 

  // Update Content by Adding.
  auto contentItr = this->contents_.find(id);
  if (contentItr == this->contents_.end()) {
    // Not found
    DEBUG_cout << "Could not update content. Could not find content. Id: " << id << endl; 
    return 0;
  } 

  Content& prevContent = contentItr->second;

  prevContent.SetStatus(Content::Status::UPDATED);

  Content newContent;
  newContent.SetContent(content);
  newContent.SetPrevId(id);
  newContent.SetType(prevContent.GetType());

  contentid_t newId = this->AddContent(newContent);

  this->dataFile_ << prevContent;

  return newId;
}

bool Contents::SyncContent(contentid_t id) {
  auto it = this->contents_.find(id);
  if (it == this->contents_.end()) {
    // not found;
    DEBUG_cout << "Content not found. Id: " << id << endl; 
    return false;
  } 

  if (it->second.IsSynced() == true) {
    DEBUG_cout << "already synced! Good" << endl; 
    return true;
  } 

  this->dataFile_ << it->second;

  DEBUG_cout << "Content " << id << " has been synced." << endl; 

  return true;
}

bool Contents::SyncContents() {

  for (auto& content : this->contents_) {
    if (content.second.IsSynced() == false) {
      this->dataFile_ << content.second;

      DEBUG_cout << "Content " << content.second.GetId() << " has been synced." << endl; 
    } 
  } 

  return true;
}


//Contents::
//Contents::





}

#define _UNIT_TEST false
//#include "liolib/Test.hpp"
#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

#include <string>


TEST(ContentsTest, OpenAndClose) {
  ASSERT_NO_THROW({
    Contents table("./testdata/contents/");
    ASSERT_TRUE(table.Open());
    ASSERT_TRUE(table.Close());
  });
}

contentid_t firstId = 0;
TEST(ContentsTest, AddContent) {
  Contents table("./testdata/contents/");
  EXPECT_TRUE(table.Open());

  std::string content = "Hello First Contents";

  ASSERT_NO_THROW({
    Content newContent;
    newContent.SetType(Content::Type::GENERAL);
    newContent.SetContent(content);
    contentid_t newId = table.AddContent(newContent);
    firstId = newId;
    EXPECT_GT(newId, 0);
    DEBUG_cout << "newId: " << newId << endl; 
    const Content* item = table.GetContentById(newId);
    ASSERT_FALSE(item == nullptr);
    EXPECT_TRUE(item->IsSynced() == true);
    EXPECT_EQ(0, item->GetPrevId());
  });

  EXPECT_TRUE(table.Close());
}


TEST(ContentsTest, GetAddedContent) {
  Contents table("./testdata/contents/");
  EXPECT_TRUE(table.Open());

  std::string contentstr = "Hello First Contents";

  ASSERT_NO_THROW({
    const Content* item = table.GetContentById(firstId);
    ASSERT_FALSE(item == nullptr);
    EXPECT_EQ(firstId, item->GetId());
    EXPECT_STREQ(contentstr.c_str(), item->GetContent().c_str());
    EXPECT_TRUE(item->IsSynced() == true);
    EXPECT_EQ(0, item->GetPrevId());
  });

  EXPECT_TRUE(table.Close());
}


const int numRowsToAdd = 20;

TEST(ContentsTest, AddManyContents) {
  Contents table("./testdata/contents/");
  srand(time(NULL));

  EXPECT_TRUE(table.Open());

  for (int i = 0; numRowsToAdd > i; ++i) {
    const size_t randomLength = rand() % 2000; 

    std::string randomContent = Util::String::RandomString(randomLength);

    ASSERT_NO_THROW({
      Content newContent;
      newContent.SetType(Content::Type::GENERAL);
      newContent.SetContent(randomContent);

      contentid_t newId = table.AddContent(newContent);
      EXPECT_GT(newId, 0);
      DEBUG_cout << "newId: " << newId << endl; 

      const Content* item = table.GetContentById(newId);
      ASSERT_FALSE(item == nullptr);
      EXPECT_EQ(0, item->GetPrevId());
      EXPECT_STREQ(randomContent.c_str(), item->GetContent().c_str());
      EXPECT_TRUE(item->IsSynced() == true);
      //item->Print();
    });
  } 

  EXPECT_TRUE(table.Close());
}

TEST(ContentsTest, GetAddedContents) {
  Contents table("./testdata/contents/");
  EXPECT_TRUE(table.Open());

  for (int i = 0; numRowsToAdd > i; ++i) {
    const contentid_t randomId = rand() % numRowsToAdd + 1; 

    ASSERT_NO_THROW({
      const Content* item = table.GetContentById(randomId);
      ASSERT_FALSE(item == nullptr);
      EXPECT_TRUE(item->IsSynced() == true);
      //item->Print();
    });
  } 

  EXPECT_TRUE(table.Close());
}

TEST(ContentsTest, UpdateContent) {
  Contents table("./testdata/contents/");
  EXPECT_TRUE(table.Open());

  const contentid_t randomId = rand() % numRowsToAdd + 1; 


  ASSERT_NO_THROW({
    const Content* item = table.GetContentById(randomId);
    ASSERT_FALSE(item == nullptr);
    EXPECT_TRUE(item->IsSynced() == true);

    contentid_t newid = table.UpdateContent(item->GetId(), "This is New Content");

    const Content* newitem = table.GetContentById(newid);
    ASSERT_FALSE(newitem == nullptr);
    EXPECT_EQ(item->GetId(), newitem->GetPrevId());
    EXPECT_TRUE(newitem->IsSynced() == true);


  });

  EXPECT_TRUE(table.Close());
}

TEST(ContentsTest, SimulateCrash) {
  Contents table("./testdata/contents/");
  EXPECT_TRUE(table.Open());
}

TEST(ContentsTest, TestRebuild) {
  Contents table("./testdata/contents/");
  EXPECT_TRUE(table.Open());

  for (int i = 0; numRowsToAdd > i; ++i) {
    const contentid_t randomId = rand() % numRowsToAdd + 1; 

    ASSERT_NO_THROW({
      const Content* item = table.GetContentById(randomId);
      ASSERT_FALSE(item == nullptr);
      EXPECT_TRUE(item->IsSynced() == true);
      //item->Print();
    });
  } 

  EXPECT_TRUE(table.Close());
}

//TEST(ContentsTest, ) {
//}


int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
#else
// Executable File's Main Comes here.

#endif

#undef _UNIT_TEST

