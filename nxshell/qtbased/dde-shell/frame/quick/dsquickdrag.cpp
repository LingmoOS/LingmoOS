// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsquickdrag_p.h"

#include <QLoggingCategory>
#include <QCoreApplication>
#include <QPointer>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformdrag.h>
#define protected public
#include <private/qsimpledrag_p.h>
#undef protected

#include <dobject_p.h>

DS_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(dsDragLog, "dde.shell.drag")

class DragWindowEventFilter : public QObject {
public:
    explicit DragWindowEventFilter(DQuickDragPrivate *drag)
        : m_drag(drag)
    {
    }
    ~DragWindowEventFilter() override
    {
    }

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWindow *dragWindow(QObject *object) const
    {
        auto platformDrag = QGuiApplicationPrivate::platformIntegration()->drag();
        if (platformDrag && platformDrag->ownsDragObject()) {
            if (auto drag = dynamic_cast<QBasicDrag *>(platformDrag)) {
                if (object == reinterpret_cast<QObject *>(drag->shapedPixmapWindow()))
                    return qobject_cast<QWindow *>(object);
            }
        }
        return nullptr;
    }
    DQuickDragPrivate *m_drag {nullptr};
};

class DQuickDragPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    D_DECLARE_PUBLIC(DQuickDrag)
    explicit DQuickDragPrivate(DQuickDrag *qq);
    virtual ~DQuickDragPrivate() override;

    void clear();
    void initDragOverlay(QWindow *dragWindow);
    void releaseDragOverlay();
    void updateOverlayPosition(const QPoint &pos);
    void showOverlay();
    void hideOverlay();

private:
    QWindow *createDragOverlay();
    void updateStartDragPoint();
    void updateCurrentDragPoint();
    QPoint hotSpot() const;

    QPointer<QWindow> m_window {nullptr};
    QPointer<DragWindowEventFilter> m_filter;
    bool m_active {false};
    QSizeF m_hotSpotScale {};
    QPoint m_startDragPoint {0, 0};
    QPoint m_currentDragPoint {0, 0};
    QQmlComponent *m_overlay = nullptr;
    bool m_isDragging = false;
};

bool DragWindowEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    auto dragWindow = this->dragWindow(watched);
    if (!dragWindow)
        return false;

    if (event->type() == QEvent::PlatformSurface) {
        auto e = dynamic_cast<QPlatformSurfaceEvent *>(event);
        qDebug(dsDragLog) << "drag window platform surface, event type:" << e->surfaceEventType();
        if (e->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            m_drag->initDragOverlay(dragWindow);
        } else if (e->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
            qDebug(dsDragLog) << "surface destroy";
            m_drag->releaseDragOverlay();
        }
    }

    if (event->type() == QEvent::Move) {
        m_drag->updateOverlayPosition(dragWindow->position());
    }

    if (event->type() == QEvent::Show) {
        qDebug(dsDragLog) << "drag window show";
        m_drag->showOverlay();
    }
    if (event->type() == QEvent::Hide) {
        qDebug(dsDragLog) << "drag window hide";
        m_drag->hideOverlay();
    }
    return false;
}

DQuickDrag::DQuickDrag(QObject *parent)
    : QObject(parent)
    , DObject(*new DQuickDragPrivate(this))
{
}

DQuickDrag *DQuickDrag::qmlAttachedProperties(QObject *object)
{
    return new DQuickDrag(object);
}

bool DQuickDrag::active() const
{
    D_DC(DQuickDrag);
    return d->m_active;
}

void DQuickDrag::setActive(bool newActive)
{
    D_D(DQuickDrag);
    if (d->m_active == newActive) {
        return;
    }

    d->clear();

    if (newActive) {
        d->m_filter = new DragWindowEventFilter(d);
        qApp->installEventFilter(d->m_filter);
    }

    d->m_active = newActive;
    emit activeChanged();
}

QPoint DQuickDrag::startDragPoint() const
{
    D_DC(DQuickDrag);
    return d->m_startDragPoint;
}

QQmlComponent *DQuickDrag::overlay() const
{
    D_DC(DQuickDrag);
    return d->m_overlay;
}

void DQuickDrag::setOverlay(QQmlComponent *newOverlay)
{
    D_D(DQuickDrag);
    if (d->m_overlay == newOverlay)
        return;
    d->m_overlay = newOverlay;
    emit overlayChanged();
}

QSizeF DQuickDrag::hotSpotScale() const
{
    D_DC(DQuickDrag);
    return d->m_hotSpotScale;
}

void DQuickDrag::setHotSpotScale(const QSizeF &newHotSpotScale)
{
    D_D(DQuickDrag);
    if (d->m_hotSpotScale == newHotSpotScale)
        return;
    d->m_hotSpotScale = newHotSpotScale;
    emit hotSpotScaleChanged();
}

void DQuickDrag::resetHotSpotScale()
{
    D_D(DQuickDrag);
    d->m_hotSpotScale = {};
    emit hotSpotScaleChanged();
}

QPoint DQuickDrag::currentDragPoint() const
{
    D_DC(DQuickDrag);
    return d->m_currentDragPoint;
}

QWindow* DQuickDrag::overlayWindow() const
{
    D_DC(DQuickDrag);
    return d->m_window;
}

bool DQuickDrag::isDragging() const
{
    D_DC(DQuickDrag);
    return d->m_isDragging;
}

DQuickDragPrivate::DQuickDragPrivate(DQuickDrag *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
}

DQuickDragPrivate::~DQuickDragPrivate()
{
    clear();
}

void DQuickDragPrivate::clear()
{
    if (m_filter) {
        qApp->removeEventFilter(m_filter);
        m_filter->deleteLater();
    }
    releaseDragOverlay();
}

void DQuickDragPrivate::showOverlay()
{
    D_Q(DQuickDrag);
    if (!m_window)
        return;
    m_window->show();
    m_isDragging = true;
    Q_EMIT q->isDraggingChanged();
}

void DQuickDragPrivate::hideOverlay()
{
    D_Q(DQuickDrag);
    if (!m_window)
        return;
    m_isDragging = false;
    Q_EMIT q->isDraggingChanged();

    m_window->hide();
}

QWindow *DQuickDragPrivate::createDragOverlay()
{
    if (!m_overlay)
        return nullptr;

    auto object = m_overlay->create(m_overlay->creationContext());
    if (!object) {
        qWarning(dsDragLog) << "Failed to create overlay" << m_overlay->errorString();
    }
    return qobject_cast<QWindow *>(object);
}

void DQuickDragPrivate::initDragOverlay(QWindow *dragWindow)
{
    D_Q(DQuickDrag);
    if (!m_window)
        m_window = createDragOverlay();

    if (!m_window)
        return;

    qDebug(dsDragLog) << "init drag overlay" << m_window;

    m_window->setTransientParent(dragWindow);

    updateOverlayPosition(dragWindow->position());
    updateStartDragPoint();
    Q_EMIT q->overlayWindowChanged();
}

void DQuickDragPrivate::releaseDragOverlay()
{
    if (m_window)
        m_window->deleteLater();
}

void DQuickDragPrivate::updateOverlayPosition(const QPoint &pos)
{
    if (!m_window)
        return;

    m_window->setPosition(pos - hotSpot());
    updateCurrentDragPoint();
}

void DQuickDragPrivate::updateStartDragPoint()
{
    if (!m_window)
        return;

    const auto pos = m_window->position();
    m_startDragPoint = pos - hotSpot();

    D_Q(DQuickDrag);
    emit q->startDragPointChanged();
}

void DQuickDragPrivate::updateCurrentDragPoint()
{
    m_currentDragPoint = m_window->position();
    D_Q(DQuickDrag);
    emit q->currentDragPointChanged();
}

QPoint DQuickDragPrivate::hotSpot() const
{
    if (!m_window || !m_hotSpotScale.isValid())
        return {};

    const auto x = m_window->width() * m_hotSpotScale.width();
    const auto y = m_window->height() * m_hotSpotScale.height();
    return QPoint(x, y);
}

DS_END_NAMESPACE
