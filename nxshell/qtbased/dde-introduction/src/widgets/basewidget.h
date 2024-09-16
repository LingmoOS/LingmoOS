// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include "borderwidget.h"

#include <QVBoxLayout>
#include <QDebug>

#include <DFrame>
#include <DImageButton>
#include <DHiDPIHelper>
#include <DLabel>

DWIDGET_USE_NAMESPACE

//桌面样式，运行模式，主题图标的类型显示基类
class BaseWidget : public DFrame
{
    Q_OBJECT
public:
    explicit BaseWidget(QWidget *parent = nullptr);

    //边框绘制类
    BorderWidget *m_borderWidget;

signals:
    void clicked();
    void sizeChanged();

public slots:
    //设置第一次启动图片函数封装
    void setBigPixmap(const QString &url);
    //设置日常启动图片函数封装
    void setSmallPixmap(const QString &url);
    //类外设置图片函数
    void setPixmap(const QPixmap &pixmap);
    //设置第一次启动图片函数
    void setBigPixmap(const QPixmap &pixmap);
    //设置日常启动图片函数
    void setSmallPixmap(const QPixmap &pixmap);
    //设置图片的标题
    void setTitle(const QString &title);
    //设置边框可检查状态
    void setChecked(bool checked);
    void setLayoutSpacing(int i);
    //设置边框大小
    void updateInterface(float);

protected:
    //模仿鼠标点击功能
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //调整大小事件
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QVBoxLayout *m_layout;
    DLabel *m_title;
    QSize m_size;
};

#endif  // BASEWIDGET_H
