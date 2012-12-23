This whole thing assumes you've got expat installed and built.  So do that first.
Then modify the Makefiles in the wftk and xmlapi directories so that they know
where expat is.  Then make.  You have to be using GNU make, because I'm doing that
recurse-into-subdirs thing.

Good luck.
