// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_STAT_VIEW_WIDGET_H
#define BLOCK_DEV_STAT_VIEW_WIDGET_H

#include <QMap>
#include <QScrollArea>
#include "system/block_device.h"

using namespace core::system;

class BlockDevItemWidget;
class BlockStatViewWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit BlockStatViewWidget(QWidget *parent = nullptr);

signals:
    void changeInfo(const QString &deviceName);

protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void updateWidgetGeometry();
    void onUpdateData();
    void fontChanged(const QFont &font);
    void onSetItemStatus(const QString &deviceName);

private:
    void showItem1();
    void showItem2();
    void showItemLg2(int count);
    void resetMapInfo();

private:
    QList<BlockDevice> m_listDevice;
    QList<BlockDevItemWidget *> m_listBlockItemWidget;

    QMap<QString, BlockDevItemWidget *> m_mapDeviceItemWidget;

    QWidget *m_centralWidget;
    QFont m_font;
};

#endif // BLOCK_DEV_STAT_VIEW_WIDGET_H
