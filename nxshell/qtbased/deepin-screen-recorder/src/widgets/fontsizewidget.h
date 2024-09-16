// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTSIZEWIDGET_H
#define FONTSIZEWIDGET_H

#include <DLabel>
#include <DWidget>
#include <DLineEdit>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class Separator : public DLabel
{
    Q_OBJECT
public:
    explicit Separator(DWidget *parent);
    ~Separator();
};

class FontSizeWidget : public DLabel
{
    Q_OBJECT
public:
    explicit FontSizeWidget(DWidget *parent = 0);
    ~FontSizeWidget();

    void initWidget();
    void adjustFontSize(bool add);
    void setFontSize(int fontSize);

signals:
    void fontSizeChanged(int fontSize);

private:
    DLineEdit *m_fontSizeEdit;
    DPushButton *m_addSizeBtn;
    DPushButton *m_reduceSizeBtn;
    int m_fontSize;
};
#endif // FONTSIZEWIDGET_H
