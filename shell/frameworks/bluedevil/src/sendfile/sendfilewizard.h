/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <QStringList>
#include <QWizard>

#include <BluezQt/Manager>
#include <BluezQt/PendingCall>

#include "discoverwidget.h"

class QDBusObjectPath;

class SendFilesJob;

class SendFileWizard : public QWizard
{
    Q_OBJECT

public:
    explicit SendFileWizard(const QString &device, const QStringList &files);
    ~SendFileWizard() override;

    void done(int result) override;

    BluezQt::Manager *manager() const;

    QStringList files() const;
    void setFiles(const QStringList &files);

    BluezQt::DevicePtr device() const;
    void setDevice(BluezQt::DevicePtr device);

    QString errorMessage() const;
    void setErrorMessage(const QString &message);

    void startTransfer(const QDBusObjectPath &session);

Q_SIGNALS:
    void obexServiceInitialized();

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void slotServicePendingCallFinished(BluezQt::PendingCall *call);
    void initWizard();

private:
    const QString m_deviceUrl;
    QStringList m_files;
    QString m_errorMessage;

    BluezQt::Manager *m_manager = nullptr;
    BluezQt::DevicePtr m_device;
    SendFilesJob *m_job = nullptr;

    bool m_obexServiceInitialized = false;
    BluezQt::PendingCall::Error m_obexdServiceError = BluezQt::PendingCall::NoError;
};
