TEMPLATE = subdirs
CONFIG += NO_KEYWORDS

SUBDIRS = libdisomaster

BUILD_TESTS {
        SUBDIRS += tests
}
