TEMPLATE = lib
TARGET = lingmo-appwidget-plugin
VERSION = 0.0.0
QT += qml quick dbus
CONFIG += plugin c++11 link_pkgconfig
PKGCONFIG += gsettings-qt
LIBS +=-L../liblingmo-appwidget-manager/ -llingmo-appwidget-manager

INCLUDEPATH += ../liblingmo-appwidget-manager/
DEPENDPATH += ../liblingmo-appwidget-manager/
TARGET = $$qtLibraryTarget($$TARGET)
uri = org.lingmo.appwidget.private

include(../../liblingmo-appwidget-common/liblingmo-appwidget-common.pri)
# Input
SOURCES += \
        kappwidgetconnect.cpp \
        kappwidgetplugin.cpp \
        translatorhelper.cpp

HEADERS += \
        kappwidgetconnect.h \
        kappwidgetplugin.h \
        translatorhelper.h

DISTFILES = qmldir


!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) "$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_LIBS]/qt5/qml/org/lingmo/appwidget/private
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}
