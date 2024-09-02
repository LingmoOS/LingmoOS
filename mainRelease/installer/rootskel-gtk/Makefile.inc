outdir = $(DESTDIR)/$(dir)

define check_destdir
@if [ -z "$(DESTDIR)" ]; then \
  echo "You *must* give a DESTDIR!"; \
  exit 1; \
fi
endef

build: build-local build-recursive

build-local::

install: install-auto install-local install-recursive

install-auto: $(outdir) install-auto-dirs install-auto-files install-auto-files-exec

install-auto-dirs:
	$(call check_destdir)
	@list='$(subdirs_empty)'; for file in $$list; do \
	  echo "install -d -m755 $(outdir)/$$file"; \
	  install -d -m755 $(outdir)/$$file; \
	done

install-auto-files:
	$(call check_destdir)
	@list='$(files)'; for file in $$list; do \
	  case $$file in \
	    *:*) in=`echo "$$file" | sed 's,[^:]*:,,'` \
	         out=$(outdir)/`echo "$$file" | sed 's,:.*,,'`;; \
	    *) in=$$file out=$(outdir);; \
	  esac; \
	  echo "install -m644 $$in $$out"; \
	  install -m644 $$in $$out; \
	done

install-auto-files-exec:
	$(call check_destdir)
	@list='$(files_exec)'; for file in $$list; do \
	  case $$file in \
	    *:*) in=`echo "$$file" | sed 's,[^:]*:,,'` \
	         out=$(outdir)/`echo "$$file" | sed 's,:.*,,'`;; \
	    *) in=$$file out=$(outdir);; \
	  esac; \
	  echo "install -m755 $$in $$out"; \
	  install -m755 $$in $$out; \
	done

install-local::

clean: clean-local clean-recursive

clean-local::

build-recursive clean-recursive install-recursive:
	@target=`echo $@ | sed s/-recursive//`; \
	list='$(subdirs)'; \
	for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  (cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

$(outdir):
	install -d $(outdir)

