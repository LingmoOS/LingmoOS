// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEM_DETAIL_VIEW_WIDGET_H
#define MEM_DETAIL_VIEW_WIDGET_H

#include "base/base_detail_view_widget.h"

/**
 * @brief Memory detail view widget
 */
class MemStatViewWidget;
class MemSummaryViewWidget;
class MemDetailViewWidget : public BaseDetailViewWidget
{
    Q_OBJECT

public:
    explicit MemDetailViewWidget(QWidget *parent = nullptr);

public slots:
    void detailFontChanged(const QFont &font);
    void onModelUpdate();

private:
    MemStatViewWidget *m_memstatWIdget;
    MemSummaryViewWidget *m_memsummaryWidget;
};

#endif // MEM_DETAIL_VIEW_WIDGET_H
