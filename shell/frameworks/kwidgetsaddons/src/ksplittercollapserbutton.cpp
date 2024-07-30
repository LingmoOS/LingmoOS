/*
    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>
    based on code:
    SPDX-FileCopyrightText: 2009 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2009 Kåre Sårs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ksplittercollapserbutton.h"

// Qt
#include <QEvent>
#include <QSplitter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTimeLine>

enum Direction {
    LeftToRight = 0,
    RightToLeft,
    TopToBottom,
    BottomToTop,
};

const static int TIMELINE_DURATION = 500;

const static qreal MINIMUM_OPACITY = 0.3;

static const struct {
    Qt::ArrowType arrowVisible;
    Qt::ArrowType notArrowVisible;
} s_arrowDirection[] = {{Qt::LeftArrow, Qt::RightArrow}, {Qt::RightArrow, Qt::LeftArrow}, {Qt::UpArrow, Qt::DownArrow}, {Qt::DownArrow, Qt::UpArrow}};

class KSplitterCollapserButtonPrivate
{
public:
    KSplitterCollapserButtonPrivate(KSplitterCollapserButton *qq);

    KSplitterCollapserButton *q;
    QSplitter *splitter;
    QWidget *childWidget;
    Direction direction;
    QTimeLine *opacityTimeLine;
    QList<int> sizeAtCollapse;

    bool isVertical() const;

    bool isWidgetCollapsed() const;

    void updatePosition();

    void updateArrow();

    void widgetEventFilter(QEvent *event);

    void updateOpacity();

    void startTimeLine();
};

KSplitterCollapserButtonPrivate::KSplitterCollapserButtonPrivate(KSplitterCollapserButton *qq)
    : q(qq)
    , splitter(nullptr)
    , childWidget(nullptr)
    , opacityTimeLine(nullptr)
{
}

bool KSplitterCollapserButtonPrivate::isVertical() const
{
    return (splitter->orientation() == Qt::Vertical);
}

bool KSplitterCollapserButtonPrivate::isWidgetCollapsed() const
{
    const QRect widgetRect = childWidget->geometry();
    if ((widgetRect.height() == 0) || (widgetRect.width() == 0)) {
        return true;
    } else {
        return false;
    }
}

void KSplitterCollapserButtonPrivate::updatePosition()
{
    int x = 0;
    int y = 0;
    const QRect widgetRect = childWidget->geometry();
    const int handleWidth = splitter->handleWidth();

    if (!isVertical()) {
        const int splitterWidth = splitter->width();
        const int width = q->sizeHint().width();
        // FIXME: Make this configurable
        y = 30;
        if (direction == LeftToRight) {
            if (!isWidgetCollapsed()) {
                x = widgetRect.right() + handleWidth;
            } else {
                x = 0;
            }
        } else { // RightToLeft
            if (!isWidgetCollapsed()) {
                x = widgetRect.left() - handleWidth - width;
            } else {
                x = splitterWidth - handleWidth - width;
            }
        }
    } else {
        x = 30;
        const int height = q->sizeHint().height();
        const int splitterHeight = splitter->height();
        if (direction == TopToBottom) {
            if (!isWidgetCollapsed()) {
                y = widgetRect.bottom() + handleWidth;
            } else {
                y = 0;
            }
        } else { // BottomToTop
            if (!isWidgetCollapsed()) {
                y = widgetRect.top() - handleWidth - height;
            } else {
                y = splitterHeight - handleWidth - height;
            }
        }
    }
    q->move(x, y);
}

void KSplitterCollapserButtonPrivate::updateArrow()
{
    q->setArrowType(isWidgetCollapsed() ? s_arrowDirection[direction].notArrowVisible : s_arrowDirection[direction].arrowVisible);
}

void KSplitterCollapserButtonPrivate::widgetEventFilter(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
        updatePosition();
        updateOpacity();
        updateArrow();
        break;

    default:
        break;
    }
}

void KSplitterCollapserButtonPrivate::updateOpacity()
{
    const QPoint pos = q->parentWidget()->mapFromGlobal(QCursor::pos());
    const QRect opaqueRect = q->geometry();
    const bool opaqueCollapser = opaqueRect.contains(pos);
    if (opaqueCollapser) {
        opacityTimeLine->setDirection(QTimeLine::Forward);
        startTimeLine();
    } else {
        opacityTimeLine->setDirection(QTimeLine::Backward);
        startTimeLine();
    }
}

void KSplitterCollapserButtonPrivate::startTimeLine()
{
    if (opacityTimeLine->state() == QTimeLine::Running) {
        opacityTimeLine->stop();
    }
    opacityTimeLine->start();
}

KSplitterCollapserButton::KSplitterCollapserButton(QWidget *childWidget, QSplitter *splitter)
    : QToolButton()
    , d(new KSplitterCollapserButtonPrivate(this))
{
    setObjectName(QStringLiteral("splittercollapser"));
    // We do not want our collapser to be added as a regular widget in the
    // splitter!
    setAttribute(Qt::WA_NoChildEventsForParent);

    d->opacityTimeLine = new QTimeLine(TIMELINE_DURATION, this);
    d->opacityTimeLine->setFrameRange(int(MINIMUM_OPACITY * 1000), 1000);
    connect(d->opacityTimeLine, &QTimeLine::valueChanged, this, qOverload<>(&QWidget::update));

    d->childWidget = childWidget;
    d->childWidget->installEventFilter(this);

    d->splitter = splitter;
    setParent(d->splitter);

    switch (splitter->orientation()) {
    case Qt::Horizontal:
        if (splitter->indexOf(childWidget) < splitter->count() / 2) {
            d->direction = LeftToRight;
        } else {
            d->direction = RightToLeft;
        }
        break;
    case Qt::Vertical:
        if (splitter->indexOf(childWidget) < splitter->count() / 2) {
            d->direction = TopToBottom;
        } else {
            d->direction = BottomToTop;
        }
        break;
    }

    connect(this, &KSplitterCollapserButton::clicked, this, &KSplitterCollapserButton::slotClicked);
}

KSplitterCollapserButton::~KSplitterCollapserButton() = default;

bool KSplitterCollapserButton::isWidgetCollapsed() const
{
    return d->isWidgetCollapsed();
}

bool KSplitterCollapserButton::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->childWidget) {
        d->widgetEventFilter(event);
    }
    return QToolButton::eventFilter(object, event);
}

void KSplitterCollapserButton::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    d->updateOpacity();
}

void KSplitterCollapserButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    d->updateOpacity();
}

void KSplitterCollapserButton::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    d->updateOpacity();
}

QSize KSplitterCollapserButton::sizeHint() const
{
    QStyleOption opt;
    opt.initFrom(this);
    const int extent = style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt);
    QSize sh(extent * 3 / 4, extent * 240 / 100);
    if (d->isVertical()) {
        sh.transpose();
    }
    return sh;
}

void KSplitterCollapserButton::slotClicked()
{
    QList<int> sizes = d->splitter->sizes();
    const int index = d->splitter->indexOf(d->childWidget);
    if (!d->isWidgetCollapsed()) {
        d->sizeAtCollapse = sizes;
        sizes[index] = 0;
    } else {
        if (!d->sizeAtCollapse.isEmpty()) {
            sizes = d->sizeAtCollapse;
        } else {
            if (d->isVertical()) {
                sizes[index] = d->childWidget->sizeHint().height();
            } else {
                sizes[index] = d->childWidget->sizeHint().width();
            }
        }
    }
    d->splitter->setSizes(sizes);
    d->opacityTimeLine->setDirection(QTimeLine::Backward);
    d->startTimeLine();
}

void KSplitterCollapserButton::collapse()
{
    if (!d->isWidgetCollapsed()) {
        slotClicked();
    }
    // else do nothing
}

void KSplitterCollapserButton::restore()
{
    if (d->isWidgetCollapsed()) {
        slotClicked();
    }
    // else do nothing
}

void KSplitterCollapserButton::setCollapsed(bool collapse)
{
    if (collapse == d->isWidgetCollapsed()) {
        slotClicked();
    }
    // else do nothing
}

void KSplitterCollapserButton::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    const qreal opacity = d->opacityTimeLine->currentFrame() / 1000.;
    painter.setOpacity(opacity);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    if (d->isVertical()) {
        if (d->direction == TopToBottom) {
            opt.rect.setTop(-height());
        } else {
            opt.rect.setHeight(height() * 2);
        }
    } else {
        if (d->direction == LeftToRight) {
            opt.rect.setLeft(-width());
        } else {
            opt.rect.setWidth(width() * 2);
        }
    }
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

    QStyleOptionToolButton opt2;
    initStyleOption(&opt2);
    painter.drawControl(QStyle::CE_ToolButtonLabel, opt2);
}

#include "moc_ksplittercollapserbutton.cpp"
