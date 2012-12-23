WIKISRC = $(wildcard ./*.wiki)
WIKIPG = $(WIKISRC:.wiki=.html)

all: $(WIKIPG)

%.html: %.wiki
	perl /home/michael/vivtek/pages/index.pl $<
	perl /home/michael/vivtek/pages/weave.pl $< > $@

