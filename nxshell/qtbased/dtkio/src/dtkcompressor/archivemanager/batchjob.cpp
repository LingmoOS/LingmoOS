// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "batchjob.h"
#include "uitools.h"
#include "singlejob.h"

#include <QDir>
#include <QDebug>
DCOMPRESSOR_USE_NAMESPACE
BatchJob::BatchJob(QObject *parent)
    : ArchiveJob(parent)
{
}

BatchJob::~BatchJob()
{
    clearSubjobs();
}

bool BatchJob::addSubjob(ArchiveJob *job)
{
    if (nullptr == job || m_listSubjobs.contains(job)) {
        return false;
    }

    job->setParent(this);
    m_listSubjobs.append(job);
    return true;
}

bool BatchJob::removeSubjob(ArchiveJob *job)
{
    if (m_listSubjobs.removeAll(job) > 0) {
        job->setParent(nullptr);
        delete job;
        return true;
    }

    return false;
}

bool BatchJob::hasSubjobs() const
{
    return !m_listSubjobs.isEmpty();
}

const QList<ArchiveJob *> &BatchJob::subjobs() const
{
    return m_listSubjobs;
}

void BatchJob::clearSubjobs()
{
    Q_FOREACH (ArchiveJob *job, m_listSubjobs) {
        job->setParent(nullptr);
        delete job;
    }

    m_listSubjobs.clear();
}

void BatchJob::doPause()
{
    if (m_pCurJob) {
        m_pCurJob->doPause();
    }
}

void BatchJob::doContinue()
{
    if (m_pCurJob) {
        m_pCurJob->doContinue();
    }
}

bool BatchJob::status()
{
    if (m_pCurJob) {
        return m_pCurJob->status();
    }

    return false;
}

BatchExtractJob::BatchExtractJob(QObject *parent)
    : BatchJob(parent)
{
    m_eJobType = JT_BatchExtract;
}

BatchExtractJob::~BatchExtractJob()
{
}

void BatchExtractJob::start()
{
    if (subjobs().count() == 0) {
        return;
    }

    m_iCurArchiveIndex = 0;
    m_pCurJob = subjobs().at(0);
    m_pCurJob->start();
}

void BatchExtractJob::setExtractPath(const QString &strPath /*, bool bAutoCreatDir*/)
{
    m_strExtractPath = strPath;
}

bool BatchExtractJob::setArchiveFiles(const QStringList &listFile)
{
    m_listFiles = listFile;
    m_qBatchTotalSize = 0;
    m_iArchiveCount = m_listFiles.count();

    bool bResult = false;
    foreach (QString strFileName, listFile) {
        QFileInfo fileInfo(strFileName);
        if (addExtractItem(fileInfo)) {
            bResult = true;
            m_qBatchTotalSize += fileInfo.size();
        }
    }

    return bResult;
}

bool BatchExtractJob::addExtractItem(const QFileInfo &fileInfo)
{
    QString strName = fileInfo.filePath();
    UnCompressParameter::SplitType eType = UnCompressParameter::ST_No;
    UiTools::transSplitFileName(strName, eType);
    UiTools::AssignPluginType ePluginType = (UnCompressParameter::ST_Zip == eType) ? (UiTools::AssignPluginType::APT_Cli7z) : (UiTools::AssignPluginType::APT_Auto);
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(fileInfo.filePath(), false, ePluginType);

    if (pIface) {
        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strExtractPath;

        QString strpath = "";
        strpath = UiTools::handleFileName(fileInfo.filePath());

        stOptions.strTargetPath += QDir::separator() + strpath;
        stOptions.qComressSize = fileInfo.size();
        stOptions.bExistList = false;
        stOptions.bAllExtract = true;
        stOptions.bBatchExtract = true;
        if (strName.endsWith(QLatin1String(".tar.7z"))
            && determineMimeType(strName).name() == QLatin1String("application/x-7z-compressed")) {
            stOptions.bTar_7z = true;
            ePluginType = UiTools::AssignPluginType::APT_Cli7z;
        }

        pIface->setParent(this);
        if (!stOptions.bTar_7z) {
            ExtractJob *pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
            connect(pExtractJob, &ExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
            connect(pExtractJob, &ExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
            connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &BatchExtractJob::slotHandleSingleJobFinished);
            addSubjob(pExtractJob);
        } else {
            StepExtractJob *pStepExtractJob = new StepExtractJob(fileInfo.absoluteFilePath(), stOptions);
            connect(pStepExtractJob, &StepExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
            connect(pStepExtractJob, &StepExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
            connect(pStepExtractJob, &StepExtractJob::signalJobFinshed, this, &BatchExtractJob::slotHandleSingleJobFinished);
            addSubjob(pStepExtractJob);
        }
        return true;
    }

    return false;
}

void BatchExtractJob::slotHandleSingleJobProgress(double dPercentage)
{
    QFileInfo fileInfo(m_listFiles[m_iCurArchiveIndex]);
    qint64 qCurSize = fileInfo.size();
    double dProgress = double(qCurSize) * dPercentage / m_qBatchTotalSize + m_dLastPercentage;
    emit signalprogress(dProgress);
}

void BatchExtractJob::slotHandleSingleJobCurFileName(const QString &strName)
{
    emit signalCurFileName(strName);
}

void BatchExtractJob::slotHandleSingleJobFinished()
{
    if (m_pCurJob != nullptr) {
        if (PFT_Error == m_pCurJob->m_eFinishedType || PFT_Cancel == m_pCurJob->m_eFinishedType) {
            m_eJobType = m_pCurJob->m_eJobType;
            m_eFinishedType = m_pCurJob->m_eFinishedType;
            m_eErrorType = m_pCurJob->m_eErrorType;

            emit signalJobFinshed();
            return;
        }

        removeSubjob(m_pCurJob);

        if (!hasSubjobs()) {
            emit signalJobFinshed();
        } else {
            ++m_iCurArchiveIndex;
            QFileInfo curFileInfo(m_listFiles[m_iCurArchiveIndex]);
            m_dLastPercentage += double(curFileInfo.size()) / m_qBatchTotalSize * 100;
            emit signalCurArchiveName(curFileInfo.fileName());
            m_pCurJob = subjobs().at(0);
            m_pCurJob->start();
        }
    }
}
