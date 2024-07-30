/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QUrl>

#include "controller.h"
#include "plasma-welcome-version.h"

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowSystem>

int main(int argc, char *argv[])
{
    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setApplicationName(QStringLiteral("plasma-welcome"));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("plasma-welcome"));

    const QString description = i18nc("@info:usagetip", "Friendly onboarding wizard for Plasma");
    KAboutData aboutData(
        // The program name used internally.
        QStringLiteral("plasma-welcome"),
        // A displayable program name string.
        i18nc("@title", "Welcome Center"),
        // The program version string.
        QStringLiteral(PLASMA_WELCOME_VERSION_STRING),
        // Short description of what the app does.
        description,
        // The license this code is released under.
        KAboutLicense::GPL,
        // Copyright Statement.
        i18nc("@info copyright string", "© 2021–2024, KDE Community"));
    aboutData.setProgramLogo(QIcon::fromTheme(QStringLiteral("start-here-kde-plasma")));
    aboutData.addAuthor(i18nc("@info:credit", "Felipe Kinoshita"),
                        i18nc("@info:credit", "Author"),
                        QStringLiteral("kinofhek@gmail.com"),
                        QStringLiteral("https://fhek.gitlab.io.com"));
    aboutData.addAuthor(i18nc("@info:credit", "Nate Graham"),
                        i18nc("@info:credit", "Author"),
                        QStringLiteral("nate@kde.org"),
                        QStringLiteral("https://pointieststick.com"));
    aboutData.addAuthor(i18nc("@info:credit", "Oliver Beard"), i18nc("@info:credit", "Author"), QStringLiteral("olib141@outlook.com"));
    aboutData.setBugAddress(QByteArrayLiteral("https://bugs.kde.org/enter_bug.cgi?product=Welcome%20Center"));
    KAboutData::setApplicationData(aboutData);

    QQmlApplicationEngine engine;

    // Parse CLI args
    QCommandLineParser parser;
    parser.setApplicationDescription(description);
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringLiteral("post-update"), i18n("Display release notes for the current Plasma release.")));
    parser.addOption(QCommandLineOption(QStringLiteral("post-update-beta"), i18n("Display release notes for the current Plasma release, for beta versions.")));
    parser.addOption(QCommandLineOption(QStringLiteral("live-environment"), i18n("Display the live page intended for distro live environments.")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    auto controller = engine.singletonInstance<Controller *>("org.kde.plasma.welcome", "Controller");
    if (parser.isSet(QStringLiteral("post-update"))) {
        controller->setMode(Controller::Mode::Update);
    } else if (parser.isSet(QStringLiteral("post-update-beta"))) {
        controller->setMode(Controller::Mode::Beta);
    } else if (parser.isSet(QStringLiteral("live-environment"))) {
        controller->setMode(Controller::Mode::Live);
    }

    engine.loadFromModule("org.kde.plasma.welcome", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    // Make it single-instance so it raises e.g. when middle-clicking on taskmanager
    // or accidentally launching it while it's already open
    KDBusService service(KDBusService::Unique);
    QObject::connect(&service, &KDBusService::activateRequested, &engine, [&engine](const QStringList & /*arguments*/, const QString & /*workingDirectory*/) {
        const auto rootObjects = engine.rootObjects();
        for (auto obj : rootObjects) {
            auto view = qobject_cast<QQuickWindow *>(obj);
            if (view) {
                KWindowSystem::updateStartupId(view);
                KWindowSystem::activateWindow(view);
                return;
            }
        }
    });

    return app.exec();
}
