#include "DataFile.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio { namespace db {

// ===== Exception Implementation ===== 
const char* const
DataFile::Exception::exceptionMessages_[] = {
  DATAFILE_EXCEPTION_MESSAGES
};
#undef DATAFILE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


DataFile::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
DataFile::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const DataFile::ExceptionType
DataFile::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


DataFile::DataFile() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

DataFile::~DataFile() {
  DEBUG_FUNC_START;

}

bool DataFile::Open() {
  if (Util::IsFileExisting(this->filePath) == false) {
    DEBUG_cerr << "File does not exist." << endl; 
    return false;
  } 

  this->file.open(this->filePath, std::ios::in | std::ios::out | std::ios::binary);
  if (file.is_open() == false) {
    DEBUG_cerr << "Could not open file." << endl; 
    return false;
  } 

  return true;
}

bool DataFile::Close() {
  if (file.is_open() == false) {
    DEBUG_cerr << "File is not open or already closed." << endl; 
    return true;
  } 

  file.flush();
  file.sync();
  file.close();

  return true;
}

std::fstream& DataFile::GetFile() {
  if (this->file.is_open() == false) {
    DEBUG_cerr << "File is not open!" << endl; 
    throw Exception(ExceptionType::FILE_NOT_OPEN);
  } 

  return this->file;
}
//DataFile::
//DataFile::

} }

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockDataFile : public DataFile {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(DataFile, TESTNAME) {
  MockDataFile mockDataFile;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

