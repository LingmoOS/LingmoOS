// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOPTIPS_H
#define TOPTIPS_H

#include <DLabel>
#include <DWidget>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class TopTips : public QLabel
{
    Q_OBJECT
public:
    explicit TopTips(DWidget *parent = nullptr);
    ~TopTips() override;
    void setRecorderTipsInfo(const bool showState);
public slots:
    void setContent(const QSize &rect);
    void updateTips(QPoint pos, const QSize &rect);

protected:
    //void mousePressEvent(QMouseEvent *ev) override;
    bool m_showRecorderTips = false;
    int m_width = 0;
    int m_height = 0;
};
#endif // TOPTIPS_H
