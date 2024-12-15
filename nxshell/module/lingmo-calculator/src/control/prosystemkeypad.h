// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROSYSTEMKEYPAD_H
#define PROSYSTEMKEYPAD_H

#include "../../3rdparty/core/evaluator.h"
#include "../widgets/probitwidget.h"

#include <DWidget>
#include <DGuiApplicationHelper>
#include <QGridLayout>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief 二进制按钮键盘
 */
class ProSystemKeypad : public DWidget
{
    Q_OBJECT

public:
    explicit ProSystemKeypad(QWidget *parent = nullptr);
    ~ProSystemKeypad() override;
    void setSystem(int system, int oldsystem); //system-64, 32, 16, 8 oldsystem-转换前的位数
    bool buttonHasFocus();
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void valuechanged(QString num);
    void bitbuttonclicked();
    void longbitcut(Quantity ans);

public slots:
    void getFocus(int dir, int key);
    void setvalue(QString num);
    void longBitCut(const QString &num);

private:
    void initUI();
    void initconnects();
    void changeBinaryValue(int i);

    QGridLayout *m_layout;
    QHash<int, ProBitWidget *> m_keys; //存储每一块ProBitWidget，以便设置不同位数
    QHash<int, BitButton *> m_buttons; //存储所有按键
//    QChar m_binaryValue[64];
    QString m_binaryValue;
};

#endif // PROSYSTEMKEYPAD_H
