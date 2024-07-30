/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <processcore/process_attribute.h>
#include <processcore/process_data_provider.h>

class QProcess;

class NvidiaPlugin : public KSysGuard::ProcessDataProvider
{
    Q_OBJECT
public:
    NvidiaPlugin(QObject *parent, const QVariantList &args);
    void handleEnabledChanged(bool enabled) override;

private:
    void setup();

    KSysGuard::ProcessAttribute *m_usage = nullptr;
    KSysGuard::ProcessAttribute *m_memory = nullptr;

    QString m_sniExecutablePath;
    QProcess *m_process = nullptr;
};
