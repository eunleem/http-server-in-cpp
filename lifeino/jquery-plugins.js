(function ($) {
  $.fn.lioInputLabel = function () {
    var base = this;
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
        if (input.val().length === 0) { // IF the field is empty
          // Fill it with Label Text and make it fade out
          input.val(labelTxt);
          applyLabelStyle(input);

        } else if (input.val() === labelTxt) { // if input is already same as label text,
          applyLabelStyle(input);

        } else {
          input.css("opacity" , "1");
          input.css("font-style", "");
        }

        input.focus(function () {
          if (labelTxt === input.val()) {
            input.val("");
          }
          input.css("cursor", "text");
          input.stop().css("opacity" , "1");
          input.css("font-style", "");

        }).blur(function () {
          if (input.val().length === 0) {
            input.val(labelTxt);
            applyLabelStyle(input);

          } else {
            input.css("font-style", "");
          }
        }).hover(function () {
          if (input.val() == labelTxt) {
            input.stop().fadeTo(300, 0.7);
          }
        }, function () {
          if (input.val() == labelTxt) {
            input.stop().fadeTo(300, 0.5);
          }
        });
      }
    });
  };

  function applyLabelStyle(target, fadeTo, animationTime) {
    target.css("cursor", "pointer");
    target.css("font-style", "italic");

    if (typeof fadeTo === "string") {
      if (fadeTo.toLowerCase() === "fade") {
        if (typeof animationTime !== "number") {
          animationTime = 700;
        }
        target.stop().fadeTo(animationTime, 0.5);
      } else {
        target.css("opacity" , "0.5");
      }
    } else {
      target.css("opacity" , "0.5");
    }
  }
})(jQuery);

/*!
	Autosize 3.0.8
	license: MIT
	http://www.jacklmoore.com/autosize
*/
(function (global, factory) {
	if (typeof define === 'function' && define.amd) {
		define(['exports', 'module'], factory);
	} else if (typeof exports !== 'undefined' && typeof module !== 'undefined') {
		factory(exports, module);
	} else {
		var mod = {
			exports: {}
		};
		factory(mod.exports, mod);
		global.autosize = mod.exports;
	}
})(this, function (exports, module) {
	'use strict';

	function assign(ta) {
		var _ref = arguments[1] === undefined ? {} : arguments[1];

		var _ref$setOverflowX = _ref.setOverflowX;
		var setOverflowX = _ref$setOverflowX === undefined ? true : _ref$setOverflowX;
		var _ref$setOverflowY = _ref.setOverflowY;
		var setOverflowY = _ref$setOverflowY === undefined ? true : _ref$setOverflowY;

		if (!ta || !ta.nodeName || ta.nodeName !== 'TEXTAREA' || ta.hasAttribute('data-autosize-on')) return;

		var heightOffset = null;
		var overflowY = 'hidden';

		function init() {
			var style = window.getComputedStyle(ta, null);

			if (style.boxSizing === 'content-box') {
				heightOffset = -(parseFloat(style.paddingTop) + parseFloat(style.paddingBottom));
			} else {
				heightOffset = parseFloat(style.borderTopWidth) + parseFloat(style.borderBottomWidth);
			}

			update();
		}

		function changeOverflow(value) {
			{
				// Chrome/Safari-specific fix:
				// When the textarea y-overflow is hidden, Chrome/Safari do not reflow the text to account for the space
				// made available by removing the scrollbar. The following forces the necessary text reflow.
				var width = ta.style.width;
				ta.style.width = '0px';
				// Force reflow:
				/* jshint ignore:start */
				ta.offsetWidth;
				/* jshint ignore:end */
				ta.style.width = width;
			}

			//overflowY = value;

			if (setOverflowY) {
				//ta.style.overflowY = value;
			}

			resize();
		}

		function resize() {
      var screenHeight = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;

			var htmlTop = window.pageYOffset;
			var bodyTop = document.body.scrollTop;
			var originalHeight = ta.style.height;

			ta.style.height = 'auto';

			var endHeight = ta.scrollHeight + heightOffset;

			if (ta.scrollHeight === 0) {
				// If the scrollHeight is 0, then the element probably has display:none or is detached from the DOM.
				ta.style.height = originalHeight;
				return;
			}

      var marginTop = 160;
      var maxHeight = screenHeight - marginTop;

      if (endHeight > maxHeight) {
        endHeight = maxHeight;
      }

			ta.style.height = endHeight + 'px';

      var viewportOffset = ta.getBoundingClientRect();
      // these are relative to the viewport
      var top = viewportOffset.top;

      var bottomMargin = 0;
      if (top + endHeight + 30 > screenHeight) {
        bottomMargin = 30;
      }

			// prevents scroll-position jumping
			document.documentElement.scrollTop = htmlTop + bottomMargin;
			document.body.scrollTop = bodyTop + bottomMargin;
		}

		function update() {
			var startHeight = ta.style.height;

			resize();

			var style = window.getComputedStyle(ta, null);

			if (style.height !== ta.style.height) {
				if (overflowY !== 'visible') {
					changeOverflow('visible');
				}
			} else {
				if (overflowY !== 'hidden') {
					changeOverflow('hidden');
				}
			}

			if (startHeight !== ta.style.height) {
				var evt = document.createEvent('Event');
				evt.initEvent('autosize:resized', true, false);
				ta.dispatchEvent(evt);
			}
		}

		var destroy = (function (style) {
			window.removeEventListener('resize', update);
			ta.removeEventListener('input', update);
			ta.removeEventListener('keyup', update);
			ta.removeAttribute('data-autosize-on');
			ta.removeEventListener('autosize:destroy', destroy);

			Object.keys(style).forEach(function (key) {
				ta.style[key] = style[key];
			});
		}).bind(ta, {
			height: ta.style.height,
			resize: ta.style.resize,
			overflowY: ta.style.overflowY,
			overflowX: ta.style.overflowX,
			wordWrap: ta.style.wordWrap });

		ta.addEventListener('autosize:destroy', destroy);

		// IE9 does not fire onpropertychange or oninput for deletions,
		// so binding to onkeyup to catch most of those events.
		// There is no way that I know of to detect something like 'cut' in IE9.
		if ('onpropertychange' in ta && 'oninput' in ta) {
			ta.addEventListener('keyup', update);
		}

		window.addEventListener('resize', update);
		ta.addEventListener('input', update);
		ta.addEventListener('autosize:update', update);
		ta.setAttribute('data-autosize-on', true);

		if (setOverflowY) {
			//ta.style.overflowY = 'hidden';
		}
		if (setOverflowX) {
			ta.style.overflowX = 'hidden';
			ta.style.wordWrap = 'break-word';
		}

		init();
	}

	function destroy(ta) {
		if (!(ta && ta.nodeName && ta.nodeName === 'TEXTAREA')) return;
		var evt = document.createEvent('Event');
		evt.initEvent('autosize:destroy', true, false);
		ta.dispatchEvent(evt);
	}

	function update(ta) {
		if (!(ta && ta.nodeName && ta.nodeName === 'TEXTAREA')) return;
		var evt = document.createEvent('Event');
		evt.initEvent('autosize:update', true, false);
		ta.dispatchEvent(evt);
	}

	var autosize = null;

	// Do nothing in Node.js environment and IE8 (or lower)
	if (typeof window === 'undefined' || typeof window.getComputedStyle !== 'function') {
		autosize = function (el) {
			return el;
		};
		autosize.destroy = function (el) {
			return el;
		};
		autosize.update = function (el) {
			return el;
		};
	} else {
		autosize = function (el, options) {
			if (el) {
				Array.prototype.forEach.call(el.length ? el : [el], function (x) {
					return assign(x, options);
				});
			}
			return el;
		};
		autosize.destroy = function (el) {
			if (el) {
				Array.prototype.forEach.call(el.length ? el : [el], destroy);
			}
			return el;
		};
		autosize.update = function (el) {
			if (el) {
				Array.prototype.forEach.call(el.length ? el : [el], update);
			}
			return el;
		};
	}

	module.exports = autosize;
});
