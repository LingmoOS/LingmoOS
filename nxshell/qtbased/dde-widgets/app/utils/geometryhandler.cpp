// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "geometryhandler.h"

#include <DDBusInterface>

#include <QDebug>
#include <QApplication>
#include <QScrollArea>
#include <QScreen>

WIDGETS_FRAME_BEGIN_NAMESPACE
using MonitorInter = org::deepin::dde::display1::Monitor;
static const QString DisplayDaemonDBusServie = "org.deepin.dde.Display1";
static const QString DisplayDaemonDBusPath = "/org/deepin/dde/Display1";
static const QString DockDaemonDBusServie = "org.deepin.dde.daemon.Dock1";
static const QString DockDaemonDBusPath = "/org/deepin/dde/daemon/Dock1";
namespace Geo
{
    static const int DockMargin = 5;

    enum DockPosition {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };

    enum DockModel {
        Fashion = 0,
        Efficient = 1
    };
    static const int CenterMargin = 0;
}

GeometryHandler::GeometryHandler(QObject *parent)
    : QObject (parent)
{
    m_displayInter = new DisplayInter(DisplayDaemonDBusServie, DisplayDaemonDBusPath,
                                          QDBusConnection::sessionBus(), this);
    m_dockDeamonInter = new DockInter(DockDaemonDBusServie, DockDaemonDBusPath,
                                      QDBusConnection::sessionBus(), this);
    QObject::connect(m_displayInter, &DisplayInter::PrimaryRectChanged, this, &GeometryHandler::geometryChanged, Qt::UniqueConnection);
    QObject::connect(m_dockDeamonInter, &DockInter::FrontendWindowRectChanged, this, &GeometryHandler::dockFrontendWindowRectChanged, Qt::UniqueConnection);
}

GeometryHandler::~GeometryHandler()
{
    if (m_displayInter) {
        m_displayInter->deleteLater();
        m_displayInter = nullptr;
    }
    if (m_dockDeamonInter) {
        m_dockDeamonInter->deleteLater();
        m_dockDeamonInter = nullptr;
    }
}

QRect GeometryHandler::getGeometry(const int expectedWidth, const bool reduceDockHeight)
{
    QRect dockRect = m_dockDeamonInter->frontendWindowRect();
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    auto drect = QRect(dockRect.x(), dockRect.y(),
                        dockRect.width() / ratio, dockRect.height() / ratio);

    auto displayRect = calcDisplayRect(dockRect);

    auto dockPos = static_cast<Geo::DockPosition>(m_dockDeamonInter->position());
    auto dockMode = m_dockDeamonInter->displayMode();

    int height = displayRect.height() - Geo::CenterMargin * 2;
    if (dockPos == Geo::DockPosition::Top || dockPos == Geo::DockPosition::Bottom) {
        if (reduceDockHeight)
            height -= drect.height();

        if(dockMode == Geo::DockModel::Fashion) {
            if (drect.height() != 0) {
                height -= Geo::DockMargin * 2;
            }
        }
    }

    int x = displayRect.x() + displayRect.width() - expectedWidth - Geo::CenterMargin;
    if (dockPos == Geo::DockPosition::Right) {
        if(dockMode == Geo::DockModel::Fashion) {
            x =  displayRect.x() + displayRect.width() - (expectedWidth + drect.width() + Geo::DockMargin * 2 + Geo::CenterMargin);
            if (drect.width() == 0) {
                x += Geo::DockMargin * 2;
            }
        } else {
            x =  displayRect.x() + displayRect.width() - (expectedWidth + drect.width() + Geo::CenterMargin);
        }
    }

    int y = displayRect.y() + Geo::CenterMargin;
    if (dockPos == Geo::DockPosition::Top) {
        if(dockMode == Geo::DockModel::Fashion) {
            y = displayRect.y() + Geo::CenterMargin + drect.height() + Geo::DockMargin * 2;
        } else {
            y = displayRect.y() + Geo::CenterMargin + drect.height();
        }
    }

    return QRect(x - UI::Widget::WindowMargin, y, expectedWidth, height);
}

QRect GeometryHandler::calcDisplayRect(const QRect &dockRect)
{
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QRect displayRect = m_displayInter->primaryRect();
    displayRect = QRect(displayRect.x(), displayRect.y(),
                        displayRect.width() / ratio, displayRect.height() / ratio);

    QList<QDBusObjectPath> screenList = m_displayInter->monitors();

    for (const auto &screen : screenList) {
        MonitorInter monitor("org.deepin.dde.Display1", screen.path(), QDBusConnection::sessionBus());
        QRect monitorRect(monitor.x(), monitor.y(), monitor.width(), monitor.height());
        if (monitor.enabled() && monitorRect.contains(dockRect.center())) {
            displayRect = QRect(monitorRect.x(), monitorRect.y(),
                                monitorRect.width() / ratio, monitorRect.height() / ratio);
            break;
        }
    }
    return displayRect;
}
WIDGETS_FRAME_END_NAMESPACE
