#include "qmlWidget.h"
#include "widgets/timelineview/timelineview.h"
#include "widgets/importtimelineview/importtimelineview.h"
//#include "../config.h"

#include <DApplication>
#include <QTimer>
#include <QQuickWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
QmlCustomInternalWidget::QmlCustomInternalWidget(QQuickPaintedItem *parent) :
    QWidget(nullptr)
{
    m_qquickContainer = parent;
    setFocusPolicy(Qt::WheelFocus);
    setAttribute(Qt::WA_WState_Created, true);// don't create window on setVisible(true)
    setVisible(true);// if not visible some operations will not done.

#ifdef USE_INNER
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    m_timeView = new TimeLineView();
    horizontalLayout->addWidget(m_timeView);
#endif
}

QPaintEngine *QmlCustomInternalWidget::paintEngine() const
{
    return nullptr;
}

void QmlCustomInternalWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    m_qquickContainer->update(event->rect());
}

QmlWidget::QmlWidget(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    setOpaquePainting(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    setFlag(QQuickItem::ItemAcceptsDrops, true);
    setFlag(QQuickItem::ItemHasContents, true);
#ifdef USE_INNER
    m_view = new QmlCustomInternalWidget(this);
#endif
    connect(this, &QQuickItem::widthChanged, this, &QmlWidget::updateGeometry);
    connect(this, &QQuickItem::heightChanged, this, &QmlWidget::updateGeometry);
    connect(this, &QQuickItem::visibleChanged, this, &QmlWidget::updateGeometry);
}

void QmlWidget::paint(QPainter *painter)
{
    if (!m_view)
        return;

    m_view->render(painter, QPoint(), QRegion(),
                     QWidget::DrawWindowBackground | QWidget::DrawChildren);
}

void QmlWidget::setViewType(int widgetType)
{
    if (m_viewType == widgetType)
        return;

    m_viewType = widgetType;

    initWidget();
    viewTypeChanged();
}

int QmlWidget::viewType()
{
    return m_viewType;
}

void QmlWidget::setFilterType(int filterType)
{
    if (m_filterType == filterType)
        return;

    m_filterType = filterType;

    filterTypeChanged();
}

int QmlWidget::filterType()
{
    return m_filterType;
}

void QmlWidget::initWidget()
{
    if (nullptr == m_view) {
        if (0 == m_viewType)
            m_view = new TimeLineView(this);
        else if (1 == m_viewType)
            m_view = new ImportTimeLineView(this);
        m_view->setFocusPolicy(Qt::WheelFocus);
        m_view->setAttribute(Qt::WA_WState_Created, true);
        m_view->setAutoFillBackground(true);   // 添加这行
        m_view->setVisible(true);
    }
}

void QmlWidget::refresh()
{
    if (m_view) {
        m_disableHoverEvent = true;
        m_lastHoveredWidget = nullptr;
        if (m_viewType == Types::WidgetDayView) {
            if (TimeLineView* timeLine = dynamic_cast<TimeLineView*>(m_view))
                timeLine->clearAndStartLayout();
        } else if (m_viewType == Types::WidgetImportedView) {
            if (ImportTimeLineView* importTimeLine = dynamic_cast<ImportTimeLineView*>(m_view))
                importTimeLine->clearAndStartLayout();
        }
        m_disableHoverEvent = false;
    }
}

void QmlWidget::navigateToMonth(const QString &month)
{
    if (m_view) {
        if (TimeLineView* timeLine = dynamic_cast<TimeLineView*>(m_view))
            timeLine->getThumbnailListView()->navigateToMonth(month);
    }
}

QVariantList QmlWidget::allUrls()
{
    if (m_view) {
        if (m_viewType == Types::WidgetDayView) {
            if (TimeLineView* timeLine = dynamic_cast<TimeLineView*>(m_view))
                return timeLine->getThumbnailListView()->allUrls();
        } else if (m_viewType == Types::WidgetImportedView) {
            if (ImportTimeLineView* importTimeLine = dynamic_cast<ImportTimeLineView*>(m_view))
                return importTimeLine->getListView()->allUrls();
        }
    }

    return QVariantList();
}

void QmlWidget::unSelectAll()
{
    if (m_view) {
        if (m_viewType == Types::WidgetDayView) {
            if (TimeLineView* timeLine = dynamic_cast<TimeLineView*>(m_view))
                timeLine->clearAllSelection();
        } else if (m_viewType == Types::WidgetImportedView) {
            if (ImportTimeLineView* importTimeLine = dynamic_cast<ImportTimeLineView*>(m_view))
                importTimeLine->clearAllSelection();
        }
    }
}

void QmlWidget::selectUrls(const QStringList &urls)
{
    if (m_view) {
        if (m_viewType == Types::WidgetDayView) {
            if (TimeLineView* timeLine = dynamic_cast<TimeLineView*>(m_view))
                timeLine->getThumbnailListView()->selectUrls(urls);
        } else if (m_viewType == Types::WidgetImportedView) {
            if (ImportTimeLineView* importTimeLine = dynamic_cast<ImportTimeLineView*>(m_view))
                importTimeLine->getListView()->selectUrls(urls);
        }
    }
}

void QmlWidget::setFocus(bool arg)
{
    QQuickPaintedItem::setFocus(arg);

    if (!m_view)
        return;

    if(arg){
        m_view->setFocus();
        forceActiveFocus();
    }
    else{
        m_view->clearFocus();
    }
    setActiveFocusOnTab(arg);
}

void QmlWidget::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    qDebug() << "QmlCustomWidget::geometryChanged, " << newGeometry;
    if(newGeometry == oldGeometry)
        return;
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    updateGeometry();
}

bool QmlWidget::event(QEvent *e)
{
    if (!m_view)
        return QQuickPaintedItem::event(e);

    static QPoint lastPos;
    static bool isPressed = false;
    static QWidget* pressedWidget = nullptr;
    QObject* pScrollArea = m_view->findChild<QObject*>("qt_scrollarea_viewport");
    // 处理鼠标事件
    if (auto mouseEvent = dynamic_cast<QMouseEvent *>(e)) {
        QPoint pos = mouseEvent->pos();

        switch (mouseEvent->type()) {
            case QEvent::MouseButtonPress:
                isPressed = true;
                lastPos = pos;
                pressedWidget = m_view->childAt(pos);
                break;
            case QEvent::MouseMove:
                if (!isPressed) {
                    pressedWidget = m_view->childAt(pos);
                }
                break;
            case QEvent::MouseButtonRelease:
                isPressed = false;
                // 即使鼠标释放在 QmlWidget 范围外，也要处理框选结束
                if (pressedWidget) {
                    QPoint localPos = pressedWidget->mapFrom(m_view, pos);
                    QMouseEvent mappedEvent(QEvent::MouseButtonRelease, localPos, mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                    QCoreApplication::sendEvent(pressedWidget, &mappedEvent);
                    update();
                }
                pressedWidget = nullptr;
                break;
            default:
                break;
        }

        QWidget* targetWidget = isPressed ? pressedWidget : m_view->childAt(pos);
        if (targetWidget) {
            QPoint localPos = targetWidget->mapFrom(m_view, pos);

            QMouseEvent mappedEvent(mouseEvent->type(), localPos, mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());

            bool handled = QCoreApplication::sendEvent(targetWidget, &mappedEvent);

            if (handled) {
                update();
            }

            // 右键点击事件
            if (mouseEvent->button() == Qt::RightButton && mouseEvent->type() == QEvent::MouseButtonPress) {
                QContextMenuEvent contextMenuEvent(QContextMenuEvent::Mouse, localPos, mouseEvent->globalPos());
                QCoreApplication::sendEvent(targetWidget, &contextMenuEvent);
            }

            // 对于鼠标移动事件，即使没有被处理也返回true，以确保持续接收移动事件
            if (mouseEvent->type() == QEvent::MouseMove) {
                return true;
            }

            return handled;
        }
    } else if (auto keyEvent = dynamic_cast<QKeyEvent*>(e)) {
        if(pScrollArea)
            QCoreApplication::sendEvent(pScrollArea, keyEvent);
    } else if (auto wheelEvent = dynamic_cast<QWheelEvent *>(e)) {
        // 处理鼠标滚轮事件
        // Qt6下，滚轮事件只有转发到scrollArea上，缩略图列表才能在正常接收滚轮事件
        // 转发到其子控件，比如Label或TimeLineDateWidget上，事件都不会路由到缩略图列表
        if(pScrollArea)
            QCoreApplication::sendEvent(pScrollArea, wheelEvent);
    } else if(e->type() == QEvent::HoverMove) {
        //qWarning() << "e->type()" << e->type();
        // 处理鼠标悬停事件
        QHoverEvent *hoverEvent = dynamic_cast<QHoverEvent *>(e);
        if (hoverEvent && !m_disableHoverEvent) {
            QWidget* ss = m_view->childAt(hoverEvent->pos());
            if (ss) {
                if (ss != m_lastHoveredWidget) {
                    if (m_lastHoveredWidget) {
                        QEvent leaveEvent(QEvent::Leave);
                        QCoreApplication::sendEvent(m_lastHoveredWidget, &leaveEvent);
                    }

                    QEvent enterEvent(QEvent::Enter);
                    QCoreApplication::sendEvent(ss, &enterEvent);
                    m_lastHoveredWidget = ss;
                }

                QCoreApplication::sendEvent(ss, e);
            }
        }
    } else if (e->type() == QEvent::HoverEnter) {
        setFocus(true);
    }

    return QQuickPaintedItem::event(e);
}

void QmlWidget::updateGeometry()
{
    if (!isVisible() || !m_view)
        return;

    QPointF newPos(0, 0);
    newPos = mapToItem(0, newPos);
    qDebug() << "QmlCustomWidget::updateGeometry, top left mapped to window: " << newPos;

    if(this->window()){
        newPos += this->window()->position();
        qDebug() << "QmlCustomWidget::updateGeometry, top left mapped to screen: " << newPos;
    }
    QRectF absRect(newPos, contentsBoundingRect().size());
    m_view->setGeometry(absRect.toRect());
    // const qreal ratio = qApp->devicePixelRatio();
    // if (ratio > 1) {
    //     m_view->resize(m_view->width() * ratio, m_view->height() * ratio);
    // }
}
