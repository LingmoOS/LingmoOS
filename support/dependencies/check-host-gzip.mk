ifeq (,$(call suitable-host-package,gzip))
LINGMO_GZIP_HOST_DEPENDENCY = host-gzip
endif
