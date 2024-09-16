// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MODIFYSCHEDULEITEM_H
#define MODIFYSCHEDULEITEM_H

#include "itemwidget.h"

class modifyScheduleItem : public ItemWidget
{
    Q_OBJECT
public:
    explicit modifyScheduleItem(QWidget *parent = nullptr);
    int getIndex() const;
    void setIndex(int Index);

protected:
    QColor LineColor() const;
    void setLineColor(const QColor &LineColor);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setTheMe(const int type) override;
    void DrawItem(QPainter &painter) override;
    void drawIndex(QPainter &painter);
    void drawLine(QPainter &painter);
    void drawTitleContent(QPainter &painter);
    void drawScheduleColorLine(QPainter &painter);
    void drawTime(QPainter &painter);
    void drawDate(QPainter &painter);
signals:
    void signalSendSelectScheduleIndex(int index);
public slots:
private:
    int m_Index = 1;
    const int m_IndexWidth {16};
    const int m_IndexX {14};
    QColor m_LineColor {"#000000"};
    const int m_TitleX {51};
    const int m_TItleRightMargin {97};
    const int m_DateTimeOffset = 3;
};

#endif // MODIFYSCHEDULEITEM_H
