/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FRAMEWORKINTEGRATIONPLUGIN_H
#define FRAMEWORKINTEGRATIONPLUGIN_H

#include <KMessageBoxDontAskAgainInterface>
#include <KMessageBoxNotifyInterface>
#include <QObject>

class KConfig;

class KMessageBoxDontAskAgainConfigStorage : public KMessageBoxDontAskAgainInterface
{
public:
    KMessageBoxDontAskAgainConfigStorage()
        : KMessageBox_againConfig(nullptr)
    {
    }
    ~KMessageBoxDontAskAgainConfigStorage() override
    {
    }

    bool shouldBeShownTwoActions(const QString &dontShowAgainName, KMessageBox::ButtonCode &result) override;
    bool shouldBeShownContinue(const QString &dontShowAgainName) override;
    void saveDontShowAgainTwoActions(const QString &dontShowAgainName, KMessageBox::ButtonCode result) override;
    void saveDontShowAgainContinue(const QString &dontShowAgainName) override;
    void enableAllMessages() override;
    void enableMessage(const QString &dontShowAgainName) override;
    void setConfig(KConfig *cfg) override
    {
        KMessageBox_againConfig = cfg;
    }

private:
    KConfig *KMessageBox_againConfig;
};

class KMessageBoxNotify : public KMessageBoxNotifyInterface
{
public:
    void sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget *parent) override;
};

class KFrameworkIntegrationPlugin : public QObject
{
    Q_PLUGIN_METADATA(IID "org.kde.FrameworkIntegrationPlugin")
    Q_OBJECT
public:
    KFrameworkIntegrationPlugin();

public Q_SLOTS:
    void reparseConfiguration();

private:
    KMessageBoxDontAskAgainConfigStorage m_dontAskAgainConfigStorage;
    KMessageBoxNotify m_notify;
};

#endif // FRAMEWORKINTEGRATIONPLUGIN_H
