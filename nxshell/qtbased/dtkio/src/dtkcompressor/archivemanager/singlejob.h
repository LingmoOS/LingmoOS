// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SINGLEJOB_H
#define SINGLEJOB_H

#include "archivejob.h"
#include "archiveinterface.h"
#include "commonstruct.h"
#include "queries.h"
#include "dcompressornamespace.h"

#include <QTextCodec>
#include <QThread>
#include <QElapsedTimer>

DCOMPRESSOR_BEGIN_NAMESPACE

class SingleJob;

class SingleJobThread : public QThread
{
    Q_OBJECT
public:
    SingleJobThread(SingleJob *job, QObject *parent = nullptr)
        : QThread(parent), q(job)
    {
    }

    /**
     * @brief run
     */
    void run() override;

private:
    SingleJob *q;
};

class SingleJob : public ArchiveJob
{
    Q_OBJECT
public:
    explicit SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~SingleJob() override;

    virtual void doWork() = 0;

    void start() override;

    void doPause() override;

    void doContinue() override;

    bool status() override;

    SingleJobThread *getdptr();

protected:
    void initConnections();
    bool doKill() override;

protected Q_SLOTS:

    void slotFinished(PluginFinishType eType);

protected:
    ReadOnlyArchiveInterface *m_pInterface;
    SingleJobThread *const d;
    QElapsedTimer jobTimer;
};

class LoadJob : public SingleJob
{
    Q_OBJECT
public:
    explicit LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~LoadJob() override;

    void doWork() override;

private:
};

class AddJob : public SingleJob
{
    Q_OBJECT
public:
    explicit AddJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent = nullptr);
    ~AddJob() override;

    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;
    QString m_strDestination;
    CompressOptions m_stCompressOptions;
};

class CreateJob : public SingleJob
{
    Q_OBJECT
public:
    explicit CreateJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent = nullptr);
    ~CreateJob() override;

    void doWork() override;

protected:
    bool doKill() override;

private:
    void cleanCompressFileCancel();

private:
    QList<FileEntry> m_vecFiles;
    CompressOptions m_stCompressOptions;
};

class ExtractJob : public SingleJob
{
    Q_OBJECT
public:
    explicit ExtractJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const ExtractionOptions &options, QObject *parent = nullptr);
    ~ExtractJob() override;

    void doWork() override;

    bool errorcode = true;

private:
    QList<FileEntry> m_vecFiles;
    ExtractionOptions m_stExtractionOptions;
};

class DeleteJob : public SingleJob
{
    Q_OBJECT
public:
    explicit DeleteJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~DeleteJob() override;

    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;
};

class RenameJob : public SingleJob
{
    Q_OBJECT
public:
    explicit RenameJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~RenameJob() override;

    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;
};

class OpenJob : public SingleJob
{
    Q_OBJECT
public:
    explicit OpenJob(const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~OpenJob() override;

    void doWork() override;
    QString getProgramPathByExec(const QString &strExec);

protected Q_SLOTS:

    void slotFinished(PluginFinishType eType);

private:
    FileEntry m_stEntry;
    QString m_strTempExtractPath;
    QString m_strProgram;
};

class UpdateJob : public SingleJob
{
    Q_OBJECT
public:
    explicit UpdateJob(const UpdateOptions &options, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~UpdateJob() override;

    void start() override;

    void doWork() override;

private:
    UpdateOptions m_stOptions;
};

class CommentJob : public SingleJob
{
    Q_OBJECT
public:
    explicit CommentJob(const QString &strComment, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~CommentJob() override;

    void doWork() override;

private:
    QString m_strComment;
};

class ComplexJob : public ArchiveJob
{
    Q_OBJECT
public:
    explicit ComplexJob(const QString strOriginalArchiveFullPath, QObject *parent = nullptr);
    ~ComplexJob() override;

    virtual void start() override = 0;

    void doPause() override;

    void doContinue() override;

    bool doKill() override;

protected Q_SLOTS:

    void slotHandleSingleJobProgress(double dPercentage);

    void slotHandleSingleJobCurFileName(const QString &strName);

    virtual void slotHandleExtractFinished() = 0;

protected:
    ReadOnlyArchiveInterface *m_pIface = nullptr;
    QString m_strOriginalArchiveFullPath;
    int m_iStepNo = 0;
};

class ConvertJob : public ComplexJob
{
    Q_OBJECT
public:
    explicit ConvertJob(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, const QString strNewArchiveFullPath, QObject *parent = nullptr);
    ~ConvertJob() override;

    void start() override;

private Q_SLOTS:

    void slotHandleExtractFinished() override;

private:
    ExtractJob *m_pExtractJob = nullptr;
    CreateJob *m_pCreateJob = nullptr;
    QString m_strTargetFullPath;
    QString m_strNewArchiveFullPath;
};

class StepExtractJob : public ComplexJob
{
    Q_OBJECT
public:
    explicit StepExtractJob(const QString strOriginalArchiveFullPath, const ExtractionOptions &stOptions, QObject *parent = nullptr);
    ~StepExtractJob() override;

    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:

    void slotHandleExtractFinished() override;

private:
    ExtractJob *m_pExtractJob = nullptr;
    ExtractJob *m_pExtractJob2 = nullptr;
    QString m_strTempFilePath;
    ExtractionOptions m_stExtractionOptions;
};
DCOMPRESSOR_END_NAMESPACE
#endif
