// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_SUMMARY_VIEW_WIDGET_H
#define BLOCK_DEV_SUMMARY_VIEW_WIDGET_H

#include <DTableView>
DWIDGET_USE_NAMESPACE
/**
 * @brief Block device summary view widget
 */
class DeailTableModelBlock;
class BlockDevSummaryViewWidget : public DTableView
{
    Q_OBJECT
public:
    explicit BlockDevSummaryViewWidget(QWidget *parent = nullptr);

signals:

public slots:
    void fontChanged(const QFont &font);
    void chageSummaryInfo(const QString& deviceName);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QFont m_font;
    DeailTableModelBlock *m_model;
};

#endif // BLOCK_DEV_SUMMARY_VIEW_WIDGET_H
