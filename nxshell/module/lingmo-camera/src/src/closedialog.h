// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLOSEDIALOG_H
#define CLOSEDIALOG_H

#include <DDialog>
#include <DWidget>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DSuggestButton>

#include <QVBoxLayout>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

typedef DLabel QLbtoDLabel;

/**
* @brief CloseDialog　关闭对话框
*/
class CloseDialog : public DDialog
{
    Q_OBJECT
    
public:
    CloseDialog(QWidget *parent = nullptr, QString strText = "");

private:
    const QString m_leftNamebtn;
    const QString m_rightNamebtn;
    QVBoxLayout  *m_vlayout;
    DWidget      *m_widget;
    QLbtoDLabel  *m_labtitle;
};

#endif // CLOSEDIALOG_H
