// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_ITEM_WIDGET_H
#define BLOCK_DEV_ITEM_WIDGET_H

#include <QObject>
#include <QWidget>
#include "system/block_device.h"

using namespace core::system;
class ChartViewWidget;
class BlockDevInfoModel;
class QGridLayout;
class BlockDevItemWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };

    explicit BlockDevItemWidget(QWidget *parent = nullptr);
    virtual ~BlockDevItemWidget() override;

signals:
    void clicked(const QString &deviceName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

public slots:
    void fontChanged(const QFont &font);
    void activeItemWidget(bool isShow);

public:
    void updateData(const BlockDevice &info);
    void setMode(int mode);
    bool isActiveItem() { return  m_isActive;}

private:
    void updateWidgetGeometry();

private:
    int m_mode = TITLE_HORIZONTAL;
    QColor readColor {"#8F88FF"};
    QColor writeColor {"#6AD787"};
    ChartViewWidget *m_memChartWidget;

    QFont m_font;
    BlockDevice  m_blokeDeviceInfo;
    QList<qreal> m_listWriteSpeed;
    QList<qreal> m_listReadSpeed;
    bool m_isActive = false;
};

#endif // BLOCK_DEV_ITEM_WIDGET_H
