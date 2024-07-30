// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "DetailsLoader.h"

#include <KLocalizedString>

void DetailsLoader::setPatient(Patient *patient)
{
    m_patient = patient;
    if (m_patient) {
        load();
    } else {
        m_LoaderProcess = nullptr;
    }
}

void DetailsLoader::load()
{
    m_LoaderProcess = std::make_unique<QProcess>();
    m_LoaderProcess->setProgram(QStringLiteral("coredumpctl"));
    m_LoaderProcess->setArguments(m_patient->coredumpctlArguments(QStringLiteral("info")));
    m_LoaderProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_LoaderProcess.get(), &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        switch (exitStatus) {
        case QProcess::NormalExit:
            if (exitCode == 0) {
                Q_EMIT details(QString::fromLocal8Bit(m_LoaderProcess->readAll()));
            } else {
                Q_EMIT error(i18nc("@info", "Subprocess exited with error: %1", QString::fromLocal8Bit(m_LoaderProcess->readAll())));
            }
            break;
        case QProcess::CrashExit:
            Q_EMIT error(i18nc("@info", "Subprocess crashed. Check your installation."));
            break;
        }
        m_LoaderProcess.release()->deleteLater();
    });
    m_LoaderProcess->start();
}

#include "moc_DetailsLoader.cpp"
