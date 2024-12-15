// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LONGTEXTLABEL_H
#define LONGTEXTLABEL_H

#include <QObject>

#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

/**
 * @brief The LongTextLabel class
 * 封装富文本标签
 */
class LongTextLabel : public DLabel
{
    Q_OBJECT

public:
    explicit LongTextLabel(DWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;

private:
    int m_width;

};

#endif // LONGTEXTLABEL_H
