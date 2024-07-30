/*
 * kstart.C. Part of the KDE project.
 *
 * SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
 * SPDX-FileCopyrightText: David Faure <faure@kde.org>
 * SPDX-FileCopyrightText: Richard Moore <rich@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <config-kde-cli-tools.h>

#include "kstart.h"

#include <iostream>
#include <stdlib.h>

#if WITH_X11
#include "private/qtx11extras_p.h"
#endif
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QUrl>

#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kprocess.h>

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>

// some globals

static QString serviceName;
static QString exe;
static QStringList exeArgs;
static QString url;

KStart::KStart()
    : QObject()
{
#if WITH_X11
    if (QX11Info::isPlatformX11()) {
        // propagate the startup identification to the started process
        qputenv("DESKTOP_STARTUP_ID", QX11Info::nextStartupId());
    }
#endif

    if (!serviceName.isEmpty()) {
        KService::Ptr service = KService::serviceByDesktopName(serviceName);
        if (!service) {
            qCritical() << "No such service" << exe;
        } else {
            auto *job = new KIO::ApplicationLauncherJob(service);
            if (!url.isEmpty()) {
                job->setUrls({QUrl(url)}); // TODO use QUrl::fromUserInput(PreferLocalFile)?
            }
            job->exec();
            if (job->error()) {
                qCritical() << job->errorString();
            }
        }
    } else {
        auto *job = new KIO::CommandLauncherJob(exe, exeArgs);
        job->exec();
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kstart"));

    KAboutData aboutData(QStringLiteral("kstart"),
                         i18n("KStart"),
                         QString::fromLatin1(PROJECT_VERSION),
                         i18n("Utility to launch applications"),
                         KAboutLicense::GPL,
                         i18n("(C) 1997-2000 Matthias Ettrich (ettrich@kde.org)"));

    aboutData.addAuthor(i18n("Matthias Ettrich"), QString(), QStringLiteral("ettrich@kde.org"));
    aboutData.addAuthor(i18n("David Faure"), QString(), QStringLiteral("faure@kde.org"));
    aboutData.addAuthor(i18n("Richard J. Moore"), QString(), QStringLiteral("rich@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("command"), i18n("Command to execute"));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("application"),
                                        i18n("Alternative to <command>: desktop file name to start, e.g. org.kde.kate"),
                                        QLatin1String("desktopfile")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("url"),
                                        i18n("Optional URL to pass to the application when using --application"),
                                        QLatin1String("url")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.isSet(QStringLiteral("application"))) {
        serviceName = parser.value(QStringLiteral("application"));
        url = parser.value(QStringLiteral("url"));
    } else {
        QStringList positionalArgs = parser.positionalArguments();
        if (positionalArgs.isEmpty()) {
            qCritical() << i18n("No command specified");
            parser.showHelp(1);
        }

        exe = positionalArgs.takeFirst();
        exeArgs = positionalArgs;
    }

    KStart start;

    return app.exec();
}

#include "moc_kstart.cpp"
