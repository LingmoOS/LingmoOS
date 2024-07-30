/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "network.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KPluginFactory>

#include "networkconstants.h"
#include "networklogging.h"

using namespace KSysGuard;

NetworkPlugin::NetworkPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
{
    const auto executable = NetworkConstants::HelperLocation;
    if (!QFile::exists(executable)) {
        qCWarning(KSYSGUARD_PLUGIN_NETWORK) << "Could not find ksgrd_network_helper";
        return;
    }

    qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Network plugin loading";
    qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Found helper at" << qPrintable(executable);

    m_inboundSensor = new ProcessAttribute(QStringLiteral("netInbound"), i18nc("@title", "Download Speed"), this);
    m_inboundSensor->setShortName(i18nc("@title", "Download"));
    m_inboundSensor->setUnit(KSysGuard::UnitByteRate);
    m_inboundSensor->setVisibleByDefault(true);
    m_outboundSensor = new ProcessAttribute(QStringLiteral("netOutbound"), i18nc("@title", "Upload Speed"), this);
    m_outboundSensor->setShortName(i18nc("@title", "Upload"));
    m_outboundSensor->setUnit(KSysGuard::UnitByteRate);
    m_outboundSensor->setVisibleByDefault(true);

    addProcessAttribute(m_inboundSensor);
    addProcessAttribute(m_outboundSensor);

    m_process = new QProcess(this);
    m_process->setProgram(executable);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus status) {
        if (exitCode != 0 || status != QProcess::NormalExit) {
            qCWarning(KSYSGUARD_PLUGIN_NETWORK) << "Helper process terminated abnormally:" << m_process->readAllStandardError().trimmed();
        }
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        while (m_process->canReadLine()) {
            const QString line = QString::fromUtf8(m_process->readLine());

            // Each line consists of: timestamp|PID|pid|IN|in_bytes|OUT|out_bytes
            const auto parts = QStringView(line).split(QLatin1Char('|'), Qt::SkipEmptyParts);
            if (parts.size() < 7) {
                continue;
            }

            long pid = parts.at(2).toLong();

            auto timeStamp = QDateTime::currentDateTimeUtc();
            timeStamp.setTime(QTime::fromString(parts.at(0).toString(), QStringLiteral("HH:mm:ss")));

            auto bytesIn = parts.at(4).toUInt();
            auto bytesOut = parts.at(6).toUInt();

            auto process = getProcess(pid);
            if (!process) {
                return;
            }

            m_inboundSensor->setData(process, bytesIn);
            m_outboundSensor->setData(process, bytesOut);
        }
    });
}

void NetworkPlugin::handleEnabledChanged(bool enabled)
{
    if (enabled) {
        qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Network plugin enabled, starting helper";
        m_process->start();
    } else {
        qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Network plugin disabled, stopping helper";
        m_process->terminate();
    }
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "networkplugin.json", registerPlugin<NetworkPlugin>();)

#include "network.moc"
