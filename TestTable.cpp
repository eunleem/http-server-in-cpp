#include "TestTable.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
TestTable::Exception::exceptionMessages_[] = {
  TESTTABLE_EXCEPTION_MESSAGES
};
#undef TESTTABLE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


TestTable::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
TestTable::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const TestTable::ExceptionType
TestTable::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 



//TestTable::

}

#if _UNIT_TEST

#include <iostream>

using namespace lio;
int main() {
  TestTable table;
  table.Open();

  TestRow row;
  row.id = 1;
  row.name = "I am the KING";
  row.num = 12;
  table.AddRow(row);
  row.id = 3;
  row.name = "GOOGLE";
  row.num = 17;
  table.AddRow(row);

  table.Close();

  table.Open();
  TestRow* rowa = table.GetRowById(1);
  if (rowa != nullptr) {
    DEBUG_cout << "Row Name: " << rowa->name << endl; 
    DEBUG_cout << "Row Num: " << (int)rowa->num << endl; 
  } 

  rowa = table.GetRowById(3);
  if (rowa != nullptr) {
    DEBUG_cout << "Row Name: " << rowa->name << endl; 
    DEBUG_cout << "Row Num: " << (int)rowa->num << endl; 
  } 

  rowa = table.GetRowById(4);
  if (rowa != nullptr) {
    DEBUG_cout << "Row Name: " << rowa->name << endl; 
    DEBUG_cout << "Row Num: " << (int)rowa->num << endl; 
  } 

  //table.Close();

  return 0;
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

