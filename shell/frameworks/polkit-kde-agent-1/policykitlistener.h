#ifndef POLICYKITLISTENER_H
#define POLICYKITLISTENER_H

/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Jaroslav Reznik <jreznik@redhat.com>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QHash>
#include <QPointer>

#include <PolkitQt1/Agent/Listener>

class QuickAuthDialog;

using namespace PolkitQt1::Agent;

class PolicyKitListener : public Listener
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Polkit1AuthAgent")
public:
    explicit PolicyKitListener(QObject *parent = nullptr);
    ~PolicyKitListener() override;

public Q_SLOTS:
    void initiateAuthentication(const QString &actionId,
                                const QString &message,
                                const QString &iconName,
                                const PolkitQt1::Details &details,
                                const QString &cookie,
                                const PolkitQt1::Identity::List &identities,
                                PolkitQt1::Agent::AsyncResult *result) override;
    bool initiateAuthenticationFinish() override;
    void cancelAuthentication() override;

    void tryAgain();
    void finishObtainPrivilege();

    void request(const QString &request, bool echo);
    void completed(bool gainedAuthorization);
    void showError(const QString &text);
    void showInfo(const QString &text);

    void setWIdForAction(const QString &action, qulonglong wID);
    void setWindowHandleForAction(const QString &action, const QString &handle);
    void setActivationTokenForAction(const QString &action, const QString &token);

private:
    QPointer<QuickAuthDialog> m_dialog;
    QPointer<Session> m_session = nullptr;
    bool m_inProgress;
    bool m_gainedAuthorization;
    bool m_wasCancelled;
    int m_numTries;
    PolkitQt1::Identity::List m_identities;
    PolkitQt1::Agent::AsyncResult *m_result;
    QString m_cookie;
    PolkitQt1::Identity m_selectedUser;

    QHash<QString /*action*/, QString /*window handle*/> m_windowHandles;
    QHash<QString /*action*/, QString /*activation token*/> m_activationTokens;

private Q_SLOTS:
    void dialogAccepted();
    void dialogCanceled();
    void userSelected(const PolkitQt1::Identity &identity);

private:
    void handleParentWindow(const QString &action, const QString &handle);
    void handleWaylandActivation(const QString &action, const QString &token);
};

#endif
