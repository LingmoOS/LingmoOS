// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DMTREEVIEWDELEGATE_H
#define DMTREEVIEWDELEGATE_H

#include "commondef.h"

#include <DApplicationHelper>
#include <DStyledItemDelegate>
#include <DPalette>

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QRect>
#include <QAbstractItemView>
#include <QImage>

struct DiskInfoData {
    QString m_diskPath;
    QString m_diskSize;
    QImage m_iconImage;
    QString m_partitionPath;
    QString m_partitionSize;
    QString m_used;
    QString m_unused;
    QString m_fstype;
    QString m_sysLabel;
    QString m_mountpoints;
    Sector m_sectorsUnallocated;
    Sector m_start;
    Sector m_end;

    int m_level;
    int m_flag;
    int m_vgFlag;
    int m_luksFlag;
};
Q_DECLARE_METATYPE(DiskInfoData)

DWIDGET_USE_NAMESPACE

/**
 * @class DmTreeviewDelegate
 * @brief 设备树结构代理类
*/

class DmTreeviewDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DmTreeviewDelegate(QAbstractItemView *parent = nullptr);

protected:
    /**
     * @brief 重写绘画事件
     * @param painter 画笔
     * @param option 视图小部件中绘制项的参数
     * @param index 数据模型中的数据
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief 重写窗口初始大小函数
     * @param option 视图小部件中绘制项的参数
     * @param index 数据模型中的数据
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:

private slots:

    /**
     * @brief 主题切换信号响应的槽函数
     */
    void onHandleChangeTheme();

private:
    QAbstractItemView *m_parentView {nullptr};
    DPalette m_parentPb;
};

#endif // DMTREEVIEWDELEGATE_H
