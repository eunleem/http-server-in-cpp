// Template
(function (window, $) {
  var TemplateModule = function (name, sectionId) {
    console.log("TemplateModule constructor called.");
    Module.call(this, name, sectionId);

  };

  TemplateModule.prototype = Object.create(Module.prototype);
  TemplateModule.prototype.constructor = TemplateModule;

  TemplateModule.prototype.onEveryMinute = function(time) {

  };

  TemplateModule.prototype.Run = function() {
    console.log("TemplateModule.Run() has been called.");

  };

  window.TemplateModule = TemplateModule;
})(window, $);

// HeaderModule
(function (window, $) {
  var HeaderModule = function (name, sectionId) {
    console.log("HeaderModule constructor called.");
    Module.call(this, name, sectionId);

    this.elems.set("#lblTodayDate", this.section.find("#lblTodayDate"));
  };

  HeaderModule.prototype = Object.create(Module.prototype);
  HeaderModule.prototype.constructor = HeaderModule;

  HeaderModule.prototype.onEveryMinute = function(time) {
    this.elems.get("#lblTodayDate").text(time.getMonthStrAndDate() + " " + time.getWeekday() );
  };


  HeaderModule.prototype.Run = function() {
    console.log("HeaderModule.Run() has been called.");

  };

  window.HeaderModule = HeaderModule;

})(window, $);

// Notification Moduel
(function (window, $) {
  var NotificationModule = function (name, sectionId) {
    console.log("NotificationModule constructor called.");
    Module.call(this, name, sectionId);

    this.config = { "defaultTimeout": 5000 };

    this.numNewNotis = 0;
    this.notis = [];

    this.autoHideTimer = null;

    this.elems.set("IconButton", this.section.find("#boxIcon"));
    this.elems.set("IconAlert", this.section.find("#boxIcon #imgAlertRedSmall"));
    this.elems.set("Logo", this.section.find("#boxIcon #imgLogo"));
    this.elems.set("BoxNotification",  this.section.find("#boxNotification"));
    this.elems.set("MessageList",  this.section.find("#boxMessage ul"));
    this.elems.set("Message",  this.section.find("#boxMessage .wrapMessage"));
    this.elems.set("Close",  this.section.find("#boxNotification .btnClose"));

  };

  NotificationModule.prototype = Object.create(Module.prototype);
  NotificationModule.prototype.constructor = NotificationModule;

  NotificationModule.prototype.onEveryMinute = function(time) { };

  NotificationModule.prototype.onReceiveMessage = function(from, type, message) {
    console.log(this.name + " has received message from " + from + ". type:" + type + " msg: " + message);
    this.AddMessage(from, type, message);
  };

  NotificationModule.prototype.AddMessage = function(from, type, message) {
    var now = new Date();

    var timeStr = now.getTimeStr({
      "showSeconds": true
    });
    var message = $("<li>", {
      "html": message
    });
    var time = $("<time>", {
      "datetime": now.toISOString(),
      "text": timeStr
    });
    message.append(time);

    console.log("MesssageType: " + type);

    if (type === "error" ||
        type.substr(0, 4) === "fail" ||
        type === "bad") {
      message.addClass("errorMessage");
    } else if (type.substr(0, 7) === "success" ||
               type === "good" ||
               type === "ok") {
      message.addClass("successMessage");
    }
    var list = this.elems.get("MessageList")
    if (list.children("li").length >= 1) {
      list.children("li:last").detach();
    }
    list.prepend(message);
    var options = {
      "type": type,
      "timeout": 5000
    };
    this.ShowNotification(options);
  };


  NotificationModule.prototype.ShowNotification = function(options) {
    this.elems.get("BoxNotification").addClass("open");

    var type = "normal";
    var timeout = this.config.defaultTimeout;

    if (typeof options === "undefined") {
      options = {};
    }

    if (typeof options.timeout === "number") {
      timeout = options.timeout;
    }

    if (typeof options.type === "string") {
      type = options.type;
    }

    console.log("Type: " + type);

    if (type === "urgent" ||
        type === "error") {
      this.elems.get("Logo").attr("class", "urgent");
      this.elems.get("IconAlert").attr("class", "active");
      //this.elems.get("IconButton").addClass("blink-flash");
    } else if (type === "success") {
      this.elems.get("IconAlert").attr("class", "");
      this.elems.get("Logo").addClass("good");
    }
    this.HideNotification(timeout);
  };

  NotificationModule.prototype.HideNotification = function(timeout) {
    if (this.autoHideTimer != null) {
      clearTimeout(this.autoHideTimer);
      this.autoHideTimer = null;
    }

    var that = this;
    this.autoHideTimer = setTimeout(function() {
      that.elems.get("BoxNotification").removeClass("open");
      //that.elems.get("IconButton").removeClass("blink-flash");
      that.elems.get("Logo").attr("class", "normal");
      that.elems.get("IconAlert").attr("class", "");
      that.autoHideTimer = null;
    }, timeout);
  };

  NotificationModule.prototype.SetState = function(state, options) {
    if (typeof state !== "string") {
      console.log("typeof state is not string!");
      return;
    }
    state = state.toLowerCase();
    console.log("SetState() called! " + this.state + " -> " + state);

    if (typeof options !== "object") {
      options = { };
    }

    if (state === "normal") {

    } else if (state === "posting") {
    } else {
      console.log("Unhandled state.");
    }

    this.state = state.toLowerCase();
  };


  NotificationModule.prototype.Run = function() {
    console.log("NotificationModule.Run() has been called.");

    this.internals = Internals(this);

  };

  function Internals(module) {
    var section = module.section;
    var elems = module.elems;

    addEventsToElements();

    return this;


    function addEventsToElements() {
      section.on("click", "#boxIcon", function() {
        module.ShowNotification();
      });
      section.on("click", "#boxNotification .btnClose", function() {
        module.HideNotification();
      });
    }
  }

  window.NotificationModule = NotificationModule;
})(window, $);

// JournalModule
(function (window, $) {
  var JournalModule = function (name, sectionId) {
    console.log("JournalModule constructor called.");
    Module.call(this, name, sectionId);

    this.numIdeasToLoad = 10;
    this.currentPage = 0;
    this.autolinker = new Autolinker();

    this.elems.set("WriteTime", this.section.find("#boxJournalWriteTime > time"));
    this.elems.set("Textarea",  this.section.find("#txtJournalWrite"));
    this.elems.set("EntryBox",  this.section.find("#boxJournalEntries"));
    this.elems.set("PostButton",  this.section.find("#btnJournalPost"));
    //this.elems.set("Preview",   this.section.find("#boxJournalWritePreview"));
  };

  JournalModule.prototype = Object.create(Module.prototype);
  JournalModule.prototype.constructor = JournalModule;

  JournalModule.prototype.onEveryMinute = function(time) {
    this.elems.get("WriteTime").text(time.getTimeStr())
      .attr("datetime", time.toISOString());
  };

  JournalModule.prototype.onLeave = function() {
    if (this.elems.get("Textarea").val() !== "") {
      return false;
    }
  };

  JournalModule.prototype.SetState = function(state, options) {
    if (typeof state !== "string") {
      console.log("typeof state is not string!");
      return;
    }
    state = state.toLowerCase();
    console.log("SetState() called! " + this.state + " -> " + state);

    if (typeof options !== "object") {
      options = { };
    }

    var txt = this.elems.get("Textarea");
    var postbtn = this.elems.get("PostButton");

    var editBtns = this.section.find(".journal-entry-edit button.save");

    if (state === "normal") {
      txt.enable();
      if (this.state === "posting") {
        txt.val("");
      }
      txt.removeClass("error");
      postbtn.text(postbtn.data("orgtext"));
      postbtn.enable();
      editBtns.text("save edits");
      editBtns.enable();

      this.section.find("ins,del,time").each(function() {
        var $this = $(this);
        //console.log(this.tagName);
        var time = new Date($this.attr("datetime"));
        var title = "";
        if (this.tagName === "INS") {
          title = "Inserted at ";
        } else if (this.tagName === "DEL") {
          title = "Deleted at ";
        } else if (this.tagName === "TIME") {
          title = "";
        }
        title += time.toString();
        $this.attr("title", title);
      });

      if (this.state === "posting" ||
          this.state === "editing")
      {
        this.internals.GetIdeas(this.numIdeasToLoad, 0);
      }

      setTimeout(function() {
        autosize.update(txt);
      }, 100);

    } else if (state === "posting") {
      txt.disable();
      postbtn.text("posting...");
      postbtn.disable();

    } else if (state === "editing") {
      txt.disable();
      postbtn.disable();
      editBtns.text("saving...");
      editBtns.disable();

    } else if (state === "failed-gettingideas") {

    }

    if (state.substr(0, 4) === "fail") {
      if (state === "failed-posting") {
        txt.addClass("error");
      }
    }

    this.state = state.toLowerCase();
  };

  JournalModule.prototype.Run = function() {
    console.log(this.name + "Module.Run() has been called.");
    if (this.state !== "registered") {
      console.log(this.name + " module is either not registred or already running! Failed to Run the module.");
      return;
    }
    this.internals = Internals(this);
    this.internals.GetIdeas(this.numIdeasToLoad, 0);
  };

  function Internals(module) {
    var section = module.section;
    var elems = module.elems;


    // Register All the Functions to be used out of Internal
    this.GetIdeas = getIdeas;

    addEventsToElements();

    return this;

    function addEventsToElements() {
      console.log('addEvents');

      section.on("click", "#btnJournalPost", postIdea);
      section.on("focus", "#txtJournalWrite", function(event) {
        $(this).removeClass("error");
      });

      section.on("click", "#btnPageFirst", function() {
        module.currentPage = 0;
        getIdeas(module.numIdeasToLoad, 0);
      });

      section.on("click", "#btnPagePrev", function() {
        module.currentPage -= 1;
        var skip = module.currentPage * module.numIdeasToLoad;
        if (skip < 0) {
          module.currentPage = 0;
          skip = 0;
        }
        getIdeas(module.numIdeasToLoad, skip);
      });

      section.on("click", "#btnPageNext", function() {
        module.currentPage += 1;
        var skip = module.currentPage * module.numIdeasToLoad;
        if (skip < 0) {
          skip = 0;
          module.currentPage = 0;
        }
        getIdeas(module.numIdeasToLoad, skip);
      });

      //module.section.on("keyup", "#txtJournalWrite", updatePreview);

      //module.section.on("click", "#btnJournalWriteFontLarger", { command: "increase" }, changeFontSize);
      //module.section.on("click", "#btnJournalWriteFontSmaller", { command: "decrease" }, changeFontSize);
      //module.section.on("click", "#btnJournalWriteFontRegular", { command: "reset" }, changeFontSize);


      //module.section.on("click", "#btnJournalGet", {"count": 20, "skip": 0}, getIdeas);

      //module.section.on("click", "#btnJournalPreview", updatePreview);
    }


    function getIdeas(count, skip) {
      if (typeof count === "undefined" || count === 0) {
        if (typeof this.numIdeasToLoad === "undefined") {
          count = 20;
        } else {
          count = this.numIdeasToLoad;
        }
      }
      if (typeof skip === "undefined") {
        skip = 0;
      }

      var box = elems.get("EntryBox");

      module.SetState("gettingideas");

      $.ajax({
        type: "POST",
        url: "/planner",
        data: {
          "action": "getideas",
          "count": count.toString(),
          "skip": skip.toString()
        }
      }).fail(function() {
        box.prepend("<div class='error'>Failed to get response from server.</div>");

      }).done(function(result) {
        console.log(result);
        if (result["code"] === 0) {
          if (result["ideas"].length === 0) {
            module.currentPage -= 1;
            return;
          }
          var list = EntryList(result["ideas"]);
          if (list == null) {
            module.SetState("failed-gettingideas");
            return;
          }

          box.html("");
          box.append(list);
          module.SetState("normal");
        } else {
          box.prepend("<div class='error'>Failed to get ideas.</div>");
        }
        //$(".entry").last().css("color", "red");
      });
      return;
    }

    function postIdea(event) {
      var txt = elems.get("Textarea");
      var content = EditableToJsonString(txt.val());
      if (content.trim().length === 0) {
        module.SetState("failed-posting");
        module.SendMessage("Notification", "error",
         "Post content cannot be empty!");
        return false;
      }

      module.SetState("posting");

      $.ajax({
        type: "POST",
        url: "/planner",
        data: {
          "action": "postidea",
          "content": content
        }
      })
      .done(function(result) {
        console.log(result);
        if (result.code === 0) {
          module.SetState("normal");
          module.SendMessage("Notification", "success",
            "Journal Entry has been posted!");

        } else if (result.code === -1) {
          module.SendMessage("Notification", "error",
            "Failed Posting");
          module.SetState("normal");

        } else if (result.code === -2) {
          module.SendMessage("Notification", "error",
            "Failed Posting");
          module.SetState("normal");
        } else {
          module.SendMessage("Notification", "error",
            "Failed Posting");
          module.SetState("normal");

        }

      })
      .fail(function() {
        module.SetState("failed-posting");
        module.SendMessage("Notification", "error",
         "Failed to post journal due to connection or server error!");
      });
    }

    /**********************************/

    function JournalEntry (idea) {
      if (typeof idea === "undefined") {
        // To show Structure and Type of idea
        idea = { "ideaid": 0, "content": "Empty", "created": 100000 };
        console.log("JournalEntry(idea): idea is undefined!");
        return;
      }

      var ideaid = idea.ideaid;
      var content = JsonStringToViewFriendly(idea.content);
      //content = module.autolinker.link(content);
      var time = new Date(idea.created);
      var entry = $("<li>", {
        "id": "entry-" + ideaid.toString(),
        "data-ideaid": ideaid
      });

      var boxIdeaId = $("<div>", {
        "class": "journal-entry-id",
        "text": ideaid.toString()
      });

      var boxTime = $("<div>", {
        "class": "journal-entry-time"
      }).append($("<time>", {
        "datetime": time.toISOString(),
        "text": time.getMonthStrAndDate() + " " + time.getTimeStr()
      }));

      var boxContentOriginal = $("<div>", {
        "class": "journal-entry-content-original",
        "html": content,
        "style": "display: none;"
      });

      var boxContent = $("<div>", {
        "class": "journal-entry-content",
        "html": content
      });

      var boxEdit = $("<div>", {
        "class": "journal-entry-edit",
        "style": "display: none;"
      }).append(
        $("<textarea>", {
          "id": "txtJournalEdit" + ideaid.toString(),
          "class": "entryeditor expandable",
          "placeholder": "hehe. editing takes some efforts, doesn't it? XD"
        })
      ).append(
        $("<div>", {
          "class": "editcontrols"
        }).append(
          $("<button>", {
            "class": "save ok",
            "text": "save edits"
          })
        ).append(
          $("<button>", {
            "class": "cancel",
            "text": "Cancel"
          })
        )
      );

      var boxControls = $("<div>", {
        "class": "journal-entry-controls"
      })
      .append($("<a>", {
        "class": "history",
        "text": "history",
        "style": "display: none;"
      }))
      .append($("<button>", {
        "class": "edit",
        "text": "edit"
      }));

      entry.append(boxIdeaId);
      entry.append(boxTime);
      entry.append(boxContent);
      entry.append(boxContentOriginal);
      entry.append(boxEdit);
      entry.append(boxControls);

      return entry;
    };

    function EntryList (ideas) {
      if (typeof ideas === "undefined" ||
          typeof ideas !== "object") {
        console.log("ideas is undefined or not an array");
        return null;
      }

      var list = $("<ul>", {
         "id": "lstJournalEntriesRecent",
         "class": "eventhandler"
       });
      for (var i in ideas) {
        var elem = JournalEntry(ideas[i]);
        list.append(elem);
      }

      list.on("click", ".journal-entry-controls > .edit", function(event) {
        var entry = $(this).closest("li");
        //var boxContent = entry.children(".journal-entry-content");
        changeMode(entry, "editing");
        //li.data("mode", "edit");
        //alert("clicked. id: " + id.toString());
      });

      list.on("click", ".journal-entry-controls > .history", function(event) {
        var entry = $(this).closest("li");
        var ideaid = entry.data("ideaid");
        getHistory(ideaid);
        //var boxContent = entry.children(".journal-entry-content");
        //changeMode(entry, "history");
        //li.data("mode", "edit");
        //alert("clicked. id: " + id.toString());
      });

      list.on("blur", ".journal-entry-edit textarea", function(event) {
        //var entry = $(this).closest("li");
        //changeMode(entry, "normal");
      });

      list.on("click", ".journal-entry-edit button.save", function(event) {
        var entry = $(this).closest("li");
        changeMode(entry, "normal");
        var boxContent = entry.find(".journal-entry-content");
        var boxContentOrg = entry.find(".journal-entry-content-original");
        if (boxContent.html() !== boxContentOrg.html()) {
          var ideaid = entry.data("ideaid");
          var editor = entry.find(".journal-entry-edit .entryeditor");
          editIdea(ideaid, editor);
        } else {
          console.log("Content not edited.");
        }
      });

      list.on("click", ".journal-entry-edit .cancel", function(event) {
        var entry = $(this).closest("li");
        changeMode(entry, "normal");
        var boxContentOrg = entry.find(".journal-entry-content-original");
        var boxContent = entry.find(".journal-entry-content");
        boxContent.html(boxContentOrg.html());
      });

      return list;

      function changeMode(entry, changeTo) {
        if (typeof entry === "undefined") {
          console.log("changeMode(entry, ...): entry cannot be undef!");
          return;
        }
        if (typeof changeTo === "undefined") {
          if (typeof entry.data === "undefined" ||
              entry.data("mode") === "normal" ||
              entry.data("mode") === "") {
            changeTo = "editing";
          } else {
            changeTo = "normal";
          }
        }

        var btn = entry.find(".journal-entry-controls");
        var boxEdit = entry.find(".journal-entry-edit");
        var boxContent = entry.find(".journal-entry-content");

        var txt = boxEdit.find(".entryeditor");

        var ideaid = entry.data("ideaid")

        if (changeTo === "editing") {
          var content = boxContent.html()
          content = ViewFriendlyToEditable(content);
          boxContent.hide();
          boxEdit.show();
          txt.focus();
          txt.val(content);
          autosize(txt);
          btn.hide();
          entry.data("mode", "editing");

        } else {
          var editedContent = txt.val();
          editedContent = EditableToJsonString(editedContent);
          editedContent = JsonStringToViewFriendly(editedContent);
          boxContent.html(editedContent);

          boxContent.show();
          boxEdit.hide();
          btn.show();
          entry.data("mode", "normal");
        }

        return changeTo;
      }

      function getHistory(ideaid) {
        //console.log("EDIT IDEA");
        //console.log(editor);
        $.ajax({
          type: "POST",
          url: "/planner",
          data: {
            "action": "gethistory",
            "ideaid": ideaid
          }
        }).fail(function() {
            module.SendMessage("Notification", "error",
              "Failed to get history!");

        }).done(function(result) {
          console.log(result);
          if (result.code === 0) {
            //module.SendMessage("Notification", "success", "Journal Entry has been edited!");
          }
        }).always(function(){
          //module.SetState("normal");
          //changeMode(ideaid)
        });
      }

      function editIdea(ideaid, editor) {
        if (typeof ideaid === "undefined") {
          console.log("ideaid cannot be undef!");
          return false;
        }
        if (typeof editor === "undefined") {
          console.log("editor cannot be undef!");
          return false;
        }

        var content = editor.val();
        content = EditableToJsonString(content);

        console.log("editIdea. id: " + ideaid + "  c: " + content);

        module.SetState("editing");

        $.ajax({
          type: "POST",
          url: "/planner",
          data: {
            "action": "editidea",
            "ideaid": ideaid,
            "content": content
          }
        })
        .done(function(result) {
          console.log(result);
          if (result.code === 0) {
            module.SendMessage("Notification", "success",
              "Journal Entry has been edited!");
          }
        })
        .fail(function() {
          changeMode(ideaid, "editing");
          module.SendMessage("Notification", "error",
            "Failed to edit entry!");

        })
        .always(function() {
          module.SetState("normal");
        });
      }
    };

    /**********************************/

  } // END OF INTERNAL


  function EditableToJsonString(editableStr) {
    return editableStr.trim().toJsonString();
  }
  function JsonStringToViewFriendly(jsonStr) {
    return jsonStr.fromJsonStringToPrettyHtml();
  }
  function ViewFriendlyToEditable(viewFriendlyStr) {
    return viewFriendlyStr.toEditable();
  }


  // IGNORE FUNCTIONS BELOW
  function getIdea(ideaid) {
    $.ajax({
      type: "POST",
      url: "/planner",
      data: {
        "action": "getidea",
        "ideaid": ideaid
      }
    }).fail(function() {
      alert("Failed.");

    }).done(function(result) {
      console.log(result);
      //var content = JsonStringToViewFriendly(result.content);
      //module.section.find("#boxJournalPosted").html(content);
    });
  }

  function changeFontSize(event) {
    var fontSize = parseInt(txt.css("font-size"));

    if (event.data.command === "reset") {
      fontSize = "2em";
    } else if (event.data.command === "increase") {
      fontSize = (fontSize * 1.1).toString() + "px";
    } else if (event.data.command === "decrease") {
      fontSize = (fontSize * 0.9).toString() + "px";
    } else {
      console.log("Invalid command. command: " + event.data.command);
    }
    txt.css("font-size", fontSize);
    autosize.update(txt);
  }

  function EvaluateTimeForLioTags(content) {
    var now = new Date();
    return String(content.trim())
      .replace(/(\s?)DEL:/g, "$1DEL\[" + now.toISOString() + "\]:")
      .replace(/(\s?)INS:/g, "$1INS\[" + now.toISOString() + "\]:")
      .replace(/:TS:/g, ":TS\[" + now.toISOString() + "\]:");
  }

  function ConvertLioTagsToHtml(content) {
    var now = new Date();
    content = EvaluateTimeForLioTags(content);
    return String(content.trim() + "\n")
      .replace(/\s?Title: (.*?)\n/g, "<h2>$1</h2>")
      .replace(/\s?Subtitle: (.*?)\n/g, "<h3>$1</h3>")
      .replace(/\s?Subsubtitle: (.*?)\n/g, "<h4>$1</h4>")
      .replace(/Center:/g, "<div style='text-align: center;'>")
      .replace(/:Center/g, "</div>")
      .replace(/Indent:/g, "<div class='indent15'>")
      .replace(/:Indent/g, "</div>")
      .replace(/Code:/g, "<code>")
      .replace(/:Code/g, "</code>")
      .replace(/(\s*)PG:/g, "$1<p>")
      .replace(/:PG(\s*)/g, "</p>$1")
      .replace(/BQ:/g, "<blockquote>")
      .replace(/:BQ/g, "</blockquote>")
      .replace(/(\s?)ST:/g, "$1<strong>")
      .replace(/:ST(\s?)/g, "</strong>$1")
      .replace(/(\s+)Cite:/g, "$1<cite>")
      .replace(/:Cite(\s+)/g, "</cite>$1")
      .replace(/:TS\[(.*?)\]:/g, "<time datetime='$1'></time>")
      .replace(/(\s?)DEL\[(.*?)\]:/g, "$1<del datetime='$2'>")
      .replace(/(\s?)INS\[(.*?)\]:/g, "$1<ins datetime='$2'>")
      .replace(/<time datetime='(.*?)'>/g, IsoStringTimeToLocalTime)
      .replace(/<del datetime='(.*?)'>/g, IsoStringTimeToLocalTime)
      .replace(/<ins datetime='(.*?)'>/g, IsoStringTimeToLocalTime)
      .replace(/:DEL(\s?)/g, "</del>$1")
      .replace(/:INS(\s?)/g, "</ins>$1")
      .replace(/(\s?)EM:/g, "$1<em>")
      .replace(/:EM(\s?)/g, "</em>$1")
      .replace(/(\s+)DFN:/g, "$1<dfn>")
      .replace(/:DFN(\s+)/g, "</dfn>$1")
      .replace(/(\s+)IT:/g, "$1<i>")
      .replace(/:IT(\s+)/g, "</i>$1")
      .replace(/RED:/g, "<span class='color-red'>")
      .replace(/:RED/g, "</span>")
      .replace(/ORG:/g, "<span class='color-orange'>")
      .replace(/:ORG/g, "</span>")
      .replace(/(\s?)li:\s?/g, "$1<li>")
      .replace(/\s?:li(\s?)/g, "</li>$1")
      .replace(/ol:\s?/g, "<ol>")
      .replace(/\s?:ol/g, "</ol>")
      .replace(/ul:\s?/g, "<ul>")
      .replace(/\s?:ul/g, "</ul>")
      //.replace(/[\s]*<(h|div|blockquote|p|ol|ul|li|code)(.*?)>\s*/g, "<$1$2>")
      //.replace(/[\s]*<\/(h|div|blockquote|p|ol|ul|li|code)>\s*/g, "</$1>")
      .replace(/\n/g, "<br>");

    function IsoStringTimeToLocalTime(match, p1) {
      var date = new Date(p1);
      var userFriendlyDateTime = date.toDateString() + " " + date.getTimeStr();
      if(match.substr(1, 3) === "del") {
        return match.slice(0, -1) + " title='Deleted on " + userFriendlyDateTime + "'>";
      } else if (match.substr(1, 3) === "ins") {
        return match.slice(0, -1) + " title='Inserted on " + userFriendlyDateTime + "'>";
      } else if (match.substr(1, 4) === "time") {
        return match + "<i>" + userFriendlyDateTime + "</i>";
      }
      console.log("Unhandled.")
      return match;
    }
  }



  window.JournalModule = JournalModule;
})(window, $);
