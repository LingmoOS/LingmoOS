// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALLDAYSCHEDULEITEM_H
#define CALLDAYSCHEDULEITEM_H

#include "draginfoitem.h"
#include "dschedule.h"

class CAllDayScheduleItem : public DragInfoItem
{
    Q_OBJECT
public:
    explicit CAllDayScheduleItem(QRectF rect, QGraphicsItem *parent = nullptr);
    bool hasSelectSchedule(const DSchedule::Ptr &info);

protected:
    void paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap = false) override;
};

#endif // CALLDAYSCHEDULEITEM_H
