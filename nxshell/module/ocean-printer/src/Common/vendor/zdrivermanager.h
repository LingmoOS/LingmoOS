// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZLOCALPPDMANAGER_H
#define ZLOCALPPDMANAGER_H

#include "zdevicemanager.h"

#include <QList>
#include <QMap>
#include <QVariant>
#include <QFile>

enum {
    PPDFrom_Database = 0,
    PPDFrom_File,
    PPDFrom_Server,
    PPDFrom_EveryWhere
};

class RefreshLocalPPDS;

class DriverSearcher : public QObject
{
    Q_OBJECT

public:
    DriverSearcher(const TDeviceInfo &printer, QObject *parent = nullptr);

    void startSearch();

    QList<QMap<QString, QVariant>> getDrivers();

    TDeviceInfo getPrinter();

    void setMatchLocalDriver(bool match);

    bool searchOffineDriver(QString mfg, QString model);

    void parseJsonInfo(QJsonArray value);

    bool hasOfflineDriver();

    void WriteSearchLog(int iCode, const QString &reply);
signals:
    void signalDone();

protected slots:
    void slotDriverDone(int iCode, const QByteArray &result);
    void slotDriverInit(int id, int state);

private:
    int getLocalDrivers();
    void sortDrivers();
    void askForFinish();
    bool hasExcatDriver();
    int getLocalDbDrivers();

    TDeviceInfo m_printer;
    QList<QMap<QString, QVariant>> m_drivers;
    int m_localIndex;

    QString m_strMake;
    QString m_strModel;
    QString m_strCMD;

    bool m_matchLocalDriver;
    bool m_isOfflineDriverExist = false;
    bool m_isNetOffline = false;
};

class DriverManager : public QObject
{
    Q_OBJECT

public:
    static DriverManager *getInstance();
    /*!
    * @brief 获取刷新ppd的状态
    */
    int getStatus();

    int stop();

    /*!
    * @brief 刷新ppd列表
    *       非阻塞，开启新的线程，getStatus可以获取当前状态
    */
    int refreshPpds();

    QStringList getAllMakes();
    const QMap<QString, QString> *getModelsByMake(const QString &strMake);

    /*!
    * @brief 获取所有ppd文件的信息
    */
    const QMap<QString, QMap<QString, QString>> *getPPDs();

    /*!
    * @brief 获取Generic Text-Only Printer，作为默认驱动
    */
    QMap<QString, QString> getTextPPD();

    /*!
    * @brief 判断两个PPD文件路径是否为同一个
    */
    bool isSamePPD(const QString &ppd1, const QString &ppd2);

    QStringList getDriverDepends(const char *strPPD);

    QMap<QString, QVariant> getEveryWhereDriver(const QString &strUri);

    /*!
    * @brief 查找驱动
    */
    DriverSearcher *createSearcher(const TDeviceInfo &device);

signals:
    void signalStatus(int, int);

protected:
    DriverManager(QObject *parent = nullptr);

private:
    RefreshLocalPPDS *m_refreshTask;
};

#define g_driverManager DriverManager::getInstance()

#endif // ZLOCALPPDMANAGER_H
