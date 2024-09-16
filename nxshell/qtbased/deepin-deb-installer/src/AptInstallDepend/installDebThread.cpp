// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "installDebThread.h"
#include <QDebug>

InstallDebThread::InstallDebThread()
{
    m_proc = new KProcess;

    // Note: 目前 deepin-deb-installer 使用 KPty 捕获所有通道进行设置，因此旧版的
    // installDebThread 手动捕获输入流程不再响应。
    // 修改输入模式为响应主进程输入，而不是手动管理。
    m_proc->setInputChannelMode(QProcess::ForwardedInputChannel);

    connect(m_proc, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readoutput()));
}
InstallDebThread::~InstallDebThread()
{
    if (m_proc)
        delete m_proc;
}

void InstallDebThread::setParam(const QStringList &tParam)
{
    m_listParam = tParam;
}

void InstallDebThread::getDescription(const QString &debPath)
{
    // system() 存在可控命令参数注入漏洞，即使拼接也存在命令分隔符（特殊字符）机制，因此更换方式去执行命令
    QProcess process;
    process.start("dpkg", {"-e", debPath, TEMPLATE_DIR});
    process.waitForFinished(-1);

    QFile file;
    file.setFileName(TEMPLATE_PATH);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tmpData;
        while (!file.atEnd()) {
            tmpData = file.readLine().data();
            if (tmpData.size() > 13) {
                if (tmpData.contains("Description: ")) {
                    QString str = tmpData.mid(13, tmpData.size() - 13);
                    str.remove(QChar('\n'), Qt::CaseInsensitive);
                    m_listDescribeData << str;
                }
            }
        }

        file.close();
    }
}

void InstallDebThread::on_readoutput()
{
    QString tmp = m_proc->readAllStandardOutput().data();
    qDebug() << tmp;

    foreach (QString eachData, m_listDescribeData) {
        if (tmp.contains(eachData)) {
            char c_input[20];
            while (fgets(c_input, 10, stdin)) {
                QString str = c_input;
                str.remove(QChar('\\'), Qt::CaseInsensitive);
                str.remove(QChar('"'), Qt::CaseInsensitive);

                m_proc->write(str.toLatin1().data());

                m_proc->waitForFinished(1500);

                break;
            }
        }
    }
}

void InstallDebThread::onFinished(int num)
{
    m_resultFlag = num;
}

void InstallDebThread::run()
{
    if (m_listParam.size() > 0) {
        if (m_listParam[0] == "InstallDeepinWine") {
            qDebug() << "StartInstallDeepinwine";
            QStringList depends;

            for (int i = 1; i < m_listParam.size(); i++) {
                depends << m_listParam[i];
            }

            system("echo 'libc6 libraries/restart-without-asking boolean true' | sudo debconf-set-selections\n");
            m_proc->setProgram("sudo",
                               QStringList() << "apt-get"
                                             << "install" << depends << "--fix-missing"
                                             << "-y");
            m_proc->start();
            m_proc->waitForFinished(-1);
            m_proc->close();
        } else if (m_listParam[0] == "InstallConfig") {
            if (m_listParam.size() <= 1)
                return;
            const QFileInfo info(m_listParam[1]);
            const QFile debFile(m_listParam[1]);
            QString debPath = m_listParam[1];
            if (debPath.contains(" ") || debPath.contains("&") || debPath.contains(";") || debPath.contains("|") ||
                debPath.contains("`")) {  // 过滤反引号,修复中危漏洞，bug 115739，处理命令连接符，命令注入导致无法软链接成功
                debPath = SymbolicLink(debPath, "installPackage");
            }

            if (debFile.exists() && info.isFile() && info.suffix().toLower() == "deb") {  // 大小写不敏感的判断是否为deb后缀
                qInfo() << "StartInstallAptConfig";

                getDescription(debPath);

                // m_proc->start("sudo", QStringList() << "-S" <<  "dpkg-preconfigure" << "-f" << "Teletype" << m_listParam[1]);
                //                m_proc->start("sudo", QStringList() << "-S" <<  "dpkg" << "-i" << debPath);
                m_proc->setProgram("sudo",
                                   QStringList() << "-S"
                                                 << "dpkg"
                                                 << "-i" << debPath);
                m_proc->start();
                m_proc->waitForFinished(-1);

                QDir filePath(TEMPLATE_DIR);
                if (filePath.exists()) {
                    filePath.removeRecursively();
                }

                m_proc->close();
            }
        }
    }
}

/**
 * @brief PackagesManager::SymbolicLink 创建软连接
 * @param previousName 原始路径
 * @param packageName 软件包的包名
 * @return 软链接的路径
 */
QString InstallDebThread::SymbolicLink(const QString &previousName, const QString &packageName)
{
    if (!mkTempDir()) {
        qWarning() << "InstallDebThread:"
                   << "Failed to create temporary folder";
        return previousName;
    }
    return link(previousName, packageName);
}

/**
 * @brief PackagesManager::mkTempDir 创建软链接存放的临时目录
 * @return 创建目录的结果
 */
bool InstallDebThread::mkTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (!tempPath.exists()) {
        return tempPath.mkdir(m_tempLinkDir);
    } else {
        return true;
    }
}

/**
 * @brief PackagesManager::rmTempDir 删除存放软链接的临时目录
 * @return 删除临时目录的结果
 */
bool InstallDebThread::rmTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (tempPath.exists()) {
        return tempPath.removeRecursively();
    } else {
        return true;
    }
}

/**
 * @brief PackagesManager::link 创建软链接
 * @param linkPath              原文件的路径
 * @param packageName           包的packageName
 * @return                      软链接之后的路径
 */
QString InstallDebThread::link(const QString &linkPath, const QString &packageName)
{
    qDebug() << "InstallDebThread: Create soft link for" << packageName;
    QFile linkDeb(linkPath);

    // 创建软链接时，如果当前临时目录中存在同名文件，即同一个名字的应用，考虑到版本可能有变化，将后续添加进入的包重命名为{packageName}_1
    // 删除后再次添加会在临时文件的后面添加_1,此问题不影响安装。如果有问题，后续再行修改。
    int count = 1;
    QString tempName = packageName;
    while (true) {
        QFile tempLinkPath(m_tempLinkDir + tempName);
        if (tempLinkPath.exists()) {
            tempName = packageName + "_" + QString::number(count);
            qWarning() << "InstallDebThread:"
                       << "A file with the same name exists in the current temporary directory,"
                          "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            break;
        }
    }
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName))
        return m_tempLinkDir + tempName;
    else {
        qWarning() << "InstallDebThread:"
                   << "Failed to create Symbolick link error.";
        return linkPath;
    }
}
