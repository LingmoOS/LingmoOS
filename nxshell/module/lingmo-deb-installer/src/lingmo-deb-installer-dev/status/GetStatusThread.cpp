// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetStatusThread.h"
#include "package/Package.h"

#include <QMetaType>

#include <QFile>

GetStatusThread::GetStatusThread(PackageStatus *packageStatus)
{
    qRegisterMetaType<DependsStatus>("DependsStatus");  // 注册PortConfig类型
    qRegisterMetaType<InstallStatus>("InstallStatus");
    m_pPackageStatus = packageStatus;
}

void GetStatusThread::setPackage(int index, const QString &packagePath)
{
    m_index = index;
    m_packagePath = packagePath;
}

void GetStatusThread::run()
{
    QFile debFile(m_packagePath);

    if (!debFile.exists()) {
        qWarning() << "GetStatusThread"
                   << "run"
                   << "getPackageDeoebdsStatus"
                   << "文件不存在";
    }
    DependsStatus DependsStatus = m_pPackageStatus->getPackageDependsStatus(m_packagePath);

    emit signal_dependsStatus(m_index, DependsStatus);
    InstallStatus installStatus = m_pPackageStatus->getPackageInstallStatus(m_packagePath);
    emit signal_installStatus(m_index, installStatus);
}
