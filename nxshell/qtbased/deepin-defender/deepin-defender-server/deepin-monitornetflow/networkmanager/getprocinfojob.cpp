// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "getprocinfojob.h"
#include "common.h"

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QSettings>

#include <fstream>
#include <unistd.h> //readlink()

#define X_DEEPIN_VENDOR_STR "deepin"
#define ONLY_SHOW_IN_VALUE_DEEPIN "Deepin"

//通过链接文件名获取被目标文件绝对路径
//为了防止buffer不够大，包装一下readlink()
QString getFilePathByLink(const char *filename)
{
    QString retStr;
    unsigned int size = 100;
    char *buffer = nullptr;

    while (1) {
        buffer = new char[size];
        //readlink()返回的路径名并没有以'\0'结尾
        long nchars = readlink(filename, buffer, size - 1);
        if (nchars < 0) {
            break;
        }
        if (nchars <= size - 1) {
            buffer[nchars] = '\0'; //让路径名以'\0'结尾
            retStr = buffer;
            break;
        }
        size *= 2; //buffer空间不足，扩大为2倍
        delete[] buffer;
    }
    delete[] buffer;
    return retStr;
}

GetProcInfoJob::GetProcInfoJob()
{
}

GetProcInfoJob::~GetProcInfoJob()
{
}

void GetProcInfoJob::updateLauncherItemInfoList()
{
    m_installedAppInfoList.clear();
    QDir dir("/usr/share/applications");
    QFileInfoList desktopFileList = dir.entryInfoList(QDir::Filter::Files);
    QString sysLanguage = getSysLanguage();
    for (const QFileInfo &info : desktopFileList) {
        QString desktopPath;
        if (info.isSymLink()) {
            desktopPath = info.readLink();
        } else {
            desktopPath = info.filePath();
        }

        QString pkgName = getPkgNameByExecPath(desktopPath);
        if (pkgName.isEmpty()) {
            continue;
        }

        DefenderProcInfo procInfo;
        procInfo.sPkgName = pkgName;

        // 是否为界面应用
        QSettings readIniSettingMethod(desktopPath, QSettings::Format::IniFormat);
        QTextCodec *textCodec = QTextCodec::codecForName("UTF-8");
        readIniSettingMethod.setIniCodec(textCodec);
        readIniSettingMethod.beginGroup("Desktop Entry");
        // 判断是否不显示
        bool noDisplay = readIniSettingMethod.value("NoDisplay").toBool();
        if (noDisplay) {
            continue;
        }
        // "OnlyShowIn"属性为空或Deepin，则显示
        QString onlyShowIn = readIniSettingMethod.value("OnlyShowIn").toString();
        if (!onlyShowIn.isEmpty() && ONLY_SHOW_IN_VALUE_DEEPIN != onlyShowIn) {
            continue;
        }
        // 桌面文件路径
        procInfo.sDesktopPath = desktopPath;
        // 应用名称
        QString xDeepinVendor = readIniSettingMethod.value("X-Deepin-Vendor").toString();
        if (X_DEEPIN_VENDOR_STR == xDeepinVendor) {
            procInfo.sAppName = readIniSettingMethod.value(QString("GenericName[%1]").arg(sysLanguage)).toString();
        } else {
            procInfo.sAppName = readIniSettingMethod.value(QString("Name[%1]").arg(sysLanguage)).toString();
        }
        if (procInfo.sAppName.isEmpty()) {
            procInfo.sAppName = readIniSettingMethod.value("Name").toString();
        }
        // 获取执行路径
        procInfo.sExecPath = readIniSettingMethod.value("Exec").toString().split(" ").first();
        // 判断是否是系统应用
        procInfo.isbSysApp = true;
        if (procInfo.sExecPath.contains("/opt/"))
            procInfo.isbSysApp = false;
        // 获取图标
        procInfo.sThemeIcon = readIniSettingMethod.value("Icon").toString();
        readIniSettingMethod.endGroup();

        m_installedAppInfoList.append(procInfo);
    }

    Q_EMIT sendInstalledAppInfoList(m_installedAppInfoList);
}

// 更新进程信息列表
void GetProcInfoJob::updateProInfoList(const DefenderProcInfoList &list)
{
    DefenderProcInfoList lastProcInfoList = m_procInfoList;
    m_procInfoList.clear();

    // 由接收到的包含流量信息的进程列表更新包含所有信息的进程列表
    for (const DefenderProcInfo &info : list) {
        // 如果是已存在进程，则保持信息不变
        bool isbNewProc = true;
        for (int i = 0; i < lastProcInfoList.size(); i++) {
            DefenderProcInfo lastInfo = lastProcInfoList.at(i);
            if (info.nPid == lastInfo.nPid) {
                isbNewProc = false;
                // 更新流量数据
                lastInfo.dDownloads = info.dDownloads;
                lastInfo.dUploads = info.dUploads;
                lastInfo.dDownloadSpeed = info.dDownloadSpeed;
                lastInfo.dUploadSpeed = info.dUploadSpeed;
                m_procInfoList.append(lastInfo);
                lastProcInfoList.removeAt(i);
                break;
            }
        }

        if (!isbNewProc) {
            continue;
        }

        // 只更新新打开的进程
        DefenderProcInfo procInfo;
        procInfo.nPid = info.nPid;

        // 获取执行路径
        QString sExecPathTmp = getProcExecPath(procInfo.nPid);
        if (sExecPathTmp.isEmpty()) {
            procInfo.sProcName = getProcCommName(procInfo.nPid);
            procInfo.sExecPath = procInfo.sProcName;
        } else {
            // 获取进程名
            procInfo.sProcName = sExecPathTmp.split("/").last();
            procInfo.sExecPath = sExecPathTmp;
        }

        // 判断是否是系统应用
        procInfo.isbSysApp = true;
        if (procInfo.sExecPath.contains("/opt/")) {
            procInfo.isbSysApp = false;
        }

        // 根据进程名从启动器应用信息列表中获取 桌面文件名、应用名称、图标
        DefenderProcInfo procInfoFromLauncher = getProcInfoByExecPath(procInfo.sExecPath);
        procInfo.sDesktopPath = procInfoFromLauncher.sDesktopPath;
        procInfo.sPkgName = procInfoFromLauncher.sPkgName;
        procInfo.sAppName = procInfoFromLauncher.sAppName;
        procInfo.sThemeIcon = procInfoFromLauncher.sThemeIcon;

        QString sDesktopPath = getProcEnvProperties(procInfo.nPid, "GIO_LAUNCHED_DESKTOP_FILE");
        // 判断是否为链接
        QFileInfo fileInfo(sDesktopPath);
        if (fileInfo.isSymLink()) {
            sDesktopPath = fileInfo.readLink();
        }
        // 从已安装应用信息列表中获取wine程序的信息
        for (DefenderProcInfo appInfo : m_installedAppInfoList) {
            if (sDesktopPath == appInfo.sDesktopPath && !sDesktopPath.isEmpty()) {
                procInfo.isbSysApp = appInfo.isbSysApp;
                procInfo.sDesktopPath = appInfo.sDesktopPath;
                procInfo.sPkgName = appInfo.sPkgName;
                procInfo.sAppName = appInfo.sAppName;
                procInfo.sThemeIcon = appInfo.sThemeIcon;
                break;
            }
        }

        if (procInfo.sPkgName.isEmpty()) {
            procInfo.sPkgName = getProcEnvProperties(procInfo.nPid, "DEB_PACKAGE_NAME");
        }

        // 更新流量数据
        procInfo.dDownloads = info.dDownloads;
        procInfo.dUploads = info.dUploads;
        procInfo.dDownloadSpeed = info.dDownloadSpeed;
        procInfo.dUploadSpeed = info.dUploadSpeed;

        m_procInfoList.append(procInfo);
    }
    // 发送进程信息列表
    Q_EMIT sendProcInfos(m_procInfoList);
}

QString GetProcInfoJob::getSysLanguage() const
{
    QString language;
    QFile file("/etc/default/locale");
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
        qDebug() << Q_FUNC_INFO << file.fileName() << "open failed" << file.errorString();
        return "";
    }
    QString localeStr = file.readAll();
    file.close();

    QStringList localList = localeStr.split("\n");
    for (const QString &localeData : localList) {
        if (localeData.startsWith("LANGUAGE=")) {
            language = localeData.split("=").last();
        }
    }
    return language;
}

QString GetProcInfoJob::getProcExecPath(pid_t pid) const
{
    // 待返回的进程名
    QString retStr;

    // 获取执行路径
    QString sExeLinkPath = "/proc/" + QString::number(pid) + "/exe";
    QFileInfo info(sExeLinkPath);
    retStr = info.readLink();
    if (!retStr.isEmpty()) {
        return retStr;
    }

    // 如无法获取执行路径，则使用cmdline代替
    // 获取/prc/路径下对应pid目录下cmdline文件中内容
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(long(pid)) + "/cmdline", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ifstream::failure e) {
        return "FAILED TO READ PROC";
    }

    // 如果获取cmdline文件内容为空，则返回空名称
    if (temp.size() < 1) {
        return retStr;
    }

    // change \0 to ' '
    std::replace(temp.begin(), temp.end(), '\0', ' ');

    QString sTmpStr = QString::fromStdString(temp);
    // 按空格分割进程命令行
    QStringList retStrList = sTmpStr.split(" ");
    // 取字符串中"/"后的进程名
    if (0 < retStrList.count()) {
        retStr = retStrList.first();
    }

    return retStr;
}

QString GetProcInfoJob::getProcCommName(pid_t pid) const
{
    // 待返回的进程名
    QString retStr;

    // 获取执行路径
    QString commNamePath = "/proc/" + QString::number(pid) + "/comm";

    QFile f(commNamePath);
    if (!f.exists()) {
        return retStr;
    }

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Open commNamePath failed." << pid << endl;
        return retStr;
    }

    retStr = QString(f.readAll());

    f.close();
    return retStr;
}

QString GetProcInfoJob::getProcEnvProperties(pid_t pid, QString sEvnKey)
{
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(pid) + "/environ", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ios_base::failure e) {
        return "FAILED TO READ PROC";
    }

    if (temp.size() < 1) {
        return "";
    }

    QStringList sEnvs = QString::fromStdString(temp).trimmed().split('\0');
    foreach (auto envValue, sEnvs) {
        if (envValue.startsWith(QString("%1=").arg(sEvnKey))) {
            return envValue.remove(0, sEvnKey.length() + 1);
        }
    }

    return "";
}

QString GetProcInfoJob::getPkgNameByExecPath(QString sExecPath)
{
    return Utils::getPkgNameByFilePathFromDb(sExecPath, "GetProcInfoJob");
}

DefenderProcInfo GetProcInfoJob::getProcInfoByExecPath(QString sExecPath)
{
    DefenderProcInfo retInfo;
    QString sPkgName = getPkgNameByExecPath(sExecPath);

    // 由包名从启动器服务中获取进程信息（应用名称、图标、包名）
    retInfo = getProcInfoByPkgName(sPkgName);

    return retInfo;
}

// 通过包名，从已安装列表中获取进程信息
DefenderProcInfo GetProcInfoJob::getProcInfoByPkgName(QString sPkgName)
{
    DefenderProcInfo retProcInfo;

    if (sPkgName.isEmpty()) {
        return retProcInfo;
    }

    // 匹配包名
    for (DefenderProcInfo info : m_installedAppInfoList) {
        QString sPkgNameTmp = info.sPkgName;
        if (sPkgNameTmp.contains(sPkgName)) {
            retProcInfo = info;
            break;
        }
    }

    return retProcInfo;
}
