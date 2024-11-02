TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    lingmosdk-soundeffects \
    lingmosdk-notification

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/lingmosdk/desktop/
target.path = /usr/lib/$$QMAKE_HOST.arch-linux-gnu
INSTALLS += target headers
