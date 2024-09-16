// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef PROCPARTITIONSINFO_H
#define PROCPARTITIONSINFO_H
#include <QVector>
#include <QString>

namespace DiskManager {

/**
 * @class ProcPartitionsInfo
 * @brief 程序分区信息类
 */

class ProcPartitionsInfo
{
public:
    /**
     * @brief 加载分区信息缓存
     */
    static void loadCache();

    /**
     * @brief 获取设备路径
     * @return 设备路径表
     */
    static const QVector<QString> &getDevicePaths();

private:
    /**
     * @brief 是否需要初始化
     */
    static void initializeIfRequired();

    /**
     * @brief 加载进程分区信息缓存
     */
    static void loadProcPartitionsInfoCache();

    static bool procPartitionsInfoCacheInitialized;     //进程分区信息缓存初始化状态
    static QVector<QString> devicePathsCache;             //设备路径缓存
};

} // namespace DiskManager
#endif // PROCPARTITIONSINFO_H
