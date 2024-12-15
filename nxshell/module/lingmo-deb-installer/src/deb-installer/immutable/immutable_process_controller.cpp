// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "immutable_process_controller.h"

#include <QDebug>

#include "process/Pty.h"
#include "utils/hierarchicalverify.h"
#include "utils/package_defines.h"

namespace Immutable {

static const QString kPkexecBin = "pkexec";
static const QString kInstallProcessorBin = "lingmo-deb-installer-dependsInstall";
static const QString kParamImmutable = "--install_immutable";
static const QString kParamInstallConfig = "--install_config";
static const QString kParamInstall = "--install";
static const QString kParamRemove = "--remove";

ImmutableProcessController::ImmutableProcessController(QObject *parent)
    : QObject{parent}
{
}

const Deb::DebPackage::Ptr &ImmutableProcessController::currentPackage() const
{
    return m_currentPackage;
}

bool ImmutableProcessController::isRunning() const
{
    if (m_process && QProcess::NotRunning != m_process->state()) {
        return true;
    }

    return false;
}

bool ImmutableProcessController::install(const Deb::DebPackage::Ptr &package)
{
    if (!package || !package->isValid() || isRunning()) {
        return false;
    }
    if (!ensureProcess()) {
        return false;
    }

    // e.g.: pkexec lingmo-deb-installer-dependsInstall Compatible InstallConfig[Optional]
    //       install --rootfs [rootfs name] [path to deb file]
    m_type = Install;
    m_outputList.clear();
    m_currentPackage = package;

    // reset state
    m_currentPackage->setError(Pkg::NoError, {});

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    QStringList params{kPkexecBin, kInstallProcessorBin, kParamImmutable, kParamInstall, m_currentPackage->filePath()};
    if (m_currentPackage->containsTemplates()) {
        params << kParamInstallConfig;
    }

    m_process->start(kPkexecBin, params, {}, 0, false);

    qInfo() << "Comaptible install:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool ImmutableProcessController::uninstall(const Deb::DebPackage::Ptr &package)
{
    if (!package || !package->isValid() || isRunning()) {
        return false;
    }
    if (!ensureProcess()) {
        return false;
    }

    // e.g.: pkexec lingmo-deb-installer-dependsInstall Compatible
    //       remove --rootfs [rootfs name] [package name]
    m_type = Unintsall;
    m_outputList.clear();
    m_currentPackage = package;

    // reset state
    m_currentPackage->setError(Pkg::NoError, {});

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    m_process->start(
        kPkexecBin,
        {kPkexecBin, kInstallProcessorBin, kParamImmutable, kParamRemove, m_currentPackage->debInfo()->packageName()},
        {},
        0,
        false);

    qInfo() << "Comaptible uninstall:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool ImmutableProcessController::needTemplates() const
{
    // Unisntallation not need config templates.
    return (Unintsall != m_type) && m_currentPackage && m_currentPackage->containsTemplates();
}

void ImmutableProcessController::writeConfigData(const QString &configData)
{
    if (m_process) {
        m_process->pty()->write(configData.toUtf8());
        m_process->pty()->write("\n");
    }
}

bool ImmutableProcessController::ensureProcess()
{
    if (!m_process) {
        m_process = new Konsole::Pty(this);

        connect(m_process, &Konsole::Pty::receivedData, this, &ImmutableProcessController::onReadOutput);
        connect(m_process,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                &ImmutableProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qWarning() << "Unable to restart compatible process is still running";
        return false;
    }

    return true;
}

void ImmutableProcessController::onReadOutput(const char *buffer, int length, bool isCommandExec)
{
    Q_UNUSED(isCommandExec);

    const QByteArray output = QByteArray::fromRawData(buffer, length);
    Q_EMIT processOutput(output);

    // simulate porgress, progress = log2(x) * 10
    m_outputList.append(output);

    int bitMax = 0;
    int logCount = m_outputList.size();
    while (logCount > 0) {
        logCount >>= 1;
        bitMax++;
    }

    float progress = qMin(9, bitMax) * 10;
    Q_EMIT progressChanged(progress);
}

void ImmutableProcessController::onFinished(int exitCode, int exitStatus)
{
    const bool success = (Pkg::ExitNoError == exitCode && QProcess::NormalExit == exitStatus);
    if (!success) {
        switch (exitCode) {
            case Pkg::ExitAuthError:
                m_currentPackage->setError(Pkg::ConfigAuthCancel, {});
                break;
            default: {  // up to 512 outputs
                static const int kMaxCheckLen = 512;
                for (int i = m_outputList.size() - 1; i >= qMax(0, m_outputList.size() - kMaxCheckLen); --i) {
                    if (HierarchicalVerify::instance()->checkTransactionError(m_currentPackage->debInfo()->packageName(),
                                                                              m_outputList.at(i))) {
                        // mark hierachical verify failed
                        m_currentPackage->setError(Pkg::DigitalSignatureError, {});
                    }
                }
            } break;
        }
    }

    Q_EMIT processFinished(success);

    // release temp data
    m_outputList.clear();
}

};  // namespace Immutable
