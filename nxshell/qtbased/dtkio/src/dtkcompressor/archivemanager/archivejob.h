// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARCHIVEJOB_H
#define ARCHIVEJOB_H

#include "commonstruct.h"
#include "dcompressornamespace.h"
#include <QObject>

DCOMPRESSOR_BEGIN_NAMESPACE

class ArchiveJob : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveJob(QObject *parent = nullptr);
    ~ArchiveJob() override;
    enum JobType {
        JT_NoJob = 0,
        JT_Create,
        JT_Add,
        JT_Load,
        JT_Extract,
        JT_Delete,
        JT_Rename,
        JT_BatchExtract,
        JT_Open,
        JT_Update,
        JT_Comment,
        JT_Convert,
        JT_StepExtract,
    };

    virtual void start() = 0;

    virtual void doPause() = 0;

    virtual void doContinue() = 0;

    virtual void doCancel() {}

    virtual void kill();

    virtual bool status();

private:
    void finishJob();

protected:
    virtual bool doKill();

Q_SIGNALS:

    void signalJobFinshed();
    void signalprogress(double dPercentage);
    void signalCurFileName(const QString &strName);
    void signalFileWriteErrorName(const QString &strName);

public:
    JobType m_eJobType = JT_NoJob;
    PluginFinishType m_eFinishedType = PFT_Nomral;
    ErrorType m_eErrorType = ET_NoError;
};
DCOMPRESSOR_END_NAMESPACE
#endif   // ARCHIVEJOB_H
