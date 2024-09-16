// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mem_detail_view_widget.h"
#include "mem_stat_view_widget.h"
#include "mem_summary_view_widget.h"
#include "system/system_monitor.h"

#include <DApplicationHelper>
#include <DApplication>
#include <QVBoxLayout>

using namespace core::system;
MemDetailViewWidget::MemDetailViewWidget(QWidget *parent)
    : BaseDetailViewWidget(parent)
{
    this->setObjectName("MemDetailViewWidget");
    m_memstatWIdget = new MemStatViewWidget(this);
    m_memsummaryWidget = new MemSummaryViewWidget(this);

    setTitle(DApplication::translate("Process.Graph.Title", "Memory"));
    m_centralLayout->addWidget(m_memstatWIdget);
    m_centralLayout->addWidget(m_memsummaryWidget);

    detailFontChanged(DApplication::font());

    onModelUpdate();
    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &MemDetailViewWidget::onModelUpdate);

    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
                this, &MemDetailViewWidget::detailFontChanged);
}

void MemDetailViewWidget::onModelUpdate()
{
    m_memstatWIdget->onModelUpdate();
    m_memsummaryWidget->onModelUpdate();
}

void MemDetailViewWidget::detailFontChanged(const QFont &font)
{
    BaseDetailViewWidget::detailFontChanged(font);
    m_memstatWIdget->fontChanged(font);
    m_memsummaryWidget->fontChanged(font);
}
