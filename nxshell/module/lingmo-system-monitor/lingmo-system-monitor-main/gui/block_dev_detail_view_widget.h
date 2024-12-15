// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_DETAIL_VIEW_WIDGET_H
#define BLOCK_DEV_DETAIL_VIEW_WIDGET_H

#include "base/base_detail_view_widget.h"

/**
 * @brief Block device detail view widget
 */
class BlockStatViewWidget;
class BlockDevSummaryViewWidget;
class BlockDevDetailViewWidget : public BaseDetailViewWidget
{
    Q_OBJECT
public:
    explicit BlockDevDetailViewWidget(QWidget *parent = nullptr);
    virtual ~BlockDevDetailViewWidget();

public slots:
    void detailFontChanged(const QFont &font);

private:
    BlockStatViewWidget *m_blockStatWidget;
    BlockDevSummaryViewWidget *m_blocksummaryWidget;

};

#endif // BLOCK_DEV_DETAIL_VIEW_WIDGET_H
