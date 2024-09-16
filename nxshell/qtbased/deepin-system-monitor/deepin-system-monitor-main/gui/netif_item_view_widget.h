// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_ITEM_VIEW_WIDGET_H
#define NETIF_ITEM_VIEW_WIDGET_H

#include <QWidget>
#include <memory>

namespace core {
namespace system {
class NetifInfo;
}
}

class ChartViewWidget;
class NetifItemViewWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };

    explicit  NetifItemViewWidget(QWidget *parent = nullptr, const QByteArray &mac = "");

public:
    void updateActiveStatus(bool active);
    void setMode(int mode);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(const QString &mac);

public slots:
    void fontChanged(const QFont &font);
    void updateData(const std::shared_ptr<class core::system::NetifInfo> &netifInfo);

private:
    void updateWidgetGeometry();

private:
    int m_mode = TITLE_HORIZONTAL;
    QColor m_recvColor {"#E14300"};
    QColor m_sentColor {"#004EEF"};

    QString m_ifname;   // 网卡名称
    QString m_recv_bps; // 接受速率
    QString m_sent_bps; // 发送速率
    ChartViewWidget *m_ChartWidget;

    QByteArray m_mac;

    QFont m_font;
    bool m_isActive = false;  // 是否被点击
};

#endif // NETIF_ITEM_VIEW_WIDGET_H
