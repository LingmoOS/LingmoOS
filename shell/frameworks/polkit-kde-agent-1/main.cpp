/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Jaroslav Reznik <jreznik@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"
#include "policykitlistener.h"
// KF
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
// PolkitQt1
#include <PolkitQt1/Subject>
// Qt
#include <QApplication>
#include <QDebug>
#include <QSessionManager>
// std
#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif
#if HAVE_SYS_PROCCTL_H
#include <sys/procctl.h>
#include <unistd.h>
#endif

int main(int argc, char *argv[])
{
    // disable ptrace
#if HAVE_PR_SET_DUMPABLE
    prctl(PR_SET_DUMPABLE, 0);
#endif
#if HAVE_PROC_TRACE_CTL
    int mode = PROC_TRACE_CTL_DISABLE;
    procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
#endif
    KCrash::setFlags(KCrash::AutoRestart);

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("polkit-kde-authentication-agent-1"));

    KAboutData aboutData("polkit-kde-authentication-agent-1", QString(), POLKIT_KDE_1_VERSION);
    aboutData.addLicense(KAboutLicense::GPL);
    aboutData.addCredit(i18n("(c) 2009 Red Hat, Inc."));
    aboutData.addAuthor(i18n("Lukáš Tinkl"), i18n("Maintainer"), "ltinkl@redhat.com");
    aboutData.addAuthor(i18n("Jaroslav Reznik"), i18n("Former maintainer"), "jreznik@redhat.com");
    aboutData.setProductName("policykit-kde/polkit-kde-authentication-agent-1");

    KAboutData::setApplicationData(aboutData);

    // ensure singleton run
    KDBusService service(KDBusService::Unique | KDBusService::Replace);

    // disable session management
    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };

    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    // register agent
    PolicyKitListener *listener = new PolicyKitListener(&app);

    PolkitQt1::UnixSessionSubject session(getpid());

    const bool result = listener->registerListener(session, "/org/kde/PolicyKit1/AuthenticationAgent");

    qDebug() << "Authentication agent result:" << result;

    if (!result) {
        qWarning() << "Couldn't register listener!";
        exit(1);
    }

    app.exec();
}
