// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netif_detail_view_widget.h"
#include "netif_stat_view_widget.h"
#include "netif_summary_view_widget.h"
#include "system/system_monitor.h"

#include <DApplication>

using namespace core::system;
NetifDetailViewWidget::NetifDetailViewWidget(QWidget *parent)
    : BaseDetailViewWidget(parent)
{
    this->setObjectName("NetifDetailViewWidget");

    m_netifstatWIdget = new NetifStatViewWidget(this);
    m_netifsummaryWidget = new NetifSummaryViewWidget(this);

    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &NetifDetailViewWidget::updateData);
    connect(m_netifstatWIdget, &NetifStatViewWidget::netifItemClicked, m_netifsummaryWidget, &NetifSummaryViewWidget::onNetifItemClicked);

    setTitle(DApplication::translate("Process.Graph.View", "Network"));
    m_centralLayout->addWidget(m_netifstatWIdget);
    m_centralLayout->addWidget(m_netifsummaryWidget);

    detailFontChanged(DApplication::font());

    updateData();
}

NetifDetailViewWidget::~NetifDetailViewWidget()
{

}

void NetifDetailViewWidget::detailFontChanged(const QFont &font)
{
    BaseDetailViewWidget::detailFontChanged(font);

    m_netifstatWIdget->fontChanged(font);
    m_netifsummaryWidget->fontChanged(font);
}

void NetifDetailViewWidget::updateData()
{
    m_netifstatWIdget->onModelUpdate();
    m_netifsummaryWidget->onModelUpdate();
}
