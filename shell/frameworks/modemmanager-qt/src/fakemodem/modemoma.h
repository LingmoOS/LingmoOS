/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_FAKE_MODEM_MODEM_OMA_H
#define MODEMMANAGERQT_FAKE_MODEM_MODEM_OMA_H

#include "generictypes.h"

#include <QDBusAbstractAdaptor>

class ModemOma : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakemodem.Modem.Oma")
public:
    explicit ModemOma(QObject *parent = nullptr);
    ~ModemOma() override;

    Q_PROPERTY(uint Features READ features)
    Q_PROPERTY(ModemManager::OmaSessionTypes PendingNetworkInitiatedSessions READ pendingNetworkInitiatedSessions)
    Q_PROPERTY(int SessionState READ sessionState)
    Q_PROPERTY(uint SessionType READ sessionType)

    uint features() const;
    ModemManager::OmaSessionTypes pendingNetworkInitiatedSessions() const;
    int sessionState() const;
    uint sessionType() const;

    /* Not part of dbus interface */
    void setModemPath(const QString &path);
    void setEnableNotifications(bool enable);
    void setFeatures(uint features);
    void setPendingNetworkInitiatedSessions(const ModemManager::OmaSessionTypes &sessions);
    void setSessionState(int state);
    void setSessionType(uint type);

    QVariantMap toMap() const;

public Q_SLOTS:
    Q_SCRIPTABLE void AcceptNetworkInitiatedSession(uint session_id, bool accept);
    Q_SCRIPTABLE void CancelSession();
    Q_SCRIPTABLE void Setup(uint features);
    Q_SCRIPTABLE void StartClientInitiatedSession(uint session_type);

Q_SIGNALS: // SIGNALS
    Q_SCRIPTABLE void SessionStateChanged(int old_session_state, int new_session_state, uint session_state_failed_reason);

private:
    QString m_modemPath;
    bool m_enabledNotifications;
    uint m_features;
    ModemManager::OmaSessionTypes m_sessions;
    int m_sessionState;
    uint m_sessionType;
};

#endif
