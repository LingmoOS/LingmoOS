// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "DDLog.h"

#include <QProcess>
#include <QDir>
#include <QUuid>
#include <QFile>
#include <QLoggingCategory>

#include <sys/utsname.h>
#include <unistd.h>
#include <fcntl.h>

using namespace DDLog;

const QString BLACKLIST_CONF = "/etc/modprobe.d/blacklist-devicemanager.conf";

Utils::Utils()
{

}

QString Utils::file2PackageName(const QString &fileName)
{
    QString packageName("");
    QProcess process;
    process.start("dpkg", QStringList() << "-S" << fileName);
    if (!process.waitForFinished())
        return packageName;
    //content like linux-image-4.19.0-amd64-desktop: /lib/modules/4.19.0-amd64-desktop/kernel/drivers/hid/usbhid/usbhid.ko
    QString strContent = process.readAll();
    QStringList strList = strContent.split(":");

    if (strList.size() > 0) {
        packageName = strList.at(0);
    }

    return  packageName;
}


QString Utils::getModuleFilePath(const QString &moduleName)
{
    QString koPath("");
    QProcess process;
    process.start("modinfo", QStringList() << "-n" << moduleName);
    if (!process.waitForFinished())
        return koPath;
    koPath = process.readAll();
    koPath.remove("\n");

    return  koPath;
}

QString Utils::kernelRelease()
{
    struct utsname name;
    uname(&name);
    QString strKernel(name.release);

    return  strKernel;
}

QString Utils::machineArch()
{
    struct utsname name;
    uname(&name);
    QString strArch(name.machine);
    qCInfo(appLog) << name.machine << name.sysname << name.domainname << name.nodename << name.release;

    return  strArch;
}

bool Utils::addModBlackList(const QString &moduleName)
{
    QProcess process;
    process.start(QString("echo blacklist %1 >> %2").arg(moduleName).arg(BLACKLIST_CONF));
    if (!process.waitForFinished())
        return  false;
    return  true;
}

bool Utils::unInstallPackage(const QString &packageName)
{
    QProcess process;
    process.start(QString("apt remove %1").arg(packageName));
    if (!process.waitForFinished())
        return  false;
    return  true;
}

/**
 * @brief Utils::isDriverPackage 判断Deb是否为驱动包
 * @param filepath 包文件路径
 * @return true: 是 false: 否
 */
bool Utils::isDriverPackage(const QString &filepath)
{
    if (!QFile::exists(filepath)) {
        return  false;
    }
    bool bsuccess = false;
    QDir tmpDir = QDir::temp();
    QString tmpPath = QString("devicemanager-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (tmpDir.mkdir(tmpPath)) {
        tmpDir.cd(tmpPath);
        QString strExtract = tmpDir.absolutePath();
        executeCmd("dpkg-deb", QStringList() << QString("-x") << filepath << strExtract);
        // 2021-12-24 liujuna@uniontech.com 修改过滤规则
        // 关键字查找 insmod modprobe和 路径 /lib/module 会在设备管理器本身(后台服务)和libhd等安装包中返回true，因此暂不可使用
        // 英伟达驱动中找不到 .ko 和 .ppd 等信息 ， 但是可以找到 nvidia*.ko 字段，因此添加 nvidia*.ko 过滤字段
        // 不能直接通过包名判断 比如 "deepin-devicemanager_1.0.deb" 判断是否包含 "deepin-devicemanager" 此时同样会过滤 "/home/uos/deepin-devicemanager/driver.deb"
        QString outInfo1 = executeCmd("grep", QStringList() << QString("-irHE") << QString("nvidia*.ko") << strExtract);
        if (!outInfo1.isEmpty()) {
            bsuccess = true;
        }
        if(!bsuccess) {
            QString outInfo2 = executeCmd("find", QStringList() << strExtract << QString("-name") << QString("*.ko") << QString("-o") << QString("-name") << QString("*.ppd"));
            if (!outInfo2.isEmpty()) {
                bsuccess = true;
            }
        }
        //此处主动调用删除临时文件，临时文件在/tmp目录每次重启会自动清除，所以不对删除结果做处理
        tmpDir.removeRecursively();
    }
    return  bsuccess;
}

bool Utils::updateModDeps(bool bquick)
{
    QProcess process;
    QString strcomd;
    if (bquick) {
        strcomd = QString("depmod %1").arg("--quick");
    } else {
        strcomd = QString("depmod %1").arg("--all");
    }
    process.start(strcomd);
    if (!process.waitForFinished())
        return  false;

    return  true;
}

bool Utils::isFileLocked(const QString &filepath, bool bread)
{
    short int ltype;
    int opentype;
    if (bread) {
        ltype = F_RDLCK;
        opentype = O_RDONLY;
    } else {
        ltype = F_WRLCK;
        opentype = O_WRONLY;
    }
    struct flock fl;
    fl.l_type   = ltype;  /*F_RDLCK read/F_WRLCK write lock */
    fl.l_whence = SEEK_SET; /* beginning of file */
    fl.l_start  = 0;        /* offset from l_whence */
    fl.l_len    = 0;        /* length, 0 = to EOF */
    fl.l_pid    = getpid(); /* PID */

    int fd = open(filepath.toStdString().c_str(), opentype);
    //文件打开失败默认为被锁住
    if (fd < 0) {
        return  true;
    }

    if (-1 == fcntl(fd, F_SETLK, &fl)) {
        //设置失败，文件已被锁住
        qCInfo(appLog) << __func__ << "file already locked";
        return true;
    }
    //file not locked,set lock success
    fl.l_type   = F_UNLCK;
    //unset lock 必须取消锁定否则其它地方无法获取文件锁
    fcntl(fd, F_SETLK, &fl);
    return  false;
}

bool Utils::isDpkgLocked()
{
    QProcess proc;
    proc.setProgram("ps");
    proc.setArguments(QStringList() << "-e" << "-o" << "comm");
    proc.start();
    proc.waitForFinished();
    QString info = proc.readAllStandardOutput();
    if (!info.contains("dpkg"))
        return false;

    // Split the output  search for the 'grep dpkg ' pattern
      foreach (QString out, info.split("\n")) {
          if (out.contains("dpkg")) {
            if(out.trimmed() == "dpkg-query")
                return false;
          }
      }
    return true;
}

QString Utils::getUrl()
{
    QFile file(QDir::homePath() + "/url");
    if (!file.open(QIODevice::ReadOnly)) {
        return "https://driver.uniontech.com/api/v1/drive/search";
    }
    QString info = file.readAll();
    if ("true" == info) {
        return "https://driver.uniontech.com/api/v1/drive/search";
    } else {
        return "https://drive-pre.uniontech.com/api/v1/drive/search";
    }
}

QByteArray Utils::executeCmd(const QString &cmd, const QStringList &args, const QString &workPath, int msecsWaiting/* = 30000*/)
{
    QProcess process;
    if (!workPath.isEmpty())
        process.setWorkingDirectory(workPath);

    process.setProgram(cmd);
    process.setArguments(args);
    process.setEnvironment({"LANG=en_US.UTF-8", "LANGUAGE=en_US"});
    process.start();
    // Wait for process to finish without timeout.
    process.waitForFinished(msecsWaiting);
    QByteArray outPut = process.readAllStandardOutput();
    int nExitCode = process.exitCode();
    bool bRet = (process.exitStatus() == QProcess::NormalExit && nExitCode == 0);
    if (!bRet) {
        qCWarning(appLog) << "run cmd error, caused by:" << process.errorString() << "output:" << outPut;
        return QByteArray();
    }
    return outPut;
}
