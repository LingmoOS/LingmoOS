/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2007 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalaccel_version.h"
#include "kglobalacceld.h"
#include "logging_p.h"

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <QCommandLineParser>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    // On Wayland the shortcuts are ran as part of kwin_wayland
    // no-op when started on Wayland
    if (qEnvironmentVariable("XDG_SESSION_TYPE") == QLatin1String("wayland")) {
        return 0;
    }

    auto sessionManager = qgetenv("SESSION_MANAGER");
    // Disable Session Management the right way (C)
    //
    // ksmserver has global shortcuts. disableSessionManagement() does not prevent Qt from
    // registering the app with the session manager. We remove the address to make sure we do not
    // get a hang on kglobalaccel restart (kglobalaccel tries to register with ksmserver,
    // ksmserver tries to register with kglobalaccel).
    qunsetenv("SESSION_MANAGER");

    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication::setQuitLockEnabled(false);
    QGuiApplication app(argc, argv);
    KAboutData aboutdata(QStringLiteral("kglobalaccel"),
                         QObject::tr("KDE Global Shortcuts Service"),
                         QStringLiteral(KGLOBALACCEL_VERSION_STRING),
                         QObject::tr("KDE Global Shortcuts Service"),
                         KAboutLicense::LGPL,
                         QStringLiteral("(C) 2007-2009  Andreas Hartmetz, Michael Jansen"));
    aboutdata.addAuthor(QStringLiteral("Andreas Hartmetz"), QObject::tr("Maintainer"), QStringLiteral("ahartmetz@gmail.com"));
    aboutdata.addAuthor(QStringLiteral("Michael Jansen"), QObject::tr("Maintainer"), QStringLiteral("kde@michael-jansen.biz"));

    KAboutData::setApplicationData(aboutdata);

    {
        QCommandLineParser parser;
        aboutdata.setupCommandLine(&parser);
        parser.process(app);
        aboutdata.processCommandLine(&parser);
    }

    KDBusService service(KDBusService::Unique);

    app.setQuitOnLastWindowClosed(false);

    if (!sessionManager.isEmpty()) {
        qputenv("SESSION_MANAGER", sessionManager);
    }

    // Restart on a crash
    KCrash::setFlags(KCrash::AutoRestart);

    KGlobalAccelD globalaccel;
    if (!globalaccel.init()) {
        return -1;
    }

    return app.exec();
}
