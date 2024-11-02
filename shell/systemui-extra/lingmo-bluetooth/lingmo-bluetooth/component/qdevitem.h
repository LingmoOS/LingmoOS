/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef QDEVITEM_H
#define QDEVITEM_H

#include <config/config.h>
#include <QFrame>
#include <KF5/BluezQt/bluezqt/device.h>
#include <QLabel>
#include <QIcon>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <lingmo-log4qt.h>
#include <QToolTip>

#include "config/config.h"
#define POWERMANAGER_SCHEMA         "org.lingmo.power-manager"
#define PERCENTAGE_LOW              "percentage-low"
#define PERCENTAGE_LOW_KEY          "percentageLow"
#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

class QDevItem : public QWidget
{
    Q_OBJECT
public:
    enum Status{
        Hover = 0,
        Nomal,
        Click,
    };
    Q_ENUM(Status)

    int FontTable[6] = {32, 30, 26, 24, 23, 22};

    QDevItem(QString address, QMap<QString, QVariant> devAttr, QWidget *parent = nullptr);
    ~QDevItem();
    void setFocusStyle(bool val);
    void pressEnterCallback();
    void startIconTimer();

    void InitMemberVariables();
    bool isConnected();

signals:
    void devConnect(bool);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    bool event(QEvent *event);

private:
    QMap<int, QString> *typeMap = nullptr;
    QColor getPainterBrushColor();
    QColor getDevStatusColor();
    QPixmap getDevTypeIcon();
    void initMap();
    void DrawBackground(QPainter &);
    void DrawStatusIcon(QPainter &);
    void DrawBattery(QPainter &painter);
    void DrawText(QPainter &);
    void MouseClickedFunc();
    void DevConnectFunc();

    int iconFlag = 7;
    int m_Battery = -1;
    int powerPercentage;

    Status _MStatus;
    double tran =1;
    bool _clicked;
    bool _pressFlag;
    bool _leaved = false;
    bool _focused = false;
    bool _clickEnabled = true;

    QMap<QString, QVariant> _MDev;
    QTimer        *_iconTimer = nullptr;
    QLabel        *m_pBatteyIconLabel = nullptr;
    QGSettings *styleGSetting = nullptr;
    QGSettings *transparencyGsetting = nullptr;
    QGSettings *powerGsetting = nullptr;

public slots:
    void attrChangedSlot(QMap<QString, QVariant> devAttr);
};

#endif // QDEVITEM_H
