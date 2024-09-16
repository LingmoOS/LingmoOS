// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontmanagercore.h"
#include "dcopyfilesmanager.h"

#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

static FontManagerCore *INSTANCE = nullptr;

/*************************************************************************
 <Function>      instance
 <Description>   获取字体管理线程类的单例对象
 <Author>
 <Input>         Null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
FontManagerCore *FontManagerCore::instance()
{
    QMutex mutex;
    if (INSTANCE == nullptr) {
        QMutexLocker locker(&mutex);
        INSTANCE = new FontManagerCore;
    }

    return INSTANCE;
}

/*************************************************************************
 <Function>      FontManagerCore
 <Description>   构造函数-字体管理线程类
 <Author>
 <Input>
    <param1>     parent          Description:父类对象
 <Return>        FontManagerCore    Description:返回字体管理器线程类对象
 <Note>          null
*************************************************************************/
FontManagerCore::FontManagerCore(QObject *parent)
    : QThread(parent)
{

}

/*************************************************************************
 <Function>      ~FontManagerCore
 <Description>   析构函数-析构字体管理线程类对象
 <Author>
 <Input>
    <param1>     parent          Description:父类对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
FontManagerCore::~FontManagerCore()
{
}

/*************************************************************************
 <Function>      setType
 <Description>   设置线程执行类型
 <Author>
 <Input>
    <param1>     type            Description:类型参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::setType(Type type)
{
    qDebug() << type << endl;
    m_type = type;
}

/*************************************************************************
 <Function>      setInstallFileList
 <Description>   传入待安装字体列表
 <Author>
 <Input>
    <param1>     list            Description:待安装字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::setInstallFileList(const QStringList &list)
{
    qDebug() << __FUNCTION__ << "start" << endl;
    if (!m_instFileList.isEmpty()) {
        m_instFileList.clear();
    }

    m_instFileList << list;
}

/*************************************************************************
 <Function>      setUnInstallFile
 <Description>   传入待删除字体列表
 <Author>
 <Input>
    <param1>     filePath        Description:待删除字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::setUnInstallFile(const QStringList &filePath)
{
    if (!m_uninstFile.isEmpty())
        m_uninstFile.clear();
    m_uninstFile = filePath;
}

/*************************************************************************
 <Function>      run
 <Description>   线程执行入口函数-安装、中途安装、重装与卸载
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::run()
{
    qInfo() << __FUNCTION__ << "start" << m_type << endl;
    switch (m_type) {
    case Install:
    case HalfwayInstall:
    case ReInstall:
        handleInstall();
        break;
    case UnInstall:
        handleUnInstall();
        break;
    case DoCache:
        doCache();
        break;
    default:
        break;
    }
    qInfo() << __FUNCTION__ << "end" << m_type;
}

/*************************************************************************
 <Function>      doCmd
 <Description>   线程执行函数
 <Author>
 <Input>
    <param1>     arguments       Description:动作执行参数列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::doCmd(QStringList &arguments)
{
    qDebug() << "QProcess start";
    qDebug() << m_type << endl;
    switch (m_type) {
    case Install:
    case ReInstall:
    case HalfwayInstall:
        doInstall(arguments);
        break;
    case UnInstall:
        doUninstall(arguments);
        break;
    default:
        break;
    }
}

/*************************************************************************
 <Function>      handleInstall
 <Description>   字体安装-函数入口
 <Author>
 <Input>
    <param1>     null
 <Return>        null             Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::handleInstall()
{
    doCmd(m_instFileList);
}

/*************************************************************************
 <Function>      handleUnInstall
 <Description>   字体卸载-函数入口
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::handleUnInstall()
{
    qDebug() << "waitForFinished";
    doCmd(m_uninstFile);
//        qDebug() << __FUNCTION__ << m_uninstFile.size();
//        emit uninstallFontFinished(m_uninstFile);
    //clear
    m_uninstFile.clear();
}

/*************************************************************************
 <Function>      doInstall
 <Description>   字体安装-具体执行函数
 <Author>
 <Input>
    <param1>     fileList        Description:待安装字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::doInstall(QStringList &fileList)
{
    qDebug() << __func__ << "s" << endl;

    m_installOutList.clear();
    m_installCanceled = false;
    m_installedCount = 0;

    DCopyFilesManager::instance()->copyFiles(CopyFontThread::INSTALL, fileList);

    //delete installed fonts to prevent next time install take long time
    if (!m_installCanceled) {
        return;
    }

    m_installCanceled = false;
    Q_EMIT requestCancelInstall();
}

/*************************************************************************
 <Function>      doUninstall
 <Description>   字体卸载-具体执行函数
 <Author>
 <Input>
    <param1>     fileList        Description:待卸载字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::doUninstall(const QStringList &fileList)
{
    bool isDelete = false; // 是否删除ttc
    bool isAapplyToAll = false; // 选择应用于全部ttc
    for (const QString &file : fileList) {
        if (file.endsWith(".ttc", Qt::CaseInsensitive)) {
            if (!isAapplyToAll) {
                Q_EMIT handleDeleteTTC(file, isDelete, isAapplyToAll); // 使用Qt::BlockingQueuedConnection连接的信号槽
            }

            if (!isDelete) { // 保留ttc字体集
                m_uninstFile.removeOne(file);
                continue;
            }
        }

        QFileInfo openFile(file);

        QDir fileDir(openFile.path());

        QFile::remove(file);

        //Fonts with same family name, different style may be
        //installed in same dir, so only delete dir when it's
        //empty
        if (fileDir.isEmpty()) {
            fileDir.removeRecursively();
        }

#ifdef QT_DEBUG
//        qDebug() << "Delete font ok:" << fileDir.path() << " " << openFile.completeSuffix();
#endif
    }

    //修复删除时间有时候长的问题, 35479
    Q_EMIT uninstallFontFinished(m_uninstFile);
    qDebug() << __FUNCTION__ << m_uninstFile.size();

//发现开机后先删除字体再安装字体时，偶现安装进程无法启动，修改这里后现象消失
    bool ret = QProcess::startDetached("fc-cache");
    Q_EMIT uninstallFcCacheFinish();
    qDebug() << __FUNCTION__ << ret;
}

/**
* @brief FontManagerCore::onInstallResult 安装拷贝字体文件槽函数
* @param familyName 字体文件的familyName
* @param target 拷贝的目的文件路径
* @return void
*/
void FontManagerCore::onInstallResult(const QString &familyName, const QString &target)
{

    m_installedCount += 1;
    m_installOutList << target;
    const int totalCount = m_instFileList.count();

    double percent = m_installedCount / double(totalCount) * 100;
    /* 此处需要优化,信号太频繁,进度每增加1%发送一次信号即可 UT000591 */
    static double lastSendPercent = 0.0;
    if ((lastSendPercent < 0.001) || (percent - lastSendPercent > 0.999) || (percent - lastSendPercent < -0.001)) {
        Q_EMIT batchInstall(familyName, percent);
        lastSendPercent = percent;
    }

    if (m_installedCount != totalCount) {
        return;
    } else {
        lastSendPercent = 0.0;
    }

    qDebug() << __FUNCTION__ << m_installOutList.size() << m_CacheStatus;
    //  bug 47332 47325 Ut000442 在字体验证框弹出时进行安装，类型是HalfwayInstall，之前只对Install类型的做了
    //  安装后的处理导致bug现象的出现
    if (m_type == Install || m_type == HalfwayInstall) {
        Q_EMIT installFinished(InstallStatus::InstallSuccess, m_installOutList);
    } else if (m_type == ReInstall) {
        Q_EMIT reInstallFinished(0, m_installOutList);
    }

    //clear
    m_installOutList.clear();
    m_installedCount = 0;
}

/*************************************************************************
 <Function>      setCacheStatus
 <Description>   设置fc-cache命令执行的状态
 <Author>
 <Input>
    <param1>     CacheStatus     Description:命令执行状态的枚举
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::setCacheStatus(const CacheStatus &CacheStatus)
{
    m_CacheStatus = CacheStatus;
}

/*************************************************************************
 <Function>      cancelInstall
 <Description>   取消安装
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::cancelInstall()
{
    if (m_installCanceled)
        return;

    m_installCanceled = true;
    DCopyFilesManager::cancelInstall();
    Q_EMIT cacheFinish();
}

/*************************************************************************
 <Function>      doCache
 <Description>   执行fc-cache命令
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManagerCore::doCache()
{
    qDebug() << __FUNCTION__;
    QProcess process;
    process.start("fc-cache");
    process.waitForFinished(-1);
    Q_EMIT  cacheFinish();
    qDebug() << __FUNCTION__ << "end";
}
