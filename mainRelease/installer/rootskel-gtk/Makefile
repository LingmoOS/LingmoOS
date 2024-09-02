subdirs = \
	src \

build: build-recursive

install:
	@$(MAKE) install -C src DESTDIR=$(CURDIR)/debian/rootskel-gtk/

clean: clean-recursive

build-recursive clean-recursive:
	@target=`echo $@ | sed s/-recursive//`; \
	list='$(subdirs)'; \
	for subdir in $$list; do \
	  echo "Making install in $$subdir"; \
	  (cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

