/*
 * SPDX-FileCopyrightText: 2011~2011 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "config.h"
#include "keyboardlayoutwidget.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QMessageBox>
#include <QX11Info>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "xcb");

    QApplication app(argc, argv);
    app.setApplicationName(QLatin1String("kbd-layout-viewer"));
    app.setApplicationVersion(QLatin1String(PROJECT_VERSION));

    if (!QX11Info::isPlatformX11()) {
        QMessageBox msgBox(QMessageBox::Critical, _("Error"),
                           _("This program only works on X11."));
        msgBox.exec();
        return 1;
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(_("A general keyboard layout viewer"));
    parser.addHelpOption();
    parser.addOptions(
        {{{"g", "group"}, _("Keyboard layout <group> (0-3)"), _("group")},
         {{"l", "layout"}, _("Keyboard <layout>"), _("layout")},
         {{"v", "variant"}, _("Keyboard layout <variant>"), _("variant")}});

    parser.process(app);

    int group = -1;
    QString variant, layout;
    if (parser.isSet("group")) {
        group = parser.value("group").toInt();
    }
    if (parser.isSet("layout")) {
        layout = parser.value("layout");
    }
    if (parser.isSet("variant")) {
        variant = parser.value("variant");
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    QMainWindow mainWindow;
    mainWindow.setWindowIcon(QIcon::fromTheme("input-keyboard"));
    mainWindow.setWindowTitle(_("Keyboard Layout viewer"));
    mainWindow.setMinimumSize(QSize(900, 400));
    fcitx::kcm::KeyboardLayoutWidget widget;
    if (group > 0 || layout.isNull()) {
        if (group < 0)
            group = 0;
        widget.setGroup(group);
    } else if (!layout.isNull()) {
        widget.setKeyboardLayout(layout, variant);
    }

    mainWindow.setCentralWidget(&widget);
    mainWindow.show();

    return app.exec();
}
