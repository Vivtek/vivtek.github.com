VIVTEK CONTENT
--------------

I registered vivtek.com on September 25, 1997 and have "operated" it continuously ever since.
In practice, that means I write things occasionally and put it there, or build tools and put them
there (since the widespread existence of Web-side spammers, that's been a lot less fun).

I never delete anything.  (Except spam, because otherwise I'd have the world's largest spam
archive.)

Every few years, I do something major, like moving to a new server or trying online editing.
This year (Christmas 2012) I'm moving static content to Github.  But since I still don't delete
anything, ever, the last fifteen years of my Web doings are right here.  Some of it might be
interesting, some maybe not so much.  I don't even know what all is here, and don't really
care - I still can't bear to part with any of it.

It is hereby CC BY-SA. Use anything you like, just tell people you got it from me, and we're
good.

So anyway, what you see here on this site is the *content*, plus the Perl machinery to build
the actual static site.  What that means right now isn't much at all - I used to have some
Tcl code on my server that replaced [##tags##] with *.tag files of the same name in the
directory (with upwards inheritance) and that ended up working fairly well.

Then I decided to do some Markdown-type stuff and page templates, so many of the pages are
on that basis.  I'm not even 100% sure which are which, but going forward I may or may not
clean them all up.

Short version: some HTML here is just HTML.  Some HTML has [##tags##] that are replaced into
the final file.  Some *.wiki files are passed through template.template to get the final
HTML version.  And eventually, some files will also be real Markdown using my Heckle module,
assuming I ever finish writing it.

Clear?  Sure it is.

The whole thing builds into a separate directory that is also in Github, and that serves up
all the static content I've written to date.  Tools will still be on my server, but now when
a tool goes insane it won't bring my static content down.  (It'll still clobber my mail,
but I'll deal with that later.)