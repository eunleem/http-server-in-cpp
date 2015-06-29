#include "Topics.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Topics::Exception::exceptionMessages_[] = {
  TOPICS_EXCEPTION_MESSAGES
};
#undef TOPICS_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Topics::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Topics::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Topics::ExceptionType
Topics::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Topics::Topics() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

Topics::~Topics() {
  DEBUG_FUNC_START;
}


const Topics::Topic& Topics::GetTopicById(uint32_t id) {
  auto itr = this->topicById.find(id);
  if (itr == this->topicById.end()) {
    // Not Found
    DEBUG_cerr << "Topic not found. id: " << id << endl; 
    throw Exception(ExceptionType::NOT_FOUND);
  }

  return (*itr).second;
}
const Topics::Topic& Topics::CreateNew(string name, string description) {
  Topic&& newTopic = Topic();
  this->summary_.lastId += 1;
  newTopic.id = this->summary_.lastId;
  newTopic.name = name;
  newTopic.description = description;
  newTopic.created = system_clock::now();

  this->topicById[newTopic.id] = newTopic;

  return this->topicById[newTopic.id];
}

//Topics::
//Topics::
//Topics::
//Topics::
//Topics::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockTopics : public Topics {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(Topics, TESTNAME) {
  MockTopics mockTopics;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

