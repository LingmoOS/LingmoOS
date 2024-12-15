// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "commondef.h"
#include "../partition.h"

#include <QtMath>

namespace DiskManager {

/**
 * @class FileSystem
 * @brief 文件系统类
 */

class FileSystem
{
public:
    FileSystem();
    virtual ~FileSystem() {}

    /**
     * @brief 获取文件系统支持
     * @return 文件系统
     */
    virtual FS getFilesystemSupport() = 0;

    /**
     * @brief 文件系统繁忙状态
     * @param
     * @return true成功false失败
     */
    virtual bool isBusy(const QString &) { return false; }

    /**
     * @brief 读取表
     * @param partition：分区信息
     */
    virtual void readLabel(Partition &) {}

    /**
     * @brief 写表
     * @param partition：分区信息
     * @return true成功false失败
     */
    virtual bool writeLabel(const Partition &) { return false; }

    /**
     * @brief 读取UUID
     * @param partition：分区信息
     */
    virtual void readUuid(Partition &) {}

    /**
     * @brief 写UUID
     * @param partition：分区信息
     * @return true成功false失败
     */
    virtual bool writeUuid(const Partition &) { return false; }

    /**
     * @brief 设置已用空间
     * @param partition：分区信息
     */
    virtual void setUsedSectors(Partition &) {}

    /**
     * @brief 获取文件系统限制
     * @param partition：分区信息
     * @return 文件系统限制信息
     */
    virtual FS_Limits getFilesystemLimits(const Partition &) { return m_fsLimits; }



    /**
     * @brief 获取文件系统限制
     * @param path：设备路径
     * @return 文件系统限制信息
     */
    virtual FS_Limits getFilesystemLimits(const QString &) { return m_fsLimits; }

    /**
     * @brief 创建
     * @param partition：分区信息
     * @return true成功false失败
     */
    virtual bool create(const Partition &) { return false; }

    /**
     * @brief 扩容
     * @param partition：分区信息
     * @param
     * @return true成功false失败
     */
    virtual bool resize(const Partition &, bool) { return false; }



    /**
     * @brief 调整大小
     * @param path：设备路径
     * @param size：调整后大小
     * @param fill_partition：标记位
     * @return true成功false失败
     */
    virtual bool resize(const QString &, const QString &, bool)  { return false; }


    /**
     * @brief 检查修补
     * @param partition：分区信息
     * @return true成功false失败
     */
    virtual bool checkRepair(const Partition &) { return false; }

    /**
     * @brief 检查修补
     * @param path：设备路径
     * @return true成功false失败
     */
    virtual bool checkRepair(const QString &) { return false; }

    Sector m_totalNumOfBlock, m_numOfFreeOrUsedBlocks, m_blocksSize; //File system [T]otal num of blocks, [N]um of free (or used) blocks, block [S]ize
public:
    FS_Limits m_fsLimits;

private:
};
} // namespace DiskManager
#endif // FILESYSTEM_H
