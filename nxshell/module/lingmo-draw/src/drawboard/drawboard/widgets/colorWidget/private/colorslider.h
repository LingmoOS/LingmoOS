// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORSLIDER_H
#define COLORSLIDER_H

//#include <QSlider>
#include "globaldefine.h"
#include <QPaintEvent>
#include <QSlider>


class DRAWLIB_EXPORT ColorSlider : public QSlider
{
    Q_OBJECT
public:
    explicit ColorSlider(QWidget *parent = nullptr);
    ~ColorSlider();

    //h∈(0, 360), s∈(0, 1), v∈(0, 1)
    QColor getColor(qreal h, qreal s, qreal v);

protected:
    void paintEvent(QPaintEvent *ev);

private:
    int m_value;
    QImage m_backgroundImage;
};

#endif // COLORSLIDER_H
