/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nvidia.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KPluginFactory>

#include <processcore/process.h>

using namespace KSysGuard;

NvidiaPlugin::NvidiaPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
    , m_sniExecutablePath(QStandardPaths::findExecutable(QStringLiteral("nvidia-smi")))
{
    if (m_sniExecutablePath.isEmpty()) {
        return;
    }

    m_usage = new ProcessAttribute(QStringLiteral("nvidia_usage"), i18n("GPU Usage"), this);
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_memory = new ProcessAttribute(QStringLiteral("nvidia_memory"), i18n("GPU Memory"), this);
    m_memory->setUnit(KSysGuard::UnitPercent);

    addProcessAttribute(m_usage);
    addProcessAttribute(m_memory);
}

void NvidiaPlugin::handleEnabledChanged(bool enabled)
{
    if (enabled) {
        if (!m_process) {
            setup();
        }
        m_process->start();
    } else {
        if (m_process) {
            m_process->terminate();
        }
    }
}

void NvidiaPlugin::setup()
{
    m_process = new QProcess(this);
    m_process->setProgram(m_sniExecutablePath);
    m_process->setArguments({QStringLiteral("pmon")});

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        while (m_process->canReadLine()) {
            const QString line = QString::fromLatin1(m_process->readLine());
            if (line.startsWith(QLatin1Char('#'))) { // comment line
                if (line != QLatin1String("# gpu        pid  type    sm   mem   enc   dec   command\n")
                    && line != QLatin1String("# Idx          #   C/G     %     %     %     %   name\n")) {
                    // header format doesn't match what we expected, bail before we send any garbage
                    m_process->terminate();
                }
                continue;
            }
            const auto parts = QStringView(line).split(QLatin1Char(' '), Qt::SkipEmptyParts);

            // format at time of writing is
            // # gpu        pid  type    sm   mem   enc   dec   command
            if (parts.count() < 5) { // we only access up to the 5th element
                continue;
            }

            long pid = parts[1].toUInt();
            int sm = parts[3].toUInt();
            int mem = parts[4].toUInt();

            KSysGuard::Process *process = getProcess(pid);
            if (!process) {
                continue; // can in race condition etc
            }
            m_usage->setData(process, sm);
            m_memory->setData(process, mem);
        }
    });
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "nvidia.json", registerPlugin<NvidiaPlugin>();)

#include "nvidia.moc"
