// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENABLECONFIG_H
#define ENABLECONFIG_H

#include <QMap>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <mutex>

class EnableSqlManager : public QObject
{
    Q_OBJECT
public:

    inline static EnableSqlManager *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        EnableSqlManager *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new EnableSqlManager();
                s_Instance.store(sin);
            }
        }
        return sin;
    }

    /**
     * @brief insertDataToRemove 将数据插入到remove表
     * @param path 插入的路径
     * @param hclass 类型
     * @param name 名称
     */
    void insertDataToRemoveTable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, const QString &strDriver = "");

    /**
     * @brief removeDateFromRemoveTable 从数据库里面删除数据
     * @param path
     */
    void removeDateFromRemoveTable(const QString &path);

    /**
     * @brief insertDataToAuthorizedTable 将数据插入remove表格
     * @param key
     */
    void insertDataToAuthorizedTable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, bool exist, const QString &strDriver = "");

    /**
     * @brief removeDataFromAuthorizedTable 从数据库里面删除数据
     * @param key
     */
    void removeDataFromAuthorizedTable(const QString &key);

    /**
     * @brief updateDataToAuthorizedTable
     */
    void updateDataToAuthorizedTable(const QString &unique_id, const QString &path);

    /**
     * @brief clearEnableFromAuthorizedTable 清空数据库里面 enable = 1 数据
     */
    void clearEnableFromAuthorizedTable();

    /**
     * @brief insertDataToAuthorizedTable 讲数据插入打印机表格
     * @param hclass 数据类型
     * @param name 名称
     * @param path 路径
     */
    void insertDataToPrinterTable(const QString &hclass, const QString &name, const QString &path);

    /**
     * @brief removeDataFromPrinterTable
     * @param name
     */
    void removeDataFromPrinterTable(const QString &name);

    /**
     * @brief uniqueIDExisted 判断数据库中可否查到数据
     * @param key
     * @return
     */
    bool uniqueIDExisted(const QString &key);

    /**
     * @brief uniqueIDExistedForEnable
     * @param key
     * @return
     */
    bool uniqueIDExistedEX(const QString &key);

    /**
     * @brief isUniqueIdEnabled 判断设备是否被禁用了
     * @param key
     * @return
     */
    bool isUniqueIdEnabled(const QString &key);

    /**
     * @brief removeInfo 返回数据库里面的所有信息
     * @return
     */
    QString removedInfo();

    /**
     * @brief authorizedInfo 获取没有被授权的设备的信息
     * @return
     */
    QString authorizedInfo();

    /**
     * @brief authorizedPath
     * @return
     */
    QString authorizedPath(const QString &unique_id);

    /**
     * @brief authorizedPathUniqueIDList 获取path 和 unique_id两个字段
     * @param lstPair
     */
    void authorizedPathUniqueIDList(QList<QPair<QString, QString> > &lstPair);

    /**
     * @brief removePathList 获取所有被remove的数据
     * @param lsPath
     */
    void removePathList(QStringList &lsPath);

    /**
     * @brief removePathUniqueIDList 获取path 和 unique_id两个字段
     * @param lstPair
     */
    void removePathUniqueIDList(QList<QPair<QString, QString> > &lstPair);

    /**
     * @brief insertWakeupData
     * @param unique_id
     * @param path
     * @param wakeup
     */
    void insertWakeupData(const QString &unique_id, const QString &path, bool wakeup);

    /**
     * @brief isWakeupUniqueIdExisted
     * @param unique_id
     * @return
     */
    bool isWakeupUniqueIdExisted(const QString &unique_id);

    /**
     * @brief updateWakeData
     * @param unique_id
     * @param path
     * @return
     */
    void updateWakeData(const QString &unique_id, const QString &path, bool wakeup);

    /**
     * @brief wakeupPath
     * @param unique_id
     * @return
     */
    QString wakeupPath(const QString &unique_id);

    /**
     * @brief isWakeup
     * @param unique_id
     * @return
     */
    bool isWakeup(const QString &unique_id);

    /**
     * @brief insertNetworkWakeup
     */
    void insertNetworkWakeup(const QString &logical_name, bool wake);

    /**
     * @brief isNetworkWakeup 判断当前是否可以唤醒
     * @param logical_name 逻辑名称
     * @return
     */
    bool isNetworkWakeup(const QString &logical_name);

    /**
     * @brief monitorWorkingFlag 判断设备是否监控
     * @return 设备是否监控
     */
    bool monitorWorkingFlag();

    /**
     * @brief setMonitorWorkingFlag 设置设备是否监控
     * @param flag 设备是否监控
     */
    void setMonitorWorkingFlag(const bool &flag);

protected:
    explicit EnableSqlManager(QObject *parent = nullptr);

private:
    void initDB();

private:
    static std::atomic<EnableSqlManager *> s_Instance;
    static std::mutex                  m_mutex;
    QSqlDatabase                       m_db;
    QSqlQuery                          m_sqlQuery;
};

#endif // ENABLECONFIG_H
