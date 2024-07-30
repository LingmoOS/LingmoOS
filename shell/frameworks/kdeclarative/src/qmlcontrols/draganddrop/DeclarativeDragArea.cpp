/*
    SPDX-FileCopyrightText: 2010 BetterInbox <contact@betterinbox.com>
    SPDX-FileContributor: Gregory Schlomoff <greg@betterinbox.com>

    SPDX-License-Identifier: MIT
*/

#include "DeclarativeDragArea.h"

#include <QDrag>
#include <QGuiApplication>
#include <QIcon>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QStyleHints>

#include <QDebug>

/*!
    A DragArea is used to make an item draggable.
*/

DeclarativeDragArea::DeclarativeDragArea(QQuickItem *parent)
    : QQuickItem(parent)
    , m_delegate(nullptr)
    , m_source(parent)
    , m_target(nullptr)
    , m_enabled(true)
    , m_draggingJustStarted(false)
    , m_dragActive(false)
    , m_supportedActions(Qt::MoveAction)
    , m_defaultAction(Qt::MoveAction)
    , m_data(new DeclarativeMimeData()) // m_data is owned by us, and we shouldn't pass it to Qt directly
                                        // as it will automatically delete it after the drag and drop.
    , m_pressAndHoldTimerId(0)
{
    m_startDragDistance = QGuiApplication::styleHints()->startDragDistance();
    setAcceptedMouseButtons(Qt::LeftButton);
    //     setFiltersChildEvents(true);
    setFlag(ItemAcceptsDrops, m_enabled);
    setFiltersChildMouseEvents(true);
}

DeclarativeDragArea::~DeclarativeDragArea()
{
    if (m_data) {
        delete m_data;
    }
}

/*!
  The delegate is the item that will be displayed next to the mouse cursor during the drag and drop operation.
  It usually consists of a large, semi-transparent icon representing the data being dragged.
*/
QQuickItem *DeclarativeDragArea::delegate() const
{
    return m_delegate;
}

void DeclarativeDragArea::setDelegate(QQuickItem *delegate)
{
    if (m_delegate != delegate) {
        // qDebug() << " ______________________________________________ " << delegate;
        m_delegate = delegate;
        Q_EMIT delegateChanged();
    }
}
void DeclarativeDragArea::resetDelegate()
{
    setDelegate(nullptr);
}

/*!
  The QML element that is the source of this drag and drop operation. This can be defined to any item, and will
  be available to the DropArea as event.data.source
*/
QQuickItem *DeclarativeDragArea::source() const
{
    return m_source;
}

void DeclarativeDragArea::setSource(QQuickItem *source)
{
    if (m_source != source) {
        m_source = source;
        Q_EMIT sourceChanged();
    }
}

void DeclarativeDragArea::resetSource()
{
    setSource(nullptr);
}

bool DeclarativeDragArea::dragActive() const
{
    return m_dragActive;
}

// target
QQuickItem *DeclarativeDragArea::target() const
{
    // TODO: implement me
    return nullptr;
}

// data
DeclarativeMimeData *DeclarativeDragArea::mimeData() const
{
    return m_data;
}

// startDragDistance
int DeclarativeDragArea::startDragDistance() const
{
    return m_startDragDistance;
}

void DeclarativeDragArea::setStartDragDistance(int distance)
{
    if (distance == m_startDragDistance) {
        return;
    }

    m_startDragDistance = distance;
    Q_EMIT startDragDistanceChanged();
}

// delegateImage
QVariant DeclarativeDragArea::delegateImage() const
{
    return m_delegateImage;
}

void DeclarativeDragArea::setDelegateImage(const QVariant &image)
{
    if (image.canConvert<QImage>() && image.value<QImage>() == m_delegateImage) {
        return;
    }

    if (image.canConvert<QImage>()) {
        m_delegateImage = image.value<QImage>();
    } else if (image.canConvert<QString>()) {
        m_delegateImage = QIcon::fromTheme(image.toString()).pixmap(QSize(48, 48)).toImage();
    } else {
        m_delegateImage = image.value<QIcon>().pixmap(QSize(48, 48)).toImage();
    }

    Q_EMIT delegateImageChanged();
}

// enabled
bool DeclarativeDragArea::isEnabled() const
{
    return m_enabled;
}
void DeclarativeDragArea::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged();
    }
}

// supported actions
Qt::DropActions DeclarativeDragArea::supportedActions() const
{
    return m_supportedActions;
}
void DeclarativeDragArea::setSupportedActions(Qt::DropActions actions)
{
    if (actions != m_supportedActions) {
        m_supportedActions = actions;
        Q_EMIT supportedActionsChanged();
    }
}

// default action
Qt::DropAction DeclarativeDragArea::defaultAction() const
{
    return m_defaultAction;
}
void DeclarativeDragArea::setDefaultAction(Qt::DropAction action)
{
    if (action != m_defaultAction) {
        m_defaultAction = action;
        Q_EMIT defaultActionChanged();
    }
}

void DeclarativeDragArea::mousePressEvent(QMouseEvent *event)
{
    m_pressAndHoldTimerId = startTimer(QGuiApplication::styleHints()->mousePressAndHoldInterval());
    m_buttonDownPos = event->globalPosition();
    m_draggingJustStarted = true;
    setKeepMouseGrab(true);
}

void DeclarativeDragArea::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    killTimer(m_pressAndHoldTimerId);
    m_pressAndHoldTimerId = 0;
    m_draggingJustStarted = false;
    setKeepMouseGrab(false);
    ungrabMouse();
}

void DeclarativeDragArea::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_pressAndHoldTimerId && m_draggingJustStarted && m_enabled) {
        // Grab delegate before starting drag
        if (m_delegate) {
            // Another grab is already in progress
            if (m_grabResult) {
                return;
            }
            m_grabResult = m_delegate->grabToImage();
            if (m_grabResult) {
                connect(m_grabResult.data(), &QQuickItemGrabResult::ready, this, [this]() {
                    startDrag(m_grabResult->image());
                    m_grabResult.reset();
                });
                return;
            }
        }

        // No delegate or grab failed, start drag immediately
        startDrag(m_delegateImage);
    }
}

void DeclarativeDragArea::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_enabled || QLineF(event->globalPosition(), m_buttonDownPos).length() < m_startDragDistance) {
        return;
    }

    // don't start drags on move for touch events, they'll be handled only by press and hold
    // reset timer if moved more than m_startDragDistance
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        killTimer(m_pressAndHoldTimerId);
        m_pressAndHoldTimerId = 0;
        return;
    }

    if (m_draggingJustStarted) {
        // Grab delegate before starting drag
        if (m_delegate) {
            // Another grab is already in progress
            if (m_grabResult) {
                return;
            }
            m_grabResult = m_delegate->grabToImage();
            if (m_grabResult) {
                connect(m_grabResult.data(), &QQuickItemGrabResult::ready, this, [this]() {
                    startDrag(m_grabResult->image());
                    m_grabResult.reset();
                });
                return;
            }
        }

        // No delegate or grab failed, start drag immediately
        startDrag(m_delegateImage);
    }
}

bool DeclarativeDragArea::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        // qDebug() << "press in dragarea";
        mousePressEvent(me);
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        // qDebug() << "move in dragarea";
        mouseMoveEvent(me);
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        // qDebug() << "release in dragarea";
        mouseReleaseEvent(me);
        break;
    }
    default:
        break;
    }

    return QQuickItem::childMouseEventFilter(item, event);
}

void DeclarativeDragArea::startDrag(const QImage &image)
{
    grabMouse();
    m_draggingJustStarted = false;

    QDrag *drag = new QDrag(parent());
    DeclarativeMimeData *dataCopy = new DeclarativeMimeData(m_data); // Qt will take ownership of this copy and delete it.
    drag->setMimeData(dataCopy);

    const qreal devicePixelRatio = window() ? window()->devicePixelRatio() : 1;
    const int imageSize = 48 * devicePixelRatio;

    if (!image.isNull()) {
        drag->setPixmap(QPixmap::fromImage(image));
    } else if (mimeData()->hasImage()) {
        const QImage im = qvariant_cast<QImage>(mimeData()->imageData());
        drag->setPixmap(QPixmap::fromImage(im));
    } else if (mimeData()->hasColor()) {
        QPixmap px(imageSize, imageSize);
        px.fill(mimeData()->color());
        drag->setPixmap(px);
    } else {
        // Icons otherwise
        QStringList icons;
        if (mimeData()->hasText()) {
            icons << QStringLiteral("text-plain");
        }
        if (mimeData()->hasHtml()) {
            icons << QStringLiteral("text-html");
        }
        if (mimeData()->hasUrls()) {
            for (int i = 0; i < std::min<int>(4, mimeData()->urls().size()); ++i) {
                icons << QStringLiteral("text-html");
            }
        }
        if (!icons.isEmpty()) {
            QPixmap pm(imageSize * icons.count(), imageSize);
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            int i = 0;
            for (const QString &ic : std::as_const(icons)) {
                p.drawPixmap(QPoint(i * imageSize, 0), QIcon::fromTheme(ic).pixmap(imageSize));
                i++;
            }
            p.end();
            drag->setPixmap(pm);
        }
    }

    // drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2)); // TODO: Make a property for that
    // setCursor(Qt::OpenHandCursor);    //TODO? Make a property for the cursor

    m_dragActive = true;
    Q_EMIT dragActiveChanged();
    Q_EMIT dragStarted();

    Qt::DropAction action = drag->exec(m_supportedActions, m_defaultAction);
    setKeepMouseGrab(false);

    m_dragActive = false;
    Q_EMIT dragActiveChanged();
    Q_EMIT drop(action);

    ungrabMouse();
}

#include "moc_DeclarativeDragArea.cpp"
