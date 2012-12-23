wftk : open-source workflow toolkit.                  Version:      0.9beta
http://www.wftk.org                                   Platform:     Unix (Solaris anyway)
(c) 2001 Vivtek.                                      Release date: 04/20/2001

Original author: Michael Roberts                      Email: wftk@vivtek.com

WFTK LICENSE TERMS
------------------
This is the wftk.  It's available to you under the GNU Public License (GPL), which you can
find in COPYING.txt.  The GNU License states that you can copy and modify this program
however you like, and give it to whomever you like, as long as you credit Vivtek with
the original work, make it clear that it's not Vivtek's fault if you broke things, and
that Vivtek doesn't warrant anything anyway.  In addition, your derived works have to
be GPL'd as well, to preserve other people's rights.

Parts of the wftk are available under the GNU Library License, meaning that you
can statically link library portions of the code into your own proprietary products, but 
you still have to make it known that portions of the wftk were used to make your products,
and that Vivtek did that work, and that people can get the source code at the project
home page at the Vivtek site.

For all you regular open-source types out there, I'm aiming this program smack dab at the
corporate M$ junky crowd.  They don't know the GPL from ZZ Top, so I wanted to get
everybody on the same page before getting down to business.

WHAT'S WFTK?
------------
The wftk is an open-source workflow toolkit.  This means that it is a design goal to be able
to use wftk components to build and integrate any type of workflow functionality into your
corporate systems, Web sites, desktop programs, or whatever other software you might have
lying around.  This release, the 0.9beta release, realizes that goal.  The reason this is a
beta release is that I haven't used it in any real-life situations.  That will hopefully b
changing rather soon.

If you are involved in the installation or specification of wftk, consider me a partner.
If you want to pay me, that's gravy, but even if you don't, I really want to see wftk functioning
in a varied set of environments.  If you're running into problems and you can grant me a telnet
or ssh login, let's see what I can do.  NO PROMISES (see the licensing) but I'm motivated.

WHAT'S WHERE
------------
OK.  This version is v0.9beta.  With this version, I've established the following directories:
 - datasheets - the default datasheet repository
 - groups     - the default user group repository
 - perms      - the default permissions repository
 - procdefs   - the default procdef repository
 - users      - the default user repository

 - doc        - installation, customization, code, and user documentation
 - src        - all source code and documentation of internals, including modules
 - include    - all include files used by C code, including localdefs.h
 - lib        - all library files.  See lib directory for licensing terms.

The source directory, which is optional for distribution, has the following structure:
 - wftk       - code and executable directory for the core workflow engine
 - xmlapi     - code directory for the C-language XML manipulation module

As I get more integration scenarios, there will be more source directories.  Note that
adaptor modules are located in subdirectories of the src/wftk directory.

The explanations of these are opaque, but read the documentation and the code and it
will all be crystal clear.  Note that the default repository directories needn't be
located in the wftk directory, and can be specified in the config.xml file which is right
in the main directory.

MODIFYING THE CODE
------------------
A word of warning.  You may get this installed, see all those juicy C files, modify some
stuff, and think that when you send me diffs I'll rejoice.  I won't.  The C code *isn't
the source code* -- it's derivative of the markup-language XML files which I use to 
document the code.  So changes must be made to those XML files, run through lpml.pl (which
is right up at the top of the directory tree where you can't miss it), and *then* compiled.

The Makefiles reflect this priority (at least I hope they all do).  But of course if you
modify the C files directly, the Makefile won't warn you.  That's why I'm warning you.
Now, if you actually do modify C files and fix or extend something, of course I will read
through your patches and incorporate the good stuff -- but it will be a lot more work if
you do it that way, and so I'll do it very, very slowly.

A NOTE FOR WINDOWS USERS
------------------------
Compiling under Windows is something I do less and less these days.  If you want to use my
system, note that there is a build.bat in each code directory.  That batch file runs LPML
to generate C source, then calls MSVC to compile each binary.  It's not smart, but it works.
For your local path settings to work, you'll need to modify MSVC.BAT in the top directory.
That just sets the PATH, INCLUDE, and LIB variables so MSVC can find things.  I haven't
tried this with other Windows compilers because I don't have any.  You're welcome to try
if that's what floats your boat.

If anybody wants to build an MSVC workspace that does what it needs to do, feel free to do
so.  I'll incorporate it in this distribution.

Note that to run LPML you'll need Perl.  I use a build of Perl for MINGW32 (Minimal GNU Win32)
which I rolled myself.  Not that I needed a whole lot of brainpower to do that, but it just
*sounds* like big juju magic, doesn't it?

INSTALLATION GUIDE
------------------
It's in the documentation directory (/doc) as, um, install.html.


Michael Roberts
wftk@vivtek.com
http://www.vivtek.com/wftk.html

