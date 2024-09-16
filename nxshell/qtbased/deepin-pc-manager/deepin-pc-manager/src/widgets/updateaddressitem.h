// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATEADDRESSITEM_H
#define UPDATEADDRESSITEM_H

#include <DLineEdit>

#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DLineEdit;
DWIDGET_END_NAMESPACE

class QLabel;
class QCheckBox;
DWIDGET_USE_NAMESPACE

/**
 * @brief 设置中升级病毒地址的Item
 */
class UpdateAddressItem : public QWidget
{
    Q_OBJECT
public:
    explicit UpdateAddressItem(QWidget *parent = nullptr);

    // 设置地址
    void setUpdateAddres(QString address);
    // 设置选择框状态
    void setCheckboxStatus(bool status);
    // 编辑改变
    void lineEditChange(QString str);
    // 焦点改变
    void focusChanged(bool change);

    // 获得地址
    inline QString getAddress() const { return m_address->text(); }

    bool getCheckBoxStatus() const;

signals:
    // 发送结束完成的信号
    void sendInputFinised(QString address);
    // 选择框的状态信号
    void sendCheckBoxChange(int status);

private:
    QLabel *m_context;     // 内容
    DLineEdit *m_address;  // 地址输入框
    QCheckBox *m_checkbox; // 选择框
};

#endif // UPDATEADDRESSITEM_H
