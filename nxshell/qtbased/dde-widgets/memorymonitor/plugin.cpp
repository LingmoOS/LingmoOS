// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "common/helper.hpp"
#include "accessible/accessible.h"

#include "common/utils.h"
#include "handler/mem.h"

#include <QAccessible>
#include <QDBusInterface>

QString MemoryMonitorWidgetPlugin::title() const
{
    return tr("MemoryMonitor");
}

QString MemoryMonitorWidgetPlugin::description() const
{
    return tr("Memory Monitor");
}

IWidget *MemoryMonitorWidgetPlugin::createWidget()
{
    return new MemoryMonitorWidget();
}

void MemoryMonitorWidget::updateMemory()
{
    using namespace Utils;

    core::system::MemInfo info;
    info.readMemInfo();

    Q_ASSERT(info.memTotal() > 0);

    QString swapUsage("");
    QString swapPercent("");

    QString memPercent = QString::number((info.memTotal() - info.memAvailable()) * 1. / info.memTotal() * 100, 'f', 1);

    if (info.swapTotal() > 0) {
        swapUsage = formatUnit((info.swapTotal() - info.swapFree()) << 10, B, 1);
        swapPercent = QString::number((info.swapTotal() - info.swapFree()) * 1. / info.swapTotal() * 100, 'f', 1);
    }
    // This process is the same as `deepin-system-monitor-plugin`
    if (swapUsage.split(" ").size() != 2)
        swapPercent = "";

    if (m_view) {
        m_view->updateMemoryInfo(memPercent, swapPercent);
        m_view->update();
    }
}

bool MemoryMonitorWidget::initialize(const QStringList &arguments) {
    Q_UNUSED(arguments);
    static bool hasLoaded = false;
    if (!hasLoaded)
        hasLoaded = BuildinWidgetsHelper::instance()->loadTranslator("dde-widgets-memorymonitor_");

    m_view = new MemoryWidget();
    m_view->setRoundedCornerRadius(handler()->roundedCornerRadius());
    m_timer.reset(new QBasicTimer());
    m_view->installEventFilter(this);

    return true;
}

void MemoryMonitorWidget::delayInitialize()
{
    // enable accessible
    QAccessible::installFactory(memoryMonitorAccessibleFactory);
}

void MemoryMonitorWidget::typeChanged(const IWidget::Type type)
{
    Q_UNUSED(type)
    m_view->setFixedSize(handler()->size());
}

void MemoryMonitorWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer->timerId())
        updateMemory();

    return QObject::timerEvent(event);
}

void MemoryMonitorWidget::showWidgets()
{
    updateMemory();

    const int Interval = 1000;
    m_timer->start(Interval, this);
}

void MemoryMonitorWidget::hideWidgets()
{
    m_timer->stop();
}

bool MemoryMonitorWidget::eventFilter(QObject *watched, QEvent *event)
{
    do {
        if (watched != m_view)
            break;
        if (event->type() != QEvent::MouseButtonPress &&
                event->type() != QEvent::MouseButtonRelease)
            break;
        auto me = dynamic_cast<QMouseEvent *>(event);
        if (me->button() != Qt::LeftButton)
            break;

        if (event->type() == QEvent::MouseButtonPress) {
            m_isPressed = true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            if (m_isPressed) {
                showSystemMonitorDetail();
                m_isPressed = false;
            }
        }
    } while (false);

    return QObject::eventFilter(watched, event);
}

void MemoryMonitorWidget::showSystemMonitorDetail()
{
    QDBusInterface interface("com.deepin.SystemMonitorMain", "/com/deepin/SystemMonitorMain",
                              "com.deepin.SystemMonitorMain");
     if (!interface.isValid()) {
         qWarning() << "Get com.deepin.SystemMonitorMain interface error." << interface.lastError().message();
         return;
     }
     qDebug() << "showSystemMonitorDetail()";
     const QString MsgCodeName("MSG_PROCESS");
     interface.call("slotJumpProcessWidget", MsgCodeName);
}

QIcon MemoryMonitorWidgetPlugin::logo() const
{
    return QIcon();
}

QStringList MemoryMonitorWidgetPlugin::contributors() const
{
    return {BuildinWidgetsHelper::instance()->contributor()};
}
