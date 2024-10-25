dir = usr/share/themes

files = \
	Clearlooks/gtk-2.0/gtkrc \
	dark/gtk-2.0/gtkrc

#  I should put Makefiles in all intermediate directories, let's get lazy
build-recursive clean-recursive install-recursive:
	-@:

build clean:
	-@:

install:
	for file in $(files); \
	do \
	  mkdir -p `dirname $(DESTDIR)/$(dir)/$$file`; \
	  cp $$file $(DESTDIR)/$(dir)/$$file; \
	done
