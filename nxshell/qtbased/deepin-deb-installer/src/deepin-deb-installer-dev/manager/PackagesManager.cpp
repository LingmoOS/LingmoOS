// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PackagesManager.h"
#include "status/PackageSigntureStatus.h"
#include "status/GetStatusThread.h"
#include "installer/PackageInstaller.h"
#include "package/Package.h"

#include <QFileInfo>

PackagesManager::PackagesManager()
    : m_pPackageStatus(new PackageStatus())
    , m_pPackageInstaller(new PackageInstaller(m_pPackageStatus->m_backendFuture.result()))
{
    m_pGetStatusThread = new GetStatusThread(m_pPackageStatus);

    initConnection();
}

void PackagesManager::initConnection()
{
    connect(m_pPackageInstaller, &PackageInstaller::signal_startInstall, this, &PackagesManager::signal_startInstallPackages);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installProgress, this, &PackagesManager::signal_installProgress);

    connect(
        m_pPackageInstaller, &PackageInstaller::signal_installDetailStatus, this, &PackagesManager::signal_installDetailStatus);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installError, this, &PackagesManager::signal_installErrorOccured);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installFinished, this, &PackagesManager::slot_installFinished);

    connect(m_pPackageInstaller, &PackageInstaller::signal_uninstallFinished, this, &PackagesManager::slot_uninstallFinished);

    connect(m_pGetStatusThread, &GetStatusThread::signal_dependsStatus, this, &PackagesManager::slot_getDependsStatus);

    connect(m_pGetStatusThread, &GetStatusThread::signal_installStatus, this, &PackagesManager::slot_getInstallStatus);
}

void PackagesManager::slot_getDependsStatus(int index, DependsStatus dependsStatus)
{
    Package *pkg = searchByIndex(index);
    if (pkg) {
        pkg->setPackageDependStatus(dependsStatus);

        // 根据获取的依赖结果，发送信号
        switch (dependsStatus) {
            case DependsOk:
            case DependsAvailable:  // 依赖满足，不发送信号
                break;
            case DependsBreak:  // 依赖不满足
                emit signal_dependStatusError(index, DependsBreak);
                break;
            case DependsAuthCancel:  // 依赖下载授权被取消
                emit signal_dependStatusError(index, DependsAuthCancel);
                break;
            case DependsUnknown:  // 依赖未知（下载依赖失败）
                emit signal_dependStatusError(index, DependsUnknown);
                break;
            case ArchBreak:  // 依赖架构错误
                emit signal_dependStatusError(index, ArchBreak);
                break;
        }
    } else {
        // 未获取到 当前包的下标
        qWarning() << "[PackagesManager]<< slot_getDependsStatus"
                   << "Package not found";
    }
}

void PackagesManager::slot_getInstallStatus(int index, InstallStatus installStatus)
{
    Package *pkg = searchByIndex(index);
    pkg->setPackageInstallStatus(installStatus);
    if (!m_appendFinished) {
        m_appendFinished = true;
    } else {
        emit signal_addPackageSuccess(index);
    }
}

void PackagesManager::appendPackages(QStringList packages)
{
    for (int i = 0; i < packages.size(); i++) {
        getPackageInfo(packages[i], i);
    }
}

void PackagesManager::removePackage(int index)
{
    Package *pkg = searchByIndex(index);
    if (pkg) {
        m_packages.removeOne(pkg);
        m_packagesMd5.remove(pkg->getMd5());
        emit signal_removePackageSuccess(index);
    }
    delete pkg;
}

int PackagesManager::checkInstallStatus(int index)
{
    Package *pkg = searchByIndex(index);
    if (pkg) {
        return pkg->getInstallStatus();
    } else {
        qWarning() << "[PackagesManager]<< checkInstallStatus"
                   << "Package not found";
        return InstallStatusUnknown;
    }
}

bool PackagesManager::checkPackageValid(int index)
{
    Package *package = searchByIndex(index);
    if (package) {
        return package->getValid();
    } else {
        qWarning() << "[PackagesManager]<< checkPackageValid"
                   << "Package not found";
        return false;
    }
}

bool PackagesManager::checkPackageSignture(int index)
{
    Package *package = searchByIndex(index);
    if (package) {
        if (SigntureVerifySuccess == package->getSigntureStatus()) {
            return true;
        } else {
            qWarning() << "[PackagesManager]<< checkPackageSignture"
                       << "package verify signture" << package->getSigntureStatus();
            return false;
        }
    } else {
        qWarning() << "[PackagesManager]<< checkPackageSignture"
                   << "Package not found";
        return false;
    }
}

bool PackagesManager::checkPackageDependsStatus(int index)
{
    Package *package = searchByIndex(index);
    if (package) {
        return (package->getDependStatus() == DependsOk || package->getDependStatus() == DependsAvailable);

    } else {
        qWarning() << "[PackagesManager]<< checkPackageDependsStatus"
                   << "Package not found";
        return false;
    }
}

bool PackagesManager::checkPackageSuffix(QString packagePath)
{
    const QFileInfo info(packagePath);

    if (info.exists() && info.isFile() && info.suffix().toLower() == "deb") {  // 大小写不敏感的判断是否为deb后缀
        return true;
    }
    qWarning() << "[PackagesManager]"
               << "checkPackageSuffix"
               << "Suffix error";
    return false;
}

void PackagesManager::getPackageInfo(QString packagePath, int index)
{
    m_appendFinished = false;
    if (!checkPackageSuffix(packagePath)) {
        emit signal_packageInvalid(index);
        return;
    }
    m_pGetStatusThread->setPackage(index, packagePath);
    m_pGetStatusThread->start();

    Package *packageFile = new Package(index, packagePath);

    if (!packageFile->getValid()) {
        qWarning() << "[PackagesManager]"
                   << "getPackageInfo"
                   << "packageFile->getValid()" << packageFile->getValid();
        emit signal_packageInvalid(index);
        return;
    }

    auto md5 = packageFile->getMd5();
    if (m_packagesMd5.contains(md5)) {
        qWarning() << "[PackagesManager]"
                   << "getPackageInfo"
                   << "md5 already exists";
        emit signal_packageAlreadyExits(index);
        return;
    }

    if (packageFile->getSigntureStatus() != SigntureVerifySuccess) {
        emit signal_signatureError(index, packageFile->getSigntureStatus());
        return;
    }

    m_packagesMd5 << md5;
    m_packages.append(packageFile);

    if (!m_appendFinished) {
        m_appendFinished = true;
    } else {
        emit signal_addPackageSuccess(index);
    }
}

void PackagesManager::install()
{
    if (m_packages.size() > 0) {
        m_pPackageInstaller->appendPackage(m_packages[0]);
        m_pPackageInstaller->installPackage();
    } else {
        qWarning() << "PackagesManager"
                   << "install"
                   << "index invalid";
        emit signal_invalidIndex(0);
    }
}

void PackagesManager::uninstall(int index)
{
    Package *package = searchByIndex(index);
    if (package) {
        if (package->getInstallStatus() <= NotInstalled) {
            emit signal_packageNotInstalled(index);
            return;
        }
        QStringList reverseDepends =
            m_pPackageStatus->getPackageReverseDependsList(package->getName(), package->getArchitecture());

        if (!reverseDepends.isEmpty()) {
            package->setPackageReverseDependsList(reverseDepends);
        }

        m_pPackageInstaller->appendPackage(package);
        m_pPackageInstaller->uninstallPackage();
    } else {
        qWarning() << "PackagesManager"
                   << "uninstall"
                   << "index invalid";
    }
}

Package *PackagesManager::searchByIndex(int index)
{
    auto iter =
        std::find_if(m_packages.begin(), m_packages.end(), [index](const auto &package) { return package->getIndex() == index; });
    if (iter != m_packages.end()) {
        return *iter;
    }
    emit signal_invalidIndex(index);
    qWarning() << "[PackagesManager]<< searchByIndex"
               << "Package not found";
    return nullptr;
}

void PackagesManager::slot_installFinished(QApt::ExitStatus exitStatus)
{
    if (QApt::ExitSuccess == exitStatus) {
        m_packagesMd5.remove(m_packages[0]->getMd5());
        m_packages.removeAt(0);

        if (m_packages.isEmpty()) {
            emit signal_installFinished();
            return;
        }
        QApt::Backend *backend = m_pPackageStatus->m_backendFuture.result();
        backend->reloadCache();
        install();
    }
}

void PackagesManager::slot_uninstallFinished(QApt::ExitStatus exitStatus)
{
    if (QApt::ExitSuccess == exitStatus) {
        m_packagesMd5.remove(m_packages[0]->getMd5());
        m_packages.removeAt(0);
        emit signal_uninstallFinished();
    }
}

PackagesManager::~PackagesManager()
{
    m_packages.clear();
    m_packagesMd5.clear();
    delete m_pPackageStatus;
}
