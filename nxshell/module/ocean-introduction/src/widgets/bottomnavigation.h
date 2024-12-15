// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOTTOMNAVIGATION_H
#define BOTTOMNAVIGATION_H


#include "bottombutton.h"

#include <QDesktopServices>
#include <QEvent>
#include <QHBoxLayout>
#include <QProcess>
#include <QUrl>
#include <QMap>

#include <DWidget>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

//不是用的类，可以删除，现在使用NavigationBottom
class BottomNavigation : public DWidget
{
    Q_OBJECT
public:
    explicit BottomNavigation(DWidget *parent = nullptr);

private slots:
    void onButtonClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    QMap<DWidget*, QString> m_buttons;
};

#endif // BOTTOMNAVIGATION_H
