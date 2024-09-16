// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POLICYKITLISTENER_H
#define POLICYKITLISTENER_H

#include <QPointer>
#include <QHash>

#include <polkit-qt5-1/PolkitQt1/Agent/Listener>
#include <polkit-qt5-1/PolkitQt1/Details>

#include "fprintd_interface.h"
#include "fprintddevice_interface.h"

class PluginManager;
class AuthDialog;

using namespace PolkitQt1::Agent;
using FPrintd = org::deepin::dde::Fprintd1;
using FPrintdDevice = org::deepin::dde::fprintd1::Device;

class PolicyKitListener : public Listener
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.Polkit1.AuthAgent")
public:
    explicit PolicyKitListener(QObject *parent = nullptr);
    virtual ~PolicyKitListener() override;

public slots:
    void initiateAuthentication(const QString &actionId,
                                const QString &message,
                                const QString &iconName,
                                const PolkitQt1::Details &details,
                                const QString &cookie,
                                const PolkitQt1::Identity::List &identities,
                                PolkitQt1::Agent::AsyncResult *result) override;
    bool initiateAuthenticationFinish() override;
    void cancelAuthentication() override;

    void createSessionForId(const PolkitQt1::Identity &identity);
    void finishObtainPrivilege();

    void request(const QString &request, bool echo);
    void completed(bool gainedAuthorization);
    void showError(const QString &text);
    void showInfo(const QString &info);
    void exAuthStatus(int statusCode, int authFlags, const QString &status);
    void exAuthInfo(bool isMfa, QList<int> &authTypes);

    void fillResult();
    void setWIdForAction(const QString &action, qulonglong wID);

private slots:
    void dialogAccepted();
    void dialogCanceled();

private:
    void initDialog(const QString &actionId);

private:
    QPointer<AuthDialog> m_dialog;
    QPointer<PluginManager> m_pluginManager;
    QPointer<Session> m_session;

    PolkitQt1::Identity::List m_identities;
    PolkitQt1::Agent::AsyncResult *m_result;
    QString m_cookie;

    PolkitQt1::Details m_details;
    PolkitQt1::Identity m_selectedUser;

    bool m_inProgress;
    bool m_gainedAuthorization;
    bool m_wasCancelled;

    bool m_showInfoSuccess;
    bool m_exAuth;
    bool m_isMfa;
};

#endif
