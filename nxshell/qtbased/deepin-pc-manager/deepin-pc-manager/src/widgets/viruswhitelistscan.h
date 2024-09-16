// Copyright (C) 2021 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRUSWHITELISTSCAN_H
#define VIRUSWHITELISTSCAN_H

#include <DIconButton>

#include <QToolButton>
#include <QWidget>

class QLabel;
class QCheckBox;

DWIDGET_USE_NAMESPACE

/**
 * @brief 设置中白名单扫描
 */

class VirusWhitelistScanItem : public QWidget
{
    Q_OBJECT
public:
    explicit VirusWhitelistScanItem(QWidget *parent = nullptr);
    ~VirusWhitelistScanItem();

    // 设置显示内容
    void setText(const QString &text);
    // 按钮触发弹窗
    void pushButtonTrigger(bool status);

signals:
    // 按钮点击发射信号
    void buttonTrigger(bool val);

private:
    QLabel *m_lbText;         // 内容
    QLabel *m_lbTip;          // 提示
    DIconButton *m_dialogBtn; // 前进按钮
};

#endif // VIRUSWHITELISTSCAN_H
