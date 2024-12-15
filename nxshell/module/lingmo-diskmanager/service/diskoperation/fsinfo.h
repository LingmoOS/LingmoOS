// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef FSINFO_H
#define FSINFO_H
#include "blockspecial.h"

#include <QVector>

namespace DiskManager {


/**
 * @class FsInfo
 * @brief 文件系统信息类
 */


/**
 * @struct fileSystemEntry
 * @brief 文件系统结构体
 */
struct fileSystemEntry {
    BlockSpecial m_path;    //路径
    QString m_type;           //类型
    QString m_secType;       //扇区类型
    QString m_uuid;           //UUID
    bool m_haveLabel;        //表标记位
    QString m_label;          //表
};

class FsInfo
{
public:
    /**
     * @brief 加载缓存
     */
    static void loadCache();

    /**
     * @brief 获取文件系统格式
     * @param path：路径
     * @return 文件系统格式
     */
    static QString getFileSystemType(const QString &path);

    /**
     * @brief 通过UUID获取路径
     * @param uuid：UUID
     * @return 路径信息
     */
    static QString getPathByUuid(const QString &uuid);

    /**
     * @brief 通过表获取路径
     * @param label：表
     * @return 路径信息
     */
    static QString getPathByLabel(const QString &label);

    /**
     * @brief 获取表信息
     * @param path：路径
     * @param found：标记位
     * @return 表信息
     */
    static QString getLabel(const QString &path, bool &found);

    /**
     * @brief 获取UUID
     * @param path：路径
     * @return UUID
     */
    static QString getUuid(const QString &path);

private:
    /**
     * @brief 初始化
     */
    static void initializeIfRequired();

    /**
     * @brief 设置找到的blkid命令
     */
    static void setCommandsFound();

    /**
     * @brief 加载文件系统信息缓存
     */
    static void loadFileSystemInfoCache();

    /**
     * @brief 运行blkid加载缓存
     * @param path：路径
     * @return true成功false失败
     */
    static bool runBlkidLoadCache(const QString &path = "");

    /**
     * @brief 通过路径加载缓存入口
     * @param path：路径
     * @return 文件系统信息结构
     */
    static const fileSystemEntry &getCacheEntryByPath(const QString &path);

    /**
     * @brief 通过额外路径加载文件系统信息缓存
     * @param path：路径
     */
    static void loadFileSystemInfoCacheExtraForPath(const QString &path);

    /**
     * @brief 更新文件系统信息缓存所有表
     */
    static void updateFileSystemInfoCacheAllLabels();

    /**
     * @brief 运行blkid更新缓存一个表
     * @param fsEntry:文件系统信息
     * @return true成功false失败
     */
    static bool runBlkidUpdateCacheOneLabel(fileSystemEntry &fsEntry);

    static bool m_fsInfoCacheInitialized;      //文件系统信息缓存初始化标记位
    static bool m_blkidFound;                    //blkid命令是否支持标记位
    static bool m_needBlkidVfatCacheUpdateWorkaround;    //需要blkid vfat缓存更新解决方案标记位
    static QVector<fileSystemEntry> m_fileSystemInfoCache;      //文件系统信息缓存
};

} // namespace DiskManager
#endif // FSINFO_H
