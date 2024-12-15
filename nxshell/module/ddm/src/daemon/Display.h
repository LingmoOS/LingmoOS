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

#ifndef DDM_DISPLAY_H
#define DDM_DISPLAY_H

#include <QObject>
#include <QPointer>
#include <QDir>

#include "Auth.h"
#include "Session.h"

class QLocalSocket;

namespace DDM {
    class Authenticator;
    class DisplayServer;
    class Seat;
    class SocketServer;
    class Greeter;

    class Display : public QObject {
        Q_OBJECT
        Q_DISABLE_COPY(Display)
    public:
        enum DisplayServerType {
            X11DisplayServerType,
            X11UserDisplayServerType,
            WaylandDisplayServerType,
            SingleCompositerServerType
        };
        Q_ENUM(DisplayServerType)

        static DisplayServerType defaultDisplayServerType();
        explicit Display(Seat *parent, DisplayServerType serverType);
        ~Display();

        DisplayServerType displayServerType() const;
        DisplayServer *displayServer() const;

        int terminalId() const;

        const QString &name() const;

        QString sessionType() const;
        QString reuseSessionId() const { return m_reuseSessionId; }

        Seat *seat() const;
        void switchToUser(const QString &user);

        QVector<Auth*> loginedSession() const {
            return m_auths;
        }

    public slots:
        bool start();
        void stop();

        void login(QLocalSocket *socket,
                   const QString &user, const QString &password,
                   const Session &session);
        void unlock(QLocalSocket *socket,
                   const QString &user, const QString &password);
        bool attemptAutologin();
        void displayServerStarted();

    signals:
        void stopped();
        void displayServerFailed();

        void loginFailed(QLocalSocket *socket, const QString &user);
        void loginSucceeded(QLocalSocket *socket, const QString &user);

    private:
        QString findGreeterTheme() const;
        bool findSessionEntry(const QStringList &dirPaths, const QString &name) const;

        void startAuth(const QString &user, const QString &password,
                       const Session &session);
        void startIdentify(const QString &user, const QString &password);

        DisplayServerType m_displayServerType = X11DisplayServerType;

        bool m_relogin { true };
        bool m_started { false };

        int m_terminalId = 0;
        int m_sessionTerminalId = 0;

        QString m_reuseSessionId;

        QVector<Auth*> m_auths;
        Auth* m_currentAuth { nullptr };
        DisplayServer *m_displayServer { nullptr };
        Seat *m_seat { nullptr };
        SocketServer *m_socketServer { nullptr };
        QPointer<QLocalSocket> m_socket;
        Greeter *m_greeter { nullptr };

    private slots:
        void slotRequestChanged();
        void slotAuthenticationFinished(const QString &user, bool success, bool identifyOnly);
        void slotSessionStarted(bool success);
        void slotHelperFinished(Auth::HelperExitStatus status);
        void slotAuthInfo(const QString &message, Auth::Info info);
        void slotAuthError(const QString &message, Auth::Error error);
    };
}

#endif // DDM_DISPLAY_H
