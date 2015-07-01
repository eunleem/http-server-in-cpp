#include "Lives.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Lives::Exception::exceptionMessages_[] = {
  LIVES_EXCEPTION_MESSAGES
};
#undef LIVES_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Lives::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Lives::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Lives::ExceptionType
Lives::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Lives::Lives(std::string dirPath, std::string dataFileName) :
  summary_(dirPath),
  directoryPath_(dirPath),
  dataFileName_(dataFileName)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
}

Lives::~Lives() {
}


bool Lives::open() {
  DEBUG_FUNC_START;
  srand(time(NULL));

  if (Util::IsDirectoryExisting(this->directoryPath_) == false) {
    // No Directory
    DEBUG_cerr << "Directory doesn't exist! path: " << this->directoryPath_ << endl; 
    return false;
  } 
  
  DEBUG if (this->dataFile_.is_open() == true) {
    DEBUG_cerr << "Lives data file is already open." << endl; 
    return true;
  } 

  if (this->isLockFileExisting(this->directoryPath_) == true) {
    DEBUG_cerr << "Table has been closed improperly." << endl; 
    ssize_t recoveryResult = this->recover();
    if (recoveryResult == -1) {
      DEBUG_cerr << "Recovery Failed. Something serious has gone wrong! Halting opening process!" << endl; 
      return false;
    } 
    this->deleteLockFile(this->directoryPath_);
  } 

  this->summary_.Open();

  std::string filePath = this->directoryPath_ + this->dataFileName_;
  DEBUG_cout << "dataFilPath: " << filePath << endl; 

  const bool IF_NOT_EXIST_CREATE = true;

  Util::File::IsFileExisting(filePath, IF_NOT_EXIST_CREATE);

  this->dataFile_.open(filePath,
      std::ios::in | std::ios::out | std::ios::binary);

  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "Could not open Lives Data file." << endl; 
    return false;
  }

  this->LoadAllFromStorage();

  this->createLockFile(this->directoryPath_);

  return true;
}

bool Lives::close() {
  if (this->dataFile_.is_open() == false) {
    DEBUG_cerr << "DataFile is not open and attempt was made to close it." << endl; 
    return false;
  }

  this->dataFile_.flush();
  this->dataFile_.sync();
  this->dataFile_.close();

  this->summary_.Close();

  this->deleteLockFile(this->directoryPath_);

  this->lifeById_.clear();
  this->lifeByDna_.clear();

  return true;
}

std::pair<const Life*, std::string> Lives::CreateLife() {
  DEBUG_FUNC_START;
  if (this->status_ != Status::OPEN) {
    DEBUG_cerr << "Lives table is not open!" << endl; 
    throw Exception(ExceptionType::NOT_OPEN);
  } 

  const int RANDOM_CHAR_COMPLEXITY = 1; // UpperCase Alphabets only
  const int SECRET_CODE_LENGTH = 4;

  this->summary_.lastid += 1;

  Life life;
  life.id = this->summary_.lastid;

  std::string secret_code = Util::String::RandomString(
      SECRET_CODE_LENGTH, RANDOM_CHAR_COMPLEXITY);

  life.SetHashed(secret_code);

  life.created = std::chrono::system_clock::now();

  std::string dna;
  while (true) {
    dna = life.GetDna();
    auto itr = this->lifeByDna_.find(dna);
    if (itr == this->lifeByDna_.end()) {
      break;
    } 
    life.GenerateDna();
  } 

  this->lifeById_[life.id] = life;
  this->lifeByDna_[dna] = &this->lifeById_[life.id];

  this->addLifeToFile(this->lifeById_[life.id]);

  DEBUG_cout << "CREATED NEW LIFE." << endl; 
  DEBUG_cout << "  SecretCode: " << secret_code << endl; 
  DEBUG_cout << "  lifd.id: " << life.id << endl; 
  DEBUG_cout << "  life.dna: " << dna << "." << endl; 
  DEBUG_cout << "  life.salt: " << life.GetSalt() << "." << endl; 

  return std::pair<const Life*, std::string>(&this->lifeById_[life.id], secret_code);
}


const std::unordered_map<lifeid_t, Life>& Lives::GetLives() const {
  return this->lifeById_;
}

const Life* Lives::GetLifeById(lifeid_t id) {
  auto it = this->lifeById_.find(id);
  if (it == this->lifeById_.end()) {
    DEBUG_cout << "Life not found by id." << endl; 
    throw Exception(ExceptionType::LIFE_NOT_FOUND);
  } 

  return &(it->second);
}

const Life* Lives::GetLifeByDnaAndSecretCode(
    std::string& dna,
    std::string secret_code) const 
{
  DEBUG_FUNC_START;
  DEBUG_cout << "  DNA: " << dna << endl; 
  DEBUG_cout << "  SecretCode: " << secret_code << endl; 

  auto it = this->lifeByDna_.find(dna);
  if (it == this->lifeByDna_.end()) {
    // not found
    DEBUG_cerr << "Life id not found by dna." << endl; 
    throw Exception(ExceptionType::LIFE_NOT_FOUND);
  } 

  const Life& life = *(it->second);
  std::string salt = life.GetSalt();
  DEBUG_cout << "  Salt: " << salt << endl; 
  secret_code.append(salt);

  unsigned char hash[SHA512_DIGEST_LENGTH];
  SHA512_CTX ctx;
  SHA512_Init(&ctx);
  SHA512_Update(&ctx, secret_code.c_str(), secret_code.length());
  SHA512_Final(hash, &ctx);

  const int MATCH = 0;
  int result = memcmp(hash, life.GetHashedPtr(), life.GetHashedSize());
  if (result != MATCH) {
    DEBUG_cout << "SecretCode and DNA do not match!" << endl; 
    throw Exception(ExceptionType::LIFE_NOT_FOUND);
  }

  return &life;
}

ssize_t Lives::LoadAllFromStorage() {
  DEBUG_FUNC_START;

  const ssize_t fileSizeLimit = 1024 * 1024 * 10; // 10MB
  const ssize_t fileSize = Util::File::GetSize(this->dataFile_);
  const size_t rowSize = Life::GetRowSize();
  const size_t numLoops = fileSize / rowSize;

  size_t count = 0;

  if (fileSize == -1) {
    DEBUG_cerr << "File size is -1." << endl; 
    return -1;
  } 

  if (fileSize % rowSize != 0) {
    DEBUG_cerr << "File size % rowSize != 0." << endl; 
  } 

  if (fileSize > fileSizeLimit) {
    DEBUG_cout << "Lives Table is larger than " << fileSizeLimit / 1024 / 1024 << "MB" << endl; 
  } 

  this->dataFile_.seekg(0, std::ios::beg);

  DEBUG_cout << "NumLoops for reading file: " << numLoops << endl; 

  for (size_t i = 0; numLoops > i; i++) {
    Life life;
    this->dataFile_ >> life;
    if (life.id == 0) {
      DEBUG_cerr << "life id cannot be 0!" << endl; 
      break;
    } 
    this->lifeById_[life.id] = life;
    std::string dna = life.GetDna();
    this->lifeByDna_[dna] = &this->lifeById_[life.id];
    count += 1;
  } 
  //this->dataFile_.read((char*)&this->lastId_, sizeof(this->lastId_));
  //while (this->dataFile_.eof() == false) { } 

  /* #CAUTION: seekg and clear error flags are important after reading till eof. */
  this->dataFile_.seekg(0);
  this->dataFile_.clear();

  //this->summary_.lastid = count;

  DEBUG {
    if (count != numLoops) {
      DEBUG_cerr << "count != numLoops" << endl; 
    } 
  }

  DEBUG_cout << "Read " << count << " rows." << endl; 
  return count;
}

ssize_t Lives::SaveAllToStorage() {
  DEBUG_cout << "This function is not appropriate for this Table." << endl; 
  return -1;
}

bool Lives::addLifeToFile(const Life& life) {
  DEBUG_FUNC_START;
  this->dataFile_.seekp(0, std::ios::end);
  this->dataFile_ << life;
  this->dataFile_.flush();
  this->dataFile_.sync();
  return true;
}


ssize_t Lives::recover() {
  const size_t fileSize = Util::File::GetSize(this->dataFile_);
  if (fileSize % sizeof(Life::GetRowSize()) != 0) {
    DEBUG_cerr << "File Size is not evenly divisible by Row Size. Probably file is corrupted." << endl; 
    return -1;
  } 

  return 0;
}


//Lives::
//Lives::

}

#if _UNIT_TEST

#include <iostream>
#include <string>

using namespace lio;

int main() {

  Lives lives("test.lives", "./testdata/lives/");

  lives.Open();

  std::pair<const Life*, std::string> newLifeA = lives.CreateLife();
  std::string dnaA = newLifeA.first->GetDna();

  std::pair<const Life*, std::string> newLifeB = lives.CreateLife();
  std::string dnaB = newLifeB.first->GetDna();

  std::pair<const Life*, std::string> newLifeC = lives.CreateLife();
  std::string dnaC = newLifeC.first->GetDna();

  const Life* lifeA = nullptr;
  const Life* lifeB = nullptr;
  const Life* lifeC = nullptr;


  try {
    lifeA = lives.GetLifeByDnaAndSecretCode(dnaA, newLifeA.second); 
    if (lifeA == nullptr) {
      DEBUG_cout << "Could not find life by dna and secret code." << endl; 
    } else {
      DEBUG_cout << "LifeA" << endl; 
      lifeA->Print();
    }
  } catch (Lives::Exception& ex) {
    DEBUG_cout << "Ex: " << ex.what() << endl;
  } 

  try {
    lifeB = lives.GetLifeByDnaAndSecretCode(dnaB, newLifeB.second); 
    if (lifeB == nullptr) {
      DEBUG_cout << "Could not find life by dna and secret code." << endl; 
    } else {
      DEBUG_cout << "LifeB" << endl; 
      lifeB->Print();
    }
  } catch (Lives::Exception& ex) {
    DEBUG_cout << "Ex: " << ex.what() << endl;
  } 

  try {
    lifeC = lives.GetLifeByDnaAndSecretCode(dnaC, newLifeC.second); 
    if (lifeC == nullptr) {
      DEBUG_cout << "Could not find life by dna and secret code." << endl; 
    } else {
      DEBUG_cout << "LifeC" << endl; 
      lifeC->Print();
    }
    lives.Close();

    lives.Open();

    lifeA = lives.GetLifeByDnaAndSecretCode(dnaA, newLifeA.second); 
    if (lifeA == nullptr) {
      DEBUG_cout << "Could not find life by dna and secret code." << endl; 
    } else {
      DEBUG_cout << "LifeA" << endl; 
      lifeA->Print();
    }

  } catch (Lives::Exception& ex) {
    DEBUG_cout << "Ex: " << ex.what() << endl;
  } 


  lives.Close();

  return 0;


}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

