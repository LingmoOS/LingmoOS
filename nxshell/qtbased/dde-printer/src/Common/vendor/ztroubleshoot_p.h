// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZTROUBLESHOOT_P_H
#define ZTROUBLESHOOT_P_H

#include "ztroubleshoot.h"

class CheckCupsServer : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckCupsServer(QObject *parent = nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

class CheckDriver : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckDriver(const QString &printerName, QObject *parent = nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

class CheckConnected : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckConnected(const QString &printerName, QObject *parent = nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

class CheckAttributes : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckAttributes(const QString &printerName, QObject *parent = nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

#endif //ZTROUBLESHOOT_P_H
