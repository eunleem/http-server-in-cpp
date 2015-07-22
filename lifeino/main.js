$(function() {
  updateEveryMinute();
  setInterval(updateEveryMinute, 60 * 1000);

  jGet("#btnPageClose").click(function() {
    closePage();
  });

  jGet("#sect-message").click(function() {
    jGet("#sect-message").find(".wrap").html("");
    //jGet("#sect-message").css("min-height", "50px");
    //jGet("#sect-message").css("overflow", "hidden");
    jGet("#sect-message").addClass("closed");
  });

  jGet("#sect-tips").click(function() {
    //jGet("#sect-message").css("min-height", "50px");
    //jGet("#sect-message").css("overflow", "hidden");
    jGet("#sect-tips").addClass("closed");
    setTimeout(function() {
      jGet("#sect-tips").css("height", "0 !important");
    }, 500);
  });

  jGet("#btnRefresh").click(function() {
    refreshIdeas();
  });

  jGet("#btnRefreshComments").click(function() {
    refreshComments();
  });

  jGet("#btnIdeaPost").click(function() {
    postIdea();
  });

  jGet("#lnkSendFeedback").click(function() {
    jGet("#boxFeedbackPopup").addClass("active");
    jGet("#boxFeedbackPopup h2").text("피드백 보내기");
    jGet("#txtFeedback").focus();
  });
  jGet("#lnkSendMessage").click(function() {
    jGet("#boxFeedbackPopup").addClass("active");
    jGet("#boxFeedbackPopup h2").text("메세지 보내기");
    jGet("#txtFeedback").focus();
  });
  jGet("#lnkSubscribe").click(function() {
    jGet("#boxFeedbackPopup h2").text("뉴스 & 업데이트 이메일로 받기");
    jGet("#boxFeedbackPopup").addClass("active");
    jGet("#txtFeedback").focus();
  });

  jGet("#btnFeedbackSend").click(function() {
    postFeedback();
  });

  jGet("#btnFeedbackCancel").click(function() {
    jGet("#boxFeedbackPopup").removeClass("active");
    //jGet("#txtFeedback").val("");
  });

  jGet("#btnCommentPost").click(function() {
    var ideaid = $(this).data("ideaid");
    postComment(ideaid);
    $(this).blur();
  });

  jGet("#btnCommentCancel").click(function() {
    closeCommentPost();
    $(this).blur();
  });

  jGet(".page .pageside").click(function() {
    closePage();
  });


  var defTxtVal = jGet("#txtIdea").val();

  jGet("#boxWrite").on("focus", "#txtIdea", function() {
    if ($(this).val() == defTxtVal) {
      $(this).val("");
    } 
    gIsWriting = true;
  });

  jGet("#boxWrite").on("blur", "#txtIdea", function() {
    if ($(this).val().trim() == "") {
      $(this).val(defTxtVal);
    } 
    gIsWriting = false;
  });
  
  jGet("#btnPageComment").click(function() {
    if (isCommentPostOpen() == true) {
      closeCommentPost();
    } else {
      openCommentPost();
    }
  });

  $(document).keydown(function(e) {
    if (isPageOpen() == true) {
      if (e.which == 27) { // ESC
        if (isCommentPostOpen() == true) {
          closeCommentPost();
        } else {
          closePage();
        }
      } else
      if (e.which == 67) { // C
        if (isCommentPostOpen() == false) {
          e.preventDefault();
          openCommentPost();

          if (jGet("#txtComment").val() == "c") {
            jGet("#txtComment").val("");
          } 
        } 
      } else
      if (e.which == 82) { // R
        if (isCommentPostOpen() == false) {
          refreshComments();
        }
      } 
    } else {
      // When page is Closed.
      if (gIsWriting == false) {
        if (e.which == 74) { // j
          moveIdea("down");
        } else
        if (e.which == 75) { // k 
          moveIdea("up");
        } else
        if (e.which == 82) { // r
          // Refresh
          refreshIdeas();
        } else
        if (e.which == 87) { // W
          e.preventDefault();
          jGet("#txtIdea").focus();
        } 
      } 
    }
  });

  refreshIdeas();

  // ==========================================================
  
var gIsWriting = false;
var gSelectedIdeaId = 0;


function moveIdea(direction) {
  var selected = $(".story .selected");

  if (direction == "up") {
    if (selected.length === 0) {
      $(".story:last").find("h2").addClass("selected");
      $(".story:last").find("a").focus();
      return;
    } 
    var prev = selected.parent().prev().find("h2");
    if (prev.length !== 0) {
      prev.addClass("selected");
      prev.find("a").focus();
      selected.removeClass("selected");
      gSelectedIdeaId = prev.find("a").data("ideaid");
    } 

  } else if (direction == "down") {
    if (selected.length === 0) {
      $(".story:first").find("h2").addClass("selected");
      $(".story:first").find("a").focus();
      return;
    } 
    var next = $(".story .selected").parent().next().find("h2");
    if (next.length !== 0) {
      next.addClass("selected");
      next.find("a").focus();
      selected.removeClass("selected");
      gSelectedIdeaId = next.find("a").data("ideaid");
    } 

  } else {
    console.log("invalid direction");
  }
}

function isPageOpen() {
  return jGet("#page-post-read").hasClass("active-page");
}

function isCommentPostOpen() {
  if (isPageOpen() == false) {
    return false;
  } 
  return jGet("#btnPageComment").hasClass("active-button");
}

function openCommentPost() {
  if (isPageOpen() == true) {
    if (isCommentPostOpen() == false) {
      jGet("#btnPageComment").addClass("active-button");
      jGet("#boxCommentWrite").addClass("active-comment-write");
      jGet(".page .wrap .boxContent").css("margin-bottom", "300px");
      jGet("#boxCommentWrite textarea").focus();
    } 
  } 
}

function closeCommentPost() {
  jGet("#btnPageComment").removeClass("active-button");
  jGet("#boxCommentWrite").removeClass("active-comment-write");
  jGet(".page .wrap .boxContent").css("margin-bottom", "");
}

  function refreshIdeas() {
    jGet("#btnRefresh").addClass("rotating");

    getIdeas();
    setTimeout(function() {
      jGet("#btnRefresh").removeClass("rotating");
    }, 700);
  }

  function refreshComments() {
    jGet("#btnRefreshComments").addClass("rotating");

    getComments();
    setTimeout(function() {
      jGet("#btnRefreshComments").removeClass("rotating");
    }, 700);
  }

  function getContent(id) {
    if (typeof id === "undefined") {
      console.log("content id is undefined.");
      return;
    } 
    if (id == 0) {
      console.log("id cannot be 0");
      return;
    } 
    $.ajax({
      type: "POST",
      url: "/get?content",
      data: {
        "contentid": id.toString()
      }

    }).fail(function(){
      alert("failed");

    }).done(function(result) {
      if (result["code"] < 0) {
        alert("failed " + result["code"]);

      } else if (result["code"] == 0) {
        var content = result["content"];
        content = content.replace(/(?:\r\n|\r|\n)/g, "<br>");
  
        jGet("#page-post-read").find("article").html(content);

      } else if (result["code"] > 0) {
        alert("successful " + result["code"]);
      }
    });
  }

  function getIdeas(onAlways) {

    $.ajax({
      type: "POST",
      url: "/get?idea",
      data: {
        "count": 100
      }

    }).fail(function(){
      alert("failed");

    }).done(function(result) {
      //console.log(result);
      if (result["code"] < 0) {
        alert("failed " + result["code"]);
      } else if (result["code"] == 0) {
        if (typeof result["ideas"] === "undefined") {
          return;
        } 
        var height = jGet("#lstStories").height();
        jGet("#lstStories").css("min-height", height + "px");
        jGet("#lstStories").html("");
        result["ideas"].forEach(function(element, index, arr) {
          var row = $("<li>", {
            "id": "idea-" + element["id"],
            "class": "story"
          });

          var created = new Date(element["created"]);
          var fieldCreatedDate = $("<time>", {
            "text": created.getTimeStr()
          });


          var title = element["title"];
          var linebreak = title.indexOf("\n");
          if (linebreak == -1) {
            linebreak = title.length;
          } 

          title = title.substr(0, linebreak);
          if (title.length > 50) {
            title = title.substr(0, 50) + "...";
          } 


          var preview = element["title"].replace(/\n/g, "<br>");
          var peek = element["title"]
            .replace(/&lt;/g, "<")
            .replace(/&gt;/g, ">")
            .replace(/&quot;/g, "\"")
            .replace(/&#39;/g, "'")
            .replace(/&nbsp;/g, " ")
            .replace(/&amp;/g, "&");

          var fieldTitleWrap = $("<h2>");
          var fieldTitle = $("<a>", {
            "class": "lnkPageOpen",
            "href": "#idea-" + element["id"].toString(),
            "html": title,
            "title": peek,
            "data-title": preview,
            "data-ideaid": element["id"],
            "data-created": element["created"],
            "data-contentid": element["contentid"]
          });


          var hasContent = "";
          if (title != peek ||
              element["contentid"] != 0) {
            // read more...
            hasContent = "tap on title to read more...";
          } 

          var info = element["numcomments"];
          var fieldInfo = $("<div />", {
            "class": "idea-info",
            "html": "<img src='comments.png' /> "  + info.toString() + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" + hasContent
          });

          fieldTitleWrap.append(fieldTitle);

          row.append(fieldCreatedDate);
          row.append(fieldTitleWrap);
          row.append(fieldInfo);

          row.prependTo(jGet("#lstStories"));
        });
      } else if (result["code"] > 0) {
        alert("successful " + result["code"]);
      }

      if (gSelectedIdeaId != 0) {
        var selected = $("#idea-" + gSelectedIdeaId);
        selected.find("h2").addClass("selected");
        selected.find("a").focus();
      } 

      $("#lstStories").on("click", ".lnkPageOpen", function(e) {
        e.preventDefault();
        openPage(this);
      });
    }).always(function() {
      if (typeof(onAlways) == "function") {
        onAlways();
      } 
    });
  }

  function postIdea() {
    var content = jGet("#txtIdea").val();

    if (content.bytelength() > 30000) {
      jGet("#lblPostMessage").text("글이 너무 길어요!!");
      jGet("#lblPostMessage").removeClass("disabled");
      setTimeout(function() {
        jGet("#lblPostMessage").addClass("disabled");
      }, 2000);
      return false;
    }

    if (content == defTxtVal) {
      jGet("#lblPostMessage").text("Please enter your message in the text box above!");
      jGet("#lblPostMessage").removeClass("disabled");
      setTimeout(function() {
        jGet("#lblPostMessage").addClass("disabled");
      }, 2000);
      return false;
    } 

    content = content.trim()
      .replace(/\n/g, "\\n")
      .replace(/\r/g, "\\r")
      .replace(/\t/g, "\\t")
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/  /g, "&nbsp;&nbsp;")
      .replace(/'/g, "&#39;");

    if (content.indexOf("<") != -1) {
      console.log("Invalid");
      return false;
    } 
    //console.log(content);
    var data = {}; 
    var title = content.substr(0, 200);
    var length = title.length;
    while (title.bytelength() >= 200) {
      length -= 1;
      title = title.substr(0, length);
    } 

    if (title[title.legnth - 1] == '\\') {
      title = title.substr(0, title.length - 2);
    } 

    data["title"] = title;
    if (content.bytelength() >= 200) {
      data["content"] = content;
    } 

    if (title.length == 0) {
      jGet("#lblPostMessage").text("Content cannot be empty!");
      jGet("#lblPostMessage").addClass("errorMessage");
      jGet("#lblPostMessage").removeClass("disabled");
      setTimeout(function() {
        jGet("#lblPostMessage").addClass("disabled");
      }, 2000);
      return false;
    }

    $.ajax({
      type: "POST",
      url: "/idea?post",
      data: data
    }).fail(function(){
      alert("failed");

    }).done(function(result) {
      console.log(result);
      if (result["code"] < 0) {
        alert("failed " + result["code"]);
      } else if (result["code"] == 0) {
        // POSTED
        jGet("#txtIdea").val(defTxtVal);
        jGet("#lblPostMessage").text("글이 포스팅 되었어요! 감사합니다.");
        jGet("#lblPostMessage").removeClass("errorMessage");
        jGet("#lblPostMessage").addClass("successMessage");
        jGet("#lblPostMessage").removeClass("disabled");
        setTimeout(function() {
          jGet("#lblPostMessage").addClass("disabled");
        }, 2000);
        getIdeas();
      } else if (result["code"] > 0) {
        alert("successful " + result["code"]);
      }
    }).always(function() {
    });
  }

  function getComments(ideaid) {
    if (typeof ideaid === "undefined") {
      ideaid = jGet("#btnCommentPost").data("ideaid");
    } 
    
    var height = jGet("#lstComments").height();
    jGet("#lstComments").css("min-height", height + "px");

    $.ajax({
      type: "POST",
      url: "/get?comment",
      data: {
        "ideaid": ideaid.toString()
      }
    }).fail(function(){
      alert("failed");

    }).done(function(result) {
      if (result["code"] < -1) {
        alert("failed " + result["code"]);
      } else if (result["code"] == -1) {
        // No comments
        jGet("#lstComments").html("");
        var row = $("<li>", {
          "id": "lblNocomment",
          "class": "nocomment",
          "html": "<div style='margin-left: -70px;'>No Comment yet.</div>"
        });
        row.appendTo(jGet("#lstComments"));
        
      } else if (result["code"] == 0) {
        // POSTED
        jGet("#lstComments").html("");
        var lstComments = jGet("#lstComments");
        result["comments"].forEach(function(element, index, arr) {
          var comment = element["comment"];
          comment = comment.replace(/(?:\r\n|\r|\n)/g, "<br>");
          var row = $("<li>", {
            "id": "comment-" + element["id"],
            "class": "comments",
            "data-created": element["created"],
            "html": "<div>" + comment + "</div>"
          });

          var created = new Date(element["created"]);
          var fieldCreatedDate = $("<time>", {
            "datetime": created.toString(),
            "title": created.toLocaleString(),
            "text": created.getTimeStr()
          });


          row.prepend(fieldCreatedDate);
          //row.append(row);

          row.appendTo(jGet("#lstComments"));
        });
      } else if (result["code"] > 0) {
        alert("successful " + result["code"]);
      }
    }).always(function() {
    });
  }

  function postComment(ideaid) {
    var comment = jGet("#txtComment").val();

    comment = comment.trim()
      .replace(/\n/g, "\\n")
      .replace(/\r/g, "\\n")
      .replace(/&/g, "&amp;")
      .replace(/\t/g, "&nbsp;&nbsp;&nbsp;&nbsp;")
      .replace(/  /g, "&nbsp;&nbsp;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;")
      .replace(/>/g, "&gt;")
      .replace(/</g, "&lt;");



    jGet("#boxCommentMessage").addClass("active-comment-message");
    jGet("#boxCommentMessage").find("#lblCommentMessage").removeClass("successMessage");
    if (comment.length == 0) {
      jGet("#boxCommentMessage").find("#lblCommentMessage").addClass("errorMessage");
      jGet("#boxCommentMessage").find("#lblCommentMessage").html("Comment cannot be empty.");
      jGet("#txtComment").focus();
      setTimeout(function() {
        jGet("#boxCommentMessage").removeClass("active-comment-message");
      }, 2000);
      return false;
    } 

    jGet("#boxCommentMessage").find("#lblCommentMessage").removeClass("errorMessage");
    jGet("#boxCommentMessage").find("#lblCommentMessage").text("POSTING...");
      
    $.ajax({
      type: "POST",
      url: "/comment?post",
      data: {
        "ideaid": ideaid.toString(),
        "comment": comment
      }
    }).fail(function(){
      alert("failed");

    }).done(function(result) {
      if (result["code"] < 0) {
        alert("failed " + result["code"]);
      } else if (result["code"] == 0) {
        // POSTED
        jGet(".page .wrap").animate({
          scrollTop: jGet(".page .wrap")[0].scrollHeight
        }, 800, "swing");

        jGet("#boxCommentMessage").find("#lblCommentMessage").addClass("successMessage");
        jGet("#boxCommentMessage").find("#lblCommentMessage").html("POSTED!");
        setTimeout(function() {
          jGet("#boxCommentMessage").removeClass("active-comment-message");
          jGet("#txtComment").val("");
          var hasClass = jGet("#btnPageComment").hasClass("active-button");
          if (hasClass == true) {
            jGet("#btnPageComment").removeClass("active-button");
            jGet("#boxCommentWrite").removeClass("active-comment-write");
            jGet(".page .wrap .boxContent").css("margin-bottom", "");
          }

        }, 3000);
        refreshComments();
      } else if (result["code"] > 0) {
        alert("successful " + result["code"]);
      }
    }).always(function() {
    });
  }


  function postFeedback() {
    var feedback = jGet("#txtFeedback").val();

    feedback = feedback.trim()
      .replace(/\n/g, "\\n")
      .replace(/\r/g, "\\n")
      .replace(/&/g, "&amp;")
      .replace(/\t/g, "&nbsp;&nbsp;&nbsp;&nbsp;")
      .replace(/  /g, "&nbsp;&nbsp;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;")
      .replace(/>/g, "&gt;")
      .replace(/</g, "&lt;");


    if (feedback.length == 0) {
      return false;
    } 


    jGet("#btnFeedbackSend").text("SENDING...!");
    jGet("#btnFeedbackSend").attr("disabled", "disabled");
      
    $.ajax({
      type: "POST",
      url: "/feedback?post",
      data: {
        "feedback": feedback
      }
    }).fail(function(){
      jGet("#btnFeedbackSend").text("FAILED!");

    }).done(function(result) {
      if (result["code"] < 0) {
        alert("failed " + result["code"]);
      } else if (result["code"] == 0) {
        // POSTED
        jGet("#btnFeedbackSend").text("SENT!");

        setTimeout(function() {
          jGet("#txtFeedback").val("");
          jGet("#boxFeedbackPopup").removeClass("active");

        }, 1000);
      } else if (result["code"] > 0) {
        alert("successful " + result["code"]);
      }
    }).always(function() {
      setTimeout(function() {
        jGet("#btnFeedbackSend").removeAttr("disabled");
        jGet("#btnFeedbackSend").text("SEND");
      }, 1000);
    });
  }

  function openPage(eventOrigin) {
    jGet("#page-post-read").find("article").html("LOADING...");
    jGet("#page-post-read").addClass("active-page");
    jGet("body").css("overflow", "hidden");

    var created = new Date($(eventOrigin).data("created"));
    jGet("#page-post-read").find("time").html(
        created.getMonthStr() + " " + created.getDatePad() + ", " + created.getFullYear() +
        "<br>" + created.getTimeStr()
      );

    var contentid = $(eventOrigin).data("contentid");
    if (contentid == 0) {
      var title = $(eventOrigin).data("title");
      jGet("#page-post-read").find("article").html(title);
    } else {
      getContent(contentid);
    }

    jGet("#page-post-read").find(".wrap").focus();

    var ideaid = $(eventOrigin).data("ideaid");
    jGet(".page .wrap").data("ideaid", ideaid);
    jGet("#btnCommentPost").data("ideaid", ideaid);
    refreshComments();
  }

  function closePage() {
    jGet("#page-post-read").removeClass("active-page");
    jGet("body").css("overflow", "auto");
    jGet("#lstComments").css("min-height", "0px");

    $(".story .selected").find("a").focus();

    setTimeout(function() {
      jGet("#page-post-read").find(".wrap").scrollTop("0");
      jGet("#page-post-read").find("article").html("");
      jGet("#page-post-read").find("#lstComments").html("");
      var hasClass = jGet("#btnPageComment").hasClass("active-button");
      if (hasClass == true) {
        jGet("#btnPageComment").removeClass("active-button");
        jGet("#boxCommentWrite").removeClass("active-comment-write");
        jGet(".page .wrap .boxContent").css("margin-bottom", "");
      }
    }, 300);
  }


  function updateTodayDateAndDaysLeft() {
    var now = new Date();
    var lastDayOfMonth = new Date(now.getFullYear(), now.getMonth(), 0);
    var remainingDays = lastDayOfMonth.getDate() - now.getDate();
    var remainingDaysStr = remainingDays.toString();
    if (remainingDays > 1) {
      remainingDaysStr += " days left...";
    } else if (remainingDays == 1) {
      remainingDaysStr += " day left...";
    } else if (remainingDays == 0) {
      remainingDaysStr = "LAST DAY";
    }

    $("#lblTodayDate").text(now.getMonthStr() + " " + now.getDate() + " " + now.getWeekday());
    //$("#lblThisMonth").text(now.getMonthStr() + " " + now.getFullYear());
    //$("#lblDaysLeftMonth").text(remainingDaysStr);
  }

  function updateEveryMinute() {
    gNow = new Date();

    updateTodayDateAndDaysLeft();
    $("#boxWrite time").text(gNow.getTimeStr());
  }
});
