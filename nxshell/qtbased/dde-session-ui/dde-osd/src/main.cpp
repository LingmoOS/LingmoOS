// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNIT_TEST

#include "notification/bubblemanager.h"
#include "notification/notifications_dbus_adaptor.h"
#include "manager.h"
#include "kblayoutindicator.h"
#include "accessible.h"
#include "notification/persistence.h"
#include "notification/notifysettings.h"

#include <DApplication>
#include <DDBusSender>
#include <DLog>
#include <DGuiApplicationHelper>
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include <DAccessibilityChecker>
#endif

#include <QTranslator>
#include <QFile>
#include <QDBusConnection>
#include <QProcess>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    if (!(QString(getenv("XDG_CURRENT_DESKTOP")) == QStringLiteral("Deepin") || QString(getenv("XDG_CURRENT_DESKTOP")) == QStringLiteral("DDE"))) {
        qDebug() << "I only run the Deepin Desktop!";
        return -1;
    }
    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::ColorCompositing, true);

    DApplication *app = DApplication::globalApplication(argc, argv);
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
    app->setOrganizationName("deepin");
    app->setApplicationName("dde-osd");
    app->setApplicationVersion("1.0");
    if(!app->setSingleInstance(app->applicationName())) {
        qDebug() << "dde-osd is running...";
        return 1;
    }

#if DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 9, 0)
    app->setOOMScoreAdj(500);
#endif

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    DLogManager::registerJournalAppender();
    QAccessible::installFactory(accessibleFactory);

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    app->installTranslator(&translator);

    QStringList args = app->arguments();
    QString action;
    if (args.length() > 1) {
        action = args.at(1);
    }

    // run osd
    Manager m;
    QDBusConnection connection = QDBusConnection::sessionBus();

    connection.interface()->registerService("org.deepin.dde.Osd1",
                                            QDBusConnectionInterface::ReplaceExistingService,
                                            QDBusConnectionInterface::AllowReplacement);

    connection.registerObject("/", "org.deepin.dde.Osd1", &m, QDBusConnection::ExportAllSlots);

    QObject::connect(connection.interface(), &QDBusConnectionInterface::serviceUnregistered, qApp, &QApplication::quit);

    if (!action.isEmpty()) {
        m.ShowOSD(action);
    }

    // run notification
    Persistence persistence;
    NotifySettings setting;
    BubbleManager manager(&persistence, &setting);

    DDENotifyDBus ddenotify(&manager);
    NotificationsDBusAdaptor adapter(&manager);
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    DAccessibilityChecker checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif
    return app->exec();
}
#else

#include "unittest.h"

QTEST_MAIN(UnitTest)

#endif
