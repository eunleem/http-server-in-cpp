
(function ($) {
  $.fn.lioInputMask = function () {
    this.find("label").each(function (index) {
      var label = $(this);
      var id = $(this).attr("for");
      var input = $(this).siblings("#" + id);
      //alert();
      if (input.attr("type") == "text" || 
          input.attr("type") == "password" ||
          input.get(0).nodeName == "TEXTAREA") 
      {
        var labelTxt = jQuery.trim(label.text());
        label.hide();
        if (input.val().length == 0) { // IF the field is empty
          // Fill it with Label Text and make it fade out
          input.val(labelTxt);
          input.css("font-style", "italic");
          input.css("opacity" , "0.5");
        } else if (input.val() == labelTxt) { // if input is already same as label text,
          input.stop().fadeTo(300, .5);
          input.css("font-style", "italic");
        } else {
          input.css("opacity" , "0.7");
          input.css("font-style", "none");
        }

        input.focus(function () {
          if (labelTxt == input.val()) {
            input.val("");
          } else {
          }
          input.css("cursor", "text");
          input.css("opacity" , "1");
          input.css("font-style", "none");
        }).blur(function () {
          if ($(this).val().length == 0) {
            //$(this).stop().fadeTo(300, .5);
            $(this).val(labelTxt);
            input.css("cursor", "pointer");
            input.css("font-style", "italic");
            input.stop().fadeTo(700, .5);
          } else if (labelTxt == input.val()) {
            input.css("cursor", "pointer");
            input.css("font-style", "italic");
            input.stop().fadeTo(300, .5);
          } else {
            input.stop().fadeTo(300, .7);
            input.css("font-style", "none");
          }
        }).change(function() {
          if ($(this).val().length == 0) {
            //$(this).stop().fadeTo(300, .5);
            $(this).val(labelTxt);
            input.css("cursor", "pointer");
            input.css("font-style", "italic");
            input.stop().fadeTo(700, .5);
          } else if (labelTxt == input.val()) {
            input.css("cursor", "pointer");
            input.css("font-style", "italic");
            input.stop().fadeTo(300, .5);
          } else {
            input.stop().fadeTo(300, .7);
            input.css("font-style", "none");
          }

        }).hover(function () {
          if (input.val() == labelTxt) {
            //input.stop().fadeTo(300, .7);
            input.css("cursor", "pointer");
          }
        }, function () {
          if (input.val() == labelTxt) {
            //input.stop().fadeTo(300, .5);
          }
        });
      }
    });
  }
})(jQuery);

(function ($) {

    // jQuery plugin definition
    $.fn.TextAreaExpander = function (minHeight, maxHeight) {


        // resize a textarea
        function ResizeTextarea(e) {

            // event or initialize element?
            e = e.target || e;

            // find content length and box width
            var vlen = e.value.length, ewidth = e.offsetWidth;
            if (vlen != e.valLength || ewidth != e.boxWidth) {

                if ((vlen < e.valLength || ewidth != e.boxWidth)) e.style.height = "0px";
                var h = Math.max(e.expandMin, Math.min(e.scrollHeight, e.expandMax));

                e.style.overflow = (e.scrollHeight > h ? "auto" : "hidden");
                e.style.height = h + "px";

                e.valLength = vlen;
                e.boxWidth = ewidth;
            }

            return true;
        };

        // initialize
        this.each(function () {

            // is a textarea?
            if (this.nodeName.toLowerCase() != "textarea") return;

            // set height restrictions
            var p = this.className.match(/expand(\d+)\-*(\d+)*/i);
            this.expandMin = minHeight || (p ? parseInt('0' + p[1], 10) : 0);
            this.expandMax = maxHeight || (p ? parseInt('0' + p[2], 10) : 99999);

            // initial resize
            ResizeTextarea(this);

            // zero vertical padding and add events
            if (!this.Initialized) {
                this.Initialized = true;
                $(this).css("padding-top", 0).css("padding-bottom", 0);
                $(this).bind("keyup", ResizeTextarea).bind("focus", ResizeTextarea);
            }
        });

        return this;
    };

})(jQuery);

