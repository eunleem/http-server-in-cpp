var gWeekDayNames = new Array(
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
);
var gWeekDayNamesKor = new Array(
  "일", "월", "화", "수", "목", "금", "토"
);
var gMonthNames = new Array(
  "January", "February", "March", "April", "May", "June", "July", "August", "September",
  "October", "November", "December"
);

var IDEATYPE = {
  "general": 0,
  "journal": 1,
  "goal": 2,
  "action": 3,
  "idea": 4,
  "info": 5,
  "comment": 6
};

var IDEASTATUS = {
  "active": 0,
  "deleted": 1,
  "archived": 2
};


var gCachedObjs = new Object();

function jGet(selector) {
  // #DEVEL
  if (typeof $ === "undefined") {
    console.log("function jGet cannot be used before jQuery is Loaded!");
    alert("function jGet cannot be used before jQuery is Loaded!");
    return;
  } 
  if (typeof selector !== "string") {
    console.log("function jGet only take string param.");
    return;
  } 
  // #DEVLE_END

  if (typeof gCachedObjs[selector] === "undefined") {
    if ($(selector).length == 0) {
      console.log("Could not get " + selector);
      return null;
    }

    gCachedObjs[selector] = $(selector);
  } 

  return gCachedObjs[selector];
}



Number.prototype.toStringPad = function (digits) {
  var leadingZeros = "";
  var length = this.toString().length;
  var c = digits - length;
  var s = this.toString();
  for (var i = 0; c > i; i++) {
    leadingZeros += "0";
  } 
  return leadingZeros + s;
}



Date.prototype.getMonthStr = function(format) {
  if (typeof format == "undefined") {
    return gMonthNames[this.getMonth()];
  } 
  return gMonthNames[this.getMonth()].substr(0, format);
};

Date.prototype.getMonthPad = function() {
  var s = "0" + (this.getMonth() + 1);
  return s.substr(s.length - 2);
};

Date.prototype.getDatePad = function() {
  var s = "0" + this.getDate();
  return s.substr(s.length - 2);
};


Date.prototype.getWeekNumber = function() {
  var onejan = new Date(this.getFullYear(), 0, 1);
  return Math.ceil((((this - onejan) / 86400000) + onejan.getDay() + 1) / 7);
};

Date.prototype.getWeekday = function(format) {
  var dayIdx = this.getDay();
  //console.log(dayIdx);
  if (isNaN(dayIdx) == true ||
      dayIdx === undefined) {
    return "";
  } 

  if (format !== undefined) {
    return gWeekDayNames[dayIdx].substr(0, format); 
  }

  return gWeekDayNames[dayIdx]; 
};

Date.prototype.getHoursPad = function() {
  var s = "0" + this.getHours().toString();
  return s.substr(s.length - 2);
};

Date.prototype.getMinutesPad = function() {
  var s = "0" + this.getMinutes().toString();
  return s.substr(s.length - 2);
};

Date.prototype.getSecondsPad = function() {
  var s = "0" + this.getSeconds().toString();
  return s.substr(s.length - 2);
};

Date.prototype.getTimeStr = function(config) {
  var defConfig = {
    "twelveHrs": true,
    "uppercaseAmPm": false,
    "paddedHours": false,
    "paddedMinutes": true
  };

  if (typeof config == "undefined") {
    config = defConfig;
  }

  var hours = this.getHours();
  var minutes = this.getMinutes();
  if (config.paddedMinutes == true) {
    minutes = minutes.toStringPad(2);
  }

  var result = "";
  var ending = "";
  if (config.twelveHrs == true) {
    var ampm = "am";
    if (hours >= 12) {
      hours -= 12;
      ampm = "pm";
    } 
    if (hours == 0) {
      // No Zero hours.
      hours = 12;
    } 
    if (config.uppercaseAmPm == true) {
      ampm = ampm.toUpperCase();
    } 
    ending = " " + ampm;
  }

  if (config.paddedHours == true) {
    hours = hours.toStringPad(2);
  } 
  result = hours + ":" + minutes + ending;
  return result;
};

Date.prototype.getDateTime = function() {
  return this.getMonthStr() + " " + this.getDatePad() + ", " + this.getFullYear() + " " + this.getTimeStr();
}

String.prototype.htmltrim = function() {
  return this.replace(/^\s+|\s+$/g, "")
    .replace(/^[<br>\s&nbsp;]+|[(<br>)\s&nbsp;]+$/g, "");
};

String.prototype.bytelength = function() {
  var s = this.length;
  for (var i = this.length - 1; i >= 0; --i) {
    var code = this.charCodeAt(i);
    if (code > 0x7f && code <= 0x7ff) s++;
    else if (code > 0x7ff && code <= 0xffff) s += 2;
    if (code >= 0xDC00 && code <= 0xDFFF) i--; //trail surrogate
  }
  return s;
};

String.prototype.firstline = function(maxlength) {
  var firstlineEnd = 0;
  if (typeof maxlength !== "undefined") {
    firstlineEnd = this.substr(0, maxlength).indexOf("\n");
  } else {
    firstlineEnd = this.indexOf("\n");
  }
  if (firstlineEnd > 0) {
    return this.substr(0, firstlineEnd);
  } 
  return "";
};

String.prototype.splice = function( idx, rem, s ) {
  return (this.slice(0,idx) + s + this.slice(idx + Math.abs(rem)));
};

String.prototype.truncatebyte = function(size) {
  var i = this.length;
  var result = this.substr(0, size);
  while (result.bytelength() > size) {
    result = this.substr(0, --i);
  } 
  return result;
};


function getUrlVars() {
  var vars = [], hash;
  var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
  for(var i = 0; i < hashes.length; i++) {
    hash = hashes[i].split('=');
    vars.push(hash[0]);
    vars[hash[0]] = hash[1];
  }
  return vars;
}

function getCookieValueByName(name) {
  var value = "; " + document.cookie;
  var parts = value.split("; " + name + "=");
  if (parts.length == 2) {
    return parts.pop().split(";").shift();
  }

  return null;
}

function setCookieValue(cname, cvalue, expInDays) {
  var d = new Date();
  d.setTime(d.getTime() + (expInDays * 24 * 60 * 60 * 1000));
  var expires = "expires=" + d.toGMTString();
  document.cookie = cname + "=" + cvalue + "; " + expires;
} 

function getRandomInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function enableText(target) {
  if (target.is(":disabled")) {
    target.removeAttr("disabled");
    return true;

  } else {
    console.log("text is already enabled.");
    return false;
  }
}

function disableText(target) {
  if (target.is(":disabled")) {
    console.log("Text is already disabled.");
    return false;

  } else {
    target.attr("disabled", "disabled");
    return true;
  }
}

function enableButton(target) {
  if (target.is(":disabled")) {
    target.removeAttr("disabled");
    if (target.data("orgtext") != "") {
      if (target.is("button")) {
        target.text(target.data("orgtext"));
      } else if (target.is("input")) {
        target.val(target.data("orgtext"));
      }
    } 
    return true;

  } else {
    console.log("button is already enabled.");
    return false;
  }
}

function disableButton(target, disabledText) {
  if (target.is(":disabled")) {
    console.log("Button is already disabled.");
    return false;

  } else {
    if (typeof disabledText === "string") {
      if (target.is("button")) {
        console.log(target.text());
        target.data("orgtext", target.text());
        target.text(disabledText);
      } else if (target.is("input")) {
        target.data("orgtext", target.val());
        target.val(disabledText);
      }
    }
    target.attr("disabled", "disabled");
    return true;
  }
}

function htmlEncode(str) {
  return str
    .replace(/&/g, '&amp;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/\n/g, "\\n")
    .replace(/\r/g, "\\n")
    .replace(/\t/g, "&nbsp;&nbsp;&nbsp;&nbsp;")
    .replace(/  /g, '&nbsp;&nbsp;');
}
