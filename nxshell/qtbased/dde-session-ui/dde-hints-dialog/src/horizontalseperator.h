// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HORIZONTALSEPERATOR_H
#define HORIZONTALSEPERATOR_H

#include <QWidget>

class HorizontalSeperator : public QWidget
{
    Q_OBJECT

public:
    explicit HorizontalSeperator(QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
};

#endif // HORIZONTALSEPERATOR_H
