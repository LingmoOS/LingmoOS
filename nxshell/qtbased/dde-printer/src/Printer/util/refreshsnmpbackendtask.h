// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REFRESHSNMPBACKENDTASK_H
#define REFRESHSNMPBACKENDTASK_H

#include "dprinter.h"

#include <cupssnmp.h>
#include <QVector>
#include <QMap>
#include <QThread>

typedef struct SnmpFreshNode
{
    QString strName;
    QString strUrl;
    QString strPpdName;
    QString strLoc;
} SNMPFRESHNODE;

class RefreshSnmpBackendTask : public QThread
{
    Q_OBJECT
public:
    explicit RefreshSnmpBackendTask(QObject *parent = nullptr);
    ~RefreshSnmpBackendTask();

public:
    void setPrinters(const QStringList&);
    void beginTask();
    void stopTask();
    bool isTaskRunning();

public:
    QVector<SUPPLYSDATA> getSupplyData(const QString& strName);

signals:
    void refreshsnmpfinished(const QString&, bool iResult);

public slots:

protected:
    virtual void run();

private:
    bool canGetSupplyMsg(const SNMPFRESHNODE&);
    void initColorTable();
    QString getColorName(const QString&);

private:
    QStringList m_strPrinterNames;
    QMap<QString, QVector<SUPPLYSDATA>> m_mapSupplyInfo;
    QVector<SNMPFRESHNODE> m_vecFreshNode;
    QMap<QString,QString> m_colorTable;
    bool m_bExit;
};

#endif // REFRESHSNMPBACKENDTASK_H
