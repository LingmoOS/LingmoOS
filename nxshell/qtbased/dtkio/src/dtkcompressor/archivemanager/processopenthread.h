// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PROCESSOPENTHREAD_H
#define PROCESSOPENTHREAD_H

#include "archiveinterface.h"
#include "plugin.h"

#include <QThread>
#include <QTextCodec>

#include "dcompressornamespace.h"
DCOMPRESSOR_BEGIN_NAMESPACE

class ProcessOpenThread : public QThread
{
    Q_OBJECT
public:
    explicit ProcessOpenThread(QObject *parent = nullptr);
    void setProgramPath(const QString &strProgramPath);
    void setArguments(const QStringList &listArguments);

protected:
    void run() override;

private:
    QString m_strProgramPath;
    QStringList m_listArguments;
};

DCOMPRESSOR_END_NAMESPACE
#endif   // UITOOLS_H
