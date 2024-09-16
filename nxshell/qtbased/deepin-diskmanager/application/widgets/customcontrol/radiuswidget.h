// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef RADIUSWIDGET_H
#define RADIUSWIDGET_H

#include "commondef.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QWidget>
#include <QRect>
#include <QPainterPath>
#include <QPainter>
#include <QPalette>
#include <QBrush>

DWIDGET_USE_NAMESPACE

/**
 * @class RadiusWidget
 * @brief 圆角控件窗口类
 */

struct PVInfoData {
    QString m_diskPath;
    QString m_diskSize;
    QString m_partitionPath;
    QString m_partitionSize;
    Sector m_sectorStart = 0;
    Sector m_sectorEnd = 0;
    Sector m_sectorSize;
    int m_level;
    QString m_disktype;
    int m_selectStatus;
    int m_lvmDevType;
    int m_luksFlag; //luks 标志位
    int m_isReadOnly = false;
};
Q_DECLARE_METATYPE(PVInfoData)

class RadiusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RadiusWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置数据
     * @param leftTop 左上角是否为圆角，true画圆角，反之则画直角
     * @param leftBottom 左下角是否为圆角，true画圆角，反之则画直角
     * @param rightTop 右上角是否为圆角，true画圆角，反之则画直角
     * @param rightBottom 右下角是否为圆角，true画圆角，反之则画直角
     */
    void setMode(bool leftTop, bool leftBottom, bool rightTop, bool rightBottom);

    /**
     * @brief 设置是否选中
     * @param isChecked 是否选中，true为选中，反之则没选中
     */
    void setChecked(bool isChecked);

signals:

protected:
    /**
     * @brief 重写绘画事件
     */
    virtual void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 重写进入事件
     */
    virtual void enterEvent(QEvent *event) override;

    /**
     * @brief 重写离开事件
     */
    virtual void leaveEvent(QEvent *event) override;

private:
    bool m_leftTop;
    bool m_leftBottom;
    bool m_rightTop;
    bool m_rightBottom;
    bool m_isEnter;
    bool m_isChecked;
};

#endif // RADIUSWIDGET_H
