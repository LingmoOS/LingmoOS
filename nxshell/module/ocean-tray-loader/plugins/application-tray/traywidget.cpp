// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traywidget.h"
#include "abstracttrayprotocol.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <dapplication.h>
#include <QTimer>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
namespace tray {
uint16_t trayIconSize = 16;
TrayWidget::TrayWidget(QPointer<AbstractTrayProtocolHandler> handler)
    : QWidget()
    , m_handler(handler)
    , m_attentionTimer(new QTimer(this))
{
    m_attentionTimer->setSingleShot(true);

    // TODO: read from config get attention time
    m_attentionTimer->setInterval(1000);

    setWindowTitle(m_handler->id());
    setFixedSize(trayIconSize, trayIconSize);

    m_handler->setParent(this);
    installEventFilter(m_handler);
    setMouseTracking(true);

    connect(m_handler, &AbstractTrayProtocolHandler::iconChanged, this, [this](){update();});
    connect(m_handler, &AbstractTrayProtocolHandler::overlayIconChanged, this, [this](){update();});
    connect(m_handler, &AbstractTrayProtocolHandler::attentionIconChanged, this, [this](){
        m_attentionTimer->start();
        update();
    });

    connect(m_attentionTimer, &QTimer::timeout, this, [this]{update();});
}

TrayWidget::~TrayWidget()
{
}

void TrayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    // TODO: support attentionIcon/overlayIcon
    QPalette palette;
    auto textColor = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? Qt::black : Qt::white;
    palette.setColor(QPalette::WindowText, textColor);
    setPalette(palette);

    QPainter painter(this);
    if(m_attentionTimer->isActive()) {
        m_handler->attentionIcon().paint(&painter, QRect(0, 0, trayIconSize, trayIconSize));
    } else {
        QIcon icon = m_handler->icon();
        if (icon.isNull()) {
            static const QIcon defaultIcon = QIcon::fromTheme("application-x-desktop");
            icon = defaultIcon;
        }
        icon.paint(&painter, QRect(0, 0, trayIconSize, trayIconSize));
    }
}
}
