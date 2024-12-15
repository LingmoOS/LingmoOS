// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADIENTLABEL_H
#define GRADIENTLABEL_H

#include <QLabel>

class QPaintEvent;
class GradientLabel : public QLabel
{
    Q_OBJECT
public:
    explicit GradientLabel(QWidget *parent = nullptr);

    enum Direction {
        TopToBottom,
        BottomToTop,
        LeftToRight,
        RightToLeft
    };

    void setText(const QString &);
    void setPixmap(QPixmap pixmap);

    Direction direction() const;
    void setDirection(const Direction &direction);

protected:
    void paintEvent(QPaintEvent* event);

private:
    Direction m_direction;
    QPixmap m_pixmap;
};

#endif // GRADIENTLABEL_H
