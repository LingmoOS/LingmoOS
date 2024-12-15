// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INITEMPTYPAGE_H
#define INITEMPTYPAGE_H

#include "widgets/vnoteiconbutton.h"

#include <DLabel>
#include <DPushButton>
#include <DSuggestButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE
//无数据主页
class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);

signals:
    //添加记事本信号
    void sigAddFolderByInitPage();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    //初始化UI布局
    void initUi();
    //连接槽函数
    void initConnection();

    VNoteIconButton *m_Image;
    DSuggestButton *m_PushButton;
    DLabel *m_Text;
};

#endif // INITEMPTYPAGE_H
