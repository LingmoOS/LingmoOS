/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "desktopfilemanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QDebug>
#include <KDesktopFile>
#include <fstream>
#include "processinfohelper.h"

#define XDG_AUTO_START_PATH "/etc/xdg/autostart"

DesktopFileManager::DesktopFileManager(QObject *parent)
    : QObject{parent}
{
    loadDesktopFiles();
}

std::string DesktopFileManager::syncGetDesktopFileByPid(int pid)
{
    QString desktopFile = findDesktopFileFromLocalCache(pid);
    if(!desktopFile.isEmpty()) {
        return desktopFile.toStdString();
    }
    return findDesktopFileByDpkgCommand(pid).toStdString();
}

std::vector<std::string> DesktopFileManager::desktopFilesWithName(const std::string &desktopName)
{
    if (m_loadDesktopFilesFuture.isRunning()) {
        m_loadDesktopFilesFuture.waitForFinished();
    }

    std::vector<std::string> files;
    auto filePath = desktopFilePath(QString::fromStdString(desktopName));
    if (filePath.isEmpty()) {
        return files;
    }

    auto desktopFiles = m_desktopExecs.keys();
    for (auto &desktopFile : qAsConst(desktopFiles)) {
        if (desktopFile.split("/").last() == QString::fromStdString(desktopName)) {
            files.emplace_back(desktopFile.toStdString());
        }
    }

    return files;
}

void DesktopFileManager::loadDesktopFiles()
{
    auto desktopPaths = desktopFilePaths();
    auto desktopFileExecFunction = [desktopPaths, this]() {
        for (const auto &desktopPath : desktopPaths) {
            updateDesktopExecsInfo(desktopPath);
        }
    };
    m_loadDesktopFilesFuture = QtConcurrent::run(desktopFileExecFunction);
}

QStringList DesktopFileManager::desktopFilePaths() const
{
    QStringList desktopPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    desktopPaths.append(XDG_AUTO_START_PATH);
    return desktopPaths;
}

void DesktopFileManager::updateDesktopExecsInfo(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop");
    for (auto &desktopFileName : qAsConst(desktopFiles)) {
        QString deskopFile = path + "/" + desktopFileName;
        QString exec;
        QStringList args;
        std::tie(exec, args) = getExecByDesktopFile(deskopFile);
        {
            if (exec.isEmpty()) {
                qWarning() << QString("The Exec in %1 desktop file is empty.").arg(desktopFileName);
                continue;
            }
            QMutexLocker locker(&m_mutex);
            if (!m_desktopExecs.contains(deskopFile)) {
                m_desktopExecs[deskopFile] = { exec, args };
            }
        }
    }
}

std::tuple<QString, QStringList> DesktopFileManager::getExecByDesktopFile(const QString &desktopFile) const
{
    if (desktopFile.isEmpty()) {
        return std::make_tuple(QString(), QStringList());
    }
    QString _desktopFilePath = desktopFilePath(desktopFile);
    KDesktopFile desktop(_desktopFilePath);
    QString exec = desktop.entryMap("Desktop Entry").value("Exec");
    if (exec.isEmpty()) {
        return {QString(), QStringList()};
    }

    exec.remove(QRegExp("%."));
    exec.remove(QRegExp("^\""));
    exec.remove(QRegExp(" *$"));
    exec.remove(QRegExp("\""));

    QStringList execArgs = exec.split(" ");
    if (execArgs.count() > 0) {
        exec = execArgs.first();
    }

    return { exec, execArgs };
}

QString DesktopFileManager::desktopFilePath(const QString &desktopFileName) const
{
    if (desktopFileName.isEmpty()) {
        return QString();
    }
    QFile desktopFile(desktopFileName);
    if (desktopFile.exists()) {
        return desktopFileName;
    }

    auto desktopPaths = desktopFilePaths();
    for (auto &path : qAsConst(desktopPaths)) {
        if (desktopFileName.contains(path)) {
            return desktopFileName;
        } else if (QFile::exists(path + "/" + desktopFileName)) {
            return path + "/" + desktopFileName;
        }
    }
    return QString();
}

QString DesktopFileManager::findDesktopFileFromLocalCache(const int &pid)
{
    QString cmdline = QString::fromStdString(process_info_helper::cmdline(pid));
    if (cmdline.isEmpty()) {
        qWarning() << __FUNCTION__ << "cmdline is empty!";
        return QString();
    }
    if (cmdline.contains(polkitWindowPidCmdline)) {
        return QString();
    }
    if (m_loadDesktopFilesFuture.isRunning()) {
        m_loadDesktopFilesFuture.waitForFinished();
    }
    QString desktopFile;
    if (cmdline.startsWith(kmreAppMainWindow)) {
        desktopFile = findKmreFullDesktopFileByCmdline(cmdline);
    }
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileFromPid(pid);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileByEqualExec(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    QString binaryPath = binaryPathFromCmdline(cmdline);
    desktopFile = findDesktopFileByEqualExec(binaryPath);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileByEqualStartArg(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileByContainsStartArgs(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    desktopFile = findDesktopFileByContainsExec(binaryPath);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    int ppid = process_info_helper::parentPid(pid);
    return ppid <= 1 ? QString() : findDesktopFileFromLocalCache(ppid);
}

QString DesktopFileManager::findKmreAppDesktopFileByCmdline(const QString &cmdline)
{
    //eg. /usr/bin/lingmo-kmre-window -i 5001 -w 720 -h 1280 -n 微信 -p com.tencent.mm -f 0 -r 0
    QString kmreAppDesktopFile;
    QRegExp regExp("-p\\s(\\S+)");
    if (regExp.indexIn(cmdline) == -1) {
        qWarning() << "regExp.indexIn is error!" << cmdline;
        return QString();
    }
    kmreAppDesktopFile = regExp.cap(1);
    if (!kmreAppDesktopFile.endsWith(".desktop")) {
         kmreAppDesktopFile.push_back(".desktop");
    }
    return kmreAppDesktopFile;
}

QString DesktopFileManager::findKmreFullDesktopFileByCmdline(const QString &cmdline)
{
    QString kmreAppDesktopFile = findKmreAppDesktopFileByCmdline(cmdline);
    QString kmreAppFullDesktopFile = desktopFilePath(kmreAppDesktopFile);
    if (kmreAppFullDesktopFile.isEmpty()) {
        qWarning() << __FUNCTION__ << "kmreAppFullDesktopFile is empty!" << cmdline;
        return QString();
    }
    return kmreAppFullDesktopFile;
}

QString DesktopFileManager::findDesktopFileByCompareExec(const QString &cmdline, FindDesktopFilePredicate findDesktopFilePredicate)
{
    auto desktopExecIt = std::find_if(m_desktopExecs.constBegin(), m_desktopExecs.constEnd(),
                                      [cmdline, findDesktopFilePredicate](const std::tuple<QString, QStringList> &exec) {

        return findDesktopFilePredicate(exec);
    });
    if (desktopExecIt != m_desktopExecs.constEnd()) {
        return desktopExecIt.key();
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileByEqualExec(const QString &cmdline)
{
    QString desktopFile;
    auto desktopFileEqualExec = [cmdline] (const DesktopExecMap &execMap) {
        auto it = execMap.constBegin();
        while (it != execMap.constEnd()) {
            auto desktopFile = it.key();
            QString exec;
            QStringList args;
            std::tie(exec, args) = it.value();
            if (exec == cmdline && args.count() < 2) {
                return desktopFile;
            }
            ++it;
        }
        return QString();
    };

    DesktopExecMap standardPathDesktopExecMap = standardPathDesktopFileExecMap();
    desktopFile = desktopFileEqualExec(standardPathDesktopExecMap);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    DesktopExecMap autoStartPathDesktopExecMap = autoStartPathDesktopFileExecMap();
    desktopFile = desktopFileEqualExec(autoStartPathDesktopExecMap);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    return QString();
}

QString DesktopFileManager::findDesktopFileByContainsStartArgs(const QString &cmdline)
{
    auto containFullStartArgs = [cmdline](const std::tuple<QString, QStringList> &exec) {
        QString strExec;
        for (const auto &str : std::get<1>(exec)) {
            strExec.push_back(str);
            strExec.push_back(" ");
        }
        if (strExec.endsWith(" ")) {
            strExec.remove(strExec.size()-1, 1);
        }

        return cmdline.contains(strExec);
    };
    return findDesktopFileByCompareExec(cmdline, containFullStartArgs);
}

QString DesktopFileManager::findDesktopFileByContainsExec(const QString &cmdline)
{
    auto containDesktopFileExec = [cmdline](const std::tuple<QString, QStringList> &exec) {

        // flatpak app
        if (cmdline.contains("bwrap --args")) {
            QString appName = cmdline.split(" ").last();
            const auto args = std::get<1>(exec);
            for (const auto &arg : args) {
                if (arg.contains(appName)) {
                    return true;
                }
            }
            return false;
        }

        // lingmo-screenshot sh
        // /usr/bin/vlc --started-from-file /usr/bin/vlc
        if (cmdline.contains("/") && !std::get<0>(exec).contains("/")) {
            const auto cmdlineItems = cmdline.split("/");
            return cmdlineItems.contains(std::get<0>(exec));
        }

        return cmdline == std::get<0>(exec);
    };
    return findDesktopFileByCompareExec(cmdline, containDesktopFileExec);
}

QString DesktopFileManager::findDesktopFileByDpkgCommand(const int &pid)
{
    if (pid <= 0) {
        qWarning() << "pid is inValid!";
        return QString();
    }
    auto future = std::async(std::launch::async, [this, pid]() {
        QString cmdline = QString::fromStdString(process_info_helper::cmdline(pid));
        QString binaryPath = binaryPathFromCmdline(cmdline);
        if (binaryPath.isEmpty()) {
            return QString();
        }
        QString packageName = findPackageNameFromBinaryPath(binaryPath);
        if (packageName.isEmpty()) {
            return QString();
        }
        QString desktopFile = findDesktopFileFromPackageName(packageName, cmdline);
        if (!desktopFile.isEmpty()) {
            return desktopFile;
        }

        return QString();
    });

    std::future_status futureStatus;
    do {
        futureStatus = future.wait_for(std::chrono::milliseconds(1));
        qApp->processEvents();
    } while (futureStatus != std::future_status::ready);
    return future.get();
}

QString DesktopFileManager::binaryPathFromCmdline(const QString &cmdline)
{
    if (cmdline.isEmpty()) {
        qWarning() << __FUNCTION__ << "cmdline is empty.";
        return QString();
    }
    if (QDir::isAbsolutePath(cmdline)) {
        return cmdline;
    }
    QString binaryName = cmdline;
    if (binaryName.startsWith("./")) {
        binaryName.remove("./");
    }

    QString environmentPath = QProcessEnvironment::systemEnvironment().value("PATH");
    QStringList environmentPathLists;
    environmentPathLists << environmentPath.split(':');
    QString cmdlinePath;
    for (auto &path : qAsConst(environmentPathLists)) {
        cmdlinePath = path + "/" + binaryName;
        QFileInfo file(cmdlinePath);
        if (file.isFile()) {
            return cmdlinePath;
        }
    }
    return cmdline;
}

QString DesktopFileManager::findPackageNameFromBinaryPath(const QString &binaryPath)
{
    QProcess findPackageNameProcess;
    QStringList binaryPathList = binaryPath.split(" ");
    if (binaryPathList.isEmpty()) {
        return QString();
    }
    findPackageNameProcess.start("/usr/bin/dpkg", {"-S", binaryPathList.first()});
    findPackageNameProcess.waitForFinished();
    QString packageNameInfo = findPackageNameProcess.readAll();
    QStringList packageInfoList;
    QStringList packNameList;
    packageInfoList << packageNameInfo.split("\n");
    for (auto &packageNameinfo : qAsConst(packageInfoList)) {
        if (!packageNameinfo.isEmpty()) {
            packNameList << packageNameinfo.split(":").first();
        }
    }
    if (packNameList.isEmpty()) {
        return QString();
    }

    return packNameList.first();
}

QString DesktopFileManager::findDesktopFileFromPackageName(const QString &packageName, const QString &cmdline)
{
    if (packageName.isEmpty()) {
        qWarning() << __FUNCTION__ << "packageName is empty!" << cmdline;
        return QString();
    }
    QStringList desktopFileLists = desktopFileListFromPackageName(packageName);
    if (desktopFileLists.count() == 1) {
        return desktopFileLists.first();
    }
    QString desktopFile = bestDesktopFileFromMultDesktopFileLists(cmdline, desktopFileLists);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    return QString();
}

bool DesktopFileManager::desktopFileNoDisplay(const QString &desktopFile)
{
    if (desktopFile.isEmpty()) {
        qWarning() << __FUNCTION__ << "desktopFileName is NULL！！！";
        return false;
    }
    QString content = desktopEntryFromDesktopFile(desktopFile, "NoDisplay");
    if (content.isEmpty()) {
        qWarning() << __FUNCTION__ << "NoDisplay is empty!";
        return false;
    }
    if (content == "true") {
        return true;
    }
    return false;
}

QString DesktopFileManager::desktopEntryFromDesktopFile(const QString &desktopFile, const QString &key)
{
    QString fullDesktopFile = desktopFilePath(desktopFile);
    KDesktopFile desktop(fullDesktopFile);
    QString content = desktop.entryMap("Desktop Entry").value(key);
    if (content.isEmpty()) {
        qWarning() << __FUNCTION__ << key << "is empty!";
        return QString();
    }
    content.remove(QRegExp("%."));
    content.remove(QRegExp("^\""));
    content.remove(QRegExp(" *$"));
    content.remove(QRegExp("\""));
    return content;
}

QStringList DesktopFileManager::desktopFileListFromPackageName(const QString &packageName)
{
    QProcess findDesktopFileProcess;
    findDesktopFileProcess.start("/usr/bin/dpkg", {"-L", packageName});
    findDesktopFileProcess.waitForFinished();
    QString desktopFileInfo = findDesktopFileProcess.readAll();
    QStringList desktopFileInfoList;
    QStringList desktopFileList;
    desktopFileInfoList << desktopFileInfo.split("\n");
    for (auto &desktopFileInfo : qAsConst(desktopFileInfoList)) {
        if (desktopFileInfo.isEmpty() || !desktopFileInfo.trimmed().endsWith(".desktop")) {
            continue;
        }
        QStringList desktopDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
        for (auto &desktopPath : desktopDirs) {
            if (!desktopFileInfo.trimmed().contains(desktopPath) || desktopFileNoDisplay(desktopFileInfo.trimmed())) {
                continue;
            }
            desktopFileList << desktopFileInfo.trimmed();
        }
    }

    return desktopFileList;
}

QString DesktopFileManager::bestDesktopFileFromMultDesktopFileLists(const QString &cmdline, const QStringList &desktopFileLists)
{
    QString appCmdline = cmdline;
    if (cmdline.contains("/")) {
        int cmdlineFirst = cmdline.lastIndexOf("/");
        appCmdline = cmdline.right(cmdline.length()-cmdlineFirst-1);
    }
    // 1. 判断exec与cmdline全等
    for (auto &desktopFile : desktopFileLists) {
        auto exec = getExecByDesktopFile(desktopFile);
        if (std::get<0>(exec) == appCmdline && std::get<1>(exec).count() < 2) {
            return desktopFile;
        }
    }
    // 2. 判断启动路径+参数与cmdline全等
    for (auto &desktopFile : desktopFileLists) {
        auto exec = getExecByDesktopFile(desktopFile);
        QString startArgs = std::get<1>(exec).join(" ");
        if (startArgs == appCmdline) {
            return desktopFile;
        }
    }
    // 3. 判断cmdline包含启动路径+参数
    for (auto &desktopFile : desktopFileLists) {
        auto exec = getExecByDesktopFile(desktopFile);
        QString startArgs = std::get<1>(exec).join(" ");
        if (appCmdline.contains(startArgs)) {
            return desktopFile;
        }
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileByEqualStartArg(const QString &cmdline)
{
    QString desktopFile;
    auto desktopFileEqualStartArg = [cmdline] (const DesktopExecMap &execMap) {
        auto it = execMap.constBegin();
        while (it != execMap.constEnd()) {
            QString exec;
            QStringList args;
            std::tie(exec, args) = it.value();
            QString startArgs = args.join(" ");
            if (startArgs == cmdline) {
                return it.key();
            }
            ++it;
        }
        return QString();
    };

    DesktopExecMap standardPathDesktopExecMap = standardPathDesktopFileExecMap();
    desktopFile = desktopFileEqualStartArg(standardPathDesktopExecMap);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    DesktopExecMap autoStartPathDesktopExecMap = autoStartPathDesktopFileExecMap();
    desktopFile = desktopFileEqualStartArg(autoStartPathDesktopExecMap);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    return QString();
}

QString DesktopFileManager::findDesktopFileFromPid(int pid)
{
    if (pid <= 0) {
        qWarning() << "pid is invalid!";
        return QString();
    }
    std::string environFile = "/proc/" + std::to_string(pid) + "/environ";
    return readGioLaunchEnvironFromFile(environFile);
}

QString DesktopFileManager::readGioLaunchEnvironFromFile(const std::string &file)
{
    std::ifstream processEnvironFile(file);
    if (!processEnvironFile.is_open()) {
        return QString();
    }
    std::string line;
    std::string gioLaunchDesktopFileStr = "GIO_LAUNCHED_DESKTOP_FILE=";
    while (std::getline(processEnvironFile, line, '\0')) {
        auto pos = line.find(gioLaunchDesktopFileStr);
        if (pos != std::string::npos) {
            line.erase(pos, gioLaunchDesktopFileStr.length());
            return QString::fromStdString(line);
        }
    }
    return QString();
}

DesktopFileManager::DesktopExecMap DesktopFileManager::standardPathDesktopFileExecMap()
{
    DesktopExecMap standardPathDesktopExecMap;
    QStringList standardPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    qDebug() << "QStandardPaths::ApplicationsLocation: " << standardPaths;
    auto it = m_desktopExecs.constBegin();
    while (it != m_desktopExecs.constEnd()) {
        auto desktopFile = it.key();
        for (const auto &path : standardPaths) {
            if (desktopFile.contains(path)) {
                standardPathDesktopExecMap[desktopFile] = it.value();
            }
        }
        ++it;
    }

    return standardPathDesktopExecMap;
}

DesktopFileManager::DesktopExecMap DesktopFileManager::autoStartPathDesktopFileExecMap()
{
    DesktopExecMap autoStartPathDesktopExecMap;
    auto it = m_desktopExecs.constBegin();
    while (it != m_desktopExecs.constEnd()) {
        auto desktopFile = it.key();
        if (desktopFile.contains(XDG_AUTO_START_PATH)) {
            autoStartPathDesktopExecMap[desktopFile] = it.value();
        }
        ++it;
    }
    return autoStartPathDesktopExecMap;
}
