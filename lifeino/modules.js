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
    //this.elems.set("Preview",   this.section.find("#boxJournalWritePreview"));
  };

  JournalModule.prototype = Object.create(Module.prototype);
  JournalModule.prototype.constructor = JournalModule;

  JournalModule.prototype.onEveryMinute = function(time) {
    this.elems.get("WriteTime").text(time.getTimeStr());
  };

  JournalModule.prototype.Run = function() {
    console.log("JournalModule.Run() has been called.");

    addEventHandlers(this);

    getIdeas(10, 0);
  };


  function postIdea(event) {
    var txt = $("#sect-journal #boxJournal textarea");
    // var txt = this.elems.get("Textarea");
    //var content = Escape(txt.val());
    var content = EditableToJsonString(txt.val());
    if (content.length === 0 ||
        content === "write here...")
    {
      alert("Cannot be Empty.")
      return false;
    }
    
    console.log(content);
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
        afterPost();
      }
    });
  }

  function afterPost() {
    var txt = $("#sect-journal #boxJournal textarea");
    txt.val("");
    txt.blur();
    getIdeas(5, 0);
  }

  function getIdeas(count, skip) {
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
      alert("Failed to get content.");

    }).done(function(result) {
      console.log(result);
      if (result["code"] === 0) {
        var list = $("<ul>", {"id": "listJournalEntriesRecent"});
        for (var i in result["ideas"]) {
          var elem = createJournalEntry(result["ideas"][i]);
          list.append(elem);
        }
        var box = $("#sect-journal #boxJournalPosted");
        box.html("");
        box.append(list);
      } else {
        alert("return code other than 0");
      }
      //$(".entry").last().css("color", "red");
    });
  }

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

  function createJournalEntry(idea) {
    if (typeof idea === "undefined") {
      // To clarify DataStructures and Types of idea
      idea = { "ideaid": 0, "content": "Empty", "created": 100000 };
    }

    var id = idea.ideaid;
    console.log(idea.content);
    var content = JsonStringToViewFriendly(idea.content);
    console.log(idea.content);
    var time = new Date(idea.created);

    var entry = $("<li>", {
      "id": "entries-" + id.toString()
    });


    var boxId = $("<div class='journal-entry-id'>" + id.toString() + "</div>");
    var boxTime = $("<div class='journal-entry-time'><time>" +
                  time.getMonthStr() + " " + time.getDatePad() + " " +
                  time.getTimeStr() + "</time></div>");
    var boxContent = $("<div class='journal-entry-content'>" + content + "</div>");
    entry.append(boxId);
    entry.append(boxTime);
    entry.append(boxContent);

    return entry;
  }
  function EditableToJsonString(editableStr) {
    return editableStr.trim().toJsonString();
  }
  function JsonStringToViewFriendly(jsonStr) {
    return jsonStr.fromJsonStringToPrettyHtml();
  }
  function ViewFriendlyToEditable(viewFriendlyStr) {
    return viewFriendlyStr.toEditable();
  }

  function addEventHandlers(module) {
    var txt = module.elems.get("Textarea");

    //module.section.on("keyup", "#txtJournalWrite", updatePreview);

    //module.section.on("click", "#btnJournalWriteFontLarger", { command: "increase" }, changeFontSize);
    //module.section.on("click", "#btnJournalWriteFontSmaller", { command: "decrease" }, changeFontSize);
    //module.section.on("click", "#btnJournalWriteFontRegular", { command: "reset" }, changeFontSize);

    module.section.on("click", "#btnJournalPost", postIdea);

    //module.section.on("click", "#btnJournalGet", {"count": 20, "skip": 0}, getIdeas);

    //module.section.on("click", "#btnJournalPreview", updatePreview);



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
  }

  window.JournalModule = JournalModule;
})(window, $);
