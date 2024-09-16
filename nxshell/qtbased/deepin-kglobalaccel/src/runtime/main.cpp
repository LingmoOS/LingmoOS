/*
  This file is part of the KDE project

  Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
  Copyright (c) 2007 Michael Jansen <kde@michael-jansen.biz>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
  */

#include "kglobalacceld.h"
#include "logging_p.h"

#include <kcrash.h>
#include <KAboutData>
#include <KDBusService>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QCommandLineParser>

static bool isEnabled()
{
    // TODO: Check if kglobalaccel can be disabled
    return true;
}

extern "C" Q_DECL_EXPORT int main(int argc, char **argv)
{
    // Disable Session Management the right way (C)
    //
    // ksmserver has global shortcuts. disableSessionManagement() does not prevent Qt from
    // registering the app with the session manager. We remove the address to make sure we do not
    // get a hang on kglobalaccel restart (kglobalaccel tries to register with ksmserver,
    // ksmserver tries to register with kglobalaccel).
    qunsetenv( "SESSION_MANAGER" );

    QGuiApplication app(argc, argv);
    KAboutData aboutdata(
            QStringLiteral("kglobalaccel"),
            QObject::tr("KDE Global Shortcuts Service"),
            QStringLiteral("0.2"),
            QObject::tr("KDE Global Shortcuts Service"),
            KAboutLicense::LGPL,
            "(C) 2007-2009  Andreas Hartmetz, Michael Jansen");
    aboutdata.addAuthor("Andreas Hartmetz", QObject::tr("Maintainer"), "ahartmetz@gmail.com");
    aboutdata.addAuthor("Michael Jansen", QObject::tr("Maintainer"), "kde@michael-jansen.biz");

    KAboutData::setApplicationData(aboutdata);

    {
        QCommandLineParser parser;
        aboutdata.setupCommandLine(&parser);
        parser.process(app);
        aboutdata.processCommandLine(&parser);
    }

    // check if kglobalaccel is disabled
    if (!isEnabled()) {
        qCDebug(KGLOBALACCELD) << "kglobalaccel is disabled!";
        return 0;
    }

    KDBusService service(KDBusService::Unique);

    app.setQuitOnLastWindowClosed( false );
    QGuiApplication::setFallbackSessionManagementEnabled(false);

    // Restart on a crash
    KCrash::setFlags(KCrash::AutoRestart);

    KGlobalAccelD globalaccel;
    if (!globalaccel.init()) {
        return -1;
    }

    return app.exec();
}
