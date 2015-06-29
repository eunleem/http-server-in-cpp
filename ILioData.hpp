#ifndef _ILIODATA_HPP_
#define _ILIODATA_HPP_
/*
  Name
    ILioData

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 15, 2015
  
  History
    June 09, 2015
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"



#include "Sessions.hpp"
#include "Lives.hpp"
#include "Ideas.hpp"

#include "Journal.hpp"

#include "liolib/Openable.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


class ILioData : public Openable {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define ILIODATA_EXCEPTION_MESSAGES \
  "ILioData Exception has been thrown."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********


  ILioData(std::string dirPath = "./data/");
  ~ILioData();


  Life& SignUp();

  Life& Login(std::string dna, std::string code);

  //Invitation& AddInvitation();
  //Invitation& UpdateInvitation();

  //Idea& GetIdeaIds();
  //Idea& GetIdeaById();

  //ideaid_t PostIdea();

  //Idea& UpdateIdea();


protected:
  Life& CreateLife();
  Life& GetLifeByDnaAndSecretCode(std::string dna, std::string code);
  
private:
  std::string dirPath_;

  Journal journal_;
  Sessions sessions_;
  
};

}

#endif

