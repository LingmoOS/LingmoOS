/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    // for simplicity we'll just send along progress-visible true whenever progress is set and otherwise false
    QCommandLineOption progressOption(QStringLiteral("progress"), QStringLiteral("Show progress, 0-100"), QStringLiteral("progress"));
    parser.addOption(progressOption);
    // same for count
    QCommandLineOption countOption(QStringLiteral("count"), QStringLiteral("Show count badge, number"), QStringLiteral("count"));
    parser.addOption(countOption);

    QCommandLineOption urgentOption(QStringLiteral("urgent"), QStringLiteral("Set urgent hint, flash task bar entry"));
    parser.addOption(urgentOption);

    parser.addPositionalArgument(QStringLiteral("desktop-filename"), QStringLiteral("Desktop file name for the application"));

    parser.process(app);

    if (parser.positionalArguments().count() != 1) {
        parser.showHelp(1); // never returns
    }

    QString launcherId = parser.positionalArguments().constFirst();
    if (!launcherId.startsWith(QLatin1String("application://"))) {
        launcherId.prepend(QLatin1String("application://"));
    }
    if (!launcherId.endsWith(QLatin1String(".desktop"))) {
        launcherId.append(QLatin1String(".desktop"));
    }

    QVariantMap properties;

    if (parser.isSet(progressOption)) {
        properties.insert(QStringLiteral("progress"), parser.value(progressOption).toInt() / 100.0);
        properties.insert(QStringLiteral("progress-visible"), true);
    } else {
        properties.insert(QStringLiteral("progress-visible"), false);
    }

    if (parser.isSet(countOption)) {
        properties.insert(QStringLiteral("count"), parser.value(countOption).toInt());
        properties.insert(QStringLiteral("count-visible"), true);
    } else {
        properties.insert(QStringLiteral("count-visible"), false);
    }

    properties.insert(QStringLiteral("urgent"), parser.isSet(urgentOption));

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/org/knotifications/UnityLauncherTest"),
                                                      QStringLiteral("com.canonical.Unity.LauncherEntry"),
                                                      QStringLiteral("Update"));
    message.setArguments({launcherId, properties});
    QDBusConnection::sessionBus().send(message);

    // FIXME can we detect that the message was sent to the bus?
    QTimer::singleShot(500, &app, QCoreApplication::quit);

    return app.exec();
}
