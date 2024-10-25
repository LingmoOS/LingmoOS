PROGS = genksyms

include $(top_rulesdir)/Makefile.inc

genksyms: genksyms.o parse.tab.o lex.lex.o

lex.lex.o: keywords.c parse.tab.h

parse.tab.c: parse.y
	$(YACC) -o$@ -t -l $<

parse.tab.h: parse.y
	$(YACC) -o/dev/null --defines=$@ -t -l $<

lex.lex.c: lex.l
	$(LEX) -o$@ -L $<
