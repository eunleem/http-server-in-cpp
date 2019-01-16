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


var gCachedObjs = {};

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
    if ($(selector).length === 0) {
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
};



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

Date.prototype.getMonthStrAndDate = function(format) {
  if (typeof format == "undefined") {
    return gMonthNames[this.getMonth()] + " " + this.getDate().toString();
  }
  return gMonthNames[this.getMonth()].substr(0, format) + " " + this.getDate().toString();
};


Date.prototype.getWeekNumber = function() {
  var onejan = new Date(this.getFullYear(), 0, 1);
  return Math.ceil((((this - onejan) / 86400000) + onejan.getDay() + 1) / 7);
};

Date.prototype.getWeekday = function(format) {
  var dayIdx = this.getDay();
  //console.log(dayIdx);
  if (isNaN(dayIdx) === true ||
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
    "paddedMinutes": true,
    "showSeconds": false
  };

  if (typeof config === "undefined") {
    config = defConfig;
  } else {
    config = extend(defConfig, config);
  }

  var hours = this.getHours();
  var minutes = this.getMinutes();
  if (config.paddedMinutes === true) {
    minutes = this.getMinutesPad();
  }

  var result = "";
  var ending = "";
  if (config.twelveHrs === true) {
    var ampm = "am";
    if (hours >= 12) {
      hours -= 12;
      ampm = "pm";
    }
    if (hours === 0) {
      // No Zero hours.
      hours = 12;
    }
    if (config.uppercaseAmPm === true) {
      ampm = ampm.toUpperCase();
    }
    ending = " " + ampm;
  }

  if (config.paddedHours === true) {
    hours = hours.toStringPad(2);
  }
  result = hours + ":" + minutes;
  if (config.showSeconds === true) {
    result += ":" + this.getSecondsPad();
  }
  result += " "+ ending;
  return result;
};

Date.prototype.getDateTime = function() {
  return this.getMonthStr() + " " + this.getDatePad() + ", " + this.getFullYear() + " " + this.getTimeStr();
};

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

String.prototype.toSafeHtml = function() {
  return this
        .replace(/>/g, "&gt;")
        .replace(/</g, "&lt;");
};

String.prototype.toPrettyHtml = function() {
  return this
        .replace(/&/g, "&amp;")
        .replace(/  /g, "&nbsp;&nbsp;")
        .replace(/\n/g, "<br>")
        .replace(/\\\\/g, "\\");
};

String.prototype.fromJsonStringToPrettyHtml = function() {
  var now = new Date();
  return this
        .replace(/&gt;/g, ">")
        .replace(/&lt;/g, "<")
        .replace(/&/g, "&amp;")
        .toSafeHtml()
        .replace(/  /g, "&nbsp;&nbsp;")
        //.replace(/\\/g, "\\")
        //.replace(/\"/g, "\"")
      .replace(/\s?Title: (.*?)\\n/g, "<h2>$1</h2>")
      .replace(/\s?Subtitle: (.*?)\\n/g, "<h3>$1</h3>")
      .replace(/\s?Subsubtitle: (.*?)\\n/g, "<h4>$1</h4>")
      .replace(/Center:\s?(.*?)\s?:Center/g, "<div class='textcenter'>$1</div>")
      .replace(/Indent:(.*?):Indent/g, "<div class='indent15'>$1</div>")
      .replace(/Code:(.*?):Code/g, "<code>$1</code>")
      .replace(/BQ:(.*?):BQ/g, "<blockquote>$1</blockquote>")
      .replace(/(\s+)Cite:/g, "$1<cite>")
      .replace(/:Cite(\s+)/g, "</cite>$1")
      .replace(/(\s?)ST:(.*?):ST(\s?)/g, "$1<strong>$2</strong>$3")
      .replace(/\\nST:(.*?)\\n/g, "\\n<strong>$1</strong>\\n")
      .replace(/(\s?)EM:(.*?):EM(\s?)/g, "$1<em>$2</em>$3")
      .replace(/(\s?)IT:(.*?):IT(\s?)/g, "$1<i>$2</i>$3")
      .replace(/IMG:(.*?):IMG/g, "<img src='$1' />")
      .replace(/:TS\[(.*?)\]\[(.*?)\]:/g, "<time datetime='$1'>$2</time>")
      .replace(/:TS:/g, "<time datetime='" + now.toISOString() + "'>" +
         now.getFullYear() + "-" + now.getMonthPad() + "-" + now.getDatePad() +
         " " + now.getTimeStr() + " " + now.getWeekday() +
         "</time>")
      .replace(/(\s?)DEL\[(.*?)\]:(.*?):DEL/g, "$1<del datetime='$2'>$3</del>")
      .replace(/(\s?)INS\[(.*?)\]:(.*?):INS/g, "$1<ins datetime='$2'>$3</ins>")
      .replace(/(\s?)INS:(.*?):INS(\s?)/g, "$1<ins datetime='" + now.toISOString() + "'>$2</ins>$3")
      .replace(/(\s?)DEL:(.*?):DEL(\s?)/g, "$1<del datetime='" + now.toISOString() + "'>$2</del>$3")
      .replace(/(\s?)INS:(.*?):INS(\s?)/g, "$1<ins datetime='" + now.toISOString() + "'>$2</ins>$3")
      .replace(/LINK\[(.*?)\]:(.*?):LINK/g, "<a href='$1' title='$1' target='_blank'>$2</a>")
      .replace(/LINK:(.*?):LINK/g, "<a href='$1' target='_blank'>$1</a>")
      .replace(/RED:(.*?):RED/g, "<span class='color-red'>$1</span>")
      .replace(/ORG:(.*?):ORG/g, "<span class='color-orange'>$1</span>")
      .replace(/li:(.*?):li\\n/g, "<li>$1</li>")
      .replace(/(\s?)ol:(.*?):ol\s?/g, "$1<ol>$2</ol>")
      .replace(/(\s?)ul:\\n(.*?):ul\s?/g, "$1<ul>$2</ul>")
        .replace(/\\"/g, "\"")
        .replace(/\\t/g, "&nbsp;&nbsp;&nbsp;&nbsp;")
        .replace(/\\r/g, "<br/>")
        .replace(/\\n/g, "<br/>")
        .replace(/\\\\/g, "\\");
};

String.prototype.toJsonString = function() {
  return this
        .replace(/\\/g, "\\\\")
        .replace(/\"/g, "\\\"")
        .replace(/\t/g, "\\t")
        .replace(/\r/g, "\\r")
        .replace(/\n/g, "\\n")
        .replace(/\u2028/g, "")
        .replace(/\u2029/g, "")
        .replace(/\b/g, "")
        .replace(/\f/g, "")
        .toSafeHtml();
};

String.prototype.toEditable = function() {
  return this
        .replace(/&nbsp;&nbsp;&nbsp;&nbsp;/g, "\t")
        .replace(/&nbsp;/g, " ")
        .replace(/<br>/g, "\n")
      .replace(/<a .*?href=\"(.*?)\".*?>\s?(.*?)\s?<\/a>/g, "LINK[$1]: $2 :LINK")
      .replace(/<a(.*?)>(.*?)<\/a>/ig, "$2")
      .replace(/<h2>(.*?)<\/h2>/ig, "Title: $1\n")
      .replace(/<h3>(.*?)<\/h3>/ig, "Subtitle: $1\n")
      .replace(/<h4>(.*?)<\/h4>/ig, "Subsubtitle: $1\n")
      .replace(/<strong>(.*?)<\/strong>/g, "ST:$1:ST")
      .replace(/<em>(.*?)<\/em>/g, "EM:$1:EM")
      .replace(/<i>(.*?)<\/i>/g, "IT:$1:IT")
      .replace(/<img src='(.*?)' \/>/g, "IMG:$1:IMG")
      .replace(/<time .*?datetime=\"(.*?)\".*?>(.*?)<\/time>/g, ":TS[$1][$2]:")
      .replace(/<del .*?datetime=\"(.*?)\".*?>(.*?)<\/del>/g, "DEL[$1]:$2:DEL")
      .replace(/<ins .*?datetime=\"(.*?)\".*?>(.*?)<\/ins>/g, "INS[$1]:$2:INS")
      .replace(/<a .*?href=\"(.*?)\".*?>(.*?)<\/a>/g, "LINK[$1]:$2:LINK")
      .replace(/<span class=\"color-red\">(.*?)<\/span>/g, "RED:$1:RED")
      .replace(/<div class=\"textcenter\">(.*?)<\/div>/g, "Center: $1 :Center")
      .replace(/<code>/g, "Code:")
      .replace(/<\/code>/g, ":Code")
      .replace(/<ul>\s?/ig, "ul:\n")
      .replace(/\s?<\/ul>/ig, ":ul")
      .replace(/<ol>(\s?)/ig, "ol:")
      .replace(/(\s?)<\/ol>/ig, ":ol")
      .replace(/<li>(.*?)<\/li>/ig, "li:$1:li\n")
        .replace(/&gt;/g, ">")
        .replace(/&lt;/g, "<")
        .replace(/&amp;/g, "&");


};

function extend (target, source) {
  var result = Object.create(target);
  Object.keys(source).map(function (prop) {
      prop in result && (result[prop] = source[prop]);
  });
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
    if (target.data("orgtext") !== "") {
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
