MWHOME=/home/michael/vivtek/pages/projects/monkeywrench
WIKISRC = $(wildcard ./*.wiki)
WIKIPG = $(WIKISRC:.wiki=.html)

all: log.html $(WIKIPG)

log.wiki: actions.log
	perl $(MWHOME)/describe_actions.pl actions.log > log.wiki

%.html: %.wiki
	perl /home/michael/vivtek/pages/index.pl $<
	perl /home/michael/vivtek/pages/weave.pl $< > $@

