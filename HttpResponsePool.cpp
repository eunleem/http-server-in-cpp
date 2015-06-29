#include "HttpResponsePool.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpResponsePool::Exception::exceptionMessages_[] = {
  HTTPRESPONSEPOOL_EXCEPTION_MESSAGES
};
#undef HTTPRESPONSEPOOL_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpResponsePool::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpResponsePool::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpResponsePool::ExceptionType
HttpResponsePool::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 

HttpResponsePool* HttpResponsePool::instance = nullptr;
size_t HttpResponsePool::currentIndex = 0;

HttpResponsePool& HttpResponsePool::GetInstance() {
  if (instance == nullptr) {
    instance = new HttpResponsePool(); 
  } else {
    DEBUG_cout << "Reusing Existing ResponsePool." << endl; 
  }


  return (*instance);
}

HttpResponse& HttpResponsePool::AcquireReusable() {
  const size_t& responseSize = this->responses_.size();
  DEBUG_cout << "ResponseSize: " << responseSize << endl; 
  for (size_t i = 0; i < responseSize; i++) {
    if (this->currentIndex >= responseSize) {
      this->currentIndex = 0;
    } 
    HttpResponse& response = this->responses_[this->currentIndex];
    this->currentIndex += 1;
    
    const HttpResponse::Status status = response.GetStatus();
    if (status == HttpResponse::Status::NEW) {
      response.SetStatus(HttpResponse::Status::IN_PROCESS);
      return response;

    } else if (status == HttpResponse::Status::SENT ||
               status == HttpResponse::Status::ERROR ||
               status == HttpResponse::Status::TIMEOUT ||
               status == HttpResponse::Status::CACHE_EXPIRED)
    {
      response.Reset();
      response.SetStatus(HttpResponse::Status::IN_PROCESS);
      return response;
    } 
  }

  for (auto& response : this->responses_) {
    if (response.CheckTimeout() == true) {
      return response;
    } 
  } 

  if (responseSize > 2000) {
    DEBUG_cerr << "Could not appoint Response." << endl; 
    throw Exception(ExceptionType::ACQUIRE_RESPONSE);
  } 

  this->responses_.resize(responseSize + 100);
  return this->responses_[responseSize];
}

bool HttpResponsePool::ReleaseReusable(HttpResponse& response) {
  response.Reset();
  return true;
}

int HttpResponsePool::SetMaxPoolSize(size_t sizeMax) {
  if (this->responses_.size() > sizeMax) {
    DEBUG_cerr << "Cannot reduce pool size." << endl; 
    return -1;
  } 
  this->responses_.reserve(sizeMax);

  return sizeMax;
}

HttpResponsePool::HttpResponsePool() :
  responses_(50, HttpResponse())
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpResponsePool::~HttpResponsePool() {
  DEBUG_FUNC_START;

}

//HttpResponsePool::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpResponsePool : public HttpResponsePool {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpResponsePool, TESTNAME) {
  MockHttpResponsePool mockHttpResponsePool;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

