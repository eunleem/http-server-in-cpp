/*
  Name
    manager.js

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    July 24, 2015

  History
    July 24, 2015
      Created

  ToDos



  Milestones
    1.0


  Learning Resources
    http://

  Copyright (c) All rights reserved to LIFEINO.
*/


(function (window, $) {

  var Manager = function () {
    console.log("Manager constructor called!");

    this.modules = new Map();
    this.timer = new Date();
    this.timerOnEveryMinute = null;

    this.autolinker = new Autolinker();

    this.engagingModule = "";


    var that = this;

    this.DeliverMessage = function(from, to, type, message) {
      console.log("Manager is delivering message!");
      if (typeof that.modules.get(to) !== "object") {
        console.error("Could not find receiving module named " + to);
        return false;
      }
      that.modules.get(to).onReceiveMessage(from, type, message);
      return true;
    };
  };


  Manager.prototype.Register = function(module) {
    console.log("Register() has been called.");
    console.log("Is Mod instance of Module? " + (module instanceof Module).toString());
    if (module instanceof Module === false) {
      console.error("Registering object that is not instanceof Module");
      return;
    }

    if (typeof this.modules[module.name] !== "undefined") {
      console.error("Registering object that has same name!");
      return;
    }

    module.state = "registered";
    //module.autolinker = this.autolinker;

    this.modules.set(module.name, module);

    module.sendMessage = this.DeliverMessage;
  };


  Manager.prototype.Run = function(mode) {

    if (this.modules.length === 0) {
      console.error("Have no modules to register.");
      return;
    }

    var that = this;
    that.onEveryMinute();
    setTimeout(function() {
      that.onEveryMinute();
      that.timerOnEveryMinute = setInterval(function() {
        that.onEveryMinute();
      }, 60 * 1000);
    }, (60 - that.timer.getSeconds()) * 1000);


    console.time("Run Modules");
    this.modules.forEach(function(module, name) {
      console.log("Running " + name.toString() + " Module!");
      console.time("Run Module " + name.toString());
      var okToRun = (module.checkRequiredElements() === 0);
      if (okToRun === true) {
        module.Run();
      }
      console.timeEnd("Run Module " + name.toString());
    });
    console.timeEnd("Run Modules");
    
    $(window).on("beforeunload", function() {
      var flag = true;
      that.modules.forEach(function(module) {
        if (module.onLeave() === false) {
          flag = false;
        }
      });
      if (flag === false) {
        return "Unsaved Changes";
      }
    });
  };

  Manager.prototype.Stop = function() {
    console.log("Manager.Stop() has been called.");
    clearInterval(this.timerOnEveryMinute);

    this.modules.forEach(function(module, name) {
      console.log("Stopping " + name.toString() + " Module!");
      module.Stop();
    });
  };

  Manager.prototype.onEveryMinute = function() {
    console.log("onEveryMinute() has been called!");

    this.timer = new Date();

    var that = this;
    this.modules.forEach(function(module) {
      module.onEveryMinute(that.timer);
    });
  };


  window.Manager = Manager;
})(window, $);






(function (window, $) {
  var Module = function (name, sectionId) {
    console.log("Module constructor called.");
    if (typeof name !== "string") {
      console.log("ERR: Module must have a name.");
      return;
    }
    if (typeof sectionId !== "string") {
      console.log("ERR: Module must have a name.");
      return;
    }

    //this.state = "Uninitialized";
    this.name = name;
    this.state = "starting";

    if (sectionId.charAt(0) != '#') { sectionId = "#" + sectionId; }
    this.section = $(sectionId);

    this.elems = new Map();
    this.elems.set("section", this.section);

  };

  Module.prototype.onEveryMinute = function(time) {
    console.log("Module has not implemented onEveryMinute function.");
  };

  Module.prototype.onLeave = function() {
    console.log("Module has not implemented onLeave function.");
    return true;
  };

  Module.prototype.onReceiveMessage = function(from, type, message) {
    console.log(this.name + " has received message from " + from + ". type:" + type + " msg: " + message);
  };

  Module.prototype.checkRequiredElements = function() {
    console.log("checkRequiredElements() has been called!");

    var errflag = false;
    this.elems.forEach(function(element) {
      if (element.length === 0) {
        console.log("Required element " + element.selector + " not found.");
        errflag = true;
      }
    });

    if (errflag === true) {
      return -1;
    }

    return 0;
  };

  Module.prototype.SendMessage = function(to, type, message) {
    console.log(this.name + " is sending message to " + to + ". type:" + type + " msg: " + message);
    this.sendMessage(this.name, to, type, message);
  };

  Module.prototype.Run = function() {
    console.log("Module.Run() has been called.");
  };

  Module.prototype.Stop = function() {
    console.log("Module.Stop() has been called.");
  };


  //Module.fn = Module.prototype;
  window.Module = Module;

})(window, $);
