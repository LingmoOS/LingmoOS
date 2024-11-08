TEMPLATE = subdirs
QMAKE_POST_LINK += make clean
SUBDIRS += \
    src/plugins/VirtualKeyboard \
    src/plugins/Reflex \
    src/plugins/PluginDll \
    src/plugins/PluginService \
    src/plugins/KChoiceLanguage \
    src/plugins/KTimeZone \
    src/plugins/PluginTask \
    src/plugins/KyLicense \
    src/plugins/KUserRegister \
    src/plugins/KPartition \
    src/Installer_main


CONFIG += ordered \
    qt


TRANSLATIONS += \
    $$PWD/language/zh_CN.ts \
    $$PWD/language/kk_KZ.ts \
    $$PWD/language/ug_CN.ts \
    $$PWD/language/ky_KG.ts \
    $$PWD/language/zh_HK.ts \
    $$PWD/language/bo_CN.ts \
    $$PWD/language/mn_MN.ts \
    $$PWD/language/en_US.ts \
    $$PWD/language/es_ES.ts \
    $$PWD/language/de_DE.ts \
    $$PWD/language/fr_FR.ts

#INCLUDEPATH += /home/xy/Downloads/workdir/git/2020-6new




