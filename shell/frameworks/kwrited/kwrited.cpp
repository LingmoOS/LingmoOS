/*
    kwrited is a write(1) receiver for KDE.
    SPDX-FileCopyrightText: 1997, 1998 Lars Doelle <lars.doelle@on-line.de>
    SPDX-FileCopyrightText: 2008 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "kwrited.h"

#include <QDebug>
#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <knotification.h>
#include <kptydevice.h>
#include <kuser.h>

#if BUILD_AS_EXECUTABLE
#include <QGuiApplication>
#include <QSessionManager>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <kpluginfactory.h>
#endif

static inline KAboutData aboutData()
{
    return KAboutData(QStringLiteral("kwrited"), i18n("kwrited"), QLatin1String(PROJECT_VERSION));
}

#if BUILD_AS_EXECUTABLE

static uid_t original_euid;
static gid_t original_egid;

static void sigterm_handler(int signal)
{
    Q_UNUSED(signal)
    QGuiApplication::quit();
}

int main(int argc, char **argv)
{
    // drop elevated privileges temporarily
    original_euid = geteuid();
    original_egid = getegid();
    seteuid(getuid());
    setegid(getgid());

    // install a signal handler to exit gracefully
    //(so that pty->logout() is called in ~KWrited())
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    signal(SIGHUP, sigterm_handler);

    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication a(argc, argv);
    KAboutData::setApplicationData(aboutData());
    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&a, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&a, &QGuiApplication::saveStateRequest, disableSessionManagement);

    KWrited w;
    return a.exec();
}

#else // BUILD_AS_EXECUTABLE

KWritedModule::KWritedModule(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
{
    pro = new KWrited;
}

KWritedModule::~KWritedModule()
{
    delete pro;
}

K_PLUGIN_FACTORY_WITH_JSON(KWritedFactory, "kwrited.json", registerPlugin<KWritedModule>();)

#endif // BUILD_AS_EXECUTABLE

KWrited::KWrited()
    : QObject()
{
    pty = new KPtyDevice();
    pty->open();

#if BUILD_AS_EXECUTABLE
    dup2(pty->slaveFd(), 0); // HACK: login() from glibc requires this to login correctly
    // restore elevated privileges
    seteuid(original_euid);
    setegid(original_egid);
#endif

    pty->login(KUser(KUser::UseRealUserID).loginName().toLocal8Bit().data(), qgetenv("DISPLAY").constData());

#if BUILD_AS_EXECUTABLE
    // drop privileges again
    seteuid(getuid());
    setegid(getgid());
#endif

    connect(pty, &QIODevice::readyRead, this, &KWrited::block_in);
    // qDebug() << "listening on device" << pty->ttyName();
}

KWrited::~KWrited()
{
#if BUILD_AS_EXECUTABLE
    // restore elevated privileges
    seteuid(original_euid);
    setegid(original_egid);
#endif

    pty->logout();

#if BUILD_AS_EXECUTABLE
    // drop privileges again
    seteuid(getuid());
    setegid(getgid());
#endif

    delete pty;
}

void KWrited::block_in()
{
    QByteArray buf = pty->readAll();
    QString msg = QString::fromLocal8Bit(buf.constData(), buf.size());
    msg.remove(QLatin1Char('\r'));
    msg.remove(QLatin1Char('\a'));
    msg = msg.trimmed();

    KNotification *notification = new KNotification(QStringLiteral("NewMessage"), KNotification::Persistent);
#if !BUILD_AS_EXECUTABLE
    notification->setComponentName(QStringLiteral("kwrited"));
#endif
    notification->setText(msg);
    notification->setFlags(KNotification::SkipGrouping);
    connect(notification, &KNotification::closed, notification, &QObject::deleteLater);
    notification->sendEvent();
}

#if !BUILD_AS_EXECUTABLE
#include "kwrited.moc"
#endif

#include "moc_kwrited.cpp"
