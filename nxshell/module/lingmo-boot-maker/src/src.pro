TEMPLATE  = subdirs
CONFIG   += ordered


SUBDIRS += vendor/src/libxsys \
    libdbm

linux {
    SUBDIRS += service

}
SUBDIRS += app
SUBDIRS += tests
mac* {
    TRANSLATIONS_NAME = lingmo-boot-maker
    TRANSLATIONS += $$PWD/translations/$${TRANSLATIONS_NAME}.ts \
                $$PWD/translations/$${TRANSLATIONS_NAME}_zh_CN.ts
    system(python3 $$PWD/tools/translate_generation.py $$PWD)
} else {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    linux {
        dtk_translations.path = /usr/share/lingmo-boot-maker/translations
        dtk_translations.files = $$PWD/translations/*.qm
        INSTALLS += dtk_translations
    }
}
