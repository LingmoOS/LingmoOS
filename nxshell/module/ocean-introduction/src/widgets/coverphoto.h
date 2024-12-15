// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COVERPHOTO_H
#define COVERPHOTO_H

#include <QPainter>
#include <DFrame>

DWIDGET_USE_NAMESPACE

//封面图片类
class CoverPhoto : public DFrame
{
public:
    explicit CoverPhoto(QWidget *parent = nullptr);

public slots:
    void setPixmap(const QPixmap &pixmap);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QPixmap m_pixmap;
};

#endif // COVERPHOTO_H
