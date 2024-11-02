TEMPLATE = subdirs

SUBDIRS += \
	apngPlugin

prepareRecursiveTarget(run-tests)
QMAKE_EXTRA_TARGETS += run-tests
