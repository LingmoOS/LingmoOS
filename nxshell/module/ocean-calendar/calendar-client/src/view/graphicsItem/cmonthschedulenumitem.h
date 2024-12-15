// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CMONTHSCHEDULENUMITEM_H
#define CMONTHSCHEDULENUMITEM_H

#include "cfocusitem.h"
#include "dschedule.h"
#include "scheduledaterangeinfo.h"

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CMonthScheduleNumItem class
 * 月视图 还有N项 标签
 */
class CMonthScheduleNumItem : public CFocusItem
{
    Q_OBJECT
public:
    explicit CMonthScheduleNumItem(QGraphicsItem *parent = nullptr);
    ~CMonthScheduleNumItem() override;
    //背景色
    void setColor(QColor color1, QColor color2);
    //设置字体颜色
    void setText(QColor tColor, QFont font);
    //设置字体大小
    void setSizeType(DFontSizeManager::SizeType sizeType);
    //设置显示多少项
    void setData(int num)
    {
        m_num = num;
    }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QColor m_color1;
    QColor m_color2;
    QColor m_textcolor;
    QFont m_font;
    int m_num;
    DFontSizeManager::SizeType m_SizeType = DFontSizeManager::T8;
};
#endif // CMONTHSCHEDULENUMITEM_H
