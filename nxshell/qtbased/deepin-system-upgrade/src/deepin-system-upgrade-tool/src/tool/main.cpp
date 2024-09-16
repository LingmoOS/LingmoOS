// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DLog>
#include <DWidgetUtil>
#include <DDialog>

#include <QObject>
#include <QString>
#include <QStandardPaths>
#include <QDebug>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <unistd.h>

#include "mainwindow.h"
#include "application.h"
#include "../core/constants.h"
#include "../core/dbusworker.h"

QString getOSVersion()
{
    QFile desktopFile("/etc/os-version");

    if (desktopFile.open(QFile::ReadOnly | QFile::Text))
    {
        while (!desktopFile.atEnd())
        {
            QString line = desktopFile.readLine();
            if (line.startsWith("MajorVersion"))
            {
                return line.split("=")[1];
            }
        }
        desktopFile.close();
        return QString("");
    }
    else
    {
        return QString("Unavailable");
    }
}

Q_NAMESPACE

int main(int argc, char *argv[])
{
    Application::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    Application::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    Application *app = Application::getInstance(argc, argv);

    QCoreApplication::setApplicationVersion("V6.0.10");
    app->loadTranslator();
    app->setWindowIcon(QIcon(":icons/upgrade-tool.svg"));
    app->setProductIcon(QIcon(":icons/upgrade-tool.svg"));
    app->setApplicationName("deepin-system-upgrade-tool");
    app->setProductName(app->translate("main", "System Upgrade Tool"));
    app->setApplicationDescription(app->translate("main", "System Upgrade Tool helps you upgrade the system to version 23."));
    app->connect(app, &Application::appQuit, [] {
        MainWindow::getInstance()->close();
    });
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    dInfo("Upgrade tool started with command: " + app->arguments().join(' '));

    QCommandLineOption modeOption({"m", "mode"}, "Start upgrade result report based on atomic upgrade status code", "status_code");
    QCommandLineParser parser;

    parser.setApplicationDescription(app->translate("main", "System Upgrade Tool"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(modeOption);

    parser.process(*app);
    if (!parser.isSet(modeOption))
    {
        bool isSingleInstanceSet = app->setSingleInstance("deepin-system-upgrade-tool");
        if (!isSingleInstanceSet) {
            qWarning() << "Failed to set single instance. Exiting.";
            return 0;
        }

        dInfo("start main upgrade tool application");
        MainWindow *mainWindow = MainWindow::getInstance();
        moveToCenter(mainWindow);
        if (QFile::exists("/var/cache/deepin-system-upgrade/migrate.state"))
        {
            // App Migration
            mainWindow->startMigration();
        }
        mainWindow->show();
        return app->exec();
    }
    else
    {
        // Delete useless desktop entries
        system("rm -f ~/Desktop/deepin-system-upgrade-tool.desktop");
        system("rm -f ~/Desktop/deepin-installer.desktop");
        // Display upgrade status notification
        if (parser.value(modeOption) == "100")
        {
            DBusWorker *dbusWorker = DBusWorker::getInstance();
            dbusWorker->RestorePlymouthTheme();
            qInfo() << "System successfully upgraded";
            DBusWorker::getInstance()->SendNotification(
                app->translate("main", "Upgrade successful"),
                app->translate("main", "Successfully upgraded to version %1").arg(getOSVersion())
            );
            return 0;
        }
        else
        {
            qInfo() << "Atomic update failed with code:" << parser.value(modeOption);
            MainWindow *mainWindow = MainWindow::getInstance();
            moveToCenter(mainWindow);
            mainWindow->showFailedWindow(app->translate("main", "Error code: %1").arg(parser.value(modeOption)));
            mainWindow->show();
            return app->exec();
        }
    }
}

#include "main.moc"
