#-------------------------------------------------
#
# Project created by Dharkael 2017-04-21T00:42:49
#
#-------------------------------------------------

win32:LIBS += -luser32 -lshell32

LIBS += -llingmo-log4qt -lavutil -lavformat -lavcodec -lswscale -lX11 -lXext -lXtst -lXfixes -lXinerama -lpulse

TAG_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
isEmpty(TAG_VERSION){
    TAG_VERSION = v0.6.0
}
DEFINES += APP_VERSION=\\\"$$TAG_VERSION\\\"

#DEFINES += ENABLE_RECORD
DEFINES += SUPPORT_LINGMO
DEFINES += SUPPORT_NEWUI

QT  += core gui widgets network svg x11extras KWindowSystem

unix:!macx {
    QT  += dbus
}

CONFIG += c++11 link_pkgconfig 
PKGCONFIG += gsettings-qt
QMAKE_CXXFLAGS += -std=c++0x -D_FORTIFY_SOURCE=2 -O2
PKGCONFIG += libavutil libavformat libavcodec libswscale x11 xext xfixes xinerama libpulse lingmosdk-qtwidgets lingmosdk-waylandhelper

#CONFIG += packaging   # Enables "make install" for packaging paths

TARGET = lingmo-screenshot
TEMPLATE = app

win32:RC_ICONS += img/app/lingmo_screenshot.ico

#release: DESTDIR = build/release
#debug:   DESTDIR = build/debug

#OBJECTS_DIR = $$DESTDIR/.obj
#MOC_DIR = $$DESTDIR/.moc
#RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui

TRANSLATIONS = translations/Internationalization_es.ts \
    translations/Internationalization_ca.ts \
    translations/Internationalization_ru.ts \
    translations/Internationalization_zh_CN.ts \
    translations/Internationalization_zh_HK.ts \
    translations/Internationalization_zh_TW.ts \
    translations/Internationalization_bo_CN.ts \
    translations/Internationalization_ug.ts \
    translations/Internationalization_mn.ts \
    translations/Internationalization_tr.ts \
    translations/Internationalization_ka.ts \
    translations/Internationalization_fr.ts \
    translations/Internationalization_pl.ts \
    translations/Internationalization_ja.ts \
    translations/Internationalization_pt_br.ts \
    translations/Internationalization_sr.ts \
    translations/Internationalization_uk.ts \
    translations/Internationalization_de.ts \
    translations/Internationalization_sk.ts \
    translations/Internationalization_kk.ts \
    translations/Internationalization_ky.ts \

# Generate translations in build
TRANSLATIONS_FILES =

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$shadowed($$tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        mkpath($$qmdir)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES += $$qmfile
}

DEFINES += QT_DEPRECATED_WARNINGS

include(src/third-party/singleapplication/singleapplication.pri)
include(src/third-party/Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

INCLUDEPATH += src/common src/utils

SOURCES += src/main.cpp \
    src/common/CommandLineOptions.cpp \
    src/common/Logger.cpp \
    src/core/kabuttonproxystyle.cpp \
    src/core/kagroupbutton.cpp \
    src/tools/AbstractActionToolWithWidget.cpp \
    src/utils/mysavedialog.cpp \
    src/widgets/aboutdialog.cpp \
    src/widgets/capture/buttonhandler.cpp \
    src/widgets/capture/font_options.cpp \
    src/widgets/capture/font_options2.cpp \
    src/widgets/delaywidget.cpp \
    src/widgets/infowidget.cpp \
    src/config/configwindow.cpp \
    src/widgets/capture/capturewidget.cpp \
    src/widgets/capture/colorpicker.cpp \
    src/config/buttonlistview.cpp \
    src/config/uicoloreditor.cpp \
    src/config/geneneralconf.cpp \
    src/core/controller.cpp \
    src/config/clickablelabel.cpp \
    src/config/filenameeditor.cpp \
    src/config/strftimechooserwidget.cpp \
    src/widgets/capture/capturebutton.cpp \
    src/tools/pencil/penciltool.cpp \
    src/tools/undo/undotool.cpp \
    src/tools/redo/redotool.cpp \
    src/tools/arrow/arrowtool.cpp \
    src/tools/circle/circletool.cpp \
    src/tools/copy/copytool.cpp \
    src/tools/exit/exittool.cpp \
    src/tools/imgur/imguruploadertool.cpp \
    src/tools/line/linetool.cpp \
    src/tools/marker/markertool.cpp \
    src/tools/move/movetool.cpp \
    src/tools/rectangle/rectangletool.cpp \
    src/tools/save/savetool.cpp \
    src/tools/selection/selectiontool.cpp \
    src/tools/sizeindicator/sizeindicatortool.cpp \
    src/tools/toolfactory.cpp \
    src/utils/filenamehandler.cpp \
    src/utils/screengrabber.cpp \
    src/utils/confighandler.cpp \
    src/utils/systemnotification.cpp \
    src/cli/commandlineparser.cpp \
    src/cli/commandoption.cpp \
    src/cli/commandargument.cpp \
    src/utils/screenshotsaver.cpp \
    src/tools/imgur/imguruploader.cpp \
    src/widgets/loadspinner.cpp \
    src/widgets/imagelabel.cpp \
    src/widgets/notificationwidget.cpp \
    src/widgets/capture/notifierbox.cpp \
    src/utils/desktopinfo.cpp \
    src/tools/launcher/applauncherwidget.cpp \
    src/utils/desktopfileparse.cpp \
    src/tools/launcher/launcheritemdelegate.cpp \
    src/tools/blur/blurtool.cpp \
    src/tools/pin/pintool.cpp \
    src/tools/launcher/terminallauncher.cpp \
    src/config/visualseditor.cpp \
    src/config/extendedslider.cpp \
    src/tools/launcher/openwithprogram.cpp \
    src/tools/launcher/applaunchertool.cpp \
    src/utils/pathinfo.cpp \
    src/utils/colorutils.cpp \
    src/tools/capturecontext.cpp \
    src/widgets/capture/modificationcommand.cpp \
    src/tools/abstractpathtool.cpp \
    src/tools/abstracttwopointtool.cpp \
    src/tools/abstractactiontool.cpp \
    src/utils/globalvalues.cpp \
    src/widgets/panel/utilitypanel.cpp \
    src/widgets/capture/hovereventfilter.cpp \
    src/widgets/capture/selectionwidget.cpp \
    src/tools/pin/pinwidget.cpp \
    src/widgets/capturelauncher.cpp \
    src/tools/text/texttool.cpp \
    src/tools/text/textwidget.cpp \
    src/core/capturerequest.cpp \
    src/tools/text/textconfig.cpp \
    src/widgets/panel/sidepanelwidget.cpp \
    src/widgets/capture/fontsize_color_chose.cpp \
    src/widgets/capture/fontsize_color_chose2.cpp \
    src/widgets/capture/save_location.cpp \
    src/widgets/capture/save_location2.cpp \
    src/tools/options/options.cpp \
    src/tools/save/saveastool.cpp \
    src/widgets/screentype.cpp \
    src/widgets/shortcutwidget.cpp \
    src/widgets/xatomhelper.cpp

HEADERS  += src/common/Global.h src/common/my_qt.h src/common/my_x.h src/common/my_av.h src/widgets/capture/buttonhandler.h \
    src/common/CommandLineOptions.h \
    src/common/Enum.h \
    src/common/Logger.h \
    src/core/kabuttonproxystyle.h \
    src/core/kagroupbutton.h \
    src/tools/AbstractActionToolWithWidget.h \
    src/utils/mysavedialog.h \
    src/widgets/aboutdialog.h \
    src/widgets/capture/font_options.h \
    src/widgets/capture/font_options2.h \
    src/widgets/delaywidget.h \
    src/widgets/infowidget.h \
    src/config/configwindow.h \
    src/widgets/capture/capturewidget.h \
    src/widgets/capture/colorpicker.h \
    src/config/buttonlistview.h \
    src/config/uicoloreditor.h \
    src/config/geneneralconf.h \
    src/config/clickablelabel.h \
    src/config/filenameeditor.h \
    src/utils/filenamehandler.h \
    src/config/strftimechooserwidget.h \
    src/utils/screengrabber.h \
    src/tools/capturetool.h \
    src/widgets/capture/capturebutton.h \
    src/tools/pencil/penciltool.h \
    src/tools/undo/undotool.h \
    src/tools/redo/redotool.h \
    src/tools/arrow/arrowtool.h \
    src/tools/circle/circletool.h \
    src/tools/copy/copytool.h \
    src/tools/exit/exittool.h \
    src/tools/imgur/imguruploadertool.h \
    src/tools/line/linetool.h \
    src/tools/marker/markertool.h \
    src/tools/move/movetool.h \
    src/tools/rectangle/rectangletool.h \
    src/tools/save/savetool.h \
    src/tools/selection/selectiontool.h \
    src/tools/sizeindicator/sizeindicatortool.h \
    src/tools/toolfactory.h \
    src/utils/confighandler.h \
    src/core/controller.h \
    src/utils/systemnotification.h \
    src/cli/commandlineparser.h \
    src/cli/commandoption.h \
    src/cli/commandargument.h \
    src/utils/screenshotsaver.h \
    src/tools/imgur/imguruploader.h \
    src/widgets/loadspinner.h \
    src/widgets/imagelabel.h \
    src/widgets/notificationwidget.h \
    src/widgets/capture/notifierbox.h \
    src/utils/desktopinfo.h \
    src/tools/launcher/applauncherwidget.h \
    src/utils/desktopfileparse.h \
    src/tools/launcher/launcheritemdelegate.h \
    src/tools/blur/blurtool.h \
    src/tools/pin/pintool.h \
    src/tools/launcher/terminallauncher.h \
    src/config/visualseditor.h \
    src/config/extendedslider.h \
    src/tools/launcher/openwithprogram.h \
    src/utils/pathinfo.h \
    src/tools/capturecontext.h \
    src/tools/launcher/applaunchertool.h \
    src/utils/globalvalues.h \
    src/utils/colorutils.h \
    src/widgets/capture/modificationcommand.h \
    src/tools/abstractpathtool.h \
    src/tools/abstracttwopointtool.h \
    src/tools/abstractactiontool.h \
    src/widgets/panel/utilitypanel.h \
    src/widgets/capture/hovereventfilter.h \
    src/widgets/capture/selectionwidget.h \
    src/tools/pin/pinwidget.h \
    src/widgets/capturelauncher.h \
    src/tools/text/texttool.h \
    src/tools/text/textwidget.h \
    src/core/capturerequest.h \
    src/tools/text/textconfig.h \
    src/widgets/panel/sidepanelwidget.h \
    src/widgets/capture/fontsize_color_chose.h \
    src/widgets/capture/fontsize_color_chose2.h \
    src/widgets/capture/save_location.h \
    src/widgets/capture/save_location2.h \
    src/tools/options/options.h \
    src/tools/save/saveastool.h \
    src/widgets/screentype.h \
    src/widgets/shortcutwidget.h \
    src/widgets/xatomhelper.h

unix:!macx {
    SOURCES += src/core/flameshotdbusadapter.cpp \
        src/utils/dbusutils.cpp

    HEADERS  += src/core/flameshotdbusadapter.h \
        src/utils/dbusutils.h
}

win32 {
    SOURCES += src/core/globalshortcutfilter.cpp

    HEADERS  += src/core/globalshortcutfilter.h
}

RESOURCES += \
    graphics.qrc

# installs
unix:!macx {
    isEmpty(PREFIX) {
      packaging {
        PREFIX = /usr
      } else {
        PREFIX = /usr/local
      }
    }

    DEFINES += APP_PREFIX=\\\"$$PREFIX\\\"

    target.path = $${PREFIX}/bin/

    qmfile.path = $${PREFIX}/share/lingmo-screenshot/translations/
    qmfile.files = $${TRANSLATIONS_FILES}

    dbus.path = $${PREFIX}/share/dbus-1/interfaces/
    dbus.files = dbus/org.dharkael.lingmoscreenshot.xml

    gsettings.path = $${PREFIX}/share/glib-2.0/schemas/
    gsettings.files =  gsettings/org.lingmo.screenshot.gschema.xml

    icon.path = $${PREFIX}/share/icons/hicolor/
    icon.files = img/hicolor/*

    completion.path = $${PREFIX}/share/bash-completion/completions/
    completion.files = docs/bash-completion/lingmo-screenshot

    appdata.path = $${PREFIX}/share/metainfo/
    appdata.files = docs/appdata/lingmoscreenshot.appdata.xml

    desktopentry.path = $${PREFIX}/share/applications
    desktopentry.files = docs/desktopEntry/package/lingmo-screenshot.desktop

    helpfiles.path = $${PREFIX}/share/lingmo-user-guide/data/guide/
    helpfiles.files = data/lingmo-screenshot/

    servicedbus.path = $${PREFIX}/share/dbus-1/services/

    packaging {
        servicedbus.files = dbus/package/org.dharkael.lingmoscreenshot.service
    } else {
        servicedbus.files = dbus/make/org.dharkael.lingmoscreenshot.service
    }

    INSTALLS += target \
        icon \
        desktopentry \
        qmfile \
        gsettings \
        servicedbus \
        dbus \
        completion \
        helpfiles \
        appdata
}

# Imgur API data
include(src/imgur.pri)

#if(contains(DEFINES, ENABLE_RECORD)) {
#        include(src/record/record.pri)
#}
