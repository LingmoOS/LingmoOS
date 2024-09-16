// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BORDERWIDGET_H
#define BORDERWIDGET_H

#include <DFrame>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QPainterPath>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class BorderWidget : public DFrame
{
    Q_OBJECT
public:
    explicit BorderWidget(QWidget *parent = nullptr);

public slots:
    void setPixmap(const QPixmap &pixmap);
    void setChecked(bool checked);
    void updateInterface(float);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QPixmap m_pixmap;
    bool m_checked;
    QSize m_size;
};

#endif // BORDERWIDGET_H
