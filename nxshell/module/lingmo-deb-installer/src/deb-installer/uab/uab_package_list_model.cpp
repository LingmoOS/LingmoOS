// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package_list_model.h"

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDebug>

#include "uab_backend.h"
#include "utils/utils.h"

namespace Uab {

static const float kCompleteProgress = 100.0;
static const int kPkgInitedIndex = -1;

/**
 * @class UabPackageListModel
 * @brief Manage the list model of UAB packages, and handle the install and uninstall of multiple UAB packages.
 *        Emit execution status and error messages.
 */
UabPackageListModel::UabPackageListModel(QObject *parent)
    : AbstractPackageListModel{parent}
    , m_processor(new UabProcessController(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
{
    m_supportPackageType = Pkg::Uab;

    connect(m_processor, &UabProcessController::processOutput, this, &UabPackageListModel::signalAppendOutputInfo);
    connect(m_processor, &UabProcessController::progressChanged, this, &UabPackageListModel::slotBackendProgressChanged);
    connect(m_processor, &UabProcessController::processFinished, this, &UabPackageListModel::slotBackendProcessFinished);

    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &UabPackageListModel::slotFileChanged);

    // call init uab backend while uab package list model create
    connect(Uab::UabBackend::instance(), &Uab::UabBackend::backendInitFinsihed, this, [this]() {
        if (!m_delayAppendPackages.isEmpty()) {
            slotAppendPackage(m_delayAppendPackages);
        }
        m_delayAppendPackages.clear();
    });
    Uab::UabBackend::instance()->initBackend();
}

QVariant UabPackageListModel::data(const QModelIndex &index, int role) const
{
    if (!QAbstractListModel::checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const UabPackage::Ptr &uabPtr = m_uabPkgList.at(index.row());
    if (!uabPtr || !uabPtr->isValid()) {
        return {};
    }

    switch (role) {
        case PackageNameRole:
            return uabPtr->info()->appName;
        case PackagePathRole:
            return uabPtr->info()->filePath;
        case PackageVersionRole:
            return uabPtr->info()->version;
        case PackageInstalledVersionRole:
            return uabPtr->installedVersion();
        case PackageShortDescriptionRole:
            Q_FALLTHROUGH();
        case PackageLongDescriptionRole:
            return uabPtr->info()->description;
        case PackageVersionStatusRole:
            return uabPtr->installStatus();
        case PackageDependsStatusRole:
            return uabPtr->dependsStatus();
        case PackageFailReasonRole:
            return uabPtr->failedReason();
        case PackageOperateStatusRole:
            return uabPtr->operationStatus();
        case PackageTypeRole:
            return Pkg::Uab;

        case Qt::SizeHintRole:
            return QSize(0, 48);
        default:
            break;
    }

    return {};
}

int UabPackageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_uabPkgList.size();
}

void UabPackageListModel::slotAppendPackage(const QStringList &packageList)
{
    if (!isWorkerPrepare()) {
        return;
    }

    // delay append when uab backend not ready. sa backendInitFinsihed()
    if (!Uab::UabBackend::instance()->backendInited()) {
        m_delayAppendPackages.append(packageList);
        return;
    }

    const int oldRowCount = rowCount();
    for (const QString &path : packageList) {
        auto uabPtr = preCheckPackage(path);

        if (uabPtr && uabPtr->isValid()) {
            m_uabPkgList.append(uabPtr);

            m_fileWatcher->addPath(path);
        }
    }

    if (oldRowCount != rowCount()) {
        Q_EMIT signalPackageCountChanged(rowCount());
    }
}

void UabPackageListModel::removePackage(int index)
{
    if (!isWorkerPrepare()) {
        return;
    }

    if (0 <= index && index < rowCount()) {
        auto uabPtr = m_uabPkgList.takeAt(index);
        if (uabPtr && uabPtr->info()) {
            m_fileWatcher->removePath(uabPtr->info()->filePath);
        }

        Q_EMIT signalPackageCountChanged(rowCount());
    }
}

QString UabPackageListModel::checkPackageValid(const QString &packagePath)
{
    if (Pkg::PkgReadable != Utils::checkPackageReadable(packagePath)) {
        return "You can only install local ueb packages";
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr->isValid()) {
        return "The uab package may be broken";
    }

    if (packageExists(uabPtr)) {
        return "The uab package already aoceand";
    }

    return "";
}

Pkg::PackageInstallStatus UabPackageListModel::checkInstallStatus(const QString &packagePath)
{
    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        return Pkg::NotInstalled;
    }

    return uabPtr->installStatus();
}

Pkg::DependsStatus UabPackageListModel::checkDependsStatus(const QString &packagePath)
{
    if (!Uab::UabBackend::instance()->linglongExists()) {
        return Pkg::DependsBreak;
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        return Pkg::DependsBreak;
    }
    return Pkg::DependsOk;
}

QStringList UabPackageListModel::getPackageInfo(const QString &packagePath)
{
    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        return {};
    }

    // adapt deb package info interface
    auto infoPtr = uabPtr->info();
    QStringList infoList;
    infoList << infoPtr->appName;
    infoList << infoPtr->filePath;
    infoList << infoPtr->version;
    infoList << infoPtr->architecture;
    infoList << infoPtr->description;
    infoList << infoPtr->description;

    return infoList;
}

QString UabPackageListModel::lastProcessError()
{
    auto findItr = std::find_if(m_uabPkgList.rbegin(), m_uabPkgList.rend(), [](const UabPackage::Ptr &uabPtr) {
        return Pkg::Failed == uabPtr->operationStatus();
    });

    if (findItr != m_uabPkgList.rend()) {
        return (*findItr)->processError();
    }
    return {};
}

bool UabPackageListModel::containsSignatureFailed() const
{
    // check if a pacakge signature verify failed.
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [](const UabPackage::Ptr &uabPtr) {
        return Pkg::DigitalSignatureError == uabPtr->m_errorCode;
    });

    return findItr != m_uabPkgList.end();
}

bool UabPackageListModel::slotInstallPackages()
{
    if (!isWorkerPrepare() || rowCount() <= 0) {
        return false;
    }

    if (!linglongExists()) {
        return false;
    }

    // reset, mark package waiting install
    resetInstallStatus();
    for (auto uabPtr : m_uabPkgList) {
        uabPtr->m_operationStatus = Pkg::Waiting;
        Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});
    }

    setWorkerStatus(WorkerProcessing);
    return installNextUab();
}

bool UabPackageListModel::slotUninstallPackage(const int i)
{
    bool callRet = false;

    do {
        if (!isWorkerPrepare() || rowCount() <= 0) {
            break;
        }

        if (!linglongExists()) {
            break;
        }

        if (!checkIndex(index(i))) {
            break;
        }

        // only single page show uninstall flow
        m_operatingIndex = i;
        setWorkerStatus(WorkerUnInstall);

        auto uabPtr = m_uabPkgList.value(m_operatingIndex);
        if (!uabPtr || !uabPtr->isValid()) {
            break;
        }
        auto removeInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
        if (!removeInfoPtr) {
            break;
        }

        setCurrentOperation(Pkg::Waiting);

        m_processor->reset();
        m_processor->markUninstall(Uab::UabPackage::fromInfo(removeInfoPtr));
        callRet = m_processor->commitChanges();
    } while (false);

    if (!callRet) {
        setCurrentOperation(Pkg::Failed);
        setWorkerStatus(WorkerFinished);
        return false;
    }

    return true;
}

void UabPackageListModel::reset()
{
    setWorkerStatus(WorkerPrepare);
    m_operatingIndex = kPkgInitedIndex;

    QStringList files = m_fileWatcher->files();
    if (!files.isEmpty()) {
        m_fileWatcher->removePaths(files);
    }

    m_uabPkgList.clear();
}

void UabPackageListModel::resetInstallStatus()
{
    setWorkerStatus(WorkerPrepare);
    m_operatingIndex = kPkgInitedIndex;

    // reset deb file
    for (auto &ptr : m_uabPkgList) {
        ptr->reset();
    }
    Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});
}

bool UabPackageListModel::installNextUab()
{
    m_operatingIndex++;
    Q_ASSERT_X(m_operatingIndex >= 0, "install uab", "operating index invalid");
    if (m_operatingIndex < 0) {
        setWorkerStatus(WorkerFinished);
        return false;
    }

    // check install finish
    if (m_operatingIndex >= rowCount()) {
        Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(kCompleteProgress));
        Q_EMIT signalWholeProgressChanged(static_cast<int>(kCompleteProgress));

        setWorkerStatus(WorkerFinished);
        return true;
    }

    // notify list view scroll to current package
    Q_EMIT signalCurrentProcessPackageIndex(m_operatingIndex);

    auto uabPtr = m_uabPkgList.value(m_operatingIndex);
    if (!uabPtr || Pkg::DependsOk != uabPtr->m_dependsStatus) {
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return false;
    }

    setCurrentOperation(Pkg::Operating);

    m_processor->reset();

    // Note: Current Linglong environment supports multi version package same time,
    //       check if install same version package.
    Pkg::PackageInstallStatus installStatus = uabPtr->installStatus();
    if (Pkg::InstalledLaterVersion == installStatus) {
        if (auto sameInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id, uabPtr->info()->version)) {
            const int ret = Utils::compareVersion(uabPtr->info()->version, sameInfoPtr->version);

            if (ret == 0) {
                installStatus = Pkg::InstalledSameVersion;
            } else if (ret < 0) {
                installStatus = Pkg::InstalledLaterVersion;
            } else {
                installStatus = Pkg::InstalledEarlierVersion;
            }
        }
    }

    switch (installStatus) {
        case Pkg::NotInstalled:
            m_processor->markInstall(uabPtr);
            break;
        case Pkg::InstalledSameVersion: {
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id, uabPtr->info()->version);
            m_processor->markUninstall(Uab::UabPackage::fromInfo(oldInfoPtr));
            m_processor->markInstall(uabPtr);
        } break;
        default: {
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
            m_processor->markInstall(uabPtr);
            m_processor->markUninstall(Uab::UabPackage::fromInfo(oldInfoPtr));
        } break;
    }

    if (!m_processor->commitChanges()) {
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return false;
    }

    return true;
}

void UabPackageListModel::slotBackendProgressChanged(float progress)
{
    Q_ASSERT_X(rowCount() > 0, "check count", "row count invalid");

    const float base = kCompleteProgress / rowCount();
    const float wholeProgress = (m_operatingIndex + (progress / kCompleteProgress)) * base;

    Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(progress));
    Q_EMIT signalWholeProgressChanged(static_cast<int>(wholeProgress));
}

void UabPackageListModel::slotBackendProcessFinished(bool success)
{
    Q_ASSERT_X(rowCount() > 0, "check count", "row count invalid");

    // update installed status
    setCurrentOperation(success ? Pkg::Success : Pkg::Failed);
    // update all data
    Q_EMIT dataChanged(index(m_operatingIndex), index(m_operatingIndex));

    // update progress
    const float base = kCompleteProgress / rowCount();
    Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(kCompleteProgress));
    Q_EMIT signalWholeProgressChanged(static_cast<int>(base * (m_operatingIndex + 1)));

    switch (m_workerStatus) {
        case WorkerProcessing:
            installNextUab();
            break;
        case WorkerUnInstall:
            setWorkerStatus(WorkerFinished);
            break;
        default:
            break;
    }
}

void UabPackageListModel::setCurrentOperation(Pkg::PackageOperationStatus s)
{
    if (!checkIndexValid(m_operatingIndex)) {
        return;
    }

    auto &uabPtr = m_uabPkgList[m_operatingIndex];
    uabPtr->m_operationStatus = s;

    // mark error info
    if (Pkg::Failed == s) {
        uabPtr->setProcessError(Pkg::UnknownError, tr("Installation Failed"));
    }

    Q_EMIT dataChanged(index(m_operatingIndex), index(m_operatingIndex), {PackageOperateStatusRole});
}

bool UabPackageListModel::checkIndexValid(int index) const
{
    return 0 <= index && index < rowCount();
}

UabPackage::Ptr UabPackageListModel::preCheckPackage(const QString &packagePath)
{
    auto readablilty = Utils::checkPackageReadable(packagePath);
    switch (readablilty) {
        case Pkg::PkgNotInLocal:
            Q_EMIT signalAppendFailMessage(Pkg::PackageNotLocal, Pkg::Uab);
            return {};
        case Pkg::PkgNoPermission:
            Q_EMIT signalAppendFailMessage(Pkg::PackageNotInstallable, Pkg::Uab);
            return {};
        default:
            break;
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        Q_EMIT signalAppendFailMessage(Pkg::PackageInvalid, Pkg::Uab);
        return {};
    }

    if (packageExists(uabPtr)) {
        Q_EMIT signalAppendFailMessage(Pkg::PackageAlreadyExists, Pkg::Uab);
        return {};
    }

    return uabPtr;
}

bool UabPackageListModel::packageExists(const UabPackage::Ptr &uabPtr) const
{
    if (!uabPtr) {
        return false;
    }

    // check already aoceand
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [&uabPtr](const UabPackage::Ptr &cmpUabPtr) {
        if (uabPtr->info()->filePath == cmpUabPtr->info()->filePath) {
            return true;
        }

        if (uabPtr->info()->appName == cmpUabPtr->info()->appName) {
            return true;
        }

        return false;
    });

    return findItr != m_uabPkgList.end();
}

bool UabPackageListModel::linglongExists()
{
    // check if linglong execuatable exits
    if (!Uab::UabBackend::instance()->recheckLinglongExists()) {
        for (const auto &uabPtr : m_uabPkgList) {
            uabPtr->setDependsStatus(Pkg::DependsBreak);
        }

        Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageDependsStatusRole});
        return false;
    }

    return true;
}

void UabPackageListModel::slotFileChanged(const QString &filePath)
{
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [&](const UabPackage::Ptr &uabPtr) {
        if (uabPtr && uabPtr->isValid()) {
            return uabPtr->info()->filePath == filePath;
        }
        return false;
    });

    if (findItr != m_uabPkgList.end()) {
        const QFileInfo info((*findItr)->info()->filePath);
        if (!info.exists()) {
            (*findItr)->markNotExists();
            removePackage(static_cast<int>(std::distance(findItr, m_uabPkgList.begin())));
            m_fileWatcher->removePath(filePath);

            Q_EMIT signalPackageCannotFind(info.fileName());
        }
    }
}

}  // namespace Uab
