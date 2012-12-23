# Contains no serviceable parts.  Void where prohibited.

all: toon.gif

toon.gif: cartoon.svg
	convert cartoon.svg toon.gif
	perl finish_up.pl toon.gif

cartoon.svg: panels.svg
	perl merge_svg.pl "the final cartoon" panels.svg panel-*.svg > cartoon.svg

panels.svg: panels.xml Makefile.panels
	perl draw_panels.pl panels.xml > panels.svg

instance.xml: cartoon.xml
	perl instantiate.pl cartoon.xml > instance.xml

panels.xml: instance.xml
	perl /home/michael/toon-o-matic/build_panels.pl instance.xml > panels.xml

cartoon.xml: toon.txt
	perl interpret_script.pl toon.txt > cartoon.xml

Makefile.panels: panels.xml
	perl build_panel_make.pl this panels.xml > Makefile.panels
	make -f Makefile.panels


clean:
	rm -f *.xml *.svg *.gif *.png Makefile.panels

rebuild: clean all
