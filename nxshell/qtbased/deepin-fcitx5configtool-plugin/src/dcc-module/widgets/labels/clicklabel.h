// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QIcon>
#include <QLabel>
#include <QMouseEvent>

class ClickLabel : public QLabel {
    Q_OBJECT
public:
    ClickLabel(QWidget *parent);
    ~ClickLabel();
    void setIcon(const QIcon &icon);

Q_SIGNALS:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif // CLICKLABEL_H
