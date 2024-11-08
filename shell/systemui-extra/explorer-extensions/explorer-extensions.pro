TEMPLATE = subdirs

VERSION = 3.2.2
DEFINES += VERSION='\\"$${VERSION}\\"'

SUBDIRS = \
#    explorer-admin                         \
    explorer-share                         \
    explorer-drive-rename                  \
    explorer-send-to-device                \
    explorer-set-wallpaper                 \
    explorer-bluetooth-plugin              \
    explorer-engrampa-menu-plugin          \
    explorer-extension-computer-view       \
    explorer-menu-plugin-mate-terminal     \
#    test-computer-view
