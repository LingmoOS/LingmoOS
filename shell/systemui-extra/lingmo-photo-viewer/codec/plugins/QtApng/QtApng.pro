load(qt_parts)

runtests.target = run-tests
runtests.CONFIG = recursive
runtests.recurse_target = run-tests
runtests.recurse += sub_tests sub_src
QMAKE_EXTRA_TARGETS += runtests

DISTFILES += \
	.qmake.conf \
	README.md \
	sync.profile
