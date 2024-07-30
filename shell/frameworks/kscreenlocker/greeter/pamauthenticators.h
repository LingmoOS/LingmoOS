/*
    SPDX-FileCopyrightText: 2023 Janet Blackquill <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "pamauthenticator.h"
#include <QObject>
#include <memory>

class PamAuthenticators : public QObject
{
    Q_OBJECT

    // these properties delegate to the interactive authenticator
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)

    Q_PROPERTY(QString prompt READ prompt NOTIFY promptChanged)
    Q_PROPERTY(QString promptForSecret READ promptForSecret NOTIFY promptForSecretChanged)

    Q_PROPERTY(QString infoMessage READ infoMessage NOTIFY infoMessageChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    // this property true if any of the authenticators' unlocked properties are true
    Q_PROPERTY(bool unlocked READ isUnlocked NOTIFY succeeded)

    // this property is a sum of the noninteractive authenticators' flags
    Q_PROPERTY(PamAuthenticator::NoninteractiveAuthenticatorTypes authenticatorTypes READ authenticatorTypes NOTIFY authenticatorTypesChanged)

    Q_PROPERTY(AuthenticatorsState state READ state NOTIFY stateChanged)

    Q_PROPERTY(bool hadPrompt READ hadPrompt NOTIFY hadPromptChanged)

public:
    PamAuthenticators(std::unique_ptr<PamAuthenticator> &&interactive,
                      std::vector<std::unique_ptr<PamAuthenticator>> &&noninteractive,
                      QObject *parent = nullptr);
    ~PamAuthenticators() override;

    enum AuthenticatorsState {
        Idle,
        Authenticating,
    };
    Q_ENUM(AuthenticatorsState)

    AuthenticatorsState state() const;
    Q_SIGNAL void stateChanged();
    Q_INVOKABLE void startAuthenticating();
    Q_INVOKABLE void stopAuthenticating();

    // these properties delegate to the interactive authenticator
    bool isBusy() const;
    Q_SIGNAL void busyChanged();

    QString prompt() const;
    Q_SIGNAL void promptChanged();
    QString promptForSecret() const;
    Q_SIGNAL void promptForSecretChanged();
    QString infoMessage() const;
    Q_SIGNAL void infoMessageChanged();
    QString errorMessage() const;
    Q_SIGNAL void errorMessageChanged();

    // these delegate to interactive authenticator
    Q_INVOKABLE void respond(const QByteArray &response);
    Q_INVOKABLE void cancel();

    // this property is true if any of the authenticators' unlocked properties are true
    bool isUnlocked() const;
    Q_SIGNAL void succeeded();

    PamAuthenticator::NoninteractiveAuthenticatorTypes authenticatorTypes() const;
    Q_SIGNAL void authenticatorTypesChanged();

    Q_SIGNAL void failed(PamAuthenticator::NoninteractiveAuthenticatorTypes what, PamAuthenticator *authenticator);
    Q_SIGNAL void noninteractiveError(PamAuthenticator::NoninteractiveAuthenticatorTypes what, PamAuthenticator *authenticator);
    Q_SIGNAL void noninteractiveInfo(PamAuthenticator::NoninteractiveAuthenticatorTypes what, PamAuthenticator *authenticator);

    void setGraceLocked(bool b);

    bool hadPrompt() const;
    Q_SIGNAL void hadPromptChanged();

private:
    struct Private;
    QScopedPointer<Private> d;

    // convenience internal function for setting state,
    // should not be exposed to outsiders
    void setState(AuthenticatorsState state);
};
