// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "batchjob.h"
#include "uitools.h"
#include "singlejob.h"

#include <QDir>
#include <QDebug>

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
    // 调用子job暂停接口
    if (m_pCurJob) {
        m_pCurJob->doPause();
    }
}

void BatchJob::doContinue()
{
    // 调用子job继续接口
    if (m_pCurJob) {
        m_pCurJob->doContinue();
    }
}

bool BatchJob::status()
{
    // 调用子job继续接口
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

void BatchExtractJob::setExtractPath(const QString &strPath/*, bool bAutoCreatDir*/)
{
    m_strExtractPath = strPath;
//    m_bAutoCreatDir = bAutoCreatDir;
}

bool BatchExtractJob::setArchiveFiles(const QStringList &listFile)
{
    m_listFiles = listFile;
    m_qBatchTotalSize = 0;
    m_iArchiveCount = m_listFiles.count();

    bool bResult = false;
    // 创建解压元素
    foreach (QString strFileName, listFile) {
        QFileInfo fileInfo(strFileName);
        if (addExtractItem(fileInfo)) {
            bResult = true;
            // 计算压缩包总大小
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
    UiTools::AssignPluginType ePluginType = (UnCompressParameter::ST_Zip == eType) ?
                                            (UiTools::AssignPluginType::APT_Cli7z) : (UiTools::AssignPluginType::APT_Auto);
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(fileInfo.filePath(), false, ePluginType);

    if (pIface) {
        // 创建解压参数
        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strExtractPath;

        // 自动创建文件夹的名称（去除中间带.的文件夹名称）
        QString strpath = "";
        strpath = UiTools::handleFileName(fileInfo.filePath());
//        if (fileInfo.filePath().contains(".tar.")) {
//            strpath = strpath.remove(".tar"); // 类似tar.gz压缩文件，创建文件夹的时候移除.tar
//        } else if (fileInfo.filePath().contains(".7z.")) {
//            strpath = strpath.remove(".7z"); // 7z分卷文件，创建文件夹的时候移除.7z
//        } else if (fileInfo.filePath().contains(".part01.rar")) {
//            strpath = strpath.remove(".part01"); // tar分卷文件，创建文件夹的时候移除part01
//        } else if (fileInfo.filePath().contains(".part1.rar")) {
//            strpath = strpath.remove(".part1"); // rar分卷文件，创建文件夹的时候移除.part1
//        } else if (fileInfo.filePath().contains(".zip.")) {
//            strpath = strpath.remove(".zip"); // zip分卷文件，创建文件夹的时候移除.zip
//        }


        stOptions.strTargetPath += QDir::separator() + strpath; // 批量解压自动创建文件夹
        stOptions.qComressSize = fileInfo.size();
        stOptions.bExistList = false;
        stOptions.bAllExtract = true;
        stOptions.bBatchExtract = true;
        // tar.7z特殊处理，右键解压缩到当前文件夹使用cli7zplugin
        if (strName.endsWith(QLatin1String(".tar.7z"))
                && determineMimeType(strName).name() == QLatin1String("application/x-7z-compressed")) {
            stOptions.bTar_7z = true;
            ePluginType = UiTools::AssignPluginType::APT_Cli7z;
        }

        pIface->setParent(this);    // 跟随BatchExtractJob释放
        if (!stOptions.bTar_7z) {
            ExtractJob *pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
            connect(pExtractJob, &ExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
            connect(pExtractJob, &ExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
            connect(pExtractJob, &ExtractJob::signalQuery, this, &BatchExtractJob::signalQuery);
            connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &BatchExtractJob::slotHandleSingleJobFinished);
            addSubjob(pExtractJob);
        } else {
            StepExtractJob *pStepExtractJob = new StepExtractJob(fileInfo.absoluteFilePath(), stOptions);
            connect(pStepExtractJob, &StepExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
            connect(pStepExtractJob, &StepExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
            connect(pStepExtractJob, &StepExtractJob::signalQuery, this, &BatchExtractJob::signalQuery);
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
    qint64 qCurSize = fileInfo.size();      // 当前解压的压缩包大小
    double dProgress = double(qCurSize) * dPercentage /  m_qBatchTotalSize + m_dLastPercentage;
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
            // 获取结束结果
            m_eJobType = m_pCurJob->m_eJobType;
            m_eFinishedType = m_pCurJob->m_eFinishedType;
            m_eErrorType = m_pCurJob->m_eErrorType;

            // 子job错误或取消，发送结束信号
            emit signalJobFinshed();
            return;
        }

        // 移除当前job
        removeSubjob(m_pCurJob);

        if (!hasSubjobs()) {
            // 若没有子job，发送结束信号
            emit signalJobFinshed();
        } else {
            ++m_iCurArchiveIndex;
            QFileInfo curFileInfo(m_listFiles[m_iCurArchiveIndex]);

            // 还存在子job，执行子job操作

            m_dLastPercentage += double(curFileInfo.size()) / m_qBatchTotalSize * 100;
            emit signalCurArchiveName(curFileInfo.fileName());
            m_pCurJob = subjobs().at(0);
            m_pCurJob->start();
        }
    }
}
