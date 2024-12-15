// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPTICALDISCOPERATOR_P_H
#define DOPTICALDISCOPERATOR_P_H

#include "dopticaldiscoperator.h"

DBURN_BEGIN_NAMESPACE

class DOpticalDiscOperatorPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DOpticalDiscOperator)

public:
    DOpticalDiscOperatorPrivate(const QString &dev, DOpticalDiscOperator *qq);
    bool makeStageFiles(const QString &stagePath, QString *errMsg);

public Q_SLOTS:
    void onUDFEngineJobStatusChanged(JobStatus status, int progress);
    void onXorrisoEngineJobStatusChanged(JobStatus status, int progress, QString speed);

public:
    DOpticalDiscOperator *q_ptr;

    QObject *currentEngine { nullptr };
    QString curDev;
    int curSpeed { 0 };
    QString curVolName;
    double curPrecision { 2 + 1e-6 };
    QPair<QString, QString> files;   //!< @~english first: local disk path, second: optical disk path
};

DBURN_END_NAMESPACE

#endif   // DOPTICALDISCOPERATOR_P_H
