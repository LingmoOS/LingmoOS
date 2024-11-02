TEMPLATE = subdirs

SUBDIRS += plugins 3rdparty

plugins.depends += 3rdparty

QMAKE_EXTRA_TARGETS += run-tests
