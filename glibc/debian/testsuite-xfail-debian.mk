######################################################################
# All architectures
######################################################################
# These can fail when running with other loads or in a virtual machine
test-xfail-tst-cpuclock2 = yes
test-xfail-tst-cputimer1 = yes
test-xfail-tst-timer = yes

# Due to the nature of this test, it's very sensitive to system load
# in that, strangely, it wants more, not less.  Given that's hard to
# control, we'll just let it fail
test-xfail-tst-create-detached = yes

# Due to a bug/limitation in sbuild, this test fails when running unshare
# chroot mode, see bug #1070003.
test-xfail-tst-support_descriptors = yes

######################################################################
# alpha
######################################################################
ifeq ($(config-machine)-$(config-os),alpha-linux-gnu)
test-xfail-tst-backtrace5 = yes
test-xfail-tst-backtrace6 = yes
test-xfail-tst-cancel19 = yes
test-xfail-test-double-finite-fma = yes
test-xfail-test-double-finite-llrint = yes
test-xfail-test-double-finite-llround = yes
test-xfail-test-double-finite-lrint = yes
test-xfail-test-double-finite-lround = yes
test-xfail-test-double-finite-remainder = yes
test-xfail-test-double-finite-scalbln = yes
test-xfail-test-double-finite-scalbn = yes
test-xfail-test-double-fma = yes
test-xfail-test-double-llrint = yes
test-xfail-test-double-llround = yes
test-xfail-test-double-lrint = yes
test-xfail-test-double-lround = yes
test-xfail-test-double-remainder = yes
test-xfail-test-double-scalbln = yes
test-xfail-test-double-scalbn = yes
test-xfail-test-fenv-return = yes
test-xfail-test-fexcept = yes
test-xfail-test-float-double-add = yes
test-xfail-test-float-double-sub = yes
test-xfail-test-float-finite-fma = yes
test-xfail-test-float-finite-llrint = yes
test-xfail-test-float-finite-llround = yes
test-xfail-test-float-finite-lrint = yes
test-xfail-test-float-finite-lround = yes
test-xfail-test-float-finite-scalbln = yes
test-xfail-test-float-finite-scalbn = yes
test-xfail-test-float-fma = yes
test-xfail-test-float-ldouble-add = yes
test-xfail-test-float-ldouble-sub = yes
test-xfail-test-float-llrint = yes
test-xfail-test-float-llround = yes
test-xfail-test-float-lrint = yes
test-xfail-test-float-lround = yes
test-xfail-test-float-scalbln = yes
test-xfail-test-float-scalbn = yes
test-xfail-test-float32-finite-fma = yes
test-xfail-test-float32-finite-llrint = yes
test-xfail-test-float32-finite-llround = yes
test-xfail-test-float32-finite-lrint = yes
test-xfail-test-float32-finite-lround = yes
test-xfail-test-float32-finite-scalbln = yes
test-xfail-test-float32-finite-scalbn = yes
test-xfail-test-float32-float128-add = yes
test-xfail-test-float32-float128-sub = yes
test-xfail-test-float32-float32x-add = yes
test-xfail-test-float32-float32x-sub = yes
test-xfail-test-float32-float64-add = yes
test-xfail-test-float32-float64-sub = yes
test-xfail-test-float32-float64x-add = yes
test-xfail-test-float32-float64x-sub = yes
test-xfail-test-float32-fma = yes
test-xfail-test-float32-llrint = yes
test-xfail-test-float32-llround = yes
test-xfail-test-float32-lrint = yes
test-xfail-test-float32-lround = yes
test-xfail-test-float32-scalbln = yes
test-xfail-test-float32-scalbn = yes
test-xfail-test-float32x-finite-fma = yes
test-xfail-test-float32x-finite-llrint = yes
test-xfail-test-float32x-finite-llround = yes
test-xfail-test-float32x-finite-lrint = yes
test-xfail-test-float32x-finite-lround = yes
test-xfail-test-float32x-finite-remainder = yes
test-xfail-test-float32x-finite-scalbln = yes
test-xfail-test-float32x-finite-scalbn = yes
test-xfail-test-float32x-fma = yes
test-xfail-test-float32x-llrint = yes
test-xfail-test-float32x-llround = yes
test-xfail-test-float32x-lrint = yes
test-xfail-test-float32x-lround = yes
test-xfail-test-float32x-remainder = yes
test-xfail-test-float32x-scalbln = yes
test-xfail-test-float32x-scalbn = yes
test-xfail-test-float64-finite-fma = yes
test-xfail-test-float64-finite-llrint = yes
test-xfail-test-float64-finite-llround = yes
test-xfail-test-float64-finite-lrint = yes
test-xfail-test-float64-finite-lround = yes
test-xfail-test-float64-finite-remainder = yes
test-xfail-test-float64-finite-scalbln = yes
test-xfail-test-float64-finite-scalbn = yes
test-xfail-test-float64-fma = yes
test-xfail-test-float64-llrint = yes
test-xfail-test-float64-llround = yes
test-xfail-test-float64-lrint = yes
test-xfail-test-float64-lround = yes
test-xfail-test-float64-remainder = yes
test-xfail-test-float64-scalbln = yes
test-xfail-test-float64-scalbn = yes
test-xfail-test-float64x-finite-llrint = yes
test-xfail-test-float64x-finite-llround = yes
test-xfail-test-float64x-finite-lrint = yes
test-xfail-test-float64x-finite-lround = yes
test-xfail-test-float64x-llrint = yes
test-xfail-test-float64x-llround = yes
test-xfail-test-float64x-lrint = yes
test-xfail-test-float64x-lround = yes
test-xfail-test-float128-finite-llrint = yes
test-xfail-test-float128-finite-llround = yes
test-xfail-test-float128-finite-lrint = yes
test-xfail-test-float128-finite-lround = yes
test-xfail-test-float128-llrint = yes
test-xfail-test-float128-llround = yes
test-xfail-test-float128-lrint = yes
test-xfail-test-float128-lround = yes
test-xfail-test-ldouble-finite-llrint = yes
test-xfail-test-ldouble-finite-llround = yes
test-xfail-test-ldouble-finite-lrint = yes
test-xfail-test-ldouble-finite-lround = yes
test-xfail-test-ldouble-llrint = yes
test-xfail-test-ldouble-llround = yes
test-xfail-test-ldouble-lrint = yes
test-xfail-test-ldouble-lround = yes
test-xfail-tst-eintr1 = yes
test-xfail-tst-mqueue5 = yes
test-xfail-tst-prelink-cmp = yes
test-xfail-tst-signal3 = yes
test-xfail-tst-timer4 = yes
test-xfail-tst-waitid = yes
test-xfail-tst-writev = yes
endif


######################################################################
# amd64
######################################################################
ifeq ($(config-machine)-$(config-os),x86_64-linux-gnu)
# This test fails intermittently on amd64. It could be a kernel issue.
# see https://sourceware.org/bugzilla/show_bug.cgi?id=19004
test-xfail-tst-robust8 = yes
endif


######################################################################
# arm64
######################################################################
ifeq ($(config-machine)-$(config-os),aarch64-linux-gnu)
endif


######################################################################
# armel
######################################################################
ifeq ($(config-machine)-$(config-os),arm-linux-gnueabi)
endif


######################################################################
# armhf
######################################################################
ifeq ($(config-machine)-$(config-os),arm-linux-gnueabihf)
endif


######################################################################
# hppa
######################################################################
ifeq ($(config-machine)-$(config-os),hppa-linux-gnu)
test-xfail-check-execstack = yes
test-xfail-check-localplt = yes
test-xfail-check-textrel = yes
test-xfail-tst-audit2 = yes
test-xfail-tst-backtrace2 = yes
test-xfail-tst-backtrace3 = yes
test-xfail-tst-backtrace4 = yes
test-xfail-tst-backtrace5 = yes
test-xfail-tst-backtrace6 = yes
test-xfail-tst-cleanupx4 = yes
test-xfail-tst-makecontext = yes
test-xfail-tst-preadvwritev2 = yes
test-xfail-tst-preadvwritev64v2 = yes
test-xfail-tst-protected1a = yes
test-xfail-tst-protected1b = yes
test-xfail-tst-setcontext2 = yes
test-xfail-tst-setcontext7 = yes
test-xfail-tst-stack4 = yes
# The following tests sometimes fail due to timeouts.
test-xfail-tst-barrier5 = yes
test-xfail-tst-cond24 = yes
test-xfail-tst-cond25 = yes

# The following tests fail as xsigstack.c does not allocate stack
# with executable permission
# See https://sourceware.org/bugzilla/show_bug.cgi?id=24914
test-xfail-tst-minsigstksz-1 = yes
test-xfail-tst-xsigstack = yes

# See https://sourceware.org/bugzilla/show_bug.cgi?id=27654
test-xfail-tst-readdir64-compat = yes

# https://www.spinics.net/lists/linux-parisc/msg15397.html
test-xfail-tst-minsigstksz-5 = yes

# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=106458
test-xfail-tst-scratch_buffer = yes

# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111709
test-xfail-test-double-fma = yes
test-xfail-test-double-ldouble-fma = yes
test-xfail-test-float32x-float64-fma = yes
test-xfail-test-float32x-fma = yes
test-xfail-test-float64-fma = yes
test-xfail-test-ldouble-fma = yes
endif


######################################################################
# hurd-i386 and hurd-amd64 (including optimized flavours)
######################################################################
ifeq ($(config-os),gnu-gnu)
# sysdeps/mach/hurd/dl-sysdep.c's open_file does not support the linker
# creating files.
test-xfail-tst-null-argv = yes

# We don't provide /proc/cpuinfo yet
test-xfail-test-multiarch = yes
test-xfail-tst-cpu-features-cpuinfo = yes
test-xfail-tst-cpu-features-cpuinfo-static = yes

# Need actual porting
test-xfail-exe = yes

# TODO: in _hurd_port2fd store the flags in a new field in the hurd_fd
# structure, and in __fdopendir pass over the O_NOATIME flag to the
# __file_name_lookup_under call.
test-xfail-tst-fdopendir = yes

# Overzealous test
test-xfail-tst-pathconf = yes

# aio_suspend and lio_listio emulations use pthread_cond_wait, and thus can't be interrupted by a signal
test-xfail-tst-aio10 = yes
test-xfail-tst-aio9 = yes

# Needs LD_AUDIT support
test-xfail-tst-audit1 = yes
test-xfail-tst-audit2 = yes
test-xfail-tst-audit3 = yes
test-xfail-tst-audit8 = yes
test-xfail-tst-audit9 = yes
test-xfail-tst-audit14 = yes
test-xfail-tst-audit14-cmp = yes
test-xfail-tst-audit14a = yes
test-xfail-tst-audit14a-cmp = yes
test-xfail-tst-audit15 = yes
test-xfail-tst-audit15-cmp = yes
test-xfail-tst-audit16 = yes
test-xfail-tst-audit16-cmp = yes
test-xfail-tst-audit18 = yes
test-xfail-tst-audit20 = yes
test-xfail-tst-audit23 = yes
test-xfail-tst-audit24a = yes
test-xfail-tst-audit24b = yes
test-xfail-tst-audit24c = yes
test-xfail-tst-audit24d = yes
test-xfail-tst-audit25a = yes
test-xfail-tst-audit25b = yes

# We always put LD_ORIGIN_PATH in the environment
test-xfail-tst-execvpe5 = yes

# Crashes on dividing by a profiling period 0 (not initialized)
test-xfail-tst-sprofil = yes

# Missing RT signals.
# And without rt_sigqueueinfo thread_expire_timer can't pass the si_code = SI_TIMER
test-xfail-tst-timer4 = yes
test-xfail-tst-timer5 = yes

# want /proc/self/fd
# TODO: make them use FD_TO_FILENAME_PREFIX from <arch-fd_to_filename.h>
test-xfail-tst-if_index-long = yes
test-xfail-tst-support_descriptors = yes
test-xfail-tst-updwtmpx = yes
test-xfail-tst-lchmod = yes
test-xfail-tst-closefrom = yes
test-xfail-tst-close_range = yes
test-xfail-tst-support-open-dev-null-range = yes
test-xfail-tst-spawn5 = yes
test-xfail-tst-open-tmpfile = yes
test-xfail-tst-closedir-leaks = yes
test-xfail-tst-closedir-leaks-mem = yes

# new in 2.22
test-xfail-tst-prelink = yes

# new in 2.24
test-xfail-tst-spawn2 = yes

# fails randomly
test-xfail-tst-preadvwritev64 = yes
test-xfail-tst-preadwrite64 = yes

# needs sigwaitinfo
test-xfail-tst-waitid = yes
test-xfail-tst-wait4 = yes

# new in 2.25
test-xfail-tst-posix_fadvise = yes
test-xfail-tst-posix_fadvise64 = yes

# new in 2.26
test-xfail-tst-malloc-tcache-leak = yes
test-xfail-tst-dynarray-fail-mem = yes
test-xfail-test-errno = yes

# new in 2.27
test-xfail-tst-tls1-static-non-pie = yes

# new in 2.30
test-xfail-tst-nss-files-hosts-long = yes

# wants pthread_barrierattr_setpshared
test-xfail-tst-pututxline-cache = yes
test-xfail-tst-pututxline-lockfail = yes

# new in 2.32
# Assumes some linuxish strings
test-xfail-tst-strerror = yes
# We always have several threads
test-xfail-tst-single_threaded-pthread = yes
# known to be fixed by the siginfo patch by fixing the returned value
# #  But for dlsym errors it still returns 9 instead of 127...
test-xfail-tst-latepthread = yes
test-xfail-tst-initfinilazyfail = yes

# new in 2.33
# Mach misses getting adjtime without privileges
test-xfail-tst-adjtime = yes
test-xfail-tst-join15 = yes
test-xfail-tst-reload1 = yes
test-xfail-tst-reload2 = yes
test-xfail-tst-canon-bz26341 = yes

# fixed in 2.33
test-xfail-tst-spawn4-compat = yes

# new in 2.34
test-xfail-tst-itimer = yes
test-xfail-tst-wait3 = yes
test-xfail-tst-nss-compat1 = yes
test-xfail-tst-dlinfo-phdr = yes

# new in 2.35
test-xfail-tst-compathooks-on = yes
test-xfail-tst-sched_getaffinity = yes
test-xfail-tst-malloc-tcache-leak-malloc-hugetlb1 = yes
test-xfail-tst-malloc-tcache-leak-malloc-hugetlb2 = yes

# new in 2.36
test-xfail-tst-arc4random-fork = yes
test-xfail-tst-arc4random-thread = yes
test-xfail-tst-nss-gai-actions = yes

# new in 2.37
test-xfail-tst-fcntl-lock = yes
test-xfail-tst-fcntl-lock-lfs = yes
test-xfail-tst-nss-gai-hv2-canonname = yes

# new in 2.38
test-xfail-tst-sprof-basic = yes
test-xfail-tst-nss-files-hosts-v4mapped = yes
test-xfail-test-canon = yes

# new in 2.39
test-xfail-tst-initgroups1 = yes
test-xfail-tst-initgroups2 = yes
test-xfail-tst-sprintf-fortify-rdonly = yes
test-xfail-tst-plt-rewrite1 = yes
test-xfail-tst-tunables = yes
test-xfail-tst-mremap1 = yes
test-xfail-tst-mremap2 = yes

# new in 2.40
test-xfail-tst-recursive-tls = yes
test-xfail-tst-malloc-alternate-path = yes
test-xfail-tst-malloc-alternate-path-malloc-check = yes
test-xfail-tst-malloc-alternate-path-malloc-hugetlb1 = yes
test-xfail-tst-malloc-alternate-path-malloc-hugetlb2 = yes
test-xfail-tst-malloc-alternate-path-mcheck = yes
test-xfail-tst-shutdown = yes
test-xfail-tst-truncate64 = yes

# new in 2.41
test-xfail-tst-getrandom2 = yes
test-xfail-tst-freopen2 = yes
test-xfail-tst-freopen3 = yes
test-xfail-tst-freopen5 = yes
test-xfail-tst-freopen64-2 = yes
test-xfail-tst-freopen64-3 = yes
test-xfail-tst-freopen2-mem = yes
test-xfail-tst-freopen3-mem = yes
test-xfail-tst-freopen5-mem = yes
test-xfail-tst-freopen64-2-mem = yes
test-xfail-tst-freopen64-3-mem = yes
test-xfail-tst-clock_gettime = yes
test-xfail-tst-clock_nanosleep2 = yes
test-xfail-tst-dlopen-auditdup = yes
test-xfail-tst-rtld-no-malloc = yes
test-xfail-tst-rtld-no-malloc-audit = yes
test-xfail-tst-rtld-no-malloc-preload = yes
test-xfail-tst-execstack-prog-noexecstack = yes
test-xfail-tst-support-process_state = yes
test-xfail-tst-audit12 = yes
test-xfail-tst-audit28 = yes
test-xfail-tst-execstack-prog-static-tunable = yes

# new in 2.42
test-xfail-tst-qsort7-mem = yes
test-xfail-tst-qsortx7-mem = yes
test-xfail-tst-fclose-devzero = yes
test-xfail-tst-malloc-tcache-leak-malloc-largetcache = yes
test-xfail-tst-malloc-too-large-malloc-largetcache = yes
test-xfail-tst-mallocfork2-malloc-largetcache = yes
test-xfail-tst-malloc-alternate-path-malloc-largetcache = yes

# actually never succeded
test-xfail-tst-create_format1 = yes
test-xfail-tst-getcwd-abspath = yes
test-xfail-tst-udp-error = yes

# Child seems to be inheriting the lockf from the parent?
test-xfail-tst-lockf = yes

# assumes that all st_mode flags (32bit) can exist in stx_mode flags (16bit)
test-xfail-tst-statx = yes

# Some issues with FPU flags
test-xfail-test-fenv = yes
test-xfail-test-fenv-sse-2 = yes

# we don't actually set the secure flag when setgid doesn't actually increase permissions
test-xfail-tst-env-setuid-static = yes
test-xfail-tst-env-setuid-tunables = yes
endif


######################################################################
# hurd-amd64
######################################################################
ifeq ($(config-machine)-$(config-os),x86_64-gnu-gnu)
# TODO: fix default FPU config
test-xfail-test-float64x-acos = yes
test-xfail-test-float64x-log10 = yes
test-xfail-test-float64x-log2 = yes
test-xfail-test-float64x-y0 = yes
test-xfail-test-float64x-y1 = yes
test-xfail-test-ldouble-acos = yes
test-xfail-test-ldouble-log10 = yes
test-xfail-test-ldouble-log2 = yes
test-xfail-test-ldouble-y0 = yes
test-xfail-test-ldouble-y1 = yes

# memory leak
test-xfail-tst-vfprintf-width-prec-mem = yes
test-xfail-tst-vfprintf-width-prec = yes

# timeout
test-xfail-tst-malloc-too-large = yes
test-xfail-tst-malloc-too-large-malloc-check = yes
test-xfail-tst-malloc-too-large-malloc-hugetlb1 = yes
test-xfail-tst-malloc-too-large-malloc-hugetlb2 = yes

# cmsg bug, fixed in glibc 2.41
test-xfail-tst-cmsghdr = yes

# missing support
test-xfail-tst-map-32bit-1a = yes
test-xfail-tst-map-32bit-1b = yes
test-xfail-tst-map-32bit-2 = yes

test-xfail-tst-platform-1 = yes
test-xfail-tst-audit4 = yes
test-xfail-tst-audit5 = yes
test-xfail-tst-audit6 = yes
test-xfail-tst-audit7 = yes
test-xfail-tst-audit10 = yes
endif


######################################################################
# hurd-i386
######################################################################
ifeq ($(config-machine)-$(config-os),i686-gnu-gnu)
test-xfail-tst-posix_fallocate64 = yes

# new in 2.34
test-xfail-tst-tls-allocation-failure-static-patched = yes
endif


######################################################################
# i386
######################################################################
ifeq ($(config-machine)-$(config-os),i686-linux-gnu)
endif


######################################################################
# m68k
######################################################################
ifeq ($(config-machine)-$(config-os),m68k-linux-gnu)
test-xfail-annexc = yes
test-xfail-bug-nextafter = yes
test-xfail-bug-nexttoward = yes
test-xfail-bug-regex20 = yes
test-xfail-check-localplt = yes
test-xfail-localedata/sort-test = yes
test-xfail-test-float64 = yes
test-xfail-test-fenv = yes
test-xfail-test-float32 = yes
test-xfail-test-ifloat64 = yes
test-xfail-test-ifloat32 = yes
test-xfail-test-misc = yes
test-xfail-tst-atomic-long = yes
test-xfail-tst-atomic = yes
test-xfail-tst-mqueue5 = yes
test-xfail-tst-mqueue6 = yes
test-xfail-tst-rxspencer = yes
endif


######################################################################
# mips*
######################################################################
ifneq (,$(filter $(config-machine)-$(config-os), mips-linux-gnu mipsel-linux-gnu mips64-linux-gnuabi64 mips64el-linux-gnuabi64 mips64-linux-gnuabin32 mips64el-linux-gnuabin32))
test-xfail-tst-stack4 = yes
test-xfail-tst-ro-dynamic = yes

# MIPS GCC does not use PT_GNU_STACK markers (this is a GCC issue)
test-xfail-check-execstack = yes

# Theses failures are due to a bug in the cvt.s.d instruction on some FPU
# (at least Octeon 3 and XBurst). The tininess detection is done on a
# before-rounding basis instead of an after-rounding basis.
test-xfail-test-float-double-add = yes
test-xfail-test-float-double-div = yes
test-xfail-test-float-double-fma = yes
test-xfail-test-float-double-mul = yes
test-xfail-test-float-double-sub = yes
test-xfail-test-float-finite-fma = yes
test-xfail-test-float-fma = yes
test-xfail-test-float-ldouble-add = yes
test-xfail-test-float-ldouble-div = yes
test-xfail-test-float-ldouble-fma = yes
test-xfail-test-float-ldouble-mul = yes
test-xfail-test-float-ldouble-sub = yes
test-xfail-test-float32-finite-fma = yes
test-xfail-test-float32-float32x-add = yes
test-xfail-test-float32-float32x-div = yes
test-xfail-test-float32-float32x-fma = yes
test-xfail-test-float32-float32x-mul = yes
test-xfail-test-float32-float32x-sub = yes
test-xfail-test-float32-float64-add = yes
test-xfail-test-float32-float64-div = yes
test-xfail-test-float32-float64-fma = yes
test-xfail-test-float32-float64-mul = yes
test-xfail-test-float32-float64-sub = yes
test-xfail-test-float32-fma = yes

# Theses failures are due to a bug in the cvt.d.s instruction on some FPU
# (at least Octeon 3 and XBurst). The qNaN payload is not preserved in
# the conversion and a new qNaN is generated.
test-xfail-tst-strfrom = yes
test-xfail-tst-strfrom-locale = yes

# These audit failures seems to be due to the MIPS ELF specificities:
test-xfail-tst-audit24a = yes
test-xfail-tst-audit24b = yes
test-xfail-tst-audit24c = yes
test-xfail-tst-audit24d = yes
test-xfail-tst-audit25a = yes
test-xfail-tst-audit25b = yes

# dl_profile is not supported on mips*
test-xfail-tst-sprof-basic = yes
endif


######################################################################
# little-endian mips*
######################################################################
ifneq (,$(filter $(config-machine)-$(config-os), mipsel-linux-gnu mips64el-linux-gnuabi64 mips64el-linux-gnuabin32))
# These failures are due to a bug in the Loongson 3A FPU
test-xfail-test-double-finite-lround = yes
test-xfail-test-double-lround = yes
test-xfail-test-float32x-finite-lround = yes
test-xfail-test-float32x-lround = yes
test-xfail-test-float64-finite-lround = yes
test-xfail-test-float64-lround = yes
test-xfail-test-idouble-lround = yes
test-xfail-test-ifloat32x-lround = yes
test-xfail-test-ifloat64-lround = yes
test-xfail-test-ildouble-lround = yes
test-xfail-test-ldouble-finite-lround = yes
test-xfail-test-ldouble-lround = yes
endif


######################################################################
# O32 mips*
######################################################################
ifneq (,$(filter $(config-machine)-$(config-os), mips64-linux-gnu mips64el-linux-gnu))
endif


######################################################################
# N64 mips*
######################################################################
ifneq (,$(filter $(config-machine)-$(config-os), mips64-linux-gnuabi64 mips64el-linux-gnuabi64))
endif


######################################################################
# ppc64el
######################################################################
ifeq ($(config-machine)-$(config-os),powerpc64le-linux-gnu)
endif


######################################################################
# ppc64
######################################################################
ifeq ($(config-machine)-$(config-os),powerpc64-linux-gnu)
test-xfail-tst-backtrace5 = yes
test-xfail-tst-backtrace6 = yes
test-xfail-tst-mqueue5 = yes
test-xfail-tst-waitid = yes
endif


######################################################################
# powerpc
######################################################################
ifeq ($(config-machine)-$(config-os),powerpc-linux-gnu)
test-xfail-tst-backtrace5 = yes
test-xfail-tst-backtrace6 = yes
test-xfail-tst-mqueue5 = yes
test-xfail-tst-waitid = yes

# The 32-bit protection key behavior is somewhat unclear on 32-bit powerpc.
test-xfail-tst-pkey = yes

# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67771
test-xfail-test-float-log10 = yes
test-xfail-test-float32-log10 = yes
endif


######################################################################
# riscv64
######################################################################
ifeq ($(config-machine)-$(config-os),riscv64-linux-gnu)
endif


######################################################################
# s390x
######################################################################
ifeq ($(config-machine)-$(config-os),s390x-linux-gnu)
endif


######################################################################
# sparc
######################################################################
ifeq ($(config-machine)-$(config-os),sparc-linux-gnu)
test-xfail-annexc = yes
test-xfail-check-localplt = yes
test-xfail-tst-backtrace2 = yes
test-xfail-tst-backtrace3 = yes
test-xfail-tst-backtrace4 = yes
test-xfail-tst-backtrace5 = yes
test-xfail-tst-backtrace6 = yes
test-xfail-tst-waitid = yes

# Even if configured using --with-long-double-128, the biarch32 compiler
# on sparc64 defaults to 64-bit doubles, causing the failure below. This
# should be fixed by the following gcc patch:
# http://gcc.gnu.org/ml/gcc-patches/2013-12/msg00318.html
test-xfail-isomac = yes
endif


######################################################################
# sparc64
######################################################################
ifeq ($(config-machine)-$(config-os),sparc64-linux-gnu)
test-xfail-ISO/setjmp.h/conform = yes
test-xfail-ISO11/setjmp.h/conform = yes
test-xfail-ISO99/setjmp.h/conform = yes
test-xfail-POSIX/pthread.h/conform = yes
test-xfail-POSIX/setjmp.h/conform = yes
test-xfail-POSIX2008/pthread.h/conform = yes
test-xfail-POSIX2008/setjmp.h/conform = yes
test-xfail-UNIX98/pthread.h/conform = yes
test-xfail-UNIX98/setjmp.h/conform = yes
test-xfail-XOPEN2K/pthread.h/conform = yes
test-xfail-XOPEN2K/setjmp.h/conform = yes
test-xfail-XOPEN2K8/pthread.h/conform = yes
test-xfail-XOPEN2K8/setjmp.h/conform = yes
test-xfail-XPG4/setjmp.h/conform = yes
test-xfail-isomac = yes
test-xfail-tst-cancel24-static = yes
test-xfail-tst-cond8-static = yes
test-xfail-tst-mutex8-static = yes
test-xfail-tst-mutexpi8-static = yes
test-xfail-tst-protected1a = yes
test-xfail-tst-protected1b = yes
test-xfail-tst-realloc = yes
test-xfail-tst-rtld-run-static = yes
test-xfail-tst-socket-timestamp = yes
test-xfail-tst-waitid = yes
test-xfail-test-float64x-float128-mul=yes
endif


######################################################################
# x32
######################################################################
ifeq ($(config-machine)-$(config-os),x86_64-linux-gnux32)
test-xfail-tst-platform-1 = yes
endif
