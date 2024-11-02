QT += quick
QT += concurrent
QT += dbus
QT += core gui
QT += KWindowSystem
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/animation/animationfactory.cpp \
    src/animation/disabledanimator.cpp \
    src/animation/enabledanimator.cpp \
    src/animation/expansionanimationfactory.cpp \
    src/animation/floatanimationfactory.cpp \
    src/appinputareamanager.cpp \
    src/geometrymanager/expansiongeometrymanager.cpp \
    src/geometrymanager/floatgeometrymanager.cpp \
    src/geometrymanager/geometrymanager.cpp \
    src/ipc/dbusservice.cpp \
    src/ipc/fcitxvirtualkeyboardserviceproxy.cpp \
    src/ipc/requestmerger.cpp \
    src/localsettings/viewlocalsettings.cpp \
    src/main.cpp \
    src/screenmanager.cpp \
    src/virtualkeyboard/placementmodemanager.cpp \
    src/virtualkeyboard/virtualkeyboardmanager.cpp \
    src/virtualkeyboard/virtualkeyboardmodel.cpp \
    src/virtualkeyboard/virtualkeyboardview.cpp \
    src/virtualkeyboard/virtualkeyboardviewstate.cpp \
    src/virtualkeyboardentry/floatbutton.cpp \
    src/virtualkeyboardentry/floatbuttonmanager.cpp \
    src/virtualkeyboardentry/virtualkeyboardentrymanager.cpp \
    src/virtualkeyboardentry/virtualkeyboardtrayicon.cpp \
    src/virtualkeyboardsettings/virtualkeyboardsettings.cpp 

TRANSLATIONS = translations/translation.ts \
    translations/translation_bo_CN.ts \
    translations/translation_en.ts \
    translations/translation_zh_CN.ts

RESOURCES += \
    floatbutton.qrc \
    qml.qrc \
    translations.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH  += src/

HEADERS += \
    src/animation/animationfactory.h \
    src/animation/animator.h \
    src/animation/disabledanimator.h \
    src/animation/enabledanimator.h \
    src/animation/expansionanimationfactory.h \
    src/animation/floatanimationfactory.h \
    src/appinputareamanager.h \
    src/geometrymanager/expansiongeometrymanager.h \
    src/geometrymanager/floatgeometrymanager.h \
    src/geometrymanager/geometrymanager.h \
    src/ipc/dbusservice.h \
    src/ipc/fcitxvirtualkeyboardserviceproxy.h \
    src/ipc/requestmerger.h \
    src/localsettings/localsettings.h \
    src/localsettings/viewlocalsettings.h \
    src/screenmanager.h \
    src/virtualkeyboard/placementmodemanager.h \
    src/virtualkeyboard/virtualkeyboardmanager.h \
    src/virtualkeyboard/virtualkeyboardmodel.h \
    src/virtualkeyboard/virtualkeyboardstrategy.h \
    src/virtualkeyboard/virtualkeyboardview.h \
    src/virtualkeyboardentry/fcitxvirtualkeyboardservice.h \
    src/virtualkeyboardentry/floatbutton.h \
    src/virtualkeyboardentry/floatbuttonmanager.h \
    src/virtualkeyboardentry/floatbuttonstrategy.h \
    src/virtualkeyboardentry/virtualkeyboardentrymanager.h \
    src/virtualkeyboardentry/virtualkeyboardtrayicon.h \
    src/virtualkeyboardsettings/virtualkeyboardsettings.h 

DISTFILES +=

LIBS += -lFcitx5Qt5DBusAddons

INSTALLS += target_gsettings_file
target_gsettings_file.path = /usr/share/glib-2.0/schemas/
target_gsettings_file.files += data/org.lingmo.virtualkeyboard.gschema.xml

