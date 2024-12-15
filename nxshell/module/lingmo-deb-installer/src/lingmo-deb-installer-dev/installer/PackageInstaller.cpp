// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PackageInstaller.h"
#include "manager/PackagesManager.h"
#include "package/Package.h"

#include <QTimer>

#include <QApt/Transaction>
#include <QApt/DebFile>
PackageInstaller::PackageInstaller(QApt::Backend *b)
{
    m_backend = b;
    m_packages = nullptr;
}

void PackageInstaller::appendPackage(Package *packages)
{
    m_packages = packages;
}

bool PackageInstaller::isDpkgRunning()
{
    QProcess proc;

    // 获取当前的进程信息
    proc.start("ps",
               QStringList() << "-e"
                             << "-o"
                             << "comm");
    proc.waitForFinished();

    // 获取进程信息的数据
    const QString processOutput = proc.readAllStandardOutput();

    // 查看进程信息中是否存在dpkg 存在说明已经正在安装其他包
    if (processOutput.contains("dpkg"))
        return true;  // 更换判断的方式

    return false;
}

void PackageInstaller::uninstallPackage()
{
    emit signal_startInstall();

    if (isDpkgRunning()) {
        qInfo() << "PackageInstaller"
                << "dpkg running, waitting...";
        // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
        QTimer::singleShot(1000 * 1, this, &PackageInstaller::uninstallPackage);
        return;
    }
    const QStringList rdepends = m_packages->getPackageReverseDependList();  // 检查是否有应用依赖到该包

    for (const auto &r : rdepends) {  // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (m_backend->package(r)) {
            // 更换卸载包的方式
            m_backend->package(r)->setPurge();
        } else
            qWarning() << "PackageInstaller"
                       << "reverse depend" << r << "error ,please check it!";
    }
    // 卸载当前包 更换卸载包的方式，此前的方式为markPackageForRemoval 会遗留配置文件对下次再次安装影响依赖解析
    QApt::Package *uninstalledPackage = m_backend->package(m_packages->getName() + ':' + m_packages->getArchitecture());

    // 如果未能成功根据包名以及架构名称获取到Package*对象，直接设置为卸载失败。并退出
    if (!uninstalledPackage) {
        emit signal_installError(QApt::CommitError, m_pTrans->errorDetails());
        return;
    }
    uninstalledPackage->setPurge();

    m_pTrans = m_backend->commitChanges();

    connect(m_pTrans, &QApt::Transaction::progressChanged, this, &PackageInstaller::signal_installProgress);

    // 详细状态信息（安装情况）展示链接
    connect(m_pTrans, &QApt::Transaction::statusDetailsChanged, this, &PackageInstaller::signal_installDetailStatus);

    // trans运行中出现错误
    connect(m_pTrans, &QApt::Transaction::errorOccurred, this, [=](QApt::ErrorCode error) {
        emit signal_installError(error, m_pTrans->errorDetails());
    });

    // 卸载结束，处理卸载成功与失败的情况并发送结束信号
    connect(m_pTrans, &QApt::Transaction::finished, this, &PackageInstaller::signal_uninstallFinished);

    // 卸载结束之后 删除指针
    connect(m_pTrans, &QApt::Transaction::finished, m_pTrans, &QApt::Transaction::deleteLater);

    m_pTrans->run();
}

void PackageInstaller::installPackage()
{
    emit signal_startInstall();
    if (isDpkgRunning()) {
        qInfo() << "[PackageInstaller]"
                << "dpkg running, waitting...";
        // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
        QTimer::singleShot(1000 * 1, this, &PackageInstaller::installPackage);
        return;
    }

    DependsStatus packageDependsStatus = m_packages->getDependStatus();
    switch (packageDependsStatus) {
        case DependsUnknown:
        case DependsBreak:
        case DependsAuthCancel:
        case ArchBreak:
            dealBreakPackage();
            break;
        case DependsAvailable:
            dealAvailablePackage();
            break;
        case DependsOk:
            dealInstallablePackage();
            break;
    }
    connect(m_pTrans, &QApt::Transaction::progressChanged, this, &PackageInstaller::signal_installProgress);

    // 详细状态信息（安装情况）展示链接
    connect(m_pTrans, &QApt::Transaction::statusDetailsChanged, this, &PackageInstaller::signal_installDetailStatus);

    // trans运行中出现错误
    connect(m_pTrans, &QApt::Transaction::errorOccurred, this, [=](QApt::ErrorCode error) {
        emit signal_installError(error, m_pTrans->errorDetails());
    });
    // 卸载结束之后 删除指针
    connect(m_pTrans, &QApt::Transaction::finished, m_pTrans, &QApt::Transaction::deleteLater);

    m_pTrans->run();
}

void PackageInstaller::dealBreakPackage()
{
    DependsStatus packageDependsStatus = m_packages->getDependStatus();
    switch (packageDependsStatus) {
        case DependsBreak:
        case DependsAuthCancel:
            emit signal_installError(packageDependsStatus, "Broken dependencies");
            return;
        case ArchBreak:
            emit signal_installError(packageDependsStatus, "Unmatched package architecture");
            return;
        default:
            emit signal_installError(packageDependsStatus, "unknown error");
            return;
    }
}

void PackageInstaller::dealAvailablePackage()
{
    const QStringList availableDepends = m_packages->getPackageAvailableDepends();
    // 获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
    for (auto const &p : availableDepends) {
        if (p.contains(" not found")) {  // 依赖安装失败
            emit signal_installError(DependsAvailable, p);

            return;
        }
        m_backend->markPackageForInstall(p);
    }
    m_pTrans = m_backend->commitChanges();
    connect(m_pTrans, &QApt::Transaction::finished, this, &PackageInstaller::installAvailableDepends);
}

void PackageInstaller::installAvailableDepends()
{
    const auto ret = m_pTrans->exitStatus();
    if (ret) {
        qWarning() << m_pTrans->error() << m_pTrans->errorDetails() << m_pTrans->errorString();  // transaction发生错误

        emit signal_installError(m_pTrans->error(), m_pTrans->errorDetails());
    }

    m_packages->setPackageDependStatus(DependsOk);
    installPackage();
}

void PackageInstaller::dealInstallablePackage()
{
    QApt::DebFile deb(m_packages->getPath());

    m_pTrans = m_backend->installFile(deb);  // 触发Qapt授权框和安装线程

    connect(m_pTrans, &QApt::Transaction::finished, this, &PackageInstaller::signal_installFinished);
}

PackageInstaller::~PackageInstaller() {}
