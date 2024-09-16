// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hintsdialog.h"

#include <DApplication>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QTranslator>
#include <QCommandLineOption>
#include <QCommandLineParser>

DWIDGET_USE_NAMESPACE

const int HINT_TITLE = 1;
const int HINT_CONTENT = 2;

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setApplicationName("dde-hints-dialog");
    a.setApplicationVersion("1.0");
    a.setQuitOnLastWindowClosed(true);

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    a.installTranslator(&translator);

    QStringList arguslist = a.arguments();
    if (arguslist.size() < 3) {
        qDebug() << "number of parameters must be greater than 1";
        return -1;
    }
    qDebug() << "Title:" << arguslist[HINT_TITLE] << " Content:" << arguslist[HINT_CONTENT];

    HintsDialog dialog;
    dialog.setHintTitle(arguslist[HINT_TITLE]);
    dialog.setHintContent(arguslist[HINT_CONTENT]);
    dialog.moveToCenter();
    dialog.show();

    return a.exec();
}
