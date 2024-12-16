// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "singlejob.h"
#include "processopenthread.h"
//#include "openwithdialog.h"
#include "datamanager.h"
#include "uitools.h"
#include <QCoreApplication>

#include <linux/limits.h>
#include <QUuid>
#include <QThread>
#include <QDebug>
#include <QDir>
DCOMPRESSOR_USE_NAMESPACE
void SingleJobThread::run()
{
    q->doWork();
}

SingleJob::SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : ArchiveJob(parent), m_pInterface(pInterface), d(new SingleJobThread(this))
{
}

SingleJob::~SingleJob()
{
    if (d->isRunning()) {
        d->quit();
        d->wait();
    }

    delete d;
}

void SingleJob::start()
{
    jobTimer.start();

    if (nullptr == m_pInterface) {
        slotFinished(PFT_Error);
        return;
    }

    if (m_pInterface->waitForFinished()) {
        doWork();
    } else {
        d->start();
    }
}

void SingleJob::doPause()
{
    if (m_pInterface) {
        m_pInterface->pauseOperation();
    }
}

void SingleJob::doContinue()
{
    if (m_pInterface) {
        m_pInterface->continueOperation();
    }
}

bool SingleJob::status()
{
    if (m_pInterface) {
        return m_pInterface->status();
    }

    return false;
}

SingleJobThread *SingleJob::getdptr()
{
    return d;
}

bool SingleJob::doKill()
{
    if (nullptr == m_pInterface) {
        return false;
    }

    const bool killed = m_pInterface->doKill();
    if (killed) {
        return true;
    }

    if (d->isRunning()) {   //Returns true if the thread is running
        qInfo() << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption();
        d->wait(1000);
    }

    return true;
}

void SingleJob::initConnections()
{
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &SingleJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalprogress, this, &SingleJob::signalprogress, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalCurFileName, this, &SingleJob::signalCurFileName, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFileWriteErrorName, this, &SingleJob::signalFileWriteErrorName, Qt::ConnectionType::UniqueConnection);
}

void SingleJob::slotFinished(PluginFinishType eType)
{
    qInfo() << "Job finished, result:" << eType << ", time:" << jobTimer.elapsed() << "ms";
    m_eFinishedType = eType;

    if (m_pInterface)
        m_eErrorType = m_pInterface->errorType();

    emit signalJobFinshed();
}

LoadJob::LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
{
    m_eJobType = JT_Load;
    initConnections();
}

LoadJob::~LoadJob()
{
}

void LoadJob::doWork()
{
    if (m_pInterface) {
        PluginFinishType eType = m_pInterface->list();

        if (!(m_pInterface->waitForFinished())) {
            slotFinished(eType);
        }
    }
}

AddJob::AddJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent)
    : SingleJob(pInterface, parent), m_vecFiles(files), m_stCompressOptions(options)
{
    initConnections();
    m_eJobType = JT_Add;
}

AddJob::~AddJob()
{
}

void AddJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (nullptr == pWriteInterface) {
        return;
    }

    PluginFinishType eType = pWriteInterface->addFiles(m_vecFiles, m_stCompressOptions);

    if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

CreateJob::CreateJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent)
    : SingleJob(pInterface, parent), m_vecFiles(files), m_stCompressOptions(options)
{
    initConnections();
    m_eJobType = JT_Create;
}

CreateJob::~CreateJob()
{
}

void CreateJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface == nullptr) {
        return;
    }

    PluginFinishType eType = pWriteInterface->addFiles(m_vecFiles, m_stCompressOptions);

    if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

bool CreateJob::doKill()
{
    if (nullptr == m_pInterface) {
        return false;
    }

    const bool killed = m_pInterface->doKill();
    if (killed) {
        cleanCompressFileCancel();
        return true;
    }

    if (d->isRunning()) {   //Returns true if the thread is running
        qInfo() << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption();
        d->wait(1000);
    }
    cleanCompressFileCancel();
    return true;
}

void CreateJob::cleanCompressFileCancel()
{
    if (m_stCompressOptions.bSplit) {
        QFileInfo file(dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface)->getArchiveName());
        QStringList nameFilters;
        nameFilters << file.fileName() + ".0*";
        QDir dir(file.path());
        QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

        foreach (QFileInfo fi, files) {
            QFile fiRemove(fi.filePath());
            if (fiRemove.exists()) {
                qInfo() << "FileCancel delete:" << fiRemove.fileName();
                fiRemove.remove();
            }
        }
    } else {
        QFile fiRemove(dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface)->getArchiveName());
        if (fiRemove.exists()) {
            qInfo() << "FileCancel delete:" << fiRemove.fileName();
            fiRemove.remove();
        }
    }
}

ExtractJob::ExtractJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const ExtractionOptions &options, QObject *parent)
    : SingleJob(pInterface, parent), m_vecFiles(files), m_stExtractionOptions(options)
{
    initConnections();
    m_eJobType = JT_Extract;
}

ExtractJob::~ExtractJob()
{
}

void ExtractJob::doWork()
{
    if (m_pInterface) {
        PluginFinishType eType = m_pInterface->extractFiles(m_vecFiles, m_stExtractionOptions);

        if (!(m_pInterface->waitForFinished())) {
            slotFinished(eType);
        } else {
            if (PFT_Error == eType) {
                errorcode = false;
            }
        }
    }
}

DeleteJob::DeleteJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent), m_vecFiles(files)
{
    m_eJobType = JT_Delete;
    initConnections();
    m_eJobType = JT_Delete;
}

DeleteJob::~DeleteJob()
{
}

void DeleteJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (nullptr == pWriteInterface) {
        return;
    }

    PluginFinishType eType = pWriteInterface->deleteFiles(m_vecFiles);

    if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

RenameJob::RenameJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent), m_vecFiles(files)
{
    initConnections();
    m_eJobType = JT_Rename;
}

RenameJob::~RenameJob()
{
}

void RenameJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (nullptr == pWriteInterface) {
        return;
    }

    PluginFinishType eType = pWriteInterface->renameFiles(m_vecFiles);

    if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

OpenJob::OpenJob(const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent), m_stEntry(stEntry), m_strTempExtractPath(strTempExtractPath), m_strProgram(strProgram)
{
    m_eJobType = JT_Open;
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &OpenJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    //    connect(m_pInterface, &ReadOnlyArchiveInterface::signalQuery, this, &SingleJob::signalQuery, Qt::ConnectionType::AutoConnection);
}

OpenJob::~OpenJob()
{
}

void OpenJob::doWork()
{
    if (m_pInterface) {
        ExtractionOptions options;
        options.bOpen = true;
        options.strTargetPath = m_strTempExtractPath;
        if (m_stEntry.strFullPath.contains(QDir::separator())) {
            int iIndex = m_stEntry.strFullPath.lastIndexOf(QDir::separator());
            if (iIndex > 0)
                options.strDestination = m_stEntry.strFullPath.left(iIndex + 1);
        }
        options.qSize = m_stEntry.qSize;

        PluginFinishType eType = m_pInterface->extractFiles(QList<FileEntry>() << m_stEntry, options);

        if (!(m_pInterface->waitForFinished())) {
            slotFinished(eType);
        }
    }
}

void OpenJob::slotFinished(PluginFinishType eType)
{
    if (PFT_Nomral == eType) {
        QString name = m_stEntry.strFileName;

        QString strTempFileName = m_stEntry.strFileName;
        if (NAME_MAX < QString(strTempFileName).toLocal8Bit().length() && !strTempFileName.endsWith(QDir::separator())) {
            QString strTemp = strTempFileName.left(60);
            name = strTemp + QString("(%1)").arg(1, 3, 10, QChar('0')) + "." + QFileInfo(strTempFileName).completeSuffix();
        }

        if (name.contains("%")) {
            name = m_strTempExtractPath + QDir::separator() + name.replace("%", "1");
            if (!QFile::link(m_stEntry.strFileName, name)) {
                return;
            }
        } else {
            name = m_strTempExtractPath + QDir::separator() + name;
        }

        ProcessOpenThread *p = new ProcessOpenThread;
        p->setProgramPath(getProgramPathByExec(m_strProgram));
        p->setArguments(QStringList() << name);
        p->start();
    }

    SingleJob::slotFinished(eType);
}

QString OpenJob::getProgramPathByExec(const QString &strExec)
{
    QString strProgramPath;

    if (strExec.isEmpty()) {
        strProgramPath = QStandardPaths::findExecutable("xdg-open");
    } else {
        QStringList list = strExec.split(" ");
        if (strExec.startsWith(QDir::separator())) {
            strProgramPath = list[0];
        } else {
            strProgramPath = QStandardPaths::findExecutable(list[0]);
        }
    }

    return strProgramPath;
}

UpdateJob::UpdateJob(const UpdateOptions &options, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent), m_stOptions(options)
{
    m_eJobType = JT_Update;
    //connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &UpdateJob::slotFinished, Qt::ConnectionType::UniqueConnection);
}

UpdateJob::~UpdateJob()
{
}

void UpdateJob::start()
{
    jobTimer.start();

    if (nullptr == m_pInterface) {
        slotFinished(PFT_Error);
        return;
    }

    d->start();
}

void UpdateJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        PluginFinishType eType = pWriteInterface->updateArchiveData(m_stOptions);

        slotFinished(eType);
    }
}

CommentJob::CommentJob(const QString &strComment, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent), m_strComment(strComment)
{
    m_eJobType = JT_Comment;

    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &CommentJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalprogress, this, &CommentJob::signalprogress, Qt::ConnectionType::UniqueConnection);
}

CommentJob::~CommentJob()
{
    if (m_pInterface) {
        delete m_pInterface;
        m_pInterface = nullptr;
    }
}

void CommentJob::doWork()
{
    qInfo() << "Adding comment";

    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        PluginFinishType eType = pWriteInterface->addComment(m_strComment);
        if (!(pWriteInterface->waitForFinished())) {
            slotFinished(eType);
        }
    }
}

ComplexJob::ComplexJob(const QString strOriginalArchiveFullPath, QObject *parent)
    : ArchiveJob(parent), m_strOriginalArchiveFullPath(strOriginalArchiveFullPath)
{
}

ComplexJob::~ComplexJob()
{
    if (m_pIface) {
        delete m_pIface;
        m_pIface = nullptr;
    }
}

void ComplexJob::doPause()
{
    if (m_pIface) {
        m_pIface->pauseOperation();
    }
}

void ComplexJob::doContinue()
{
    if (m_pIface) {
        m_pIface->continueOperation();
    }
}

bool ComplexJob::doKill()
{
    return m_pIface->doKill();
}

void ComplexJob::slotHandleSingleJobProgress(double dPercentage)
{
    if (0 == m_iStepNo) {
        emit signalprogress(dPercentage * 0.3);
    } else {
        emit signalprogress(30 + dPercentage * 0.7);
    }
}

void ComplexJob::slotHandleSingleJobCurFileName(const QString &strName)
{
    emit signalCurFileName(strName);
}

ConvertJob::ConvertJob(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, const QString strNewArchiveFullPath, QObject *parent)
    : ComplexJob(strOriginalArchiveFullPath, parent), m_strTargetFullPath(strTargetFullPath), m_strNewArchiveFullPath(strNewArchiveFullPath)
{
    m_eJobType = JT_Convert;
}

ConvertJob::~ConvertJob()
{
    SAFE_DELETE_ELE(m_pCreateJob)
    SAFE_DELETE_ELE(m_pExtractJob);
}

void ConvertJob::start()
{
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath);

    if (pIface) {
        qInfo() << "format transformation start extract";
        m_pIface = pIface;
        m_iStepNo = 0;

        QFileInfo file(m_strOriginalArchiveFullPath);
        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strTargetFullPath;
        stOptions.qComressSize = file.size();
        stOptions.bAllExtract = true;

        m_pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
        connect(m_pExtractJob, &ExtractJob::signalprogress, this, &ConvertJob::slotHandleSingleJobProgress);
        connect(m_pExtractJob, &ExtractJob::signalCurFileName, this, &ConvertJob::slotHandleSingleJobCurFileName);
        connect(m_pExtractJob, &ExtractJob::signalJobFinshed, this, &ConvertJob::slotHandleExtractFinished);

        m_pExtractJob->doWork();
    }
}

void ConvertJob::slotHandleExtractFinished()
{
    if (m_pExtractJob) {
        m_eFinishedType = m_pExtractJob->m_eFinishedType;
        m_eErrorType = m_pExtractJob->m_eErrorType;

        switch (m_eFinishedType) {
        case PFT_Nomral: {
            qInfo() << "format transformation start add";
            m_iStepNo = 1;

            ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strNewArchiveFullPath, true);

            if (pIface) {
                SAFE_DELETE_ELE(m_pIface);
                m_pIface = pIface;
                QList<FileEntry> listEntry;
                QDir dir(m_strTargetFullPath);
                QFileInfoList fileList = dir.entryInfoList(QDir::AllEntries | QDir::System
                                                           | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                                           | QDir::Hidden);

                foreach (QFileInfo strFile, fileList) {
                    FileEntry stFileEntry;
                    stFileEntry.strFullPath = strFile.filePath();
                    listEntry.push_back(stFileEntry);
                }

                CompressOptions options;
                options.iCompressionLevel = 3;
                options.qTotalSize = DataManager::get_instance().archiveData().qSize;

                m_pCreateJob = new CreateJob(listEntry, pIface, options);
                connect(m_pCreateJob, &CreateJob::signalprogress, this, &ConvertJob::slotHandleSingleJobProgress);
                connect(m_pCreateJob, &CreateJob::signalCurFileName, this, &ConvertJob::slotHandleSingleJobCurFileName);
                connect(m_pCreateJob, &CreateJob::signalJobFinshed, this, &ConvertJob::signalJobFinshed);

                m_pCreateJob->doWork();
            }
        } break;

        case PFT_Cancel: {
            qInfo() << "cancel format transformation";
            emit signalJobFinshed();
        } break;

        case PFT_Error: {
            qInfo() << "format transformation error";
            emit signalJobFinshed();
        } break;
        }
    }
}

StepExtractJob::StepExtractJob(const QString strOriginalArchiveFullPath, const ExtractionOptions &stOptions, QObject *parent)
    : ComplexJob(strOriginalArchiveFullPath, parent), m_stExtractionOptions(stOptions)
{
    m_eJobType = JT_StepExtract;
}

StepExtractJob::~StepExtractJob()
{
}

void StepExtractJob::start()
{
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath, false);

    if (nullptr != pIface) {
        qInfo() << "StepExtractJob:  tar.7z to tar";
        m_pIface = pIface;
        m_iStepNo = 0;

        QFileInfo file(m_strOriginalArchiveFullPath);
        QString strProcessID = QString::number(QCoreApplication::applicationPid());
        m_strTempFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                + QDir::separator() + strProcessID + QDir::separator()
                + QUuid::createUuid().toString(QUuid::Id128);

        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strTempFilePath;
        stOptions.qComressSize = file.size();
        stOptions.bAllExtract = true;

        m_pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions, this);
        connect(m_pExtractJob, &ExtractJob::signalprogress, this, &StepExtractJob::slotHandleSingleJobProgress);
        connect(m_pExtractJob, &ExtractJob::signalCurFileName, this, &StepExtractJob::slotHandleSingleJobCurFileName);
        connect(m_pExtractJob, &ExtractJob::signalJobFinshed, this, &StepExtractJob::slotHandleExtractFinished);

        m_pExtractJob->start();
    }
}

void StepExtractJob::slotHandleExtractFinished()
{
    if (nullptr != m_pExtractJob) {
        m_eFinishedType = m_pExtractJob->m_eFinishedType;
        m_eErrorType = m_pExtractJob->m_eErrorType;

        switch (m_eFinishedType) {
        case PFT_Nomral: {
            QDir dir(m_strTempFilePath);
            if (!dir.exists()) {
                return;
            }
            QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::System
                                                   | QDir::NoDotAndDotDot | QDir::Hidden);

            if (1 == list.count()
                && list.at(0).filePath().endsWith(".tar")
                && determineMimeType(list.at(0).filePath()).name() == QLatin1String("application/x-tar")) {
                QFileInfo fileInfo = list.at(0);

                ReadOnlyArchiveInterface *pIface = UiTools::createInterface(fileInfo.absoluteFilePath(), false);

                if (nullptr != pIface) {
                    qInfo() << "StepExtractJob: start extract tar";
                    SAFE_DELETE_ELE(m_pIface);
                    m_pIface = pIface;
                    m_iStepNo = 1;

                    ExtractionOptions stOptions = m_stExtractionOptions;
                    stOptions.qComressSize = fileInfo.size();
                    stOptions.qSize = fileInfo.size();
                    stOptions.bAllExtract = true;
                    stOptions.bTar_7z = false;

                    m_pExtractJob2 = new ExtractJob(QList<FileEntry>(), pIface, stOptions, this);
                    connect(m_pExtractJob2, &ExtractJob::signalprogress, this, &StepExtractJob::slotHandleSingleJobProgress);
                    connect(m_pExtractJob2, &ExtractJob::signalCurFileName, this, &StepExtractJob::slotHandleSingleJobCurFileName);
                    connect(m_pExtractJob2, &ExtractJob::signalJobFinshed, this, &StepExtractJob::signalJobFinshed);

                    m_pExtractJob2->start();
                }
            } else {
                ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath, false);

                if (nullptr != pIface) {
                    qInfo() << "StepExtractJob: Start decompressing the original file directly";
                    SAFE_DELETE_ELE(m_pIface);
                    m_pIface = pIface;
                    m_iStepNo = 1;

                    ExtractionOptions stOptions = m_stExtractionOptions;
                    stOptions.password = DataManager::get_instance().archiveData().strPassword;
                    stOptions.bAllExtract = true;
                    stOptions.bTar_7z = false;

                    m_pExtractJob2 = new ExtractJob(QList<FileEntry>(), pIface, stOptions, this);
                    connect(m_pExtractJob2, &ExtractJob::signalprogress, this, &StepExtractJob::slotHandleSingleJobProgress);
                    connect(m_pExtractJob2, &ExtractJob::signalCurFileName, this, &StepExtractJob::slotHandleSingleJobCurFileName);
                    connect(m_pExtractJob2, &ExtractJob::signalJobFinshed, this, &StepExtractJob::signalJobFinshed);

                    m_pExtractJob2->start();
                }
            }
        } break;
        case PFT_Cancel: {
            emit signalJobFinshed();
        } break;
        case PFT_Error: {
            emit signalJobFinshed();
        } break;
        }
    }
}

bool StepExtractJob::doKill()
{
    const bool killed = m_pIface->doKill();
    if (killed) {
        return true;
    }
    if (m_pExtractJob2->getdptr()->isRunning()) {   //Returns true if the thread is running
        qInfo() << "Requesting graceful thread interruption, will abort in one second otherwise.";
        m_pExtractJob2->getdptr()->requestInterruption();
        m_pExtractJob2->getdptr()->wait(1000);
    }

    return true;
}
