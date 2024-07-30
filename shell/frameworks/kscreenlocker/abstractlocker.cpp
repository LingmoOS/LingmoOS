/*
SPDX-FileCopyrightText: 1999 Martin R. Jones <mjones@kde.org>
SPDX-FileCopyrightText: 2002 Luboš Luňák <l.lunak@kde.org>
SPDX-FileCopyrightText: 2003 Oswald Buddenhagen <ossi@kde.org>
SPDX-FileCopyrightText: 2008 Chani Armitage <chanika@gmail.com>
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2015 Bhushan Shah <bhush94@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractlocker.h"
#include "kscreenlocker_logging.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QPainter>
#include <QScreen>

#include <KLocalizedString>

namespace ScreenLocker
{
BackgroundWindow::BackgroundWindow(AbstractLocker *lock)
    : QRasterWindow()
    , m_lock(lock)
{
    qCDebug(KSCREENLOCKER) << "Creating BackgroundWindow";
    setFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
    setProperty("org_kde_ksld_emergency", true);
}

BackgroundWindow::~BackgroundWindow() = default;

void BackgroundWindow::paintEvent(QPaintEvent *)
{
    qCDebug(KSCREENLOCKER) << "Painting BackgroundWindow";
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), Qt::black);
    if (m_greeterFailure) {
        auto text = ki18n(
            "The screen locker is broken and unlocking is not possible anymore.\n"
            "In order to unlock it either ConsoleKit or LoginD is needed, neither\n"
            "of which could be found on your system.");
        auto text_ck = ki18nc("%1 = other terminal",
                              "The screen locker is broken and unlocking is not possible anymore.\n"
                              "In order to unlock it, switch to a virtual terminal (e.g. Ctrl+Alt+F%1),\n"
                              "log in as root and execute the command:\n\n"
                              "# ck-unlock-session <session-name>\n\n");
        auto text_ld = ki18nc("%1 = other terminal, %2 = session ID, %3 = this terminal",
                              "The screen locker is broken and unlocking is not possible anymore.\n"
                              "In order to unlock it, switch to a virtual terminal (e.g. Ctrl+Alt+F%1),\n"
                              "log in to your account and execute the command:\n\n"
                              "loginctl unlock-session %2\n\n"
                              "Then log out of the virtual session by pressing Ctrl+D, and switch\n"
                              "back to the running session (Ctrl+Alt+F%3).\n"
                              "Should you have forgotten the instructions, you can get back to this\n"
                              "screen by pressing Ctrl+Alt+F%3\n\n");

        auto haveService = [](QString service) {
            return QDBusConnection::systemBus().interface()->isServiceRegistered(service);
        };
        if (haveService(QStringLiteral("org.freedesktop.ConsoleKit"))) {
            auto virtualTerminalId = qgetenv("XDG_VTNR").toInt();
            text = text_ck.subs(virtualTerminalId == 2 ? 1 : 2);
        } else if (haveService(QStringLiteral("org.freedesktop.login1"))) {
            text = text_ld;
            auto virtualTerminalId = qgetenv("XDG_VTNR").toInt();
            text = text.subs(virtualTerminalId == 2 ? 1 : 2);
            text = text.subs(QString::fromLocal8Bit(qgetenv("XDG_SESSION_ID")));
            text = text.subs(virtualTerminalId);
        }

        p.setPen(Qt::white);
        QFont f = p.font();
        f.setBold(true);
        f.setPointSize(24);
        // for testing emergency mode, we need to disable antialias, as otherwise
        // screen wouldn't be completely black and white.
        if (qEnvironmentVariableIsSet("KSLD_TESTMODE")) {
            f.setStyleStrategy(QFont::NoAntialias);
        }
        p.setFont(f);
        const auto screens = QGuiApplication::screens();
        for (auto s : screens) {
            p.drawText(s->geometry(), Qt::AlignVCenter | Qt::AlignHCenter, text.toString());
        }
    }
    m_lock->stayOnTop();
}

void BackgroundWindow::emergencyShow()
{
    qCDebug(KSCREENLOCKER) << "BackgroundWindow::emergencyShow() called";
    m_greeterFailure = true;
    update();
    show();
}

AbstractLocker::AbstractLocker(QObject *parent)
    : QObject(parent)
{
    if (qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
        m_background.reset(new BackgroundWindow(this));
    }
}

AbstractLocker::~AbstractLocker()
{
}

void AbstractLocker::emergencyShow()
{
    if (m_background.isNull()) {
        return;
    }
    m_background->emergencyShow();
}

void AbstractLocker::addAllowedWindow(quint32 windows)
{
    Q_UNUSED(windows);
}

}

#include "moc_abstractlocker.cpp"
