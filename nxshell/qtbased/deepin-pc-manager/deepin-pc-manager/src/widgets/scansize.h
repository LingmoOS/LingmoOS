// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCANSIZE_H
#define SCANSIZE_H

#include <DLineEdit>

#include <QWidget>

class QLabel;
class QLineEdit;
class QCheckBox;

DWIDGET_USE_NAMESPACE

/**
 * @brief 自定义设置窗口中大小设置
 */
class ScanSize : public QWidget
{
    Q_OBJECT
public:
    explicit ScanSize(QWidget *parent = nullptr);

    // 设置大小
    void setSize(int val);
    // 设置选择框
    void setcheckbox(bool status);
    // 选择框的改变
    void checkBoxChange(int status);

    // 编辑改变
    void lineEditChange(QString str);

    // 焦点改变
    void focusChanged(bool change);

signals:
    // 发送结束完成的信号
    void notifyInputFinised(int val);

    // 选择框的状态信号
    void notifyCheckBoxChange(int status);

private:
    QLabel *m_context;      // 内容
    QLabel *m_unit;         // 单位
    DLineEdit *m_pLineEdit; // 编辑器
    QCheckBox *m_check;     // 选择框
};

#endif // SCANSIZE_H
