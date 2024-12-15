/* modified by Uniontech Software Technology Co., Ltd. */
/* Copyright (C) 2019 Mike Fleetwood
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

#ifndef SUPPORTEDFILESYSTEMS_H
#define SUPPORTEDFILESYSTEMS_H
#include "filesystems/filesystem.h"
#include "commondef.h"

#include <QVector>
#include <QMap>

namespace DiskManager {

/**
 * @class SupportedFileSystems
 * @brief 文件系统格式类
 */

class SupportedFileSystems
{
public:
    SupportedFileSystems();
    ~SupportedFileSystems();

    /**
     * @brief 匹配可支持的文件系统
     */
    void findSupportedFilesystems();

    /**
     * @brief 获取文件系统
     * @param fstype：文件系统格式
     * @return 文件系统
     */
    FileSystem *getFsObject(FSType fstype) const;

    /**
     * @brief 获取文件系统支持
     * @param fstype：文件系统格式
     * @return 文件系统支持
     */
    const FS &getFsSupport(FSType fstype) const;

//    /**
//     * @brief 获取全部支持的文件系统格式
//     * @return 文件系统支持列表
//     */
//    const QVector<FS> &getAllFsSupport() const;

//    /**
//     * @brief 文件系统是否支持
//     * @param fstype：文件系统格式
//     * @return true支持false不支持
//     */
//    bool supportedFilesystem(FSType fstype) const;

    /**
     * @brief 获取支持的文件系统名字
     * @return 支持的文件系统名字列表
     */
    const QStringList &getAllFsName();

private:
    /**
     * @brief 定义一个文件系统格式类型
     */
    typedef QMap<FSType, FileSystem *> FSObjectsMap;

    QVector<FS> m_fsSupport;           //支持的文件系统列表
    FSObjectsMap m_fsObjects;          //文件系统格式对象
    QStringList m_effectivefs;          //有效性列表
};

} // namespace DiskManager
#endif // SUPPORTEDFILESYSTEMS_H
