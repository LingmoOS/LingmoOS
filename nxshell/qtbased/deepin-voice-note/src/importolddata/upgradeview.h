// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEVIEW_H
#define UPGRADEVIEW_H

#include <DLabel>
#include <DWaterProgress>

#include <QShowEvent>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class UpgradeView : public QWidget
{
    Q_OBJECT
public:
    explicit UpgradeView(QWidget *parent = nullptr);
    //开始升级
    void startUpgrade();
signals:
    //升级完成
    void upgradeDone();
public slots:
    //设置进度
    void setProgress(int progress);
    //数据从数据库中加载完成，准备升级
    void onDataReady();
    //升级结束
    void onUpgradeFinish();

protected:
    //连接槽函数
    void initConnections();

private:
    DWaterProgress *m_waterProgress {nullptr};
    DLabel *m_tooltipTextLabel {nullptr};
};

#endif // UPGRADEVIEW_H
