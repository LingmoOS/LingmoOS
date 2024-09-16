// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeepinDebInstallerLib.h"
#include "manager/PackagesManager.h"

DeepinDebInstallerLib::DeepinDebInstallerLib()
    : m_pPackageManager(new PackagesManager())

{
    initConnections();
}

void DeepinDebInstallerLib::addPackages(const QStringList &debFilePath)
{
    m_pPackageManager->appendPackages(debFilePath);
}

void DeepinDebInstallerLib::deletePackage(int index)
{
    m_pPackageManager->removePackage(index);
}

int DeepinDebInstallerLib::checkInstallStatus(int index)
{
    qDebug() << "check install status"
             << "DeepinDebInstallerLib" << m_pPackageManager;
    return m_pPackageManager->checkInstallStatus(index);
}
bool DeepinDebInstallerLib::checkPackageFile(int index)
{
    return m_pPackageManager->checkPackageValid(index);
}

bool DeepinDebInstallerLib::checkDigitalSignature(int index)
{
    return m_pPackageManager->checkPackageSignture(index);
}

bool DeepinDebInstallerLib::checkPkgDependsStatus(int index)
{
    return m_pPackageManager->checkPackageDependsStatus(index);
}

void DeepinDebInstallerLib::install()
{
    m_pPackageManager->install();
}

void DeepinDebInstallerLib::uninstall(int index)
{
    m_pPackageManager->uninstall(index);
}

void DeepinDebInstallerLib::initConnections()
{
    connect(m_pPackageManager, &PackagesManager::signal_startInstallPackages, this, &DeepinDebInstallerLib::signal_startInstall);

    connect(m_pPackageManager, &PackagesManager::signal_installProgress, this, &DeepinDebInstallerLib::signal_installProcess);

    connect(m_pPackageManager, &PackagesManager::signal_installDetailStatus, this, &DeepinDebInstallerLib::signal_installDetails);

    connect(m_pPackageManager, &PackagesManager::signal_installFinished, this, &DeepinDebInstallerLib::signal_installFinished);

    connect(
        m_pPackageManager, &PackagesManager::signal_uninstallFinished, this, &DeepinDebInstallerLib::signal_uninstallFinished);

    connect(m_pPackageManager,
            &PackagesManager::signal_installErrorOccured,
            this,
            &DeepinDebInstallerLib::signal_installFailedReason);

    connect(m_pPackageManager, &PackagesManager::signal_invalidIndex, this, &DeepinDebInstallerLib::signal_invalidIndex);

    connect(m_pPackageManager, &PackagesManager::signal_packageInvalid, this, &DeepinDebInstallerLib::signal_invalidPackage);

    connect(m_pPackageManager, &PackagesManager::signal_signatureError, this, &DeepinDebInstallerLib::signal_signtureError);

    connect(
        m_pPackageManager, &PackagesManager::signal_dependStatusError, this, &DeepinDebInstallerLib::signal_dependStatusError);

    connect(m_pPackageManager,
            &PackagesManager::signal_packageAlreadyExits,
            this,
            &DeepinDebInstallerLib::signal_packageAlreadyExits);

    connect(
        m_pPackageManager, &PackagesManager::signal_addPackageSuccess, this, &DeepinDebInstallerLib::signal_appendPackageSuccess);

    connect(m_pPackageManager,
            &PackagesManager::signal_removePackageSuccess,
            this,
            &DeepinDebInstallerLib::signal_removePackageSuccess);

    connect(m_pPackageManager,
            &PackagesManager::signal_packageNotInstalled,
            this,
            &DeepinDebInstallerLib::signal_packageNotInstalled);
}

DeepinDebInstallerLib::~DeepinDebInstallerLib()
{
    delete m_pPackageManager;
    this->deleteLater();
}
