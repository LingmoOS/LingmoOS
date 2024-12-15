// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBUTTONWIDGET_H
#define KEYBUTTONWIDGET_H

#include <DLabel>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <DWidget>

#include <QObject>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class KeyButtonWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit KeyButtonWidget(DWidget *parent = nullptr);
    ~KeyButtonWidget();

    //设置键盘按钮上的文字内容
    void setKeyLabelWord(const QString &keyWord);
signals:

public slots:

private:
    //键盘内容
    DLabel *m_word;
    QString m_keyword;
};

#endif // KEYBUTTONWIDGET_H
