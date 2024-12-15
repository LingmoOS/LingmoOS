// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "darchivemanager_p.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "singlejob.h"
#include "batchjob.h"
#include "uitools.h"
#include "archivejob.h"
#include "datamanager.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QDebug>

DCOMPRESSOR_USE_NAMESPACE

DArchiveManager::MgrFileEntry::MgrFileEntry()
{
    reset();
}

void DArchiveManager::MgrFileEntry::reset()
{
    strFullPath = "";
    strFileName = "";
    strAlias = "";
    isDirectory = false;
    qSize = 0;
    uLastModifiedTime = 0;
    iIndex = -1;
}

bool DArchiveManager::MgrFileEntry::operator==(const MgrFileEntry &t) const
{
    if (this->strFullPath == t.strFullPath && this->strFileName == t.strFileName
        && this->isDirectory == t.isDirectory && this->qSize == t.qSize
        && this->uLastModifiedTime == t.uLastModifiedTime && this->iIndex == t.iIndex) {
        return true;
    }
    return false;
}

DArchiveManager::MgrArchiveData::MgrArchiveData()
{
    reset();
}

void DArchiveManager::MgrArchiveData::reset()
{
    qSize = 0;
    qComressSize = false;
    strComment = "";
    mapFileEntry.clear();
    listRootEntry.clear();

    isListEncrypted = false;
    strPassword.clear();
}

DArchiveManager::MgrCompressOptions::MgrCompressOptions()
    : qTotalSize(0)
{
    bEncryption = false;
    bHeaderEncryption = false;
    bSplit = false;
    iVolumeSize = 0;
    iCompressionLevel = -1;
    bTar_7z = false;
    iCPUTheadNum = 1;
}

DArchiveManager::MgrExtractionOptions::MgrExtractionOptions()
    : qSize(0), qComressSize(0)
{
    bExistList = true;
    bAllExtract = false;
    bBatchExtract = false;
    bTar_7z = false;
    bOpen = false;
}

DArchiveManager::MgrUpdateOptions::MgrUpdateOptions()
{
    reset();
}

void DArchiveManager::MgrUpdateOptions::reset()
{
    eType = Delete;
    strParentPath.clear();
    listEntry.clear();
    qSize = 0;
}

QMutex DArchiveManagerPrivate::mutex;
QAtomicPointer<DArchiveManager> DArchiveManagerPrivate::instance = nullptr;

DArchiveManager::DArchiveManager(QObject *parent)
    : QObject(parent), d_ptr(new DArchiveManagerPrivate(this))
{
}

DArchiveManager::~DArchiveManager()
{
    Q_D(DArchiveManager);
    SAFE_DELETE_ELE(d->pArchiveJob);
    SAFE_DELETE_ELE(d->pInterface);
    SAFE_DELETE_ELE(d->pTempInterface);
}

DArchiveManager *DArchiveManager::get_instance()
{
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<DArchiveManager>::isTestAndSetNative())
        qInfo() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (DArchiveManagerPrivate::instance.testAndSetOrdered(nullptr, nullptr)) {
        QMutexLocker locker(&DArchiveManagerPrivate::mutex);

        DArchiveManagerPrivate::instance.testAndSetOrdered(nullptr, new DArchiveManager);
    }

    return DArchiveManagerPrivate::instance;
}

void DArchiveManager::destory_instance()
{
    Q_D(DArchiveManager);
    SAFE_DELETE_ELE(d->instance)
}

bool DArchiveManager::createArchive(const QList<MgrFileEntry> &files, const QString &strDestination, const MgrCompressOptions &stOptions, ArchivePluginType eType)
{
    Q_D(DArchiveManager);
    if (d->pInterface != nullptr) {
        delete d->pInterface;
        d->pInterface = nullptr;
    }

    d->pTempInterface = UiTools::createInterface(strDestination, true, (UiTools::AssignPluginType)eType);

    if (d->pTempInterface) {
        CreateJob *pCreateJob = new CreateJob(*((QList<FileEntry> *)&files), d->pTempInterface, *((CompressOptions *)&stOptions), this);

        connect(pCreateJob, &CreateJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
        connect(pCreateJob, &CreateJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pCreateJob, &CreateJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

        d->pArchiveJob = pCreateJob;
        pCreateJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::loadArchive(const QString &strArchiveFullPath, ArchivePluginType eType)
{
    Q_D(DArchiveManager);
    if (d->pInterface != nullptr) {
        delete d->pInterface;
        d->pInterface = nullptr;
    }

    d->pInterface = UiTools::createInterface(strArchiveFullPath, false, (UiTools::AssignPluginType)eType);

    if (d->pInterface) {
        LoadJob *pLoadJob = new LoadJob(d->pInterface);

        connect(pLoadJob, &LoadJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);

        d->pArchiveJob = pLoadJob;
        pLoadJob->start();

        return true;
    }
    return false;
}

DArchiveManager::MgrArchiveData *DArchiveManager::archiveData()
{
    return (MgrArchiveData *)&(DataManager::get_instance().archiveData());
}

bool DArchiveManager::addFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listAddEntry, const MgrCompressOptions &stOptions)
{
    Q_D(DArchiveManager);
    d->pTempInterface = UiTools::createInterface(strArchiveFullPath, true);

    if (d->pTempInterface) {
        AddJob *pAddJob = new AddJob(*(QList<FileEntry> *)&listAddEntry, d->pTempInterface, *(CompressOptions *)&stOptions);

        connect(pAddJob, &AddJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
        connect(pAddJob, &AddJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pAddJob, &AddJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

        d->pArchiveJob = pAddJob;
        pAddJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::extractFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &files, const MgrExtractionOptions &stOptions, ArchivePluginType eType)
{
    Q_D(DArchiveManager);
    if (nullptr == d->pInterface) {
        d->pInterface = UiTools::createInterface(strArchiveFullPath, false, (UiTools::AssignPluginType)eType);
    }

    if (d->pInterface) {
        if (!stOptions.bTar_7z) {
            ExtractJob *pExtractJob = new ExtractJob(*(QList<FileEntry> *)&files, d->pInterface, *(ExtractionOptions *)&stOptions);

            connect(pExtractJob, &ExtractJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
            connect(pExtractJob, &ExtractJob::signalprogress, this, &DArchiveManager::signalprogress);
            connect(pExtractJob, &ExtractJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);
            connect(pExtractJob, &ExtractJob::signalFileWriteErrorName, this, &DArchiveManager::signalFileWriteErrorName);

            d->pArchiveJob = pExtractJob;
            pExtractJob->start();

            return pExtractJob->errorcode;
        } else {
            StepExtractJob *pStepExtractJob = new StepExtractJob(strArchiveFullPath, *(ExtractionOptions *)&stOptions);
            d->pArchiveJob = pStepExtractJob;

            connect(pStepExtractJob, &StepExtractJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
            connect(pStepExtractJob, &StepExtractJob::signalprogress, this, &DArchiveManager::signalprogress);
            connect(pStepExtractJob, &StepExtractJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

            pStepExtractJob->start();
            return true;
        }
    }

    // 发送结束信号
    emit signalJobFinished(DArchiveManager::JT_Extract, PFT_Error, ET_PluginError);
    return false;
}

bool DArchiveManager::extractFiles2Path(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listSelEntry, const MgrExtractionOptions &stOptions)
{
    Q_D(DArchiveManager);
    if (nullptr == d->pInterface) {
        d->pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (d->pInterface) {
        ExtractJob *pExtractJob = new ExtractJob(*(QList<FileEntry> *)&listSelEntry, d->pInterface, *(ExtractionOptions *)&stOptions);

        connect(pExtractJob, &ExtractJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
        connect(pExtractJob, &ExtractJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pExtractJob, &ExtractJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

        d->pArchiveJob = pExtractJob;
        pExtractJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::deleteFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listSelEntry)
{
    Q_D(DArchiveManager);
    if (nullptr == d->pInterface) {
        d->pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (d->pInterface) {
        DeleteJob *pDeleteJob = new DeleteJob(*(QList<FileEntry> *)&listSelEntry, d->pInterface);

        connect(pDeleteJob, &DeleteJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
        connect(pDeleteJob, &DeleteJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pDeleteJob, &DeleteJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

        d->pArchiveJob = pDeleteJob;
        pDeleteJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::renameFiles(const QString &strArchiveFullPath, const QList<MgrFileEntry> &listSelEntry)
{
    Q_D(DArchiveManager);
    if (nullptr == d->pInterface) {
        d->pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (d->pInterface) {
        RenameJob *pRenameJob = new RenameJob(*(QList<FileEntry> *)&listSelEntry, d->pInterface);

        connect(pRenameJob, &RenameJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
        connect(pRenameJob, &RenameJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pRenameJob, &RenameJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

        d->pArchiveJob = pRenameJob;
        pRenameJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath /*, bool bAutoCreatDir*/)
{
    Q_D(DArchiveManager);
    BatchExtractJob *pBatchExtractJob = new BatchExtractJob();
    pBatchExtractJob->setExtractPath(strTargetPath /*, bAutoCreatDir*/);

    if (pBatchExtractJob->setArchiveFiles(listFiles)) {
        connect(pBatchExtractJob, &BatchExtractJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
        connect(pBatchExtractJob, &BatchExtractJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pBatchExtractJob, &BatchExtractJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);
        connect(pBatchExtractJob, &BatchExtractJob::signalCurArchiveName, this, &DArchiveManager::signalCurArchiveName);

        d->pArchiveJob = pBatchExtractJob;
        pBatchExtractJob->start();

        return true;
    }

    SAFE_DELETE_ELE(pBatchExtractJob);
    return false;
}

bool DArchiveManager::openFile(const QString &strArchiveFullPath, const MgrFileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram)
{
    Q_D(DArchiveManager);
    if (nullptr == d->pInterface) {
        d->pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (d->pInterface) {
        OpenJob *pOpenJob = new OpenJob(*(FileEntry *)&stEntry, strTempExtractPath, strProgram, d->pInterface);

        connect(pOpenJob, &OpenJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);

        d->pArchiveJob = pOpenJob;
        pOpenJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::updateArchiveCacheData(const MgrUpdateOptions &stOptions)
{
    Q_D(DArchiveManager);
    if (d->pInterface) {
        UpdateJob *pUpdateJob = new UpdateJob(*(UpdateOptions *)&stOptions, d->pInterface);

        connect(pUpdateJob, &UpdateJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);

        d->pArchiveJob = pUpdateJob;
        pUpdateJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::updateArchiveComment(const QString &strArchiveFullPath, const QString &strComment)
{
    Q_D(DArchiveManager);
    ReadOnlyArchiveInterface *pInterface = UiTools::createInterface(strArchiveFullPath, true, UiTools::APT_Libzip);
    if (pInterface) {
        CommentJob *pCommentJob = new CommentJob(strComment, pInterface);

        connect(pCommentJob, &CommentJob::signalprogress, this, &DArchiveManager::signalprogress);
        connect(pCommentJob, &CommentJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);

        d->pArchiveJob = pCommentJob;
        pCommentJob->start();

        return true;
    }

    return false;
}

bool DArchiveManager::convertArchive(const QString &strOriginalArchiveFullPath, const QString &strTargetFullPath, const QString &strNewArchiveFullPath)
{
    Q_D(DArchiveManager);
    ConvertJob *pConvertJob = new ConvertJob(strOriginalArchiveFullPath, strTargetFullPath, strNewArchiveFullPath);
    d->pArchiveJob = pConvertJob;

    connect(pConvertJob, &ConvertJob::signalJobFinshed, d, &DArchiveManagerPrivate::slotJobFinished);
    connect(pConvertJob, &ConvertJob::signalprogress, this, &DArchiveManager::signalprogress);
    connect(pConvertJob, &ConvertJob::signalCurFileName, this, &DArchiveManager::signalCurFileName);

    pConvertJob->start();
    return true;
}

bool DArchiveManager::pauseOperation()
{
    Q_D(DArchiveManager);
    if (d->pArchiveJob) {
        d->pArchiveJob->doPause();

        return true;
    }

    return false;
}

bool DArchiveManager::continueOperation()
{
    Q_D(DArchiveManager);
    if (d->pArchiveJob) {
        d->pArchiveJob->doContinue();

        return true;
    }

    return false;
}

bool DArchiveManager::cancelOperation()
{
    Q_D(DArchiveManager);
    if (d->pArchiveJob) {
        d->pArchiveJob->kill();
        d->pArchiveJob->deleteLater();
        d->pArchiveJob = nullptr;

        return true;
    }

    return false;
}

QString DArchiveManager::getCurFilePassword()
{
    Q_D(DArchiveManager);
    if (d->pInterface) {
        return d->pInterface->getPassword();
    }

    return "";
}

bool DArchiveManager::currentStatus()
{
    Q_D(DArchiveManager);
    if (d->pArchiveJob) {
        return d->pArchiveJob->status();
    }

    return false;
}

void DArchiveManagerPrivate::slotJobFinished()
{
    Q_Q(DArchiveManager);
    if (pArchiveJob) {
        ArchiveJob::JobType eJobType = pArchiveJob->m_eJobType;
        PluginFinishType eFinishType = pArchiveJob->m_eFinishedType;
        ErrorType eErrorType = pArchiveJob->m_eErrorType;

        pArchiveJob->deleteLater();
        pArchiveJob = nullptr;

        emit q->signalJobFinished((DArchiveManager::ArchiveJobType)eJobType, (DArchiveManager::MgrPluginFinishType)eFinishType, (DArchiveManager::MgrErrorType)eErrorType);
    }

    SAFE_DELETE_ELE(pTempInterface);
}
