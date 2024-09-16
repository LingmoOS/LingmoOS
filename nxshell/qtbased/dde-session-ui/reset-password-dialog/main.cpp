// SPDX-FileCopyrightText: 2016 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resetpassworddialog.h"

#include <DApplication>
#include <QTranslator>
#include <DDBusSender>
#include <QDBusConnection>
#include <LogManager.h>
#include <DStandardPaths>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "DDE");
    }

    DApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(true);
    a.setOrganizationName("deepin");

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption userNameOption(QStringList() << "u" << "username", "user name", "username", "");
    parser.addOption(userNameOption);
    QCommandLineOption appNameOption(QStringList() << "a" << "appname", "application name", "applicationName", "");
    parser.addOption(appNameOption);
    QCommandLineOption fdOption(QStringList() << "fd" << "filedescriptipn", "file descriptipn", "filedescriptipn", "");
    parser.addOption(fdOption);

    parser.parse(QCoreApplication::arguments());

    QString userName = parser.value(userNameOption);
    QString appName = parser.value(appNameOption);
    int fd = parser.value(fdOption).toInt();

    DGuiApplicationHelper::loadTranslator("dde-session-shell", DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation), QList<QLocale>() << QLocale::system());

    DLogManager::setlogFilePath(QString("/tmp/%1.log").arg(a.applicationName()));
    const QDir &logDir = QFileInfo((Dtk::Core::DLogManager::getlogFilePath())).dir();
    if (!logDir.exists())
        QDir().mkpath(logDir.path());
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    Manager *manager = new Manager(userName, appName, fd);
    manager->start();

    return a.exec();
}
