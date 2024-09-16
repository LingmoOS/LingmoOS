// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEM_STAT_VIEW_WIDGET_H
#define MEM_STAT_VIEW_WIDGET_H

#include <QWidget>
#include <DCommandLinkButton>

namespace core {
namespace system {
class MemInfo;
}
}

DWIDGET_USE_NAMESPACE
class ChartViewWidget;
class MemStatViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MemStatViewWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

public slots:
    void fontChanged(const QFont &font);
    void onModelUpdate();

private:
    void updateWidgetGeometry();

private:
    QColor memoryColor {"#00C5C0"};
    QColor swapColor {"#FEDF19"};

    ChartViewWidget *m_memChartWidget;
    ChartViewWidget *m_swapChartWidget;

    core::system::MemInfo *m_memInfo;

    QFont m_font;
};

#endif // MEM_STAT_VIEW_WIDGET_H
