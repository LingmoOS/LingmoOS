PROGS = conf

include $(top_rulesdir)/Makefile.inc

conf: conf.o confdata.o expr.o lexer.lex.o menu.o parser.tab.o preprocess.o symbol.o util.o

lexer.lex.o: parser.tab.h

%.lex.c: %.l
	flex -o$@ -L $<

%.tab.c: %.y
	bison -o$@ -t -l $<

%.tab.h: %.y
	bison -o/dev/null --defines=$@ -t -l $<
