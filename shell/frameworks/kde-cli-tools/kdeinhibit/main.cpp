/********************************************************************
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

#include <KLocalizedString>
#include <KProcess>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("kde-inhibit"));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kde-inhibit"));
    QCommandLineParser parser;
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    parser.setApplicationDescription(i18n("Inhibit various desktop functions whilst a command runs"));

    QCommandLineOption powerOption(QStringLiteral("power"), i18n("Inhibit power management"));
    parser.addOption(powerOption);

    QCommandLineOption screenSaverOption(QStringLiteral("screenSaver"), i18n("Inhibit screensaver"));
    parser.addOption(screenSaverOption);

    // TODO: Change the translated string to "Inhibit night light (blue light filter)", can't do atm because of a string freeze
    QCommandLineOption nightLightOption({QStringLiteral("nightLight"), QStringLiteral("colorCorrect")}, i18n("Inhibit colour correction (night mode)"));
    parser.addOption(nightLightOption);

    QCommandLineOption notificationsOption(QStringLiteral("notifications"), i18n("Inhibit notifications (Do not disturb)"));
    parser.addOption(notificationsOption);

    parser.addPositionalArgument(QStringLiteral("command"), i18n("Command with arguments to run"), QStringLiteral("command [args...]"));

    parser.addHelpOption();

    parser.process(app);

    QStringList command = parser.positionalArguments();

    if (command.isEmpty()) {
        parser.showHelp(-1);
    }

    auto warnOnError = [](const QDBusMessage &reply) {
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "Inhibit failed" << reply.errorMessage();
        }
    };

    if (parser.isSet(powerOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                                                                  QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"),
                                                                  QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                                                                  QStringLiteral("Inhibit"));
        inhibitCall.setArguments({i18nc("Script as in shell script", "Running Script"), command.first()});
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
        // ignore reply with the cookie as we don't really care if it failed
    }
    if (parser.isSet(screenSaverOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                                  QStringLiteral("/org/freedesktop/ScreenSaver"),
                                                                  QStringLiteral("org.freedesktop.ScreenSaver"),
                                                                  QStringLiteral("Inhibit"));
        inhibitCall.setArguments({i18nc("Script as in shell script", "Running Script"), command.first()});
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
    }
    if (parser.isSet(nightLightOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin.NightLight"),
                                                                  QStringLiteral("/org/kde/KWin/NightLight"),
                                                                  QStringLiteral("org.kde.KWin.NightLight"),
                                                                  QStringLiteral("inhibit"));
        // no arguments needed
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
    }
    if (parser.isSet(notificationsOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.Notifications"),
                                                                  QStringLiteral("/org/freedesktop/Notifications"),
                                                                  QStringLiteral("org.freedesktop.Notifications"),
                                                                  QStringLiteral("Inhibit"));
        inhibitCall.setArguments({i18nc("Script as in shell script", "Running Script"), command.first(), QVariantMap()});
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
    }

    // finally run the script and exit when we're done
    KProcess::execute(command);

    // we don't explicitly uninhibit as closing our DBus connection will release everything automatically
}
