/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSessionManager>
#include <QSurfaceFormat>
#include <QWindow>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>

#include "CommandLineArguments.h"
#include "Configuration.h"
#include "ToolsModel.h"

class SessionManager : public QObject
{
    Q_OBJECT
public:
    explicit SessionManager(QObject *parent);

private:
    QString m_pageId;
};

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
{
    connect(qApp, &QGuiApplication::saveStateRequest, this, [this](QSessionManager &manager) {
        QString lastPageId = Configuration::globalConfig()->lastPage();
        if (lastPageId.isEmpty()) {
            return;
        }
        QStringList args;
        args << qApp->applicationName();
        args << QStringLiteral("--page-id") << lastPageId;
        args << QStringLiteral("-session") << manager.sessionId();
        manager.setRestartCommand(args);
    });
}

int main(int argc, char **argv)
{
    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("utilities-system-monitor")));

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("plasma-systemmonitor"));

    KAboutData aboutData{/* componentName */ QStringLiteral("plasma-systemmonitor"),
                         /* displayName */ i18n("System Monitor"),
                         /* version */ PROJECT_VERSION,
                         /* shortDescription */ i18n("An application for monitoring system resources"),
                         /* licenseType */ KAboutLicense::GPL,
                         /* copyrightStatement */ i18n("© 2020 Plasma Development Team"),
                         /* otherText */ QString{},
                         /* homePageAddress */ QString{}};
    aboutData.setDesktopFileName(QStringLiteral("org.kde.plasma-systemmonitor"));
    aboutData.setProgramLogo(app.windowIcon());

    aboutData.addAuthor(i18n("Arjen Hiemstra"), i18n("Development"), QStringLiteral("ahiemstra@heimr.nl"));
    aboutData.addAuthor(i18n("Uri Herrera"), i18n("Design"), QStringLiteral("uri_herrera@nxos.org"));
    aboutData.addAuthor(i18n("Nate Graham"), i18n("Design and Quality Assurance"), QStringLiteral("nate@kde.org"));
    aboutData.addAuthor(i18n("David Redondo"), i18n("Development"), QStringLiteral("kde@david-redondo.de"));
    aboutData.addAuthor(i18n("Noah Davis"), i18n("Design and Quality Assurance"), QStringLiteral("noahadvs@gmail.com"));
    aboutData.addAuthor(i18n("Marco Martin"), i18n("Development"), QStringLiteral("notmart@gmail.com"));
    aboutData.addAuthor(i18n("David Edmundson"), i18n("Development"), QStringLiteral("david@davidedmundson.co.uk"));

    aboutData.setTranslator(i18ndc(nullptr, "NAME OF TRANSLATORS", "Your names"), i18ndc(nullptr, "EMAIL OF TRANSLATORS", "Your emails"));

    aboutData.setBugAddress("https://bugs.kde.org/enter_bug.cgi?product=plasma-systemmonitor");
    KAboutData::setApplicationData(aboutData);

    KCrash::initialize();

    KDBusService service(KDBusService::Unique);

    auto parser = std::make_shared<QCommandLineParser>();
    aboutData.setupCommandLine(parser.get());
    parser->addOption({QStringLiteral("page-id"), QStringLiteral("Start with the specified page ID shown"), QStringLiteral("page-id")});
    parser->addOption({QStringLiteral("page-name"), QStringLiteral("Start with the specified page name shown"), QStringLiteral("page-name")});
    parser->process(app);
    aboutData.processCommandLine(parser.get());
    CommandLineArguments::setCommandLineParser(parser);

    auto sessionManager = new SessionManager(&app);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    if (Configuration::globalConfig()->maximized()) {
        engine.setInitialProperties({{"visibility", QWindow::Maximized}});
    }
    engine.loadFromModule("org.kde.systemmonitor", "Main");

    QObject::connect(&service, &KDBusService::activateRequested, &engine, []() {
        if (!qApp->topLevelWindows().isEmpty()) {
            QWindow *win = qApp->topLevelWindows().first();
            KWindowSystem::updateStartupId(win);
            KWindowSystem::activateWindow(win);
        }
    });
    // QDBusConnectionInterface::StartService is blocking so we do it manually
    const auto busInterface = QDBusConnection::sessionBus().interface();
    busInterface->asyncCall(QStringLiteral("StartServiceByName"), QStringLiteral("org.kde.ksystemstats"), 0u);
    return app.exec();
}

#include "main.moc"
