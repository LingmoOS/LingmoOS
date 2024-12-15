// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DMDISKINFOBOX_H
#define DMDISKINFOBOX_H

#include "commondef.h"

#include <QObject>
#include <QDebug>

/**
 * @class DmDiskinfoBox
 * @brief 磁盘信息数据类
*/

class DmDiskinfoBox : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief 构造函数
     * @param level 磁盘数据类型   0代表磁盘   1代表分区
     * @param diskPath 磁盘路径
     * @param diskSize 磁盘大小
     * @param vgFlag vg标志
     * @param luksFlag 加密盘标志
     * @param partitionPath 分区路径
     * @param partitionSize 分区大小
     * @param used 已用空间
     * @param unused 未用空间
     * @param sectorsUnallocated 分区大小和文件系统大小之间的差异
     * @param start 分区扇区开始
     * @param end 分区扇区结束
     * @param fstype 分区类型
     * @param mountpoints 挂载点
     * @param systemLabel 分区分卷卷标名
     * @param flag 分区隐藏标志
    */
    DmDiskinfoBox(int level, QObject *parent = nullptr, QString diskPath = "", QString diskSize = "", int vgFlag = 0, int luksFlag = 0,
                  QString partitionPath = "", QString partitionSize = "", QString used = "", QString unused = "",
                  Sector sectorsUnallocated = 0, Sector start = 0, Sector end = 0, QString fstype = "",
                  QString mountpoints = "", QString systemLabel = "", int flag = 0);
    DmDiskinfoBox(QObject *parent = nullptr);
    ~DmDiskinfoBox();

public:
    /**
     * @brief 添加数据
     * @param child 数据指针
    */
    int addChild(DmDiskinfoBox *child);

    /**
     * @brief 获取数据个数
     * @return 返回个数
    */
    int childCount();

public:
    int m_id;
    int m_level;
    QString m_diskPath;
    QString m_diskSize;
    int m_vgFlag;
    int m_luksFlag;
    QString m_partitionPath;
    QString m_partitionSize;
    QString m_used;
    QString m_unused;
    QString m_total;
    QString m_fstype;
    QString m_mountpoints;
    QString m_syslabel;
    Sector m_sectorsUnallocated;
    Sector m_start;
    Sector m_end;
    int m_flag;
    QList<DmDiskinfoBox *> m_childs;
};

#endif // DMDISKINFOBOX_H
