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

    addEventHandler(this);
  };

  function addEventHandler(that) {
  }

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

    addEventHandler(this);
  };

  function addEventHandler(that) {
  }

  window.HeaderModule = HeaderModule;

})(window, $);


// JournalModule
(function (window, $) {
  var JournalModule = function (name, sectionId) {
    console.log("JournalModule constructor called.");
    Module.call(this, name, sectionId);

    this.elems.set("WriteTime", this.section.find("#boxJournalWriteTime > time"));
    this.elems.set("Textarea",  this.section.find("#txtJournalWrite"));
    this.elems.set("EntryBox",  this.section.find("#boxJournalEntries"));
    //this.elems.set("Preview",   this.section.find("#boxJournalWritePreview"));
  };

  JournalModule.prototype = Object.create(Module.prototype);
  JournalModule.prototype.constructor = JournalModule;

  JournalModule.prototype.onEveryMinute = function(time) {
    this.elems.get("WriteTime").text(time.getTimeStr());
  };

  JournalModule.prototype.Run = function() {
    console.log("JournalModule.Run() has been called.");


    var internals = Internals(this);
    internals.GetIdeas(5, 0);


  };

  function Internals(module) {
    var section = module.section;
    var elems = module.elems;

    addEventsToElements();

    this.GetIdeas = getIdeas;
    this.GetIdeas(5, 0);

    return this;

    function addEventsToElements() {
      console.log('addEvents');
      section.on("click", "#btnJournalPost", postIdea);

      //module.section.on("keyup", "#txtJournalWrite", updatePreview);

      //module.section.on("click", "#btnJournalWriteFontLarger", { command: "increase" }, changeFontSize);
      //module.section.on("click", "#btnJournalWriteFontSmaller", { command: "decrease" }, changeFontSize);
      //module.section.on("click", "#btnJournalWriteFontRegular", { command: "reset" }, changeFontSize);


      //module.section.on("click", "#btnJournalGet", {"count": 20, "skip": 0}, getIdeas);

      //module.section.on("click", "#btnJournalPreview", updatePreview);
    }


    function getIdeas(count, skip) {
      var box = elems.get("EntryBox");

      if (typeof count === "undefined" || count === 0) {
        count = 20;
      }
      if (typeof skip === "undefined") {
        skip = 0;
      }

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
          var list = createEntryList(result["ideas"]);
          if (list != null) {
            box.html("");
            box.append(list);
          } else {
            box.html("ERROR CREATING LIST");
          }

        } else {
          box.prepend("<div class='error'>Failed to get ideas.</div>");
        }
        //$(".entry").last().css("color", "red");
      });
      return;

      function createEntryList(ideas) {
        console.log("typeof ideas " + typeof ideas);
        if (typeof ideas === "undefined" ||
            typeof ideas !== "object") {
          console.log("ideas is undefined or not an array");
          return null;
        }

        var list = $("<ul>", {"id": "lstJournalEntriesRecent"});
        for (var i in ideas) {
          var elem = createJournalEntry(ideas[i]);
          list.append(elem);
        }

        list.on("click", ".journal-entry-controls > a", function(event) {
          var id = $(this).data("ideaid");
          alert("clicked. id: " + id.toString());
        });
        return list;

        function createJournalEntry(idea) {
          if (typeof idea === "undefined") {
            // To clarify DataStructures and Types of idea
            idea = { "ideaid": 0, "content": "Empty", "created": 100000 };
          }

          var id = idea.ideaid;
          var content = JsonStringToViewFriendly(idea.content);
          var time = new Date(idea.created);
          var entry = $("<li>", {
            "id": "entries-" + id.toString()
          });

          var boxId = $("<div class='journal-entry-id'>" + id.toString() + "</div>");
          var boxTime = $("<div class='journal-entry-time'><time>" +
                        time.getMonthStr() + " " + time.getDatePad() + " " +
                        time.getTimeStr() + "</time></div>");
          var boxContent = $("<div class='journal-entry-content'>" + content + "</div>");

          var boxControls = $("<div class='journal-entry-controls'></div>");
          var btnEdit = $("<a data-ideaid='" + id.toString() + "'>edit</a>");
          boxControls.append(btnEdit);

          entry.append(boxId);
          entry.append(boxTime);
          entry.append(boxContent);
          entry.append(boxControls);

          return entry;
        }
      }
    }

    function postIdea(event) {
      var txt = elems.get("Textarea");
      var content = EditableToJsonString(txt.val());
      if (content.length === 0 ||
          content === "write here...")
      {
        alert("Cannot be Empty.")
        return false;
      }
      $.ajax({
        type: "POST",
        url: "/planner",
        data: {
          "action": "postidea",
          "content": content
        }
      }).fail(function() {
        alert("Failed.");

      }).done(function(result) {
        console.log(result);
        if (result.code === 0) {
          txt.val("");
          txt.blur();
          autosize.update(txt);
          getIdeas(5, 0);
        }
      });
    }
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
