// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorpluginbuttonwidget.h"

// Qt
#include <QSvgRenderer>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QIcon>
#include <QPainterPath>

//Dtk
#include <DGuiApplicationHelper>
#include <DStyle>
#include "dde-dock/constants.h"

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

MonitorPluginButtonWidget::MonitorPluginButtonWidget(QWidget *parent)
    : QWidget(parent)
    , m_hover(false)
    , m_pressed(false)
{
    setMouseTracking(true);
    setMinimumSize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        update();
    });
}

void MonitorPluginButtonWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QString iconName = "status-system-monitor";
    QString fallbackIconName = "dsm_pluginicon_light";
    QPixmap pixmap;
    int iconSize = PLUGIN_ICON_MAX_SIZE;

    QPainter painter(this);
    if (std::min(width(), height()) > PLUGIN_BACKGROUND_MIN_SIZE) {
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
        iconName = "status-system-monitor-dark";
        fallbackIconName = "dsm_pluginicon_dark";
    }

    const auto ratio = devicePixelRatioF();
    painter.setOpacity(1);

    pixmap = loadSvg(iconName, fallbackIconName, ":/icons/deepin/builtin/actions/", iconSize, ratio);

    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(pixmap.rect());

    painter.drawPixmap(rf.center() - rfp.center() / ratio, pixmap);
}

void MonitorPluginButtonWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

//    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
//    // 保持横纵比
//    if (position == Dock::Bottom || position == Dock::Top) {
//        setMinimumWidth(qBound(PLUGIN_BACKGROUND_MIN_SIZE, height(), PLUGIN_BACKGROUND_MAX_SIZE));
//    } else {
//        setMaximumHeight(width());
//        setMaximumWidth(QWIDGETSIZE_MAX);
//    }
//    update();
}

void MonitorPluginButtonWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (containCursorPos()) {
        m_hover = true;
    } else {
        m_hover = false;
    }

    QWidget::mouseMoveEvent(event);
}

void MonitorPluginButtonWidget::mousePressEvent(QMouseEvent *event)
{
    if (containCursorPos()) {
        m_pressed = true;
    } else {
        m_pressed = false;
    }

    update();

    QWidget::mousePressEvent(event);
}

void MonitorPluginButtonWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_hover = false;
    update();

    QWidget::mouseReleaseEvent(event);
}

void MonitorPluginButtonWidget::leaveEvent(QEvent *event)
{
    m_hover = false;
    m_pressed = false;
    update();

    QWidget::leaveEvent(event);
}

const QPixmap MonitorPluginButtonWidget::loadSvg(const QString &iconName, const QString &fallbackIconName, const QString &localPath, const int size, const qreal ratio)
{
    QIcon fallbackIcon = QIcon::fromTheme(fallbackIconName);
    QIcon icon = QIcon::fromTheme(iconName, fallbackIcon);
    int pixmapSize = QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? size : int(size * ratio);
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(pixmapSize);
        pixmap.setDevicePixelRatio(ratio);
        return pixmap;
    }

    QPixmap pixmap(pixmapSize, pixmapSize);
    QString localIcon = QString("%1%2%3").arg(localPath).arg(fallbackIconName + "_20px").arg(fallbackIconName.contains(".svg") ? "" : ".svg");
    QSvgRenderer renderer(localIcon);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();
    pixmap.setDevicePixelRatio(ratio);

    return pixmap;
}

bool MonitorPluginButtonWidget::containCursorPos()
{
    QPoint cursorPos = this->mapFromGlobal(QCursor::pos());
    QRect rect(this->rect());

    int iconSize = qMin(rect.width(), rect.height());
    int w = (rect.width() - iconSize) / 2;
    int h = (rect.height() - iconSize) / 2;

    rect = rect.adjusted(w, h, -w, -h);

    return rect.contains(cursorPos);
}




































