// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_STAT_VIEW_WIDGET_H
#define NETIF_STAT_VIEW_WIDGET_H

#include <DScrollArea>
#include <QMap>

class ChartViewWidget;
class NetifInfoModel;
class QGridLayout;
class NetifItemViewWidget;

namespace core {
namespace system {
class NetifInfoDB;
}
}

DWIDGET_USE_NAMESPACE
class NetifStatViewWidget : public DScrollArea
{
    Q_OBJECT
public:
    explicit NetifStatViewWidget(QWidget *parent = nullptr);
    void updateWidgetGeometry();

signals:
    void netifItemClicked(const QString &mac);

private:
    void showItemOnlyeOne();
    void showItemDouble();
    void showItemLgDouble();

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void fontChanged(const QFont &font);

    void onModelUpdate();
    void onSetItemActiveStatus(const QString &mac);

private:
    core::system::NetifInfoDB *m_info;

    bool m_initStatus = false;
    QWidget *m_centralWidget;
    QByteArray m_currentMac;
    QMap<QByteArray, NetifItemViewWidget *> m_mapItemView;
};

#endif // NETIF_STAT_VIEW_WIDGET_H
