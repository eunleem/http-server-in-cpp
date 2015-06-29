/*
  Name
    LetsEngine

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jul 22, 2014
  
  History
    June 12, 2014
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

(function (window, $) {
  var LetsEngine = function () {
    return this;
  };

  LetsEngine.prototype.init = function() {
  };

  LetsEngine.Data = {
    Me: {
      IsLoggedIn: false,
      UserId: -1,
      Code: null
    }
  };

  LetsEngine.prototype.UserModule = function(config) {
    var defConfig = {

    };

    if (!config) {
      config = {};
    } 

    function UserModule() {
      this.config = $.extend({}, defConfig, config);
      //setCookieValue("code", "test", 1);
    }

    var me = LetsEngine.Data.Me; // Class variable.

    UserModule.prototype.init = function() {
    };


    UserModule.prototype.GetCodeFromCookie = function() {
      me.Code = getCookieValueByName("userCode");
      me.UserId = getCookieValueByName("userId");

      if (me.Code == null || me.UserId == null) {
        // No code available.
        console.log("No code.");
        return false;
      }

      console.log("Me.Code: " + me.Code);
      return true;
    };

    UserModule.prototype.Login = function(loggedInAction, notLoggedInAction, failAction) {
      if (me.IsLoggedIn == true) {
        console.log("Already logged in.");
        return true;
      } 

      if (me.Code == null || me.UserId == null) {
        console.log("Code is not available.");
        return false;
      } 

      // Not logged in.
      $.ajax({
        type: "POST",
        url: "/login",
        data: { "userId": + me.UserId,
                "userCode": + me.Code }

      }).fail(function() {
        failAction();

      }).done(function(result) {
        console.log(result);
        var message = "default message";
        if (result["code"] == -1) {
          notLoggedInAction();
          
        } else if (result["code"] == -2) {
          notLoggedInAction();

        } else if (result["code"] == 0) {
          loggedInAction();

        } 

      });

    };

    try {
      var um = new UserModule();
      return um;

    } catch(ex) {
      console.log("Failed to allocate UserModule.");
      return null;

    }

  };

  LetsEngine.prototype.Test = function(config) {
    var defConfig = {
      time: 200
    };
  };

  LetsEngine.prototype.ContentModule = function(config) {
    var defConfig = {

    };

    if (!config) {
      config = {};
    } 

    function ContentModule() {
      this.config = $.extend({}, defConfig, config);
    }

    var me = LetsEngine.Data.Me; // Class variable.

    ContentModule.prototype.Post = function() {
      var content = $("#txtPost").val();
      if (content == "") {
        alert("Content Empty!");
        return;
      } 

      $.ajax({
        type: "POST",
        url: "/post",
        data: { "content": content }

      }).fail(function() {
        //error();

      }).done(function(result) {
        console.log(result);
        var message = "default message";
        if (result["code"] == -1) {
          //failed();
          
        } else if (result["code"] == -2) {
          //failed();

        } else if (result["code"] == 0) {
          //successful();

        } 

      });
    };

    ContentModule.prototype.GetTeamContents = function() {
    };

    try {
      var module = new ContentModule();
      return module;

    } catch(ex) {
      console.log("Failed to allocate ContentModule.");
      return null;
    }
  };

  LetsEngine.fn = LetsEngine.prototype;
  window.$lets = LetsEngine;
})(window, $);
