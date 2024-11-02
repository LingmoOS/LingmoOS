TEMPLATE = subdirs

VERSION = 3.2.2
DEFINES += VERSION='\\"$${VERSION}\\"'

SUBDIRS = \
#    explor-admin                         \
    explor-share                         \
    explor-drive-rename                  \
    explor-send-to-device                \
    explor-set-wallpaper                 \
    explor-bluetooth-plugin              \
    explor-engrampa-menu-plugin          \
    explor-extension-computer-view       \
    explor-menu-plugin-mate-terminal     \
#    test-computer-view
