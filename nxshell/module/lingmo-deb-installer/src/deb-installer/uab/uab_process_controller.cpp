// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_process_controller.h"

#include <QRegularExpression>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include "uab_backend.h"
#include "process/Pty.h"

namespace Uab {

// linglong cli command
const QString kLinglongBin = "ll-cli";
const QString kLinglongJson = "--json";
const QString kLinglongInstall = "install";
const QString kLinglongUninstall = "uninstall";

// ll-cli output json tag
const QString kJsonPercentage = "percentage";
const QString kJsonState = "state";
const QString kJsonCode = "code";
const QString kJsonMessage = "messsage";

// transport install/uninstall task to higher level permission process.
static const QString kPkexecBin = "pkexec";
static const QString kInstallProcessorBin = "lingmo-deb-installer-dependsInstall";
static const QString kParamUab = "--uab";
static const QString kParamInstall = "--install";
static const QString kParamRemove = "--remove";

const int kInitedIndex = -1;

/**
 * @class UabProcessController
 * @brief Uab process controller. It manages the process of installing/uninstalling uab packages.
 *        Its uses `lingmo-deb-installer-dependsInstall` command to install/uninstall uab packages
 *        with higher level permission.
 */
UabProcessController::UabProcessController(QObject *parent)
    : QObject{parent}
{
    setProcessType(BackendCli);
}

void UabProcessController::setProcessType(ProcessType type)
{
    if (type == m_type) {
        return;
    }
    m_type = type;
}

UabProcessController::ProcessType UabProcessController::processType() const
{
    return m_type;
}

UabProcessController::ProcFlags UabProcessController::procFlag() const
{
    return m_procFlag;
}

bool UabProcessController::isRunning() const
{
    if (m_procFlag & (Uninstalling | Installing | Processing)) {
        return true;
    }

    if (m_process && QProcess::NotRunning != m_process->state()) {
        return true;
    }

    return false;
}

bool UabProcessController::reset()
{
    if (isRunning() || !ensureProcess()) {
        return false;
    }

    m_currentIndex = kInitedIndex;
    m_procList.clear();

    return true;
}

bool Uab::UabProcessController::markInstall(const UabPackage::Ptr &installPtr)
{
    if (isRunning() || !ensureProcess() || !installPtr || !installPtr->isValid()) {
        return false;
    }

    m_procList.append(qMakePair(Installing, installPtr));
    return true;
}

bool Uab::UabProcessController::markUninstall(const UabPackage::Ptr &uninstallPtr)
{
    if (isRunning() || !ensureProcess() || !uninstallPtr || !uninstallPtr->isValid()) {
        return false;
    }

    m_procList.append(qMakePair(Uninstalling, uninstallPtr));
    return true;
}

bool UabProcessController::commitChanges()
{
    if (isRunning() || !ensureProcess() || m_procList.isEmpty()) {
        return false;
    }

    m_procFlag = Processing;
    m_currentIndex = kInitedIndex;
    if (!nextProcess()) {
        m_procFlag = Error;
        return false;
    }

    Q_EMIT processStart();
    return true;
}

bool Uab::UabProcessController::ensureProcess()
{
    if (!m_process) {
        m_process = new Konsole::Pty(this);

        connect(m_process, &Konsole::Pty::receivedData, this, &UabProcessController::onReadOutput);
        connect(
            m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UabProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qWarning() << "Unable to restart uab process is still running";
        return false;
    }

    return true;
}

/**
   @brief Parse json data from `ll-cli --json install/uninstall [path]` output.
    Here are output example (linglong-bin 1.6.2).
   @code
    // progress
    [
        {
            "message": "prepare for installing uab",
            "percentage": "0",
            "state": "preInstall"
        }
    ]

    // successed
    {
        "message": "install uab successfully",
        "percentage": "100",
        "state": "Success"
    }

    // failed
    {"code":-1,"message":"./libs/linglong/src/linglong/cli/cli.cpp:125 download status:
   \n./libs/linglong/src/linglong/repo/ostree_repo.cpp:915 import layer dir: main:org.lingmo.editor/6.5.2.1/x86_64 exists."}

   @endcode
 */
void UabProcessController::parseProgressFromJson(const QByteArray &jsonData)
{
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (doc.isArray()) {
        // maybe percentage info
        const QJsonObject obj = doc.array().first().toObject();
        if (obj.contains(kJsonPercentage)) {
            float progress = obj.value(kJsonPercentage).toDouble();
            updateWholeProgress(progress);
        }

    } else if (doc.isObject()) {
        // maybe percentage / code info
        const QJsonObject obj = doc.object();

        if (obj.contains(kJsonPercentage)) {
            float progress = obj.value(kJsonPercentage).toDouble();
            updateWholeProgress(progress);

        } else if (UabError == obj.value(kJsonCode).toInt()) {
            const QString errorString = obj.value(kJsonMessage).toString();
            auto currUabPtr = currentPackagePtr();
            if (currUabPtr) {
                currUabPtr->setProcessError(Pkg::UnknownError, errorString);
            }

            qWarning() << qPrintable("Uab process error:") << errorString;
        }

        // TODO(renbin): signature verify error, etc.
    }
}

void UabProcessController::parseProgressFromRawOutput(const QByteArray &output)
{
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
        updateWholeProgress(progress);
    }
}

void UabProcessController::updateWholeProgress(float currentTaskProgress)
{
    const int count = m_procList.size();
    if (count > 1) {
        currentTaskProgress = (m_currentIndex * 100.0f + currentTaskProgress) / static_cast<float>(count);
    }

    Q_EMIT progressChanged(currentTaskProgress);
}

void UabProcessController::onReadOutput(const char *buffer, int length, bool isCommandExec)
{
    Q_UNUSED(isCommandExec)
    QByteArray output = QByteArray::fromRawData(buffer, length);
    Q_EMIT processOutput(output);

    // e.g: ll-cli --json install /path/to/file
    parseProgressFromJson(output);
}

void UabProcessController::onFinished(int exitCode, int exitStatus)
{
    Q_UNUSED(exitStatus)

    const bool exitSuccess = UabSuccess == exitCode;

    // continue next process
    if (exitSuccess) {
        // update uab backend
        commitCurrentChangeToBackend();

        // continue next process, if error occurred, terminate current install/uninstall.
        if (nextProcess()) {
            return;
        }
    }

    m_procFlag = Error;
    Q_EMIT processFinished(false);
}

void UabProcessController::onDBusProgressChanged(int progress, const QString &message)
{
    updateWholeProgress(static_cast<float>(progress));
    Q_EMIT processOutput(message);
}

bool UabProcessController::nextProcess()
{
    m_procFlag.setFlag(Installing, false);
    m_procFlag.setFlag(Uninstalling, false);

    // check process finish
    m_currentIndex++;
    if (m_currentIndex >= m_procList.size()) {
        m_procFlag = Finish;
        Q_EMIT processFinished(true);
        return true;
    }

    if (!checkIndexValid()) {
        qWarning() << qPrintable("Invalid process index") << m_currentIndex;
        return false;
    }

    const auto &currentProc = m_procList.at(m_currentIndex);
    switch (currentProc.first) {
        case Installing:
            return (BackendCli == m_type) ? installBackendCliImpl(currentProc.second) : installCliImpl(currentProc.second);
        case Uninstalling:
            return (BackendCli == m_type) ? uninstallBackendCliImpl(currentProc.second) : uninstallCliImpl(currentProc.second);
        default:
            break;
    }

    qWarning() << qPrintable("Invalid process type") << currentProc.first;
    return false;
}

bool UabProcessController::installBackendCliImpl(const UabPackage::Ptr &installPtr)
{
    if (!installPtr || !installPtr->isValid() || installPtr->info()->filePath.isEmpty()) {
        return false;
    }

    m_procFlag.setFlag(Installing);

    // e.g.: pkexec lingmo-deb-installer-dependsInstall --uab --install [file to package].uab
    m_process->start(
        kPkexecBin, {kPkexecBin, kInstallProcessorBin, kParamUab, kParamInstall, installPtr->info()->filePath}, {}, 0, false);

    const QString recordCommand = QString("command: %1 %2 %3 %4/%5[uab package]")
                                      .arg(kInstallProcessorBin)
                                      .arg(kParamUab)
                                      .arg(kParamInstall)
                                      .arg(installPtr->info()->id)
                                      .arg(installPtr->info()->version);
    qInfo() << recordCommand;

    return true;
}

bool UabProcessController::uninstallBackendCliImpl(const UabPackage::Ptr &uninstallPtr)
{
    if (!uninstallPtr || !uninstallPtr->isValid()) {
        return false;
    }

    m_procFlag.setFlag(Uninstalling);

    // e.g.: pkexec lingmo-deb-installer-dependsInstall --uab --remove [id/version]
    const QString mergeInfo = QString("%1/%2").arg(uninstallPtr->info()->id).arg(uninstallPtr->info()->version);
    m_process->start(kPkexecBin, {kPkexecBin, kInstallProcessorBin, kParamUab, kParamRemove, mergeInfo}, {}, 0, false);

    const QString recordCommand =
        QString("command: %1 %2 %3 %4").arg(kInstallProcessorBin).arg(kParamUab).arg(kParamRemove).arg(mergeInfo);
    Q_EMIT processOutput(recordCommand);
    qInfo() << recordCommand;

    return true;
}

bool Uab::UabProcessController::installCliImpl(const Uab::UabPackage::Ptr &installPtr)
{
    if (!installPtr || !installPtr->isValid() || installPtr->info()->filePath.isEmpty()) {
        return false;
    }

    m_procFlag.setFlag(Installing);

    // e.g.: ll-cli --json install ./path/to/file/uab_package.uab
    m_process->start(kLinglongBin, {kLinglongBin, kLinglongJson, kLinglongInstall, installPtr->info()->filePath}, {}, 0, false);

    const QString recordCommand = QString("command: %1 %2 %3 %4/%5[uab package]")
                                      .arg(kLinglongBin)
                                      .arg(kLinglongJson)
                                      .arg(kLinglongInstall)
                                      .arg(installPtr->info()->id)
                                      .arg(installPtr->info()->version);
    Q_EMIT processOutput(recordCommand);
    qInfo() << recordCommand;

    return true;
}

bool Uab::UabProcessController::uninstallCliImpl(const Uab::UabPackage::Ptr &uninstallPtr)
{
    if (!uninstallPtr || !uninstallPtr->isValid()) {
        return false;
    }

    m_procFlag.setFlag(Uninstalling);

    // e.g.: ll-cli --json uninstall org.lingmo.package/1.0.0
    m_process->start(kLinglongBin,
                     {kLinglongBin,
                      kLinglongJson,
                      kLinglongUninstall,
                      QString("%1/%2").arg(uninstallPtr->info()->id).arg(uninstallPtr->info()->version)},
                     {},
                     0,
                     false);

    const QString recordCommand = QString("command: %1 %2 %3 %4/%5")
                                      .arg(kLinglongBin)
                                      .arg(kLinglongJson)
                                      .arg(kLinglongUninstall)
                                      .arg(uninstallPtr->info()->id)
                                      .arg(uninstallPtr->info()->version);
    Q_EMIT processOutput(recordCommand);
    qInfo() << recordCommand;

    return true;
}

bool UabProcessController::checkIndexValid()
{
    return 0 <= m_currentIndex && m_currentIndex < m_procList.size();
}

UabPackage::Ptr UabProcessController::currentPackagePtr()
{
    if (!checkIndexValid()) {
        return {};
    }

    return m_procList[m_currentIndex].second;
}

void UabProcessController::commitCurrentChangeToBackend()
{
    if (!checkIndexValid()) {
        return;
    }

    const auto &currentProc = m_procList.at(m_currentIndex);
    UabPkgInfo::Ptr infoPtr;
    if (!currentProc.second || !currentProc.second->isValid()) {
        return;
    }
    infoPtr = currentProc.second->info();

    switch (currentProc.first) {
        case Installing:
            Uab::UabBackend::instance()->packageInstalled(infoPtr);
            break;
        case Uninstalling:
            Uab::UabBackend::instance()->packageRemoved(infoPtr);
            break;
        default:
            break;
    }
}

}  // namespace Uab
