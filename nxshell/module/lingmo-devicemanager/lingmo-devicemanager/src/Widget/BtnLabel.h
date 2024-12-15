// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BTNLABEL_H
#define BTNLABEL_H

#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class BtnLabel : public DLabel
{
    Q_OBJECT
public:
    explicit BtnLabel(DWidget *parent = nullptr);

public:
    void setDesc(const QString &txt);
protected:
    void paintEvent(QPaintEvent* e) override;
private:
    QString m_Desc;
};

class TipsLabel : public DLabel
{
    Q_OBJECT
public:
    explicit TipsLabel(DWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent* e) override;
};

class TitleLabel : public DLabel
{
    Q_OBJECT
public:
    explicit TitleLabel(DWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent* e) override;
};

#endif // BTNLABEL_H
