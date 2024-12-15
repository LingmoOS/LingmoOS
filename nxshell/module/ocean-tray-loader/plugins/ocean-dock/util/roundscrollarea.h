// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ROUNDSCROLLAREA_H
#define ROUNDSCROLLAREA_H

#include <QScrollArea>

class RoundScrollArea : public QScrollArea 
{
    Q_OBJECT
public:
    explicit RoundScrollArea(QWidget* parent = nullptr);

    void setRadius(int radius);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_radius;
};

#endif // ROUNDSCROLLAREA_H
