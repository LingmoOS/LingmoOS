// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATCHJOB_H
#define BATCHJOB_H

#include "archivejob.h"
#include "dcompressornamespace.h"
#include <QFileInfo>
DCOMPRESSOR_BEGIN_NAMESPACE
class BatchJob : public ArchiveJob
{
    Q_OBJECT
public:
    explicit BatchJob(QObject *parent = nullptr);
    ~BatchJob() override;

protected:
    virtual bool addSubjob(ArchiveJob *job);

    virtual bool removeSubjob(ArchiveJob *job);

    bool hasSubjobs() const;
    const QList<ArchiveJob *> &subjobs() const;
    void clearSubjobs();
    void doPause() override;
    void doContinue() override;
    bool status() override;

protected:
    ArchiveJob *m_pCurJob = nullptr;

private:
    QList<ArchiveJob *> m_listSubjobs;
};

class BatchExtractJob : public BatchJob
{
    Q_OBJECT
public:
    explicit BatchExtractJob(QObject *parent = nullptr);
    ~BatchExtractJob() override;

    void start() override;
    void setExtractPath(const QString &strPath);
    bool setArchiveFiles(const QStringList &listFile);

private:
    bool addExtractItem(const QFileInfo &fileInfo);

Q_SIGNALS:
    void signalCurArchiveName(const QString &strArchiveName);

private Q_SLOTS:
    void slotHandleSingleJobProgress(double dPercentage);
    void slotHandleSingleJobCurFileName(const QString &strName);
    void slotHandleSingleJobFinished();

private:
    QStringList m_listFiles;
    qint64 m_qBatchTotalSize = 0;
    QString m_strExtractPath;
    int m_iArchiveCount = 0;
    int m_iCurArchiveIndex = 0;
    double m_dLastPercentage = 0;
};
DCOMPRESSOR_END_NAMESPACE
#endif   // BATCHJOB_H
