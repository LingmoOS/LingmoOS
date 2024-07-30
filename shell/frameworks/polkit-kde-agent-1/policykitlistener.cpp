/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Jaroslav Reznik <jreznik@redhat.com>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDBusConnection>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlEngine>

#include <KLocalizedString>
#include <KWindowSystem>
#include <KX11Extras>

#include <PolkitQt1/ActionDescription>
#include <PolkitQt1/Agent/Session>
#include <PolkitQt1/Details>
#include <PolkitQt1/Identity>
#include <PolkitQt1/Subject>
#include <QDebug>

#include "IdentitiesModel.h"
#include "QuickAuthDialog.h"
#include "policykitlistener.h"
#include "polkit1authagentadaptor.h"

PolicyKitListener::PolicyKitListener(QObject *parent)
    : Listener(parent)
    , m_inProgress(false)
    , m_selectedUser(nullptr)
{
    (void)new Polkit1AuthAgentAdaptor(this);

    if (!QDBusConnection::sessionBus().registerObject("/org/kde/Polkit1AuthAgent",
                                                      this,
                                                      QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableProperties
                                                          | QDBusConnection::ExportAdaptors)) {
        qWarning() << "Could not initiate DBus helper!";
    }

    qmlRegisterType<IdentitiesModel>("org.kde.polkitkde", 1, 0, "IdentitiesModel");
    qmlRegisterUncreatableType<PolkitQt1::ActionDescription>("org.kde.polkitkde", 1, 0, "ActionDescription", "nope!");

    qDebug() << "Listener online";
}

PolicyKitListener::~PolicyKitListener()
{
}

void PolicyKitListener::setWIdForAction(const QString &action, qulonglong wID)
{
    // For compatibility.
    setWindowHandleForAction(action, QString::number(wID));
}

void PolicyKitListener::setWindowHandleForAction(const QString &action, const QString &handle)
{
    m_windowHandles[action] = handle;

    handleParentWindow(action, handle);
}

void PolicyKitListener::setActivationTokenForAction(const QString &action, const QString &token)
{
    if (KWindowSystem::isPlatformWayland()) {
        // On X we just forceActivateWindow, no need to store the token.
        m_activationTokens[action] = token;
        handleWaylandActivation(action, token);
    }
}

void PolicyKitListener::initiateAuthentication(const QString &actionId,
                                               const QString &message,
                                               const QString &iconName,
                                               const PolkitQt1::Details &details,
                                               const QString &cookie,
                                               const PolkitQt1::Identity::List &identities,
                                               PolkitQt1::Agent::AsyncResult *result)
{
    // The auth action might set any random old icon; all we really want here is a nice
    // generic "lock" icon, so we hardcode it and ignore the icon from the auth action.
    Q_UNUSED(iconName);

    qDebug() << "Initiating authentication";

    if (m_inProgress) {
        result->setError(i18n("Another client is already authenticating, please try again later."));
        result->setCompleted();
        qDebug() << "Another client is already authenticating, please try again later.";
        return;
    }

    if (identities.isEmpty()) {
        result->setError(i18nc("Error response when polkit calls us with an empty list of identities", "No user to authenticate as. Please check your system configuration."));
        result->setCompleted();
        qWarning() << "No user to authenticate as. Please check your system configuration.";
        return;
    }

    m_identities = identities;
    m_cookie = cookie;
    m_result = result;
    m_session.clear();

    m_inProgress = true;

    const QString parentHandle = m_windowHandles.value(actionId);
    const QString activationToken = m_activationTokens.value(actionId);

    m_dialog = new QuickAuthDialog(actionId, message, details, identities);

    if (!parentHandle.isEmpty()) {
        handleParentWindow(actionId, parentHandle);
    }

    connect(m_dialog.data(), SIGNAL(okClicked()), SLOT(dialogAccepted()));
    connect(m_dialog.data(), SIGNAL(rejected()), SLOT(dialogCanceled()));

    m_dialog->show();

    if (KWindowSystem::isPlatformWayland()) {
        if (!activationToken.isEmpty()) {
            handleWaylandActivation(actionId, activationToken);
        }
    } else if (KWindowSystem::isPlatformX11()) {
        KX11Extras::forceActiveWindow(m_dialog->windowHandle()->winId());
    }

    if (identities.length() == 1) {
        m_selectedUser = identities[0];
    } else {
        m_selectedUser = m_dialog.data()->adminUserSelected();
    }

    m_numTries = 0;
    tryAgain();
}

void PolicyKitListener::handleParentWindow(const QString &action, const QString &handle)
{
    if (!m_dialog || m_dialog->actionId() != action) {
        return;
    }

    KWindowSystem::setMainWindow(m_dialog->windowHandle(), handle);
}

void PolicyKitListener::handleWaylandActivation(const QString &action, const QString &token)
{
    if (!m_dialog || m_dialog->actionId() != action) {
        return;
    }

    qputenv("XDG_ACTIVATION_TOKEN", token.toUtf8());
    m_dialog->windowHandle()->requestActivate();
}

void PolicyKitListener::tryAgain()
{
    qDebug() << "Trying again";
    m_wasCancelled = false;

    // We will create new session only when some user is selected
    if (m_selectedUser.isValid()) {
        m_session = new Session(m_selectedUser, m_cookie, m_result);
        // clang-format off
        connect(m_session.data(), SIGNAL(request(QString,bool)), this, SLOT(request(QString,bool)));
        connect(m_session.data(), SIGNAL(completed(bool)), this, SLOT(completed(bool)));
        connect(m_session.data(), SIGNAL(showError(QString)), this, SLOT(showError(QString)));
        connect(m_session.data(), SIGNAL(showInfo(QString)), this, SLOT(showInfo(QString)));
        // clang-format on

        m_session.data()->initiate();
    }
}

void PolicyKitListener::finishObtainPrivilege()
{
    qDebug() << "Finishing obtaining privileges";

    // Number of tries increase only when some user is selected
    if (m_selectedUser.isValid()) {
        m_numTries++;
    }

    if (!m_gainedAuthorization && !m_wasCancelled && !m_dialog.isNull()) {
        m_dialog.data()->authenticationFailure();

        if (m_numTries < 3) {
            m_session.data()->deleteLater();

            tryAgain();
            return;
        }
    }

    if (!m_session.isNull()) {
        m_session.data()->result()->setCompleted();
        m_session.data()->deleteLater();
    } else {
        m_result->setCompleted();
    }

    if (!m_dialog.isNull()) {
        m_dialog.data()->hide();
        m_dialog.data()->deleteLater();
    }

    m_inProgress = false;

    qDebug() << "Finish obtain authorization:" << m_gainedAuthorization;
}

bool PolicyKitListener::initiateAuthenticationFinish()
{
    qDebug() << "Finishing authentication";
    return true;
}

void PolicyKitListener::cancelAuthentication()
{
    qDebug() << "Cancelling authentication";

    m_wasCancelled = true;
    finishObtainPrivilege();
}

void PolicyKitListener::request(const QString &request, bool echo)
{
    Q_UNUSED(echo);
    qDebug() << "Request: " << request;
}

void PolicyKitListener::completed(bool gainedAuthorization)
{
    qDebug() << "Completed: " << gainedAuthorization;

    m_gainedAuthorization = gainedAuthorization;

    finishObtainPrivilege();
}

void PolicyKitListener::showError(const QString &text)
{
    qDebug() << "Error: " << text;
    if (!m_dialog.isNull()) {
        m_dialog.data()->showError(text);
    }
}

void PolicyKitListener::showInfo(const QString &text)
{
    qDebug() << "Info: " << text;
    if (!m_dialog.isNull()) {
        m_dialog.data()->showInfo(text);
    }
}

void PolicyKitListener::dialogAccepted()
{
    qDebug() << "Dialog accepted";

    if (!m_session.isNull() && !m_dialog.isNull()) {
        m_session.data()->setResponse(m_dialog.data()->password());
    }
}

void PolicyKitListener::dialogCanceled()
{
    qDebug() << "Dialog cancelled";

    m_wasCancelled = true;
    if (!m_session.isNull()) {
        m_session.data()->cancel();
    }

    finishObtainPrivilege();
}

void PolicyKitListener::userSelected(const PolkitQt1::Identity &identity)
{
    m_selectedUser = identity;
    // If some user is selected we must destroy existing session
    if (!m_session.isNull()) {
        m_session.data()->deleteLater();
    }
    tryAgain();
}

#include "moc_policykitlistener.cpp"
