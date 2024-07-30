// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

#include "Patient.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

#include <KApplicationTrader>
#include <KLocalizedString>
#include <KShell>
#include <KTerminalLauncherJob>

#include "../coredump/coredump.h"

using namespace Qt::StringLiterals;

Patient::Patient(const Coredump &dump)
    : m_origCoreFilename(QString::fromUtf8(dump.m_rawData.value("COREDUMP_FILENAME")))
    , m_coreFileInfo(m_origCoreFilename)
    , m_signal(dump.m_rawData["COREDUMP_SIGNAL"].toInt())
    , m_appName(QFileInfo(dump.exe).fileName())
    , m_pid(dump.pid)
    , m_timestamp(dump.m_rawData["COREDUMP_TIMESTAMP"].toLong())
    , m_coredumpExe(dump.m_rawData["COREDUMP_EXE"])
    , m_coredumpCom(dump.m_rawData["COREDUMP_COMM"])
{
}

QStringList Patient::coredumpctlArguments(const QString &command) const
{
    return {command, u"COREDUMP_FILENAME=%1"_s.arg(m_origCoreFilename)};
}

void Patient::launchDebugger()
{
    const QString arguments = KShell::joinArgs({u"gdb"_s, u"--core=%1"_s.arg(m_coreFileInfo.filePath()), QString::fromUtf8(m_coredumpExe)});
    auto job = new KTerminalLauncherJob(arguments);
    connect(job, &KJob::result, this, [job] {
        job->deleteLater();
        if (job->error() != KJob::NoError) {
            qWarning() << job->errorText();
        }
    });
    job->start();
}

void Patient::debug()
{
    if (!m_excavator) {
        m_excavator = std::make_unique<AutomaticCoredumpExcavator>();
        connect(m_excavator.get(), &AutomaticCoredumpExcavator::failed, this, [this] {
            QMessageBox::critical(qApp ? qApp->activeWindow() : nullptr,
                                  i18nc("@title", "Failure"),
                                  i18nc("@info", "Failed to access crash data for unknown reasons."));
            m_excavator.release()->deleteLater();
        });
        connect(m_excavator.get(), &AutomaticCoredumpExcavator::excavated, this, [this](const QString &corePath) {
            m_coreFileInfo = QFileInfo(corePath);
            launchDebugger();
        });
        m_excavator->excavateFrom(m_coreFileInfo.filePath());
        return;
    }
    if (m_coreFileInfo.isReadable()) {
        launchDebugger();
    }
    // else supposedly still waiting for excavator
}

QString Patient::dateTime() const
{
    QDateTime time;
    time.setMSecsSinceEpoch(m_timestamp / 1000);
    return QLocale().toString(time, QLocale::LongFormat);
}

QString Patient::iconName() const
{
    // Caching it because it's an N² look-up and there generally are tons of duplicates
    static QHash<QString, QString> s_cache;
    const QString executable = m_appName;
    auto it = s_cache.find(executable);
    if (it == s_cache.end()) {
        const auto servicesFound = KApplicationTrader::query([&executable](const KService::Ptr &service) {
            return QFileInfo(service->exec()).fileName() == executable;
        });

        QString iconName;
        if (servicesFound.isEmpty()) {
            iconName = QStringLiteral("applications-science");
        } else {
            iconName = servicesFound.constFirst()->icon();
        }
        it = s_cache.insert(executable, iconName);
    }
    return *it;
}

bool Patient::canDebug()
{
    return m_coreFileInfo.exists();
}

#include "moc_Patient.cpp"
