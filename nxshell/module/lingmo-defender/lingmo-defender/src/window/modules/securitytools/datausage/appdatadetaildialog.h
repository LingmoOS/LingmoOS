// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dateusagewidgets/appdatachart.h"
#include "datausagemodel.h"
#include "window/modules/common/common.h"

#include <DBlurEffectWidget>
#include <DFrame>
#include <DLabel>
#include <DPushButton>
#include <DTipLabel>

#include <QWidget>
#include <QFrame>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

DEF_NAMESPACE_BEGIN
DEF_NETPROTECTION_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE

class NetworkProtectionModel;
class NetworkProtectionWorker;
class AppDataDetailDialog : public DFrame
{
    Q_OBJECT
public:
    explicit AppDataDetailDialog(QString appname = "", QString pidname = "", QString icon = "",
                                 DataUsageModel *model = nullptr, QWidget *parent = nullptr);
public Q_SLOTS:
    void selectTimeRangeStyle(QAction *action);

protected:
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    void initUI();
    //更新此时间段应用总流量数据
    void updateThisRangeUsageNum();

private:
    DataUsageModel *m_model;
    DBlurEffectWidget *m_blurEffect;
    QWidget *m_mainWidgt;
    QString m_appName;
    QString m_pkgName;
    QString m_appIcon;
    DTipLabel *m_totalUsageNum;
    DTipLabel *m_downloadedUsageNum;
    DTipLabel *m_uploadedUsageNum;
    DPushButton *m_timeRangeSelect;
    AppDataChart *m_chart;
};

DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END
