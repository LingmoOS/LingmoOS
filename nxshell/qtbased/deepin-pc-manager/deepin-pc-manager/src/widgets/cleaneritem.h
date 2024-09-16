// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dtkwidget_global.h"
#include "window/namespace.h"

#include <DCheckBox>
#include <DFrame>
#include <DTipLabel>

#include <QLabel>
#include <QTimer>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class CleanerItem : public DFrame
{
    Q_OBJECT
public:
    explicit CleanerItem(QWidget *parent = nullptr);
    ~CleanerItem();

    // 设置标题和提示
    void setLevelText(QString text);
    void setTipText(QString text);

    // 设置/获取 勾选框状态
    void setCheckBoxStatus(bool isbStatus);
    bool getCheckBoxStatus() const;

    // 设置相关标签名称
    void setAccessibleParentText(const QString &sAccessibleName);

protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void itemClicked();

private:
    QWidget *m_firstLayerWidgt;
    DCheckBox *m_checkBox;
    DLabel *m_levelLabel;
    DTipLabel *m_tipLabel;

    QString m_tipStr;
};
