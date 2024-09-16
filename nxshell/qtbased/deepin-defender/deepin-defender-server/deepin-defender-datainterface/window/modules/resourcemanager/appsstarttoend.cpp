// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsstarttoend.h"

#include <QTimer>
#include <proc/readproc.h>
#include <iostream>
#include <fstream>

//通过链接文件名获取被目标文件绝对路径
//为了防止buffer不够大，包装一下readlink()
QString getAppsPackageFilePathByLink(const char *filename)
{
    QString retStr;
    long size = 100;
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

        size = nchars + 1; //buffer空间不足，扩大为2倍
        delete[] buffer;
    }
    delete[] buffer;
    return retStr;
}

AppsStartToEnd::AppsStartToEnd(QObject *parent)
    : QObject(parent)
    , m_monitornetflow(new MonitorNetFlow("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_timer(nullptr)
{
}

AppsStartToEnd::~AppsStartToEnd()
{
}

void AppsStartToEnd::initAppsStartEndThread()
{
    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, [=]() {
        refreshProcInfos();
    });

    // 开启定时器
    startTimer();
}

void AppsStartToEnd::startTimer()
{
    m_timer->start(2000);
}

void AppsStartToEnd::stopTimer()
{
    m_timer->stop();
}

// 初始化进程流量信息map
void AppsStartToEnd::refreshProcInfos()
{
    m_newAppsInfoList.clear();

    DIR *procDir = opendir("/proc");
    if (procDir == nullptr) {
        std::cerr << "Error reading /proc, needed to get inode-to-pid-maping\n";
        exit(1);
    }

    dirent *entry;
    while ((entry = readdir(procDir))) {
        if (DT_DIR != entry->d_type)
            continue;

        if (!is_number_t(entry->d_name))
            continue;

        // 创建各进程流量信息初始数据
        int nPid = atoi(entry->d_name);
        if (getPkgNameByPid(nPid).isEmpty()) {
            continue;
        }

        m_newAppsInfoList.append(getPkgNameByPid(nPid));
    }
    closedir(procDir);

    if (m_oldAppsInfoList.count() <= 0) {
        m_oldAppsInfoList = m_newAppsInfoList;
    } else {
        if (m_newAppsInfoList.count() > 0) {
            Q_EMIT sendAppsStart(m_newAppsInfoList);
        }

        QStringList sAppsEndList = m_oldAppsInfoList.toSet().subtract(m_newAppsInfoList.toSet()).toList();
        QStringList sAppsStartList = m_newAppsInfoList.toSet().subtract(m_oldAppsInfoList.toSet()).toList();

        m_oldAppsInfoList.clear();
        m_oldAppsInfoList = m_newAppsInfoList;
        if (sAppsEndList.count() > 0) {
            Q_EMIT sendAppsEnd(sAppsEndList);
        }
    }
}

// 判断字符是否为纯数字
bool AppsStartToEnd::is_number_t(const char *string)
{
    while (*string) {
        if (!isdigit(*string))
            return false;
        string++;
    }
    return true;
}

// 通过pid获取进程的包名
QString AppsStartToEnd::getPkgNameByPid(int pid)
{
    // 初始包名为空
    QString sPkgName = "";
    QString sExePath = getProcExecPath(pid);
    if (sExePath.isEmpty()) {
        return "";
    }

    // 最常用方式 - 获取包名的第一种方式 直接通过执行路径从数据库获取
    sPkgName = m_monitornetflow->QueryPackageName(sExePath);

    // 异常情况处理 - 获取包名的第二种方式 通过环境变量中的desktop来获取包名
    if (sPkgName.isEmpty()) {
        QString sDesktopPath = getProcEnvProperties(pid, "GIO_LAUNCHED_DESKTOP_FILE");
        sPkgName = getPkgNameByDesktopPath(sDesktopPath);
    }

    // 异常情况处理 - 获取包名的第三种方式 获取环境变量中的包名
    if (sPkgName.isEmpty()) {
        sPkgName = getProcEnvProperties(pid, "DEB_PACKAGE_NAME");
    }

    return sPkgName;
}

// 通过pid获取进程执行路径
QString AppsStartToEnd::getProcExecPath(pid_t pid) const
{
    // 待返回的进程名
    QString retStr;

    // 获取执行路径
    QString sExeLinkPath = "/proc/" + QString::number(pid) + "/exe";
    retStr = getAppsPackageFilePathByLink(sExeLinkPath.toStdString().c_str());
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
        return "";
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

QString AppsStartToEnd::getProcEnvProperties(pid_t pid, QString sEvnKey)
{
    QString spath = "/proc/" + QString::number(pid) + "/environ";
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(pid) + "/environ", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ios_base::failure e) {
        return "";
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

QString AppsStartToEnd::getPkgNameByDesktopPath(QString sDesktopPath)
{
    // 初始包名为空
    QString sPkgName;

    QString sRealDesktopPath = getAppsPackageFilePathByLink(sDesktopPath.toStdString().c_str());
    if (sRealDesktopPath.isEmpty()) {
        sRealDesktopPath = sDesktopPath;
    }

    // 获取包名的第一种方式 直接通过执行路径从数据库获取
    sPkgName = m_monitornetflow->QueryPackageName(sRealDesktopPath);
    return sPkgName;
}
