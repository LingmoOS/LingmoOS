// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_DETAIL_VIEW_WIDGET_H
#define NETIF_DETAIL_VIEW_WIDGET_H

#include "base/base_detail_view_widget.h"

/**
 * @brief Network interface detail view widget
 */
class NetifStatViewWidget;
class NetifSummaryViewWidget;
class NetifDetailViewWidget : public BaseDetailViewWidget
{
    Q_OBJECT

public:
    explicit NetifDetailViewWidget(QWidget *parent = nullptr);
    virtual ~NetifDetailViewWidget();

public slots:
    void detailFontChanged(const QFont &font);
    void updateData();   // 更新数据

private:
    NetifStatViewWidget *m_netifstatWIdget;
    NetifSummaryViewWidget *m_netifsummaryWidget;

};

#endif // NETIF_DETAIL_VIEW_WIDGET_H
