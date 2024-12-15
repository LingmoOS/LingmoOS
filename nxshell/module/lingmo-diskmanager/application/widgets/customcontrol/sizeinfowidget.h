// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef SIZEINFOWIDGET_H
#define SIZEINFOWIDGET_H

#include "partitioninfo.h"
#include "utils.h"
#include "partedproxy/dmdbushandler.h"

#include <DWidget>
#include <DApplicationHelper>
#include <DPalette>

#include <QWidget>
#include <QRect>
#include <QPainterPath>
#include <QPainter>
#include <QPalette>
#include <QBrush>
#include <QPointF>
#include <QLine>
#include <QColor>
#include <QVector>

DWIDGET_USE_NAMESPACE

/**
 * @class SizeInfoWidget
 * @brief 分区信息中间部分实现类
 */

class SizeInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SizeInfoWidget(QWidget *parent = nullptr);

    /**
     * @brief 构造函数
     * @param used 已用空间
     * @param unused 未用空间
     * @param flag 是否显示下方标注
     */
    SizeInfoWidget(double used, double unused, bool flag, QWidget *parent = nullptr);

    /**
     * @brief 设置数据
     * @param info 当前分区信息
     * @param color 颜色列表
     * @param flag 是否显示下方标注
     */
    void setData(PartitionInfo info, QVector<QColor> color, QVector<double> size, bool flag);

    /**
     * @brief 设置数据
     * @param info 当前逻辑卷信息
     * @param color 颜色列表
     * @param flag 是否显示下方标注
     */
    void setData(LVInfo info, QVector<QColor> color, QVector<double> size, bool flag);

    /**
     * @brief 设置数据
     * @param info 当前磁盘信息
     * @param color 颜色列表
     * @param flag 是否显示下方标注
     */
    void setData(DeviceInfo info, QVector<QColor> color, QVector<double> size, bool flag);

protected:
    /**
     * @brief 重写绘画事件
     */
    virtual void paintEvent(QPaintEvent *event) override;
signals:

private slots:
    /**
     * @brief 主题切换信号响应的槽函数
     */
    void onHandleChangeTheme();

private:
    double m_used = 0.00;
    double m_noused = 0.00;
    double m_totalSize = 0.00;
    bool m_flag = false;
    QVector<QColor> m_colorInfo;
    QVector<double> m_sizeInfo;
    DPalette m_parentPb;
    QString m_totalSpaceSize;
    QString m_usedSize;
    QString m_partitionPath;
};

#endif // SIZEINFOWIDGET_H
