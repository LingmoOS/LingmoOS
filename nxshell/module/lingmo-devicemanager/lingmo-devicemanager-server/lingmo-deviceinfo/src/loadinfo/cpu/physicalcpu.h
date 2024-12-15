// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHYSICALCPU_H
#define PHYSICALCPU_H

#include <QMap>

class CoreCpu;
class LogicalCpu;
class PhysicalCpu
{
public:
    PhysicalCpu();
    explicit PhysicalCpu(int id);

    /**
     * @brief addCoreCpu
     * @param id
     * @param cpu
     */
    void addCoreCpu(int id, const CoreCpu &cpu);

    /**
     * @brief coreIsExisted
     * @return
     */
    bool coreIsExisted(int id);

    /**
     * @brief coreCpu
     * @param id
     * @return
     */
    CoreCpu &coreCpu(int id);

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
     * @brief coreNum
     * @return
     */
    int coreNum();

    /**
     * @brief logicalNum
     * @return
     */
    int logicalNum();

    /**
     * @brief diagPrintInfo
     */
    void diagPrintInfo();

    /**
     * @brief coreNums
     * @return
     */
    QList<int> coreNums();

private:
    int m_PhysicalCpu;                   //<! physical id
    QMap<int, CoreCpu> m_MapCoreCpu;      //<! core cpu
};

#endif // PHYSICALCPU_H
