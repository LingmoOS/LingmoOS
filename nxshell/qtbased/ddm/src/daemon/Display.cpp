/***************************************************************************
* Copyright (c) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
* Copyright (c) 2014 Martin Bříza <mbriza@redhat.com>
* Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
***************************************************************************/

#include "Display.h"

#include "Configuration.h"
#include "DaemonApp.h"
#include "DisplayManager.h"
#include "XorgDisplayServer.h"
#include "XorgUserDisplayServer.h"
#include "SingleWaylandDisplayServer.h"
#include "Seat.h"
#include "SocketServer.h"
#include "Greeter.h"
#include "Utils.h"

#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QLocalSocket>

#include <linux/vt.h>
#include <pwd.h>
#include <qstringliteral.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/ioctl.h>
#include <fcntl.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include "Login1Manager.h"
#include "Login1Session.h"
#include "VirtualTerminal.h"
#include "WaylandDisplayServer.h"
#include "config.h"

static int s_ttyFailures = 0;
#define STRINGIFY(x) #x

namespace DDM {
    bool isTtyInUse(const QString &desiredTty) {
        if (Logind::isAvailable()) {
            OrgFreedesktopLogin1ManagerInterface manager(Logind::serviceName(), Logind::managerPath(), QDBusConnection::systemBus());
            auto reply = manager.ListSessions();
            reply.waitForFinished();

            const auto info = reply.value();
            for(const SessionInfo &s : info) {
                OrgFreedesktopLogin1SessionInterface session(Logind::serviceName(), s.sessionPath.path(), QDBusConnection::systemBus());
                if (desiredTty == session.tTY() && session.state() != QLatin1String("closing")) {
                    qDebug() << "tty" << desiredTty << "already in use by" << session.user().path.path() << session.state()
                                      << session.display() << session.desktop() << session.vTNr();
                    return true;
                }
            }
        }
        return false;
    }

    int fetchAvailableVt() {
        if (!isTtyInUse(QStringLiteral("tty" STRINGIFY(DDM_INITIAL_VT)))) {
            return DDM_INITIAL_VT;
        }
        const auto vt = VirtualTerminal::currentVt();
        if (vt > 0 && !isTtyInUse(QStringLiteral("tty%1").arg(vt))) {
            return vt;
        }
        return VirtualTerminal::setUpNewVt();
    }

    Display::DisplayServerType Display::defaultDisplayServerType()
    {
        const QString &displayServerType = mainConfig.DisplayServer.get().toLower();
        DisplayServerType ret;
        if (displayServerType == QStringLiteral("x11-user")) {
            ret = X11UserDisplayServerType;
        } else if (displayServerType == QStringLiteral("wayland")) {
            ret = WaylandDisplayServerType;
        } else if (displayServerType == QStringLiteral("single")) {
            ret = SingleCompositerServerType;
        } else {
            if (displayServerType != QLatin1String("x11")) {
                qWarning("\"%s\" is an invalid value for General.DisplayServer: fall back to \"x11\"",
                    qPrintable(displayServerType));
            }
            ret = X11DisplayServerType;
        }
        return ret;
    }

    Display::Display(Seat *parent, DisplayServerType serverType)
        : QObject(parent),
        m_displayServerType(serverType),
        m_seat(parent),
        m_socketServer(new SocketServer(this)),
        m_greeter(new Greeter(this))
    {
        // Create display server
        switch (m_displayServerType) {
        case X11DisplayServerType:
            m_terminalId = VirtualTerminal::setUpNewVt();
            m_displayServer = new XorgDisplayServer(this);
            break;
        case X11UserDisplayServerType:
            m_terminalId = fetchAvailableVt();
            m_displayServer = new XorgUserDisplayServer(this);
            m_greeter->setDisplayServerCommand(XorgUserDisplayServer::command(this));
            break;
        case WaylandDisplayServerType:
            m_terminalId = fetchAvailableVt();
            m_displayServer = new WaylandDisplayServer(this);
            m_greeter->setDisplayServerCommand(mainConfig.Wayland.CompositorCommand.get());
            break;
        case SingleCompositerServerType:
            m_terminalId = fetchAvailableVt();
            m_displayServer = new SingleWaylandDisplayServer(m_socketServer, this);
            m_greeter->setDisplayServerCommand(mainConfig.Single.CompositorCommand.get());
            m_greeter->setSingleMode();
            break;
        }

        qDebug("Using VT %d", m_terminalId);

        // restart display after display server ended
        connect(m_displayServer, &DisplayServer::started, this, &Display::displayServerStarted);
        connect(m_displayServer, &DisplayServer::stopped, this, &Display::stop);

        // connect login signal
        connect(m_socketServer, &SocketServer::login, this, &Display::login);

        // connect unlock signal
        connect(m_socketServer, &SocketServer::unlock,this, &Display::unlock);

        // connect login result signals
        connect(this, &Display::loginFailed, m_socketServer, &SocketServer::loginFailed);
        connect(this, &Display::loginSucceeded, m_socketServer, &SocketServer::loginSucceeded);

        connect(m_greeter, &Greeter::failed, this, &Display::stop);
        connect(m_greeter, &Greeter::ttyFailed, this, [this] {
            ++s_ttyFailures;
            if (s_ttyFailures > 5) {
                QCoreApplication::exit(23);
            }
            // It might be the case that we are trying a tty that has been taken over by a
            // different process. In such a case, switch back to the initial one and try again.
            VirtualTerminal::jumpToVt(DDM_INITIAL_VT, true);
            stop();
        });
        connect(m_greeter, &Greeter::displayServerFailed, this, &Display::displayServerFailed);
    }

    Display::~Display() {
        for (auto *item : m_auths) {
            disconnect(item, &Auth::finished, this, &Display::slotHelperFinished);
        }
        stop();
    }

    Display::DisplayServerType Display::displayServerType() const
    {
        return m_displayServerType;
    }

    DisplayServer *Display::displayServer() const
    {
        return m_displayServer;
    }

    int Display::terminalId() const {
        // TODO: if not use single wayland mode, m_auths only have one element.
        return !m_auths.isEmpty() && m_auths.first()->isActive() ? m_sessionTerminalId : m_terminalId;
    }

    const QString &Display::name() const {
        return m_displayServer->display();
    }

    QString Display::sessionType() const {
        return m_displayServer->sessionType();
    }

    Seat *Display::seat() const {
        return m_seat;
    }

    void Display::switchToUser(const QString &user) {
        auto* server = reinterpret_cast<SingleWaylandDisplayServer*>(m_displayServer);
        server->activateUser(user);
    }

    bool Display::start() {
        return m_started || m_displayServer->start();
    }

    bool Display::attemptAutologin() {
        Session::Type sessionType = Session::X11Session;

        // determine session type
        QString autologinSession = mainConfig.Autologin.Session.get();
        // not configured: try last successful logged in
        if (autologinSession.isEmpty()) {
            autologinSession = stateConfig.Last.Session.get();
        }
        if (findSessionEntry(mainConfig.Wayland.SessionDir.get(), autologinSession)) {
            sessionType = Session::WaylandSession;
        } else if (findSessionEntry(mainConfig.X11.SessionDir.get(), autologinSession)) {
            sessionType = Session::X11Session;
        } else {
            qCritical() << "Unable to find autologin session entry" << autologinSession;
            return false;
        }

        Session session;
        session.setTo(sessionType, autologinSession);

        startAuth(mainConfig.Autologin.User.get(), QString(), session);

        return true;
    }

    void Display::displayServerStarted() {
        // check flag
        if (m_started)
            return;

        // setup display
        m_displayServer->setupDisplay();

        // log message
        qDebug() << "Display server started.";

        if ((daemonApp->first || mainConfig.Autologin.Relogin.get()) &&
            !mainConfig.Autologin.User.get().isEmpty()) {
            // reset first flag
            daemonApp->first = false;

            // set flags
            m_started = true;

            bool success = attemptAutologin();
            if (success) {
                return;
            } else {
                qWarning() << "Autologin failed!";
            }
        }

        // start socket server
        m_socketServer->start(m_displayServer->display());

        if (!daemonApp->testing()) {
            // change the owner and group of the socket to avoid permission denied errors
            struct passwd *pw = getpwnam("dde");
            if (pw) {
                if (chown(qPrintable(m_socketServer->socketAddress()), pw->pw_uid, pw->pw_gid) == -1) {
                    qWarning() << "Failed to change owner of the socket";
                    return;
                }
            }
        }

        m_greeter->setSocket(m_socketServer->socketAddress());
        m_greeter->setTheme(findGreeterTheme());

        // start greeter
        m_greeter->start();

        // reset first flag
        daemonApp->first = false;

        // set flags
        m_started = true;
    }

    void Display::stop() {
        // check flag
        if (!m_started)
            return;

        // stop the greeter
        m_greeter->stop();

        for (auto *item : m_auths) {
            item->stop();
        }

        // stop socket server
        m_socketServer->stop();

        // stop display server
        m_displayServer->blockSignals(true);
        m_displayServer->stop();
        m_displayServer->blockSignals(false);

        // reset flag
        m_started = false;

        // emit signal
        emit stopped();
    }

    void Display::login(QLocalSocket *socket,
                        const QString &user, const QString &password,
                        const Session &session) {
        m_socket = socket;

        //the DDM user has special privileges that skip password checking so that we can load the greeter
        //block ever trying to log in as the DDM user
        if (user == QLatin1String("dde")) {
            emit loginFailed(m_socket, user);
            return;
        }

        // authenticate
        startAuth(user, password, session);
    }

    void Display::unlock(QLocalSocket *socket,
                        const QString &user, const QString &password) {
        m_socket = socket;

        //the DDM user has special privileges that skip password checking so that we can load the greeter
        //block ever trying to log in as the DDM user
        if (user == QLatin1String("dde")) {
            emit loginFailed(m_socket, user);
            return;
        }

        // authenticate
        startIdentify(user, password);
    }

    QString Display::findGreeterTheme() const {
        QString themeName = mainConfig.Theme.Current.get();

        // an unconfigured theme means the user wants to load the
        // default theme from the resources
        if (themeName.isEmpty())
            return QString();

        QDir dir(mainConfig.Theme.ThemeDir.get());

        // return the default theme if it exists
        if (dir.exists(themeName))
            return dir.absoluteFilePath(themeName);

        // otherwise use the embedded theme
        qWarning() << "The configured theme" << themeName << "doesn't exist, using the embedded theme instead";
        return QString();
    }

    bool Display::findSessionEntry(const QStringList &dirPaths, const QString &name) const {
        const QFileInfo fileInfo(name);
        QString fileName = name;

        // append extension
        const QString extension = QStringLiteral(".desktop");
        if (!fileName.endsWith(extension))
            fileName += extension;

        for (const auto &path: dirPaths) {
            QDir dir = path;

            // Given an absolute path: Check that it matches dir
            if (fileInfo.isAbsolute() && fileInfo.absolutePath() != dir.absolutePath())
                continue;

            if (dir.exists(fileName))
                return true;
        }
        return false;
    }

    void Display::startIdentify(const QString &user, const QString &password) {
        qDebug() << "start Identify user" << user;
        auto *auth = new Auth(this);
        auth->setObjectName("userIdentify");
        m_auths.push_back(auth);

        auth->setVerbose(true);
        connect(auth, &Auth::requestChanged, this, &Display::slotRequestChanged);
        connect(auth, &Auth::authentication, this, &Display::slotAuthenticationFinished);
        connect(auth, &Auth::finished, this, &Display::slotHelperFinished);
        connect(auth, &Auth::info, this, &Display::slotAuthInfo);
        connect(auth, &Auth::error, this, &Display::slotAuthError);

        auth->setVerbose(true);
        auth->setPassword(password);
        auth->setUser(user);
        auth->setIdentifyOnly(true);
        auth->start();
    }

    void Display::startAuth(const QString &user, const QString &password, const Session &session) {

        // respond to authentication requests
        Auth *auth = nullptr;
        for (auto *item : m_auths) {
            if (item->user() == user) {
                auth = item;
                break;
            }
        }

        if (!auth) {
            auth = new Auth(this);
            m_auths << auth;
        }

        auth->setVerbose(true);
        auth->setAutologin(mainConfig.Autologin.User.get() == user);
        connect(auth, &Auth::requestChanged, this, &Display::slotRequestChanged);
        connect(auth, &Auth::authentication, this, &Display::slotAuthenticationFinished);
        connect(auth, &Auth::sessionStarted, this, &Display::slotSessionStarted);
        connect(auth, &Auth::finished, this, &Display::slotHelperFinished);
        connect(auth, &Auth::info, this, &Display::slotAuthInfo);
        connect(auth, &Auth::error, this, &Display::slotAuthError);

        if (auth->isActive()) {
            qWarning() << "Existing authentication ongoing, aborting";
            return;
        }

        auth->setPassword(password);

        // sanity check
        if (!session.isValid()) {
            qCritical() << "Invalid session" << session.fileName();
            return;
        }
        if (session.xdgSessionType().isEmpty()) {
            qCritical() << "Failed to find XDG session type for session" << session.fileName();
            return;
        }
        if (session.exec().isEmpty()) {
            qCritical() << "Failed to find command for session" << session.fileName();
            return;
        }

        m_reuseSessionId = QString();

        if (Logind::isAvailable() && mainConfig.Users.ReuseSession.get()) {
            OrgFreedesktopLogin1ManagerInterface manager(Logind::serviceName(), Logind::managerPath(), QDBusConnection::systemBus());
            auto reply = manager.ListSessions();
            reply.waitForFinished();

            const auto info = reply.value();
            for(const SessionInfo &s : reply.value()) {
                if (s.userName == user) {
                    OrgFreedesktopLogin1SessionInterface session(Logind::serviceName(), s.sessionPath.path(), QDBusConnection::systemBus());
                    if (session.service() == QLatin1String("dde") && session.state() == QLatin1String("online")) {
                        m_reuseSessionId = s.sessionId;
                        break;
                    }
                }
            }
        }

        auth->setTTY(m_terminalId);
        if ((session.type() == Session::WaylandSession && m_displayServerType == X11DisplayServerType) || (m_greeter->isRunning() && m_displayServerType != X11DisplayServerType) || (m_displayServerType == SingleCompositerServerType)) {
            // Create a new VT when we need to have another compositor running
            auth->setTTY(VirtualTerminal::setUpNewVt());
        }

        // some information
        qDebug() << "Session" << session.fileName() << "selected, command:" << session.exec() << "for VT" << auth->tty();

        QProcessEnvironment env;
        env.insert(session.additionalEnv());

        env.insert(QStringLiteral("PATH"), mainConfig.Users.DefaultPath.get());

        // session id
        {
            const QString sessionId = QStringLiteral("Session%1").arg(daemonApp->newSessionId());
            daemonApp->displayManager()->AddSession(sessionId, seat()->name(), user);
            env.insert(QStringLiteral("XDG_SESSION_PATH"), daemonApp->displayManager()->sessionPath(sessionId));
            auth->setSessionId(sessionId);
        }

        env.insert(QStringLiteral("DESKTOP_SESSION"), session.desktopSession());
        if (!session.desktopNames().isEmpty())
            env.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), session.desktopNames());
        env.insert(QStringLiteral("XDG_SESSION_CLASS"), QStringLiteral("user"));
        env.insert(QStringLiteral("XDG_SESSION_TYPE"), session.xdgSessionType());
        env.insert(QStringLiteral("XDG_VTNR"), QString::number(auth->tty()));
        env.insert(QStringLiteral("XDG_SEAT"), seat()->name());
        env.insert(QStringLiteral("XDG_SEAT_PATH"), daemonApp->displayManager()->seatPath(seat()->name()));

#ifdef HAVE_SYSTEMD
        env.insert(QStringLiteral("XDG_SESSION_DESKTOP"), session.desktopNames());
#endif

        if (session.xdgSessionType() == QLatin1String("x11")) {
          if (m_displayServerType == X11DisplayServerType)
            env.insert(QStringLiteral("DISPLAY"), name());
          else
            auth->setDisplayServerCommand(XorgUserDisplayServer::command(this));
        } else {
            if (m_displayServerType == DisplayServerType::SingleCompositerServerType) {
                #ifdef QT_DEBUG
                env.insert("WAYLAND_DEBUG", "1");
                #endif

                env.insert("DDE_CURRENT_COMPOSITOR", "TreeLand");
            }

            auth->setDisplayServerCommand(QStringLiteral());
        }

        auth->setUser(user);
        if (m_reuseSessionId.isNull()) {
            auth->setSession(session.exec());
        }
        auth->insertEnvironment(env);
        auth->setSingleMode(m_displayServerType == DisplayServerType::SingleCompositerServerType);
        auth->start();
    }

    void Display::slotAuthenticationFinished(const QString &user, bool success, bool identifyOnly) {
        Auth* auth = nullptr;

        if(identifyOnly){
            auto ret = std::find_if(m_auths.crbegin(),m_auths.crend(),[&user](Auth *ptr) { qDebug() << ptr->user(); return ptr->user() == user;});
            Q_ASSERT(ret != m_auths.crend());
            auth = *ret;
        } else {
            auto ret = std::find_if(m_auths.cbegin(),m_auths.cend(),[&user](Auth *ptr) { return ptr->user() == user;});
            Q_ASSERT(ret != m_auths.cend());
            auth = *ret;
        }

        Q_ASSERT(auth && auth->identifyOnly() == identifyOnly);

        if (success) {
            if (!m_reuseSessionId.isNull()) {
                OrgFreedesktopLogin1ManagerInterface manager(Logind::serviceName(), Logind::managerPath(), QDBusConnection::systemBus());
                manager.UnlockSession(m_reuseSessionId);
                manager.ActivateSession(m_reuseSessionId);
                m_started = true;
            } else {
                if (qobject_cast<XorgDisplayServer *>(m_displayServer))
                    auth->setCookie(qobject_cast<XorgDisplayServer *>(m_displayServer)->cookie());
            }

            // save last user and last session
            if (mainConfig.Users.RememberLastUser.get())
                stateConfig.Last.User.set(auth->user());
            else
                stateConfig.Last.User.setDefault();
            if (mainConfig.Users.RememberLastSession.get())
                stateConfig.Last.Session.set(auth->session());
            else
                stateConfig.Last.Session.setDefault();
            stateConfig.save();

            if (m_displayServerType == DisplayServerType::SingleCompositerServerType) {
                auto* server = reinterpret_cast<SingleWaylandDisplayServer*>(m_displayServer);
                server->onLoginSucceeded(user);
            } else {
                if (m_socket) {
                    emit loginSucceeded(m_socket, user);
                    daemonApp->displayManager()->setLastSession(auth->sessionId());
                }
            }

        }
        m_socket = nullptr;
    }

    void Display::slotAuthInfo(const QString &message, Auth::Info info) {
        qWarning() << "Authentication information:" << info << message;

        if (!m_socket)
            return;

        m_socketServer->informationMessage(m_socket, message);
    }

    void Display::slotAuthError(const QString &message, Auth::Error error) {
        Auth* auth = qobject_cast<Auth*>(sender());

        qWarning() << "Authentication error:" << error << message;

        if (!m_socket)
            return;

        m_socketServer->informationMessage(m_socket, message);
        if (error == Auth::ERROR_AUTHENTICATION) {
            if (m_displayServerType == DisplayServerType::SingleCompositerServerType) {
                auto* server = reinterpret_cast<SingleWaylandDisplayServer*>(m_displayServer);
                server->onLoginFailed(auth->user());
            } else {
                emit loginFailed(m_socket, auth->user());
            }
        }
    }

    void Display::slotHelperFinished(Auth::HelperExitStatus status) {
        Auth* auth = qobject_cast<Auth*>(sender());
        // Don't restart greeter and display server unless ddm-helper exited
        // with an internal error or the user session finished successfully,
        // we want to avoid greeter from restarting when an authentication
        // error happens (in this case we want to show the message from the
        // greeter

        m_auths.removeOne(auth);
        auth->deleteLater();

        if(!auth->identifyOnly()) {
            daemonApp->displayManager()->RemoveSession(auth->sessionId());
            if (m_displayServerType == DisplayServerType::SingleCompositerServerType) {
                auto* server = reinterpret_cast<SingleWaylandDisplayServer*>(m_displayServer);
                // TODO: switch to greeter
                server->activateUser("dde");
                return;
            }
        }

        if (status != Auth::HELPER_AUTH_ERROR && m_displayServerType != DisplayServerType::SingleCompositerServerType)
            stop();
    }

    void Display::slotRequestChanged() {
        Auth* auth = qobject_cast<Auth*>(sender());
        if (auth->request()->prompts().length() == 1) {
            auth->request()->prompts()[0]->setResponse(qPrintable(auth->password()));
            auth->request()->done();
        } else if (auth->request()->prompts().length() == 2) {
            auth->request()->prompts()[0]->setResponse(qPrintable(auth->user()));
            auth->request()->prompts()[1]->setResponse(qPrintable(auth->password()));
            auth->request()->done();
        }
    }

    void Display::slotSessionStarted(bool success) {
        qDebug() << "Session started" << success;
        Auth* auth = qobject_cast<Auth*>(sender());
        if (m_displayServerType == SingleCompositerServerType) {
            auto* server = reinterpret_cast<SingleWaylandDisplayServer*>(m_displayServer);
            server->activateUser(auth->user());
        }

        if (success && m_displayServerType != SingleCompositerServerType) {
            QTimer::singleShot(5000, m_greeter, &Greeter::stop);
        }
    }
}
