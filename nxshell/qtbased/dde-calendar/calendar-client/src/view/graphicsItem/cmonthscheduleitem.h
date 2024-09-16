// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CMONTHSCHEDULEITEM_H
#define CMONTHSCHEDULEITEM_H

#include "draginfoitem.h"

/**
 * @brief The CMonthScheduleItem class
 * 月视图日程标签
 */
class CMonthScheduleItem : public DragInfoItem
{
    Q_OBJECT
public:
    explicit CMonthScheduleItem(QRect rect, QGraphicsItem *parent = nullptr, int edittype = 0);
    ~CMonthScheduleItem() override;
    QPixmap getPixmap();

private:
    void paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap = false) override;

private:
    QPoint m_pos;
};

#endif // CMONTHSCHEDULEITEM_H
