# XZCAT is taken from LINGMO_XZCAT (defaults to 'xzcat') (see Makefile)
# If it is not present, build our own host-xzcat

ifeq (,$(call suitable-host-package,xzcat,$(XZCAT)))
LINGMO_XZCAT_HOST_DEPENDENCY = host-xz
XZCAT = $(HOST_DIR)/bin/xzcat
endif
