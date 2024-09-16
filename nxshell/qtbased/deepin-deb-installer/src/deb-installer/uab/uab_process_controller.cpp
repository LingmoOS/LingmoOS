// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_process_controller.h"

#include <QRegularExpression>
#include <QProcess>
#include <QDebug>

namespace Uab {

const QString kLinglongBin = "ll-cli";
const QString kLinglongUninstall = "uninstall";

enum UabExitCode {
    InstallError = -1,
    InstallSuccess = 0,
};

UabProcessController::UabProcessController(QObject *parent)
    : QObject { parent }
{
}

UabProcessController::ProcFlags UabProcessController::procFlag() const
{
    return m_procFlag;
}

bool UabProcessController::isRunning() const
{
    return m_procFlag & (Uninstalling | Installing | Upgrading);
}

bool Uab::UabProcessController::install(const UabPkgInfo::Ptr &installPtr)
{
    return commitChange(installPtr, {});
}

bool Uab::UabProcessController::uninstall(const UabPkgInfo::Ptr &uninstallPtr)
{
    return commitChange({}, uninstallPtr);
}

bool Uab::UabProcessController::upgradge(const UabPkgInfo::Ptr &installPtr, const UabPkgInfo::Ptr &uninstallPtr)
{
    return commitChange(installPtr, uninstallPtr);
}

bool Uab::UabProcessController::ensureProcess()
{
    if (!m_process) {
        m_process = new QProcess(this);

        connect(m_process, &QProcess::readyRead, this, &UabProcessController::onReadOutput);
        connect(
            m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UabProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qWarning() << "Unable to restart uab process is still running";
        return false;
    }

    return true;
}

void UabProcessController::onReadOutput()
{
    QByteArray output = m_process->readAllStandardOutput();
    Q_EMIT processOutput(output);

    // Detect progress change, filter ANSI code, receive data such as:
    // "\r\x1B[K\x1B[?25l0% prepare for installing uab\x1B[?25h"
    // "\r\x1B[K\x1B[?25l100% install uab successfully\x1B[?25h\n"
    //
    // pattern: ^\r?(\x1B\[\W?\d*\w)+(\d+)%
    static const QRegularExpression kRegExp("^\\r?(\\x1B\\[\\W?\\d*\\w)+(\\d+)%");
    QRegularExpressionMatch match = kRegExp.match(QString(output), 0, QRegularExpression::PartialPreferFirstMatch);
    if (match.hasMatch()) {
        // get (\\d+) capture
        float progress = match.captured(match.lastCapturedIndex()).toFloat();
        if (m_procFlag.testFlag(Upgrading)) {
            progress = m_procFlag.testFlag(Uninstalling) ? progress / 2 : (100 + progress) / 2;
        }

        Q_EMIT progressChanged(progress);
    }
}

void UabProcessController::onFinished(int exitCode, int exitStatus)
{
    Q_UNUSED(exitStatus)

    const bool exitSuccess = InstallSuccess == exitCode;

    // Upgrade: continue unisntall previous package.
    if (m_procFlag.testFlag(Upgrading) && m_procFlag.testFlag(Installing) && exitSuccess) {
        m_procFlag.setFlag(Installing, false);
        uninstallImpl();
        return;
    }

    m_procFlag = exitSuccess ? Finish : Error;
    Q_EMIT processFinished(exitSuccess);
}

bool UabProcessController::commitChange(const UabPkgInfo::Ptr &installPtr, const UabPkgInfo::Ptr &uninstallPtr)
{
    if (isRunning() || !ensureProcess()) {
        return false;
    }

    m_installPkg = installPtr;
    m_uninstallPkg = uninstallPtr;

    if (m_installPkg && m_uninstallPkg) {
        upgradgeImpl();
    } else if (m_installPkg) {
        installImpl();
    } else if (m_uninstallPkg) {
        uninstallImpl();
    } else {
        return false;
    }

    Q_EMIT processStart();

    return true;
}

void Uab::UabProcessController::installImpl()
{
    m_procFlag.setFlag(Installing);

    m_process->setProgram(m_installPkg->filePath);
    m_process->start();
}

void Uab::UabProcessController::uninstallImpl()
{
    m_procFlag.setFlag(Uninstalling);

    // e.g.: ll-cli uninstall org.deepin.package/1.0.0
    m_process->setProgram(kLinglongBin);
    m_process->setArguments({ kLinglongUninstall, QString("%1/%2").arg(m_uninstallPkg->id).arg(m_uninstallPkg->version) });
    m_process->start();
}

void Uab::UabProcessController::upgradgeImpl()
{
    m_procFlag.setFlag(Upgrading);

    // Linglong support diff version packge same time.
    installImpl();
}

}  // namespace Uab
