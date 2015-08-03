var gNow = new Date();


$(function() {
  updateEveryMinute();
  setInterval(updateEveryMinute, 60 * 1000);
});


function updateEveryMinute() {
  gNow = new Date();

  updateTodayDateAndDaysLeft();
  $("#boxJournalWriteTime time").text(gNow.getTimeStr());
}


function updateTodayDateAndDaysLeft() {
  var lastDayOfMonth = new Date(gNow.getFullYear(), gNow.getMonth() + 1, 0);
  //console.log("LastDayOfMonth");
  //console.log(lastDayOfMonth);
  var remainingDays = lastDayOfMonth.getDate() - gNow.getDate();
  var remainingDaysStr = remainingDays.toString();
  if (remainingDays > 1) {
    remainingDaysStr += " days left...";
  } else if (remainingDays === 1) {
    remainingDaysStr += " day left...";
  } else if (remainingDays === 0) {
    remainingDaysStr = "LAST DAY";
  }

  $("#lblTodayDate").text(gNow.getMonthStr() + " " + gNow.getDate() + " " + gNow.getWeekday());
  //$("#lblThisMonth").text(gNow.getMonthStr() + " " + gNow.getFullYear());
  //$("#lblDaysLeftMonth").text(remainingDaysStr);
}

/******************************* JOURNAL ************************************/

  $("#sect-journal").on("keyup", "#txtJournalWrite", function() {
    var content = LioTagToHtml($(this).val());
    var len = byteLength(content);
    if (len > 200) {
      $(".boxCharCounter").css("color", "red");
    } else {
      $(".boxCharCounter").css("color", "#AAA");
    }
    $(this).siblings(".textareaUnderline").find(".lblCharCount").text(len);

  });


/*
  Name
    journal.js

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    September 25, 2014
  
  History
    September 25, 2014
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/


(function (window, $) {

  var Journal = function () {
    return this;
  };

  Journal.prototype.Module = function(config) {
    var defConfig = {

    };

    if (typeof config == "undefined") {
      config = {};
    } 

    var elements = {
      "sect-entries-box": $("#sect-entries-box"),
      "sect-entries": $("#sect-entries")

    };

    function Module() {
      this.config = $.extend({}, defConfig, config);
    }

    Module.prototype.Run = function() {
      //getEntries(0, 200);

      updateTime();

      $("#btnJournalPostSubmit").click(function() {
        postEntry();
      });

      function postEntry() {
        var target = $("#sect-entry-write").find("#txtJournalWrite");
        var label = $("#sect-entry-write").find("#txtJournalWrite").siblings("label").text();

        var title = target.val().trim();
        if (title.length > 0 && title != label) {
          title = title.replace(/</g, "&lt;");
          title = title.replace(/>/g, "&gt;");
          var content = "";
          alert(title.length);
          if (title.length > 200) {
            console.log(content);
            content = title.substr(201);
            console.log(content);
            title = title.substr(0, 200);
          } 

          var dataToPost = { };
          dataToPost["type"] = "journal";
          dataToPost["title"] = title;
          if (content.length > 0) {
            dataToPost["content"] = content;
          } 
          console.log(dataToPost);


          $.ajax({
            type: "POST",
            url: "/idea/post",
            data: dataToPost
          }).fail(function() {
            alert("Failed to post content.");

          }).done(function(result) {
            console.log(result);
            if (result["code"] == 0) {
              var id = result["ideaid"];
              var contentId = result["contentid"];
              var time = new Date();

              var curdaySelector = "day-" + 
                time.getFullYear() + time.getMonthPad() + time.getDatePad();
              var curDayObj = $("#" + curdaySelector);
              if (curDayObj.size() == 0) {
                if ($("#year-" + time.getFullYear()).size() == 0) {
                  var year = createYear(time.getFullYear());
                  elements["sect-entries"].append(year);
                } 
                $("#year-" + time.getFullYear()).append(createDayWrap(time));
                curDayObj = $("#" + curdaySelector);
              }
              title = title.replace(/\n/g, "<br>");
              var entry = createEntry(id, time, title, content);
              curDayObj.append(entry);
              target.val("");
              target.focus();
              elements["sect-entries-box"].scrollTop(elements["sect-entries"].height() + 100);
              target.height("30px");
            } else {

            }

          });
        } else {
          // Show red.
          target.addClass("error");
        }
      }

      function getEntries(from, count) {
        $.ajax({
          type: "POST",
          url: "/idea/get",
          data: { "from": from.toString(), "count": count.toString() }
        }).fail(function() {
          alert("Failed to get content.");

        }).done(function(result) {
          if (result["code"] == 0) {
            var prevYearSelector;
            var curYearObj;
            var prevDaySelector;
            var curDayObj;
            for (var i in result["contents"]) {
              var id = result["contents"][i]["id"];
              var content = result["contents"][i]["content"];
              var time = new Date(Number(result["contents"][i]["date"]));
              var curdaySelector = "day-" + 
                time.getFullYear() + time.getMonthPad() + time.getDatePad();
              content = content.replace(/</g, "&lt;");
              content = content.replace(/>/g, "&gt;");
              if (prevDaySelector != curdaySelector) {
                curDayObj = $("#" + curdaySelector);
                if (curDayObj.size() == 0) {
                  if ($("#year-" + time.getFullYear()).size() == 0) {
                    var year = createYear(time.getFullYear());
                    elements["sect-entries"].append(year);
                  } 
                  $("#year-" + time.getFullYear()).find(".journal-year").after(createDayWrap(time));
                  curDayObj = $("#" + curdaySelector);
                }
              } 
              content = content.replace(/\n/g, "<br>");
              var entry = createEntry(id, time, content);
              //curDayObj.find(".journal-date").after(entry);
              curDayObj.prepend(entry);
              prevDaySelector = curdaySelector;
            } 
            elements["sect-entries-box"].find("#boxxx").animate({
              scrollTop: elements["sect-entries"].height() + 100
            }, 1000);
          } else {
            alert("return code other than 0");
          }
          //$(".entry").last().css("color", "red");
        });
      }
      
      function updateTime() {
        var target = $("#entry-new").find(".journal-time");
        setInterval(function(){
          var curTime = new Date();
          target.text(curTime.getTimeStr());
        }, 1000 * 1);
      }

      function createYear(year) {
        var box = $("<div/>", {
          "id": "year-" + year
        });
        var textBox = $("<div/>", {
          "class": "journal-year"
        });

        var text = $("<time/>", {
          "html": year
        });

        textBox.append(text);
        box.append(textBox);
       
        return box;
      }

      function createDayWrap(d) {
        var box = $("<div/>", {
          "id": "day-" + d.getFullYear().toString() + d.getMonthPad() + d.getDatePad(),
          "class": "day"
        });

        var dateBox = $("<div/>", {
          "class": "journal-date"
        });

        var dateText = $("<time/>", {
          "datetime": d.getFullYear().toString() + "-" + d.getMonthPad() + "-" + d.getDatePad(),
          "html": d.getMonthStr(3) + ". " + d.getDatePad()
        });

        var weekdayText = $("<span/>", {
          "class": "journal-weekday",
          "html": d.getWeekday()
        });

        var weekdayInt = d.getDay();
        if (weekdayInt == 0) {
          // Sunday Red
          dateText.addClass("color-red");
          weekdayText.addClass("color-red");

        } else if (weekdayInt == 6) {
          // Saturday Blue
          dateText.addClass("color-blue");
          weekdayText.addClass("color-blue");
        } 

        dateBox.append(dateText);
        dateBox.append(weekdayText);

        box.append(dateBox);
       
        return box;
      }

      function createEntry(id, timestamp, title, content) {
        var obj = $("<div/>", {
          "id": "entry-" + id.toString(),
          "class": "entry"
        });


        var time = $("<div/>", {
          "class": "journal-time"
        }).append(
          $("<time>", {
            "datetime": timestamp.toString(),
            "title": timestamp.toString(),
            "text": timestamp.getTimeStr()
          })
        );

        var body = $("<div/>", {
          "class": "body"
        });
        if (typeof content != "undefined" &&
            content.length > 0) {
          body.append(
            $("<div/>", {
              "class": "content",
              "html": title + " ... <a>more content</a>"
            })
          );
        } else {
          body.append(
            $("<div/>", {
              "class": "content",
              "html": title 
            })
          );

        }

        /*
        body.append(
          $("<div/>", {
            "class": "content-options",
            "html": "Commments"
          })
        );
        */

        obj.append(time);
        obj.append(body);

        return obj;
      }
    };

    try {
      var module = new Module();
      return module;

    } catch(ex) {
      console.log("Failed to allocate Module.");
      return null;
    }
  };

  Journal.fn = Journal.prototype;
  window.letsjournal = Journal;

})(window, $);


/******************************* JOURNAL ************************************/
