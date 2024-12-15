// Copyright (C) 2022 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVCPULIMITATION_H
#define AVCPULIMITATION_H

#include <DLabel>

#include <QWidget>

class QCheckBox;

class AVCPULimitation : public QWidget
{
    Q_OBJECT
public:
    explicit AVCPULimitation(QWidget *parent = nullptr);
    virtual ~AVCPULimitation();

    // 设置选择框
    void setcheckbox(bool status);

    // 选择框的改变
    void checkBoxChange(bool);

Q_SIGNALS:
    // 选择框的状态信号
    void notifyCheckBoxChange(bool);

private:
    DTK_WIDGET_NAMESPACE::DLabel *m_context; // 内容
    DTK_WIDGET_NAMESPACE::DLabel *m_tips;    // 单位
    QCheckBox *m_check;                      // 选择框
};

#endif // AVCPULIMITATION_H
