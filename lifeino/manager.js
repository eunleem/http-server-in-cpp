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

    this.engagingModule = "";


    var that = this;

    this.DeliverMessage = function(from, to, type, message) {
      console.log("Manager is delivering message!");
      if (typeof that.modules.get(to) !== "object") {
        console.log("Could not find receiving module named " + to);
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
      console.log("ERR: Registering object that is not instanceof Module");
      return;
    }

    if (typeof this.modules[module.name] !== "undefined") {
      console.log("ERR: Registering object that has same name!");
      return;
    }

    this.modules.set(module.name, module);

    module.SendMessage = this.DeliverMessage;
  };


  Manager.prototype.Run = function(mode) {

    if (this.modules.length === 0) {
      console.log("Have no modules registered.");
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

    this.modules.forEach(function(module, name) {
      console.log("Running Module! Module Name: " + name.toString());
      var okToRun = (module.checkRequiredElements() === 0);
      if (okToRun === true) {
        module.Run();
      }
    });
  };

  Manager.prototype.Stop = function() {
    console.log("Manager.Stop() has been called.");
    clearInterval(this.timerOnEveryMinute);
  };

  Manager.prototype.onEveryMinute = function() {
    console.log("onEveryMinute() has been called!");

    this.timer = new Date();

    var that = this;
    this.modules.forEach(function(mod) {
      mod.onEveryMinute(that.timer);
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

    //this.status = "Uninitialized";
    this.name = name;

    if (sectionId.charAt(0) != '#') { sectionId = "#" + sectionId; }
    this.section = $(sectionId);

    this.elems = new Map();
    this.elems.set("section", this.section);

  };

  Module.prototype.onEveryMinute = function(time) {
    console.log("Module has not implemented onEveryMinute function.");
  };

  Module.prototype.onReceiveMessage = function(from, type, message) {
    console.log(this.name + " has received message from " + from + ". type:" + type + " msg: " + message);
  };


  //Module.prototype.SendMessage = function() {
    //console.log("ERR: SendMessage() must be replaced by Manager's by registering.");
  //};


  Module.prototype.Run = function() {
    console.log("Module.Run() has been called.");
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

  //Module.fn = Module.prototype;
  window.Module = Module;

})(window, $);



