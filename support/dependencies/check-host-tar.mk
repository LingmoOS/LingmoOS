TAR ?= tar

ifeq (,$(call suitable-host-package,tar,$(TAR)))
TAR = $(HOST_DIR)/bin/tar
LINGMO_TAR_HOST_DEPENDENCY = host-tar
endif
