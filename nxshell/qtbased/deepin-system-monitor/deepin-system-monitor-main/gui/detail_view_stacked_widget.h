// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAIL_VIEW_STACKED_WIDGET_H
#define DETAIL_VIEW_STACKED_WIDGET_H

#include "animation_stackedwidget.h"
#include <DMenu>

/**
 * @brief Detail view stacked widget (main content stacked area)
 */
class MemDetailViewWidget;
class NetifDetailViewWidget;
class BlockDevDetailViewWidget;

DWIDGET_USE_NAMESPACE
class DetailViewStackedWidget : public AnimationStackedWidget
{
public:
    explicit DetailViewStackedWidget(QWidget *parent);

    virtual ~DetailViewStackedWidget();

    void addProcessWidget(QWidget *processWidget);

    void deleteDetailPage();

public slots:
    void onShowPerformMenu(QPoint pos);
    void onDetailInfoClicked();
    void onSwitchProcessPage();
    void onSwitchPageFinished();

    void onDbusSendMsgChangeDetailInfoWidget(QString msgCode);

private:
    QString m_lastDteailWidgetName;

    QWidget *m_processWidget = nullptr;
    QWidget *m_cpudetailWidget = nullptr;
    QWidget *m_memDetailWidget = nullptr;
    QWidget *m_netifDetailWidget = nullptr;
    QWidget *m_blockDevDetailWidget = nullptr;

    QAction *cpuAct;
    QAction *memAct;
    QAction *netifAct;
    QAction *blockDevAct;

    DMenu *m_menu = nullptr;
};

#endif // DETAIL_VIEW_STACKED_WIDGET_H
