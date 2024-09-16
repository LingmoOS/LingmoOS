// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconwidget.h"
#include "dde-dock/constants.h"

#include <DGuiApplicationHelper>
#include <DStyle>
#include <DSysInfo>

#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPixmap>
#include <QThread>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QPainterPath>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
DCORE_USE_NAMESPACE

IconWidget::IconWidget(QWidget *parent):
    QWidget(parent),
    m_blgPixmap(nullptr),
    centralLayout(nullptr)
{
    m_systemVersion = DSysInfo::minorVersion().toInt() ;
    setMouseTracking(true);
    setMinimumSize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);

    QString iconName("screen-capture");
    if(m_systemVersion >= 1070 || DSysInfo::deepinType() == DSysInfo::DeepinDesktop){
        iconName = "screenshot";
    }
    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
}

bool IconWidget::enabled()
{
    return isEnabled();
}

const QString IconWidget::itemContextMenu()
{
    QList<QVariant> items;
    items.reserve(2);
    QMap<QString, QVariant> shot;
    shot["itemId"] = "shot";
    shot["itemText"] = tr("Screenshot") + getSysShortcuts("screenshot");
    shot["isActive"] = true;
    items.push_back(shot);


    QMap<QString, QVariant> recorder;
    recorder["itemId"] = "recorder";
    recorder["itemText"] = tr("Recording") + getSysShortcuts("deepin-screen-recorder");
    recorder["isActive"] = true;
    items.push_back(recorder);


    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void IconWidget::invokedMenuItem(const QString &menuId)
{

    if (menuId == "shot") {
        QDBusInterface shotDBusInterface("com.deepin.Screenshot",
                                         "/com/deepin/Screenshot",
                                         "com.deepin.Screenshot",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("StartScreenshot");
    } else if(menuId == "recorder") {
        QDBusInterface shotDBusInterface("com.deepin.ScreenRecorder",
                                         "/com/deepin/ScreenRecorder",
                                         "com.deepin.ScreenRecorder",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("stopRecord");
    }
}

QString IconWidget::getSysShortcuts(const QString type)
{
    QDBusInterface shortcuts("com.deepin.daemon.Keybinding", "/com/deepin/daemon/Keybinding", "com.deepin.daemon.Keybinding");
    if (!shortcuts.isValid()) {
        return getDefaultValue(type);
    }

    QDBusReply<QString> shortLists = shortcuts.call(QStringLiteral("ListAllShortcuts"));
    QJsonDocument doc = QJsonDocument::fromJson(shortLists.value().toUtf8());
    QJsonArray shorts = doc.array();
    QMap<QString, QString> shortcutsMap;

    for (QJsonValue shortcut : shorts) {
        const QString Id = shortcut["Id"].toString();
        if (Id == type) {
            QJsonArray Accels = shortcut["Accels"].toArray();
            QString AccelsString;
            for (QJsonValue accel : Accels)  {
                AccelsString += accel.toString();
            }
            AccelsString.remove('<');
            AccelsString.replace('>', '+');
            AccelsString.replace("Control", "Ctrl");
            AccelsString = "(" + AccelsString + ")";
            return AccelsString;
        }
    }
    return getDefaultValue(type);
}

QString IconWidget::getDefaultValue(const QString type)
{
    QString retShortcut;
    if (type == "screenshot") {
        retShortcut = "Ctrl+Alt+A";
    } else if (type == "deepin-screen-recorder") {
        retShortcut = "Ctrl+Alt+R";
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "Shortcut Error !!!!!!!!!" << type;
    }
    return retShortcut;
}


void IconWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    QPixmap pixmap;
    QString iconName = "screen-capture";
    if(m_systemVersion >= 1070 || DSysInfo::deepinType() == DSysInfo::DeepinDesktop){
        iconName = "screenshot";
    }
    int iconSize = PLUGIN_ICON_MAX_SIZE;

    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {

        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0.5);

            if (m_hover) {
                painter.setOpacity(0.6);
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0.1);

            if (m_hover) {
                painter.setOpacity(0.2);
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, true);

        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);

        QPainterPath path;

        int minSize = std::min(width(), height());
        QRect rc(0, 0, minSize, minSize);
        rc.moveTo(rect().center() - rc.center());

        path.addRoundedRect(rc, radius, radius);
        painter.fillPath(path, color);
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        // 最小尺寸时，不画背景，采用深色图标
        iconName.append(PLUGIN_MIN_ICON_NAME);
    }

    painter.setOpacity(1);

    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
    pixmap = loadSvg(iconName, QSize(iconSize, iconSize));

    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(pixmap.rect());
    painter.drawPixmap(rf.center() - rfp.center() / pixmap.devicePixelRatioF(), pixmap);

    QWidget::paintEvent(e);
}

void IconWidget::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    update();

    QWidget::mousePressEvent(event);
}

void IconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_hover = false;
    update();

    QWidget::mouseReleaseEvent(event);
}

void IconWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_hover = true;
    QWidget::mouseMoveEvent(event);
}

void IconWidget::leaveEvent(QEvent *event)
{
    m_hover = false;
    m_pressed = false;
    update();

    QWidget::leaveEvent(event);
}

const QPixmap IconWidget::loadSvg(const QString &fileName, const QSize &size) const
{
    const auto ratio = devicePixelRatioF();

    auto pixmapSize = QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? size : (size * ratio);
    // 缩放模式 设置为非使能状态时，调整转出的位图风格模式
    // pixmapSize = size* ratio;
    QPixmap pixmap = QIcon::fromTheme(fileName, m_icon).pixmap(pixmapSize, isEnabled() ? QIcon::Normal : QIcon::Disabled);

    return pixmap;
}

IconWidget::~IconWidget()
{

}
