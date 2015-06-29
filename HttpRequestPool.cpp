#include "HttpRequestPool.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpRequestPool::Exception::exceptionMessages_[] = {
  HTTPREQUESTPOOL_EXCEPTION_MESSAGES
};
#undef HTTPREQUESTPOOL_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpRequestPool::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpRequestPool::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpRequestPool::ExceptionType
HttpRequestPool::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 

HttpRequestPool* HttpRequestPool::instance = nullptr;
size_t HttpRequestPool::currentIndex = 0;

HttpRequestPool::HttpRequestPool() : 
  requests_(50, HttpRequest())
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpRequestPool::~HttpRequestPool() {
  DEBUG_FUNC_START;
}

HttpRequestPool& HttpRequestPool::GetInstance() {
  if (instance == nullptr) {
    instance = new HttpRequestPool(); 
    //instance->SetMaxPoolSize(50);
  } else {
    DEBUG_cout << "Getting Existing RequestPool." << endl; 
  }

  return (*instance);
}


HttpRequest& HttpRequestPool::AcquireReusable() {
  HttpRequest* request = nullptr;

  DEBUG_cout << "RequestPoolSize: " << this->requests_.size() << endl; 

  const auto& poolSize = this->requests_.size();

  for (size_t i = 0; poolSize > i; ++i) {
    if (this->currentIndex >= poolSize) {
      this->currentIndex = 0;
    } 

    const auto& status = this->requests_[this->currentIndex].GetStatus();
    if (status == HttpRequest::Status::NEW ||
        status == HttpRequest::Status::PROCESSED ||
        status == HttpRequest::Status::READ_TIMEOUT ||
        status == HttpRequest::Status::PROCESS_TIMEOUT ||
        status == HttpRequest::Status::CONNECTION_CLOSED ||
        status == HttpRequest::Status::ERROR)
    {
      request = &this->requests_[this->currentIndex];
      request->Reset();
      DEBUG_cout << "Appointed Request using " << i << " loops at " << this->currentIndex << ". " << endl; 

      this->currentIndex += 1;
      break;

    } else {
      this->currentIndex += 1;
    }
  }

  if (request == nullptr) {
    // Could not find any empty spot.
    //   Check for timeout
    for (auto& slot : this->requests_) {
      bool isTimedout = slot.CheckTimedout();
      if (isTimedout == true) {
        request = &slot;
        request->Reset();
        break;
      } 
    } 
    DEBUG_cerr << "Could not appoint request." << endl; 
    throw Exception(ExceptionType::ACQUIRE_REQUEST);
  } 

  return (*request);
}

bool HttpRequestPool::ReleaseReusable(HttpRequest& request) {
  request.Reset();
  return true;
}

int HttpRequestPool::SetMaxPoolSize(size_t sizeMax) {
  if (this->requests_.size() > sizeMax) {
    DEBUG_cerr << "Cannot reduce pool size." << endl; 
    return -1;
  } 
  //this->requests_ = std::vector<HttpRequest>(50);
  this->requests_.reserve(sizeMax);

  return sizeMax;
}


//HttpRequestPool::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpRequestPool : public HttpRequestPool {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpRequestPool, TESTNAME) {
  MockHttpRequestPool mockHttpRequestPool;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

