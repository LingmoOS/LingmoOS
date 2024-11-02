TEMPLATE = subdirs
SUBDIRS =src \
         codec\
         photo-viewer-aiassistant-plugin

TRANSLATIONS += \
    translations/lingmo-photo-viewer_zh_CN.ts \
    translations/lingmo-photo-viewer_bo_CN.ts \
    translations/lingmo-photo-viewer_ky.ts \
    translations/lingmo-photo-viewer_kk.ts \
    translations/lingmo-photo-viewer_ug.ts \
    translations/lingmo-photo-viewer_zh_HK.ts

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm
qm_files.path = /usr/share/lingmo-photo-viewer/translations/

#settings.files = data/org.lingmo.photo.viewer.gschema.xml
settings.files += \
    $$PWD/data/org.lingmo.photo.viewer.gschema.xml \
    $$PWD/data/org.lingmo.log4qt.lingmo-photo-viewer.gschema.xml

settings.path = /usr/share/glib-2.0/schemas/

desktop.files = data/lingmo-photo-viewer.desktop
desktop.path = /usr/share/applications/

guide.files = data/pictures/
guide.path = /usr/share/lingmo-user-guide/data/guide/

icons.files = res/kyview_logo.png
icons.path = /usr/share/pixmaps/

INSTALLS +=desktop settings icons qm_files guide


