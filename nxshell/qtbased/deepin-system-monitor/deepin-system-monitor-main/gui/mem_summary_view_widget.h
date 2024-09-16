// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEM_SUMMARY_VIEW_WIDGET_H
#define MEM_SUMMARY_VIEW_WIDGET_H

#include <DTableView>

/**
 * @brief Memory summary view widget
 */
DWIDGET_USE_NAMESPACE
class DeailTableModel;
class MemSummaryViewWidget : public DTableView
{
    Q_OBJECT
public:
    explicit MemSummaryViewWidget(QWidget *parent = nullptr);

public slots:
    void fontChanged(const QFont &font);
    void onModelUpdate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QFont m_font;
    DeailTableModel *m_model;
};

#endif // MEM_SUMMARY_VIEW_WIDGET_H
