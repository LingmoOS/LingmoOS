// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGSCROLLBAR_H
#define LOGSCROLLBAR_H

#include <QScrollBar>
class LogScrollBar : public QScrollBar
{
public:
public:
    explicit LogScrollBar(QWidget *parent = nullptr);
    explicit LogScrollBar(Qt::Orientation o, QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    bool m_isOnPress = false;

};

#endif // LOGSCROLLBAR_H
