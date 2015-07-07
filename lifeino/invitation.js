/*
  Name
    invitation.js

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    December 25, 2014
  
  History
    December 25, 2014
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/


(function (window, $) {

  var Invitation = function () {
    return this;
  };

  Invitation.prototype.Genesis = function(config) {
    var defConfig = { };

    if (typeof config == "undefined") {
      config = {};
    } 

    function Genesis() {
      this.config = $.extend({}, defConfig, config);
    }

    Genesis.prototype.Initialize = function() {
      console.log("GEN INITIALZE");
      // Caching jQuery objects.
      jGet("#btnInvitationOpen");
      jGet("#lblInvitationOpenMessage");
      jGet("#txtInvitationCode");

      var querystring = getUrlVars();
      var code = querystring["code"];
      if (typeof code !== "undefined") {
        jGet("#txtInvitationCode").val(code);
      } 
    };

    try {
      var module = new Genesis();
      return module;

    } catch(ex) {
      console.log("Failed to allocate Module.");
      return null;
    }
  };

  Invitation.fn = Invitation.prototype;
  window.Invitation = Invitation;

})(window, $);


var currentMessage;
var msgTimers = {};

$(function() {

  var txtInvitationCodeDefTxt = jGet("#txtInvitationCode").val();
  jGet("#txtInvitationCode").focus(function() {
    if (jGet("#txtInvitationCode").val() == txtInvitationCodeDefTxt) {
      jGet("#txtInvitationCode").val("");
    } 
  });

  jGet("#txtInvitationCode").blur(function() {
    if (jGet("#txtInvitationCode").val().trim() == "") {
      jGet("#txtInvitationCode").val(txtInvitationCodeDefTxt);
    } 
  });

  currentMessage = jGet("#lblInvitationOpenMessage");

  jGet("#sect-cover").on("click", "#btnInvitationOpen", function() {
    openInvitation(jGet("#txtInvitationCode").val());
  });

  jGet("#sect-invitation").on("click", "#btnInvitationJoin", function() {
    join(jGet("#txtInvitationCode").val());
  });

  jGet("#sect-newlife").on("click", "#btnWhyTempAccount", function() {
    if (jGet("#lblWhyTempAccount").css("display") == "none") {
      jGet("#lblWhyTempAccount").show().addClass("fade-in");
    }
  });

  jGet("#sect-newlife").on("click", "#btnAccountStart", function() {
    document.location.href = "/main";
  });
});

function showMessage(target, message, type) {
  target.removeClass("disabled");
  if (type == "error") {
    target.removeClass("successMessage");
    target.addClass("errorMessage");
  } else if (type == "success") {
    target.removeClass("errorMessage");
    target.addClass("successMessage");
  } else {
    target.removeClass("errorMessage");
    target.removeClass("successMessage");
  }

  target.html(message);

  var messageDisplayTime = message.length * 100;
  if (messageDisplayTime < 5000) {
    messageDisplayTime = 5000;
  } 

  clearTimeout(msgTimers[target.selector]);
  msgTimers[target.selector] = setTimeout(function() {
    target.addClass("disabled");
  }, messageDisplayTime);
}


function openInvitation(code) {
  //console.log("code " + code);
  if (code.length < 10) {
    // invalid code
    invalidCode("코드가 너무 짧아요!")
    return;
  } 

  disableButton(jGet("#btnInvitationOpen"), "OPENING...");

  $.ajax({
    type: "POST",
    url: "/invitation",
    data: {
      "action": "open",
      "code": code
    }

  }).fail(function(){
    invalidCode("서버와의 연결이 원활치 않거나 웹사이트에 문제가 있는 것 같습니다.<br>죄송하지만 몇 분 후에 다시 시도해주시겠어요?");

  }).done(function(result) {
    console.log(result);
    if (result["code"] == -1) {
      invalidCode("초대장 코드가 올바르지 않습니다.");

    } else if (result["code"] == -2) {
      invalidCode("초대장 코드는 올바르나 모든 초대장이 소모되었습니다.<br>너무 속상해 하지 마세요! 다음에 또 기회가 있을거에요!");

    } else if (result["code"] == -3) {
      invalidCode("초대장 코드는 올바르나 마감시간이 지났습니다.<br>너무 속상해 하지마세요! 다음에 또 기회가 있을거에요!");

    } else if (result["code"] == 0) {
      validCode("환영합니다! 잠시후 초대장을 엽니다!");
      currentMessage = jGet("#lblInvitationJoinMessage");
      setTimeout(function() {
        showInvitation(result["description"]);
      }, 1000);

      setTimeout(function() {
        enableButton(jGet("#btnInvitationOpen"));
      }, 2000);

    } else {
      invalidCode("서버에서 예상치 못한 문제가 발생한듯 합니다.<br> 죄송하지만 몇 분 후에 웹브라우져를 다시 실행 후 시도해주세요.");

    }
  }).always(function() {
  });
}

function invalidCode(message) {
  showMessage(currentMessage, message, "error");
  enableButton(jGet("#btnInvitationOpen"));
}

function validCode(message) {
  showMessage(currentMessage, message, "success");
}

function showInvitation(description) {
  jGet("#sect-cover").addClass("fade-out");

  if (description == "") {
    description = "No Description has been provided for this Invitation..."
  } 

  jGet("#lblInvitationDescription").html(description);
  
  // Callback when CSS3 transition finishes.
  // #REF: http://stackoverflow.com/questions/9255279/callback-when-css3-transition-finishes
  jGet("#sect-cover").bind("animationend webkitAnimationEnd oAnimationEnd MSAnimationEnd", function() {
    jGet("#sect-cover").hide();
    jGet("#sect-invitation").show();
    jGet("#sect-invitation").addClass("fade-in");
  });
}

function join(code) {

  disableButton(jGet("#btnInvitationJoin"), "JOINING...");

  $.ajax({
    type: "POST",
    url: "/invitation",
    data: {
      "action": "signup",
      "code": code
    }

  }).fail(function(){
    invalidCode("웹사이트와의 연결이 원활치않거나 웹사이트에 문제가 있는 것 같습니다.<br>죄송하지만 몇 분 후에 다시 시도해주시겠어요?");

  }).done(function(result) {
    //console.log(result);
    if (result["code"] == 0) {
      validCode("진짜 환영합니다! 임시계정이 만들어졌어요!");
      setTimeout(function() {
        //saveSessionId(result["sid"]);
        showKey(result["account_code"]);
        enableButton(jGet("#btnInvitationJoin"));
      }, 1000);

    } else if (result["code"] == -1) {
      invalidCode("초대장 코드가 올바르지 않습니다.");
      enableButton(jGet("#btnInvitationJoin"));

    } else if (result["code"] == -2) {
      invalidCode("초대장 코드는 올바르나 모든 초대장이 소모되었습니다.<br>너무 속상해 하지 마세요! 다음에 또 기회가 있을거에요!");
      enableButton(jGet("#btnInvitationJoin"));

    } else if (result["code"] == -3) {
      invalidCode("초대장 코드는 올바르나 마감시간이 지났습니다.<br>너무 속상해 하지마세요! 다음에 또 기회가 있을거에요!");
      enableButton(jGet("#btnInvitationJoin"));

    } else {
      invalidCode("웹사이트에서 예상치 못한 문제가 발생한듯 합니다.<br> 죄송하지만 몇 분 후에 웹브라우져를 다시 실행 후 시도해주세요.");
      enableButton(jGet("#btnInvitationJoin"));

    }
  }).always(function() {
  });
}

function saveSessionId(sid) {
  setCookieValue("sid", sid, 14);
}

function showKey(tempkey) {
  //console.log(tempkey);
  jGet("#sect-invitation").removeClass("fade-in");
  jGet("#sect-invitation").addClass("fade-out");
  jGet("#sect-invitation").bind("animationend webkitAnimationEnd oAnimationEnd MSAnimationEnd", function() {
    jGet("body > header").hide();
    jGet("#sect-invitation").hide();
    jGet("#sect-newlife").show();
    jGet("#sect-newlife").addClass("fade-in");
    var grouped = tempkey.substr(0, 4) + " ";
    grouped += tempkey.substr(4, 4) + " ";
    grouped += tempkey.substr(8, 4) + " ";
    grouped += tempkey.substr(12, 4) + " ";
    grouped += tempkey.substr(16, 4) + " ";
    grouped += tempkey.substr(20, 4) + " ";
    grouped += tempkey.substr(24, 4);
    jGet("#lblAccountTempkey").text(grouped);
  });

}

