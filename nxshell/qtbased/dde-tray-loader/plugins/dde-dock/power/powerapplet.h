// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POWERAPPLET_H
#define POWERAPPLET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSpacerItem>
#include <QHBoxLayout>

#include <DTipLabel>

#include "jumpsettingbutton.h"
#include "pluginlistview.h"
#include "commoniconbutton.h"

class PowerApplet : public QWidget
{
    Q_OBJECT

public:
    explicit PowerApplet(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setDescription(const QString &description);
    void setDccPage(const QString &first, const QString &second);
    void setMinHeight(int minHeight);

    void refreshBatteryIcon(const QString &icon);
    void refreshBatteryPercentage(const QString &percentage, const QString &tips);

Q_SIGNALS:
    void requestHideApplet();

public Q_SLOTS:
    void onModeChanged(const QModelIndex &index);
    void onCurPowerModeChanged(const QString &curPowerMode);
    void onHighPerformanceSupportChanged(const bool isSupport);

protected:
    bool eventFilter(QObject *watcher, QEvent *event);

private:
    void initUI();
    void initConnect();
    void initData();
    void resizeApplet();

private:
    QWidget *m_titleWidget;
    QHBoxLayout *m_titleLayout;
    DLabel *m_titleLabel;
    PluginListView *m_view;
    QStandardItemModel *m_model;
    JumpSettingButton *m_settingButton;
    QWidget *m_batteryWidget;
    CommonIconButton *m_batteryIcon;
    DLabel *m_batteryPercentage;
    DLabel *m_batteryTips;

    int m_minHeight;
};

#endif // POWERAPPLET_H
