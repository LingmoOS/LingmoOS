// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archivemanager.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "singlejob.h"
#include "batchjob.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QDebug>

//静态成员变量初始化。
QMutex ArchiveManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<ArchiveManager> ArchiveManager::m_instance = nullptr;//原子指针，默认初始化是0

ArchiveManager::ArchiveManager(QObject *parent)
    : QObject(parent)
{

}

ArchiveManager::~ArchiveManager()
{
    SAFE_DELETE_ELE(m_pArchiveJob);
    SAFE_DELETE_ELE(m_pInterface);
    SAFE_DELETE_ELE(m_pTempInterface);
}

ArchiveManager *ArchiveManager::get_instance()
{
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<ArchiveManager>::isTestAndSetNative()) //运行时检测
        qInfo() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new ArchiveManager);//第二次检测。
    }

    return m_instance;
}

void ArchiveManager::destory_instance()
{
    SAFE_DELETE_ELE(m_instance)
}

bool ArchiveManager::createArchive(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &stOptions, UiTools::AssignPluginType eType)
{
    // 重新创建压缩包首先释放之前的interface
    if (m_pInterface != nullptr) {
        delete m_pInterface;
        m_pInterface = nullptr;
    }

    m_pTempInterface = UiTools::createInterface(strDestination, true, eType);

    if (m_pTempInterface) {
        CreateJob *pCreateJob = new CreateJob(files, m_pTempInterface, stOptions, this);

        // 连接槽函数
        connect(pCreateJob, &CreateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pCreateJob, &CreateJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pCreateJob, &CreateJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);

        m_pArchiveJob = pCreateJob;
        pCreateJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::loadArchive(const QString &strArchiveFullPath, UiTools::AssignPluginType eType)
{
    // 重新加载首先释放之前的interface
    if (m_pInterface != nullptr) {
        delete m_pInterface;
        m_pInterface = nullptr;
    }

    m_pInterface = UiTools::createInterface(strArchiveFullPath, false, eType);

    if (m_pInterface) {
        LoadJob *pLoadJob = new LoadJob(m_pInterface);

        // 连接槽函数
        connect(pLoadJob, &LoadJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pLoadJob, &LoadJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pLoadJob;
        pLoadJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::addFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listAddEntry, const CompressOptions &stOptions)
{
    m_pTempInterface = UiTools::createInterface(strArchiveFullPath, true);

    if (m_pTempInterface) {
        AddJob *pAddJob = new AddJob(listAddEntry, m_pTempInterface, stOptions);

        // 连接槽函数
        connect(pAddJob, &AddJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pAddJob, &AddJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pAddJob, &AddJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pAddJob, &AddJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pAddJob;
        pAddJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::extractFiles(const QString &strArchiveFullPath, const QList<FileEntry> &files, const ExtractionOptions &stOptions, UiTools::AssignPluginType eType)
{
    if (nullptr == m_pInterface) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath, false, eType);
    }

    if (m_pInterface) {
        if (!stOptions.bTar_7z) {
            ExtractJob *pExtractJob = new ExtractJob(files, m_pInterface, stOptions);

            // 连接槽函数
            connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
            connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
            connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
            connect(pExtractJob, &ExtractJob::signalFileWriteErrorName, this, &ArchiveManager::signalFileWriteErrorName);
            connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

            m_pArchiveJob = pExtractJob;
            pExtractJob->start();

            return pExtractJob->errorcode;
        } else {
            // tar.7z包使用分步解压流程
            StepExtractJob *pStepExtractJob = new StepExtractJob(strArchiveFullPath, stOptions/*, strTargetFullPath, strNewArchiveFullPath*/);
            m_pArchiveJob = pStepExtractJob;

            // 连接槽函数
            connect(pStepExtractJob, &StepExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
            connect(pStepExtractJob, &StepExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
            connect(pStepExtractJob, &StepExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
            connect(pStepExtractJob, &StepExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

            pStepExtractJob->start();
            return true;
        }
    }

    // 发送结束信号
    emit signalJobFinished(ArchiveJob::JT_Extract, PFT_Error, ET_PluginError);
    return false;
}

bool ArchiveManager::extractFiles2Path(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    if (nullptr == m_pInterface) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        ExtractJob *pExtractJob = new ExtractJob(listSelEntry, m_pInterface, stOptions);

        // 连接槽函数
        connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pExtractJob;
        pExtractJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::deleteFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry)
{
    if (nullptr == m_pInterface) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        DeleteJob *pDeleteJob = new DeleteJob(listSelEntry, m_pInterface);

        // 连接槽函数
        connect(pDeleteJob, &DeleteJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pDeleteJob, &DeleteJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pDeleteJob, &DeleteJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pDeleteJob, &DeleteJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pDeleteJob;
        pDeleteJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::renameFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry)
{
    if (nullptr == m_pInterface) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        RenameJob *pRenameJob = new RenameJob(listSelEntry, m_pInterface);

        // 连接槽函数
        connect(pRenameJob, &RenameJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pRenameJob, &RenameJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pRenameJob, &RenameJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pRenameJob, &RenameJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pRenameJob;
        pRenameJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath/*, bool bAutoCreatDir*/)
{
    BatchExtractJob *pBatchExtractJob = new BatchExtractJob();
    pBatchExtractJob->setExtractPath(strTargetPath/*, bAutoCreatDir*/);

    if (pBatchExtractJob->setArchiveFiles(listFiles)) {
        // 连接槽函数
        connect(pBatchExtractJob, &BatchExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pBatchExtractJob, &BatchExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pBatchExtractJob, &BatchExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pBatchExtractJob, &BatchExtractJob::signalQuery, this, &ArchiveManager::signalQuery);
        connect(pBatchExtractJob, &BatchExtractJob::signalCurArchiveName, this, &ArchiveManager::signalCurArchiveName);

        m_pArchiveJob = pBatchExtractJob;
        pBatchExtractJob->start();

        return true;
    }

    SAFE_DELETE_ELE(pBatchExtractJob);
    return false;
}

bool ArchiveManager::openFile(const QString &strArchiveFullPath, const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram)
{
    if (nullptr == m_pInterface) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        OpenJob *pOpenJob = new OpenJob(stEntry, strTempExtractPath, strProgram, m_pInterface);

        // 连接槽函数
        connect(pOpenJob, &OpenJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pOpenJob, &OpenJob::signalQuery, this, &ArchiveManager::signalQuery);


        m_pArchiveJob = pOpenJob;
        pOpenJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::updateArchiveCacheData(const UpdateOptions &stOptions)
{
    if (m_pInterface) {
        UpdateJob *pUpdateJob = new UpdateJob(stOptions, m_pInterface);

        // 连接槽函数
        connect(pUpdateJob, &UpdateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);

        m_pArchiveJob = pUpdateJob;
        pUpdateJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::updateArchiveComment(const QString &strArchiveFullPath, const QString &strComment)
{
    ReadOnlyArchiveInterface *pInterface = UiTools::createInterface(strArchiveFullPath, true, UiTools::APT_Libzip); // zip添加注释使用libzipplugin

    if (pInterface) {
        CommentJob *pCommentJob = new CommentJob(strComment, pInterface);

        // 连接槽函数
        connect(pCommentJob, &CommentJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pCommentJob, &CommentJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);

        m_pArchiveJob = pCommentJob;
        pCommentJob->start();

        return true;
    }

    return false;
}

bool ArchiveManager::convertArchive(const QString &strOriginalArchiveFullPath, const QString &strTargetFullPath, const QString &strNewArchiveFullPath)
{
    ConvertJob *pConvertJob = new ConvertJob(strOriginalArchiveFullPath, strTargetFullPath, strNewArchiveFullPath);
    m_pArchiveJob = pConvertJob;

    // 连接槽函数
    connect(pConvertJob, &ConvertJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pConvertJob, &ConvertJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pConvertJob, &ConvertJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pConvertJob, &ConvertJob::signalQuery, this, &ArchiveManager::signalQuery);

    pConvertJob->start();
    return true;
}

bool ArchiveManager::pauseOperation()
{
    // 调用job暂停接口
    if (m_pArchiveJob) {
        m_pArchiveJob->doPause();

        return true;
    }

    return false;
}

bool ArchiveManager::continueOperation()
{
    // 调用job继续接口
    if (m_pArchiveJob) {
        m_pArchiveJob->doContinue();

        return true;
    }

    return false;
}

bool ArchiveManager::cancelOperation()
{
    // 调用job取消接口
    if (m_pArchiveJob) {
        m_pArchiveJob->kill();
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;

        return true;
    }

    return false;
}

QString ArchiveManager::getCurFilePassword()
{
    if (m_pInterface) {
        return m_pInterface->getPassword();
    }

    return "";
}

bool ArchiveManager::currentStatus()
{
    // 调用job状态接口
    if (m_pArchiveJob) {
        return m_pArchiveJob->status();
    }

    return false;
}

void ArchiveManager::slotJobFinished()
{
    if (m_pArchiveJob) {
        // 获取结束结果
        ArchiveJob::JobType eJobType = m_pArchiveJob->m_eJobType;
        PluginFinishType eFinishType = m_pArchiveJob->m_eFinishedType;
        ErrorType eErrorType = m_pArchiveJob->m_eErrorType;

        // 释放job
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;

        // 发送结束信号
        emit signalJobFinished(eJobType, eFinishType, eErrorType);
    }

    // 释放临时记录的interface
    SAFE_DELETE_ELE(m_pTempInterface);
}
