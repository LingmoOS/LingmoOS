// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AddPackageThread.h"
#include "packagesmanager.h"
#include "utils/utils.h"

#include <QApt/Backend>
#include <QApt/DebFile>

#include <DRecentManager>

#include <QDir>
#include <QStorageInfo>

DCORE_USE_NAMESPACE

using namespace QApt;

AddPackageThread::AddPackageThread(QSet<QByteArray> appendedPackagesMd5)
    : m_appendedPackagesMd5(appendedPackagesMd5)
{
}

void AddPackageThread::setPackages(const QStringList &packages, int validPkgCount)
{
    m_packages.clear();
    m_packages.append(packages);
    m_validPackageCount = validPkgCount;
}

void AddPackageThread::setAppendPackagesMd5(const QSet<QByteArray> &appendedPackagesMd5)
{
    m_appendedPackagesMd5 = appendedPackagesMd5;
}

void AddPackageThread::setSamePackageMd5(const QMap<QString, QByteArray> &packagesMd5)
{
    m_allPackages = packagesMd5;
}

bool AddPackageThread::dealInvalidPackage(const QString &packagePath)
{
    if (!Utils::checkPackageReadable(packagePath)) {
        QFileInfo debFileIfo(packagePath);
        // 查看包是否能够打开，无法打开说明包不在本地或无权限。
        QFile outfile(packagePath.toUtf8());
        outfile.open(QFile::ReadOnly);

        if (!outfile.isOpen()) {  // 打不开，文件不在本地或无安装权限
            QFile::FileError error = outfile.error();
            if (error == QFile::FileError::NoError) {
                // 文件不存在或路径错误
                emit signalAppendFailMessage(Pkg::PackageNotLocal);
                return false;
            } else {
                //无安装权限
                emit signalAppendFailMessage(Pkg::PackageNotInstallable);
                return false;
            }
        }
    }
    return true;
}

QString AddPackageThread::dealPackagePath(const QString &packagePath)
{
    auto tempPath = packagePath;
    // 判断当前文件路径是否是绝对路径，不是的话转换为绝对路径
    if (!tempPath.startsWith("/")) {
        QFileInfo packageAbsolutePath(tempPath);
        // 获取绝对路径
        tempPath = packageAbsolutePath.absoluteFilePath();
    }

    // 判断当前文件路径中是否存在空格,如果存在则创建软链接并在之后的安装时使用软链接进行访问.
    if (tempPath.contains(" ")) {
        QApt::DebFile p(tempPath);
        if (p.isValid()) {
            tempPath = SymbolicLink(tempPath, p.packageName());
            qWarning() << "PackagesManager:"
                       << "There are spaces in the path, add a soft link" << tempPath;
        }
    }
    return tempPath;
}

void AddPackageThread::run()
{
    for (QString debPackage : m_packages) {
        // 处理包不在本地的情况。
        if (!dealInvalidPackage(debPackage)) {
            continue;
        }
        QString debPkg = debPackage;
        debPackage = dealPackagePath(debPackage);

        if (debPackage.endsWith(".ddim")) {
            Q_EMIT signalAppendFailMessage(Pkg::PackageNotDdim);
            continue;
        }

        QApt::DebFile pkgFile(debPackage);
        // 判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            // 根据文件无效的类型提示不同的文案
            Q_EMIT signalAppendFailMessage(Pkg::PackageInvalid);
            continue;
        }
        // 获取当前文件的md5的值,防止重复添加
        // 先查看之前检测包有效性时是否获取过md5
        QByteArray md5 = m_allPackages.value(debPkg);
        if (md5.isEmpty())
            md5 = pkgFile.md5Sum();

        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            //处理重复文件
            Q_EMIT signalAppendFailMessage(Pkg::PackageAlreadyExists);
            continue;
        }
        // 管理最近文件列表
        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(debPackage, data);

        // 添加到set中，用来判断重复
        m_appendedPackagesMd5 << md5;

        // 可以添加,发送添加信号
        emit signalAddPackageToInstaller(m_validPackageCount, debPackage, md5);
    }

    emit signalAppendFinished();
}

QString AddPackageThread::SymbolicLink(const QString &previousName, const QString &packageName)
{
    // 如果创建临时目录失败,则提示
    if (!mkTempDir()) {
        qWarning() << "AddPackageThread:"
                   << "Failed to create temporary folder";
        return previousName;
    }
    // 成功则开始创建
    return link(previousName, packageName);
}

bool AddPackageThread::mkTempDir()
{
    QDir tempPath(m_tempLinkDir);

    if (!tempPath.exists()) {
        // 如果临时目录不存在则返回创建结果
        return tempPath.mkdir(m_tempLinkDir);
    } else {
        // 临时目录已经存在,直接返回创建成功
        return true;
    }
}

QString AddPackageThread::link(const QString &linkPath, const QString &packageName)
{
    QFile linkDeb(linkPath);

    // 创建软链接时，如果当前临时目录中存在同名文件，即同一个名字的应用，考虑到版本可能有变化，将后续添加进入的包重命名为{packageName}_i
    // 删除后再次添加会在临时文件的后面添加_1,此问题不影响安装。如果有问题，后续再行修改。
    int count = 1;
    QString tempName = packageName;

    // 命名创建的软链接文件
    while (true) {
        QFile tempLinkPath(m_tempLinkDir + tempName);
        // 对已经存在重名文件的处理
        if (tempLinkPath.exists()) {  // 命名方式为在包名后+"_i" PS:i 为当前重复的数字,无实际意义,只是为了区别不同的包
            tempName = packageName + "_" + QString::number(count);
            qWarning() << "AddPackageThread:"
                       << "A file with the same name exists in the current temporary directory,"
                          "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            break;
        }
    }
    // 创建软链接
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName))
        return m_tempLinkDir + tempName;  // 创建成功,返回创建的软链接的路径.
    else {
        // 创建失败,直接返回路径
        qWarning() << "AddPackageThread:"
                   << "Failed to create Symbolick link error.";
        return linkPath;
    }
}
