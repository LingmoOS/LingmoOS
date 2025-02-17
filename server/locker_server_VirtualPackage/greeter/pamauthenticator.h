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

    Q_PROPERTY(QString prompt READ getPrompt NOTIFY prompt)
    Q_PROPERTY(QString promptForSecret READ getPromptForSecret NOTIFY promptForSecret)

    Q_PROPERTY(QString infoMessage READ getInfoMessage NOTIFY infoMessage)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessage)

    Q_PROPERTY(bool unlocked READ isUnlocked NOTIFY succeeded)

public:
    PamAuthenticator(const QString &service, const QString &user, QObject *parent = nullptr);
    ~PamAuthenticator();

    bool isBusy() const;
    bool isUnlocked() const;

    // Get prefix to de-duplicate from their signals.
    QString getPrompt() const;
    QString getPromptForSecret() const;
    QString getInfoMessage() const;
    QString getErrorMessage() const;

Q_SIGNALS:
    void busyChanged();
    void promptForSecret(const QString &msg);
    void prompt(const QString &msg);
    void infoMessage(const QString &msg);
    void errorMessage(const QString &msg);
    void succeeded();
    void failed();

public Q_SLOTS:
    void tryUnlock();
    void respond(const QByteArray &response);
    void cancel();

protected:
    void init(const QString &service, const QString &user);
    void connectNotify(const QMetaMethod &signal) override;

private:
    void setBusy(bool busy);

    const std::vector<std::pair<QMetaMethod, const QString &>> m_signalsToMembers;
    // NOTE Don't forget to reset in cancel as necessary
    QString m_prompt;
    QString m_promptForSecret;
    QString m_errorMessage;
    QString m_infoMessage;
    bool m_busy = false;
    bool m_unlocked = false;
    QThread m_thread;
    PamWorker *d;
};
