// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LingmoDebInstallerLib.h"
#include "manager/PackagesManager.h"

LingmoDebInstallerLib::LingmoDebInstallerLib()
    : m_pPackageManager(new PackagesManager())

{
    initConnections();
}

void LingmoDebInstallerLib::addPackages(const QStringList &debFilePath)
{
    m_pPackageManager->appendPackages(debFilePath);
}

void LingmoDebInstallerLib::deletePackage(int index)
{
    m_pPackageManager->removePackage(index);
}

int LingmoDebInstallerLib::checkInstallStatus(int index)
{
    qDebug() << "check install status"
             << "LingmoDebInstallerLib" << m_pPackageManager;
    return m_pPackageManager->checkInstallStatus(index);
}
bool LingmoDebInstallerLib::checkPackageFile(int index)
{
    return m_pPackageManager->checkPackageValid(index);
}

bool LingmoDebInstallerLib::checkDigitalSignature(int index)
{
    return m_pPackageManager->checkPackageSignture(index);
}

bool LingmoDebInstallerLib::checkPkgDependsStatus(int index)
{
    return m_pPackageManager->checkPackageDependsStatus(index);
}

void LingmoDebInstallerLib::install()
{
    m_pPackageManager->install();
}

void LingmoDebInstallerLib::uninstall(int index)
{
    m_pPackageManager->uninstall(index);
}

void LingmoDebInstallerLib::initConnections()
{
    connect(m_pPackageManager, &PackagesManager::signal_startInstallPackages, this, &LingmoDebInstallerLib::signal_startInstall);

    connect(m_pPackageManager, &PackagesManager::signal_installProgress, this, &LingmoDebInstallerLib::signal_installProcess);

    connect(m_pPackageManager, &PackagesManager::signal_installDetailStatus, this, &LingmoDebInstallerLib::signal_installDetails);

    connect(m_pPackageManager, &PackagesManager::signal_installFinished, this, &LingmoDebInstallerLib::signal_installFinished);

    connect(
        m_pPackageManager, &PackagesManager::signal_uninstallFinished, this, &LingmoDebInstallerLib::signal_uninstallFinished);

    connect(m_pPackageManager,
            &PackagesManager::signal_installErrorOccured,
            this,
            &LingmoDebInstallerLib::signal_installFailedReason);

    connect(m_pPackageManager, &PackagesManager::signal_invalidIndex, this, &LingmoDebInstallerLib::signal_invalidIndex);

    connect(m_pPackageManager, &PackagesManager::signal_packageInvalid, this, &LingmoDebInstallerLib::signal_invalidPackage);

    connect(m_pPackageManager, &PackagesManager::signal_signatureError, this, &LingmoDebInstallerLib::signal_signtureError);

    connect(
        m_pPackageManager, &PackagesManager::signal_dependStatusError, this, &LingmoDebInstallerLib::signal_dependStatusError);

    connect(m_pPackageManager,
            &PackagesManager::signal_packageAlreadyExits,
            this,
            &LingmoDebInstallerLib::signal_packageAlreadyExits);

    connect(
        m_pPackageManager, &PackagesManager::signal_addPackageSuccess, this, &LingmoDebInstallerLib::signal_appendPackageSuccess);

    connect(m_pPackageManager,
            &PackagesManager::signal_removePackageSuccess,
            this,
            &LingmoDebInstallerLib::signal_removePackageSuccess);

    connect(m_pPackageManager,
            &PackagesManager::signal_packageNotInstalled,
            this,
            &LingmoDebInstallerLib::signal_packageNotInstalled);
}

LingmoDebInstallerLib::~LingmoDebInstallerLib()
{
    delete m_pPackageManager;
    this->deleteLater();
}
