// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include <QLabel>

class LabelWidget : public QLabel
{
public:
    explicit LabelWidget(QWidget *parent = nullptr);
    ~LabelWidget() override;
protected:
    void paintEvent(QPaintEvent *ev) override;
};

#endif // LABELWIDGET_H
