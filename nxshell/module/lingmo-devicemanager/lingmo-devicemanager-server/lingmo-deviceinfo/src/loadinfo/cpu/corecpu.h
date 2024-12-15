// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORECPU_H
#define CORECPU_H

#include <QMap>

class LogicalCpu;
class CoreCpu
{
public:
    CoreCpu();
    explicit CoreCpu(int id);

    /**
     * @brief setCoreId : set core id
     * @param id
     */
    void setCoreId(int id);

    /**
     * @brief addLogicalCpu
     * @param id
     * @param lc
     */
    void addLogicalCpu(int id, const LogicalCpu &lc);

    /**
     * @brief logicalIsExisted
     * @param id
     * @return
     */
    bool logicalIsExisted(int id);

    /**
     * @brief logicalCpu
     * @param id
     * @return
     */
    LogicalCpu &logicalCpu(int id);

    /**
     * @brief getInfo
     * @param info
     */
    void getInfo(QString &info);

    /**
     * @brief appendKeyValue
     * @param info
     * @param key
     * @param value
     */
    void appendKeyValue(QString &info, const QString &key, const QString &value);
    void appendKeyValue(QString &info, const QString &key, int value);

    /**
     * @brief coreId
     * @return
     */
    int coreId();

    /**
     * @brief logicalNum
     * @return
     */
    int logicalNum();

    /**
     * @brief diagPrintInfo
     */
    void diagPrintInfo();

private:
    int m_CoreId;                //<! core id
    QMap<int, LogicalCpu> m_MapLogicalCpu; //<! logical info
};

#endif // CORECPU_H
