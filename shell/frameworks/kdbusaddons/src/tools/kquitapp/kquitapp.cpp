/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kquitapp"));
    app.setApplicationVersion(QStringLiteral("2.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Quit a D-Bus enabled application easily"));
    parser.addOption(QCommandLineOption(QStringLiteral("service"),
                                        QCoreApplication::translate("main", "Full service name, overrides application name provided"),
                                        QStringLiteral("service")));
    parser.addOption(QCommandLineOption(QStringLiteral("path"),
                                        QCoreApplication::translate("main", "Path in the D-Bus interface to use"),
                                        QStringLiteral("path"),
                                        QStringLiteral("/MainApplication")));
    parser.addPositionalArgument(QStringLiteral("[application]"), QCoreApplication::translate("main", "The name of the application to quit"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    QString service;
    if (parser.isSet(QStringLiteral("service"))) {
        service = parser.value(QStringLiteral("service"));
    } else if (!parser.positionalArguments().isEmpty()) {
        service = QStringLiteral("org.kde.%1").arg(parser.positionalArguments().at(0));
    } else {
        parser.showHelp(1);
    }

    QString path(parser.value(QStringLiteral("path")));

    QDBusInterface interface(service, path);
    if (!interface.isValid()) {
        qWarning() << QCoreApplication::translate("main", "Application %1 could not be found using service %2 and path %3.")
                          .arg(parser.positionalArguments().at(0), service, path);
        return 1;
    }
    interface.call(QStringLiteral("quit"));
    QDBusError error = interface.lastError();
    if (error.type() != QDBusError::NoError) {
        qWarning() << QCoreApplication::translate("main", "Quitting application %1 failed. Error reported was:\n\n     %2 : %3")
                          .arg(parser.positionalArguments().join(QStringLiteral(" ")), error.name(), error.message());
        return 1;
    }
    return 0;
}
