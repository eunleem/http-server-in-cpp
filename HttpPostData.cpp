#include "HttpPostData.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpPostData::Exception::exceptionMessages_[] = {
  HTTPPOSTDATA_EXCEPTION_MESSAGES
};
#undef HTTPPOSTDATA_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpPostData::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpPostData::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpPostData::ExceptionType
HttpPostData::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


HttpPostData::HttpPostData(DataBlock<char*> data) :
  content_(data)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
}

HttpPostData::HttpPostData(char* data, size_t length) :
  content_(DataBlock<char*>(data, 0, length))
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpPostData::~HttpPostData() {
  DEBUG_FUNC_START;

}

DataBlock<char*> HttpPostData::GetData(const std::string& fieldName) {
  if (this->content_.IsNull() == true) {
    return DataBlock<char*>();
  }

  ssize_t dataPos =
      Util::String::Find(fieldName + "=", (char*)this->content_.GetObject(),
                         this->content_.GetLength());

  if (dataPos == -1) {
    DEBUG_cout << "Not Found. fieldName: " << fieldName << endl;
    return DataBlock<char*>();
  } 

  dataPos += fieldName.length() + strlen("=");

  const size_t maxLength = this->content_.GetLength() - dataPos;
  size_t length = 0;
  
  char* ptr = (char*)this->content_.GetObject() + dataPos;

  for (length = 0; length < maxLength; ++length) {
    if (*ptr == '\n' || *ptr == '\r' || *ptr == '&') {
      break;
    }
    ++ptr;
  }

  ssize_t decodedLength = Util::String::UriDecodeFly(
      (char*)this->content_.GetObject() + dataPos, length);
  if (decodedLength > length) {
    DEBUG_clog << "DecodedLength exceeded original length. "
               << "Possible overwrite of data! "
               << "orglenth: " << length << " "
               << "decodedLength: " << decodedLength << endl;
  }

  return DataBlock<char*>((char*)this->content_.GetObject() + dataPos, 0, decodedLength); 
}

//HttpPostData::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpPostData : public HttpPostData {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpPostData, TESTNAME) {
  MockHttpPostData mockHttpPostData;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

