/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>
#include <QThread>

class PamWorker;

class PamAuthenticator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)
    Q_PROPERTY(NoninteractiveAuthenticatorTypes authenticatorType READ authenticatorType CONSTANT)

    Q_PROPERTY(QString prompt READ getPrompt NOTIFY prompt)
    Q_PROPERTY(QString promptForSecret READ getPromptForSecret NOTIFY promptForSecret)

    Q_PROPERTY(QString infoMessage READ getInfoMessage NOTIFY infoMessage)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessage)

    Q_PROPERTY(bool unlocked READ isUnlocked NOTIFY succeeded)

public:
    enum NoninteractiveAuthenticatorType {
        None = 0,
        Fingerprint = 1 << 0,
        Smartcard = 2 << 0,
    };
    Q_DECLARE_FLAGS(NoninteractiveAuthenticatorTypes, NoninteractiveAuthenticatorType)
    Q_FLAG(NoninteractiveAuthenticatorTypes)

    PamAuthenticator(const QString &service,
                     const QString &user,
                     NoninteractiveAuthenticatorTypes authenticatorType = NoninteractiveAuthenticatorType::None,
                     QObject *parent = nullptr);
    ~PamAuthenticator() override;
    Q_DISABLE_COPY_MOVE(PamAuthenticator)

    bool isBusy() const;
    bool isUnlocked() const;
    bool isAvailable() const;
    NoninteractiveAuthenticatorTypes authenticatorType() const;

    // Get prefix to de-duplicate from their signals.
    QString getPrompt() const;
    QString getPromptForSecret() const;
    QString getInfoMessage() const;
    QString getErrorMessage() const;

    QString service() const;

Q_SIGNALS:
    void busyChanged();
    void promptForSecret(const QString &msg);
    void prompt(const QString &msg);
    void infoMessage(const QString &msg);
    void errorMessage(const QString &msg);
    void succeeded();
    void failed();
    void availableChanged();

public Q_SLOTS:
    void tryUnlock();
    void respond(const QByteArray &response);
    void cancel();

protected:
    void init(const QString &service, const QString &user);

private:
    void setBusy(bool busy);

    const std::vector<std::pair<QMetaMethod, const QString &>> m_signalsToMembers;
    // NOTE Don't forget to reset in cancel as necessary
    QString m_prompt;
    QString m_promptForSecret;
    QString m_errorMessage;
    QString m_infoMessage;
    QString m_service;
    bool m_busy = false;
    bool m_unlocked = false;
    bool m_inAuthentication = false;
    bool m_unavailable = false;
    NoninteractiveAuthenticatorTypes m_authenticatorType;
    QThread m_thread;
    PamWorker *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PamAuthenticator::NoninteractiveAuthenticatorTypes)
