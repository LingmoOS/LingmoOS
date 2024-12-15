TEMPLATE  = subdirs
CONFIG   += ordered
SUBDIRS  += \
    src  \

linux {
manual_dir.files = $$PWD/src/help/lingmo-boot-maker
manual_dir.path = /usr/share/lingmo-manual/manual-assets/application
INSTALLS += manual_dir
}