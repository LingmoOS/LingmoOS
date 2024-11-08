/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tooltip-dialog.h"

#include <QDebug>
#include <QFile>
#include <QPlatformSurfaceEvent>
#include <QQmlEngine>
#include <QQuickItem>
#include <KWindowSystem>
#include <QGuiApplication>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformcursor.h>
#include <private/qhighdpiscaling_p.h>
#include <QScreen>

using namespace LingmoUIQuick;
TooltipDialog::TooltipDialog(QQuickItem *parent)
    : Dialog(parent)
    , m_qmlObject(nullptr)
    , m_hideTimeout(4000)
    , m_interactive(false)
    , m_extendTimeoutFlag(m_extendTimeoutFlags::None)
    , m_owner(nullptr)
    , m_posFollowCursor(true)
{
    setLocation(LingmoUIQuick::Dialog::Floating);
    setType(LingmoUIQuick::WindowType::ToolTip);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, [this]() {
        setVisible(false);
    });
}

TooltipDialog::~TooltipDialog()
{
}

QQuickItem *TooltipDialog::loadDefaultItem()
{
    if (!m_qmlObject) {
        m_qmlObject = new SharedEngineComponent();
    }

    if (!m_qmlObject->rootObject()) {

        m_qmlObject->setSource(QUrl("qrc:/DefaultTooltip.qml"));

    }
    return qobject_cast<QQuickItem *>(m_qmlObject->rootObject());
}

void TooltipDialog::showEvent(QShowEvent *event)
{
    if (m_hideTimeout > 0) {
        m_showTimer->start(m_hideTimeout);
    }
    Dialog::showEvent(event);
}

void TooltipDialog::hideEvent(QHideEvent *event)
{
    m_showTimer->stop();

    Dialog::hideEvent(event);
}

void TooltipDialog::resizeEvent(QResizeEvent *re)
{
    Dialog::resizeEvent(re);
}

bool TooltipDialog::event(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::Enter:
        if (m_interactive) {
            m_showTimer->stop();
        }
        break;
    case QEvent::Leave:
        if (m_extendTimeoutFlag == (m_extendTimeoutFlags::Resized | m_extendTimeoutFlags::Moved)) { 
            keepalive(); // HACK: prevent tooltips from being incorrectly dismissed (BUG439522)
        } else {
            dismiss();
        }
        m_extendTimeoutFlag = m_extendTimeoutFlags::None;
        break;
    case QEvent::Resize:
        m_extendTimeoutFlag = m_extendTimeoutFlags::Resized;
        break;
    case QEvent::Move:
        m_extendTimeoutFlag |= m_extendTimeoutFlags::Moved;
        break;
    case QEvent::MouseMove:
        m_extendTimeoutFlag = m_extendTimeoutFlags::None;
    }

    bool ret = Dialog::event(e);
    Qt::WindowFlags flags = Qt::ToolTip | Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint;
    setFlags(flags);
    return ret;
}

QObject *TooltipDialog::owner() const
{
    return m_owner;
}

void TooltipDialog::setOwner(QObject *owner)
{
    m_owner = owner;
}

void TooltipDialog::dismiss()
{
    m_showTimer->start(m_hideTimeout / 20); // pretty short: 200ms
}

void TooltipDialog::keepalive()
{
    m_showTimer->start(m_hideTimeout);
}

bool TooltipDialog::interactive()
{
    return m_interactive;
}

void TooltipDialog::setInteractive(bool interactive)
{
    m_interactive = interactive;
}

void TooltipDialog::valueChanged(const QVariant &value)
{
    setPosition(value.toPoint());
}

void TooltipDialog::setHideTimeout(int timeout)
{
    m_hideTimeout = timeout;
}

int TooltipDialog::hideTimeout() const
{
    return m_hideTimeout;
}

QPoint TooltipDialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if(m_posFollowCursor) {
        return posByCursor();
    }
    return Dialog::popupPosition(item, size);

}

bool TooltipDialog::posFollowCursor() const
{
    return m_posFollowCursor;
}

void TooltipDialog::setPosFollowCursor(bool follow)
{
    m_posFollowCursor = follow;
}

QPoint TooltipDialog::posByCursor()
{
    QPoint p = QCursor::pos();
    const QScreen *screen =QGuiApplication::screenAt(QCursor::pos());
    if (const QPlatformScreen *platformScreen = screen ? screen->handle() : nullptr) {
        QPlatformCursor *cursor = platformScreen->cursor();
        const QSize nativeSize = cursor ? cursor->size() : QSize(16, 16);
        const QSize cursorSize = QHighDpi::fromNativePixels(nativeSize, platformScreen);
        QPoint offset(2, cursorSize.height());
        if (cursorSize.height() > 2 * this->height()) {
            offset = QPoint(cursorSize.width() / 2, 0);
        }
        p += offset;

        QRect screenRect = screen->geometry();
        if (p.x() + this->width() > screenRect.x() + screenRect.width())
            p.rx() -= 4 + this->width();
        if (p.y() + this->height() > screenRect.y() + screenRect.height())
            p.ry() -= 24 + this->height();
        if (p.y() < screenRect.y())
            p.setY(screenRect.y());
        if (p.x() + this->width() > screenRect.x() + screenRect.width())
            p.setX(screenRect.x() + screenRect.width() - this->width());
        if (p.x() < screenRect.x())
            p.setX(screenRect.x());
        if (p.y() + this->height() > screenRect.y() + screenRect.height())
            p.setY(screenRect.y() + screenRect.height() - this->height());
    }
    return p;
}

void TooltipDialog::setMargin(int margin)
{
    Dialog::setMargin(margin);
}

#include "moc_tooltip-dialog.cpp"
