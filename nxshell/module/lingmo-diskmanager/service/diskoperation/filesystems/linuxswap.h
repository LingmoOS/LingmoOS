/* modified by Uniontech Software Technology Co., Ltd. */
/* Copyright (C) 2004 Bart
 * Copyright (C) 2008, 2009, 2010 Curtis Gedak
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LINUXSWAP_H
#define LINUXSWAP_H
#include "filesystem.h"

namespace DiskManager {

/**
 * @class LinuxSwap
 * @brief 交换分区格式操作类
 */

class LinuxSwap : public FileSystem
{
public:

    /**
     * @brief 获取文件系统支持
     * @return 文件系统
     */
    FS getFilesystemSupport();

    /**
     * @brief 设置已用空间
     * @param partition：分区信息
     */
    void setUsedSectors(Partition &partition);

    /**
     * @brief 读取表
     * @param partition：分区信息
     */
    void readLabel(Partition &partition);

    /**
     * @brief 写表
     * @param partition：分区信息
     * @return true成功false失败
     */
    bool writeLabel(const Partition &partition);

    /**
     * @brief 读取UUID
     * @param partition：分区信息
     */
    void readUuid(Partition &partition);

    /**
     * @brief 写UUID
     * @param partition：分区信息
     * @return true成功false失败
     */
    bool writeUuid(const Partition &partition);

    /**
     * @brief 创建
     * @param partition：分区信息
     * @return true成功false失败
     */
    bool create(const Partition &new_partition);

    /**
     * @brief 扩容
     * @param partition：分区信息
     * @param fill_partition：标记位
     * @return true成功false失败
     */
    bool resize(const Partition &partitionNew, bool fillPartition);
};

} // namespace DiskManager
#endif // LINUXSWAP_H
