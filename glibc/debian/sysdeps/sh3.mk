# Renesas SH enabled -ffinte-math-only. Some software need -mieee.
extra_cflags = -mieee

# GCC 5 and later emits calls to abort() when there is no target specific
# __builtin_trap() implementation. This is not possible to do so in ld.so
# so we need to pass the -fno-delete-null-pointer-checks option to GCC.
extra_cflags += -fno-delete-null-pointer-checks

# Workaround BZ #29575
extra_config_options = --disable-default-pie
