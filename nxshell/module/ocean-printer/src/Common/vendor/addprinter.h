// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDPRINTER_H
#define ADDPRINTER_H

#include "zdevicemanager.h"

#include <QObject>

class InstallInterface;
class InstallDriver;
class FixHplipBackend;

class AddPrinterTask : public QObject
{
    Q_OBJECT

public:
    AddPrinterTask(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent = nullptr);
    ~AddPrinterTask();

    int doWork();

    QString getErrorMassge();

    virtual void stop();

    TDeviceInfo getPrinterInfo();
    QMap<QString, QVariant> getDriverInfo();

protected:
    enum {
        STEP_Start = 0,
        STEP_FillInfo,
        STEP_FixDriver,
        STEP_InstallDriver,
        STEP_AddPrinter,
        STEP_Finished,
        STEP_Failed
    };

    virtual int fixDriverDepends();
    virtual int installDriver();
    virtual int addPrinter() = 0;

    void nextStep();

    int fillPrinterInfo();
    int checkUriAndDriver();
    int isUriAndDriverMatched();

signals:
    void signalStatus(int);

protected slots:
    void slotInstallStatus(int iStatus);
    void slotDependsStatus(int iStatus);
    void slotFixHplipStatus(int iStatus);
    void slotWriteLog(int iStatus);

protected:
    TDeviceInfo m_printer;
    QMap<QString, QVariant> m_solution;
    QString m_uri;
    InstallInterface *m_installDepends;
    InstallDriver *m_installDriver;
    bool m_bQuit;
    QString m_strErr;

    int m_iStep;

    FixHplipBackend *m_fixHplip;
};

class AddPrinterFactory
{
public:
    static AddPrinterFactory *getInstance();

    AddPrinterTask *createAddPrinterTask(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution);

    QString defaultPrinterName(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution);

protected:
    AddPrinterFactory() {}
};

#define g_addPrinterFactoty AddPrinterFactory::getInstance()

#endif
