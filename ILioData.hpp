#ifndef _ILIODATA_HPP_
#define _ILIODATA_HPP_
/*
  Name
    ILioData

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 31, 2015
  
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
#include "Invitations.hpp"
#include "Ideas.hpp"
#include "Contents.hpp"

//#include "Journal.hpp"

#include "liolib/Openable.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {


class ILioData : public Openable {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  SIGNUP,
  LOGIN
};
#define ILIODATA_EXCEPTION_MESSAGES \
  "ILioData Exception has been thrown.", \
  "SignUp failed.", \
  "Login failed."

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

  Invitation& AddInvitation(
      const std::string& description,
      size_t numTickets,
      datetime expiration);

  std::pair<Invitation&, std::string> GetInvitationByCode(const std::string& invitcode);

  std::pair<const Life*, std::string> SignUp(std::string invitcode);

  std::pair<std::string, Session&> Login(std::string dna, std::string code);

  lifeid_t GetLifeIdBySessionId(std::string session);

  Life& GetLifeById(lifeid_t id);


  ideaid_t PostIdea(lifeid_t lifeId, std::string& content,
                    Idea::Type type = Idea::Type::GENERAL,
                    Idea::Permission perm = Idea::Permission::PRIVATE);

  bool UpdateIdea(ideaid_t ideaId, std::string& content,
                  Idea::Type type = Idea::Type::GENERAL,
                  Idea::Permission perm = Idea::Permission::PRIVATE);

  std::vector<Idea*> GetIdeas(size_t count, size_t skip = 0);

  const Idea* GetIdeaById(ideaid_t id);
  const Content* GetContentById(contentid_t id);

  bool IsAdmin(const std::string& session);

  const std::unordered_map<uint32_t, Invitation>& GetInvitations() const;

  const std::unordered_map<lifeid_t, Life>& GetLives() const;
protected:
  Life& CreateLife();
  Life& GetLifeByDnaAndSecretCode(std::string dna, std::string code);

  bool open() override;
  bool close() override;
  
private:
  std::string dirPath_;

  Sessions        sessions_;
  Lives           lives_;
  Invitations     invitations_;

  Ideas           ideas_;
  Contents        contents_;
  
};

}

#endif

