/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <processcore/process_attribute.h>
#include <processcore/process_data_provider.h>

class QProcess;

class NetworkPlugin : public KSysGuard::ProcessDataProvider
{
    Q_OBJECT
public:
    NetworkPlugin(QObject *parent, const QVariantList &args);

    void handleEnabledChanged(bool enabled) override;

private:
    QProcess *m_process = nullptr;
    KSysGuard::ProcessAttribute *m_inboundSensor = nullptr;
    KSysGuard::ProcessAttribute *m_outboundSensor = nullptr;
};
