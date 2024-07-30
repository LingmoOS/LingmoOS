/*
    This file is part of the KDE frameworks
    SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <kcolumnresizer.h>

#include "loggingcategory.h"

#include <QEvent>
#include <QGridLayout>
#include <QSet>
#include <QTimer>
#include <QWidget>

class FormLayoutWidgetItem : public QWidgetItem
{
public:
    FormLayoutWidgetItem(QWidget *widget, QFormLayout *formLayout, QFormLayout::ItemRole itemRole)
        : QWidgetItem(widget)
        , m_formLayout(formLayout)
        , m_itemRole(itemRole)
    {
    }

    void setWidth(int width)
    {
        if (width != m_width) {
            m_width = width;
            invalidate();
        }
    }

    QFormLayout *formLayout() const
    {
        return m_formLayout;
    }

    QSize sizeHint() const override
    {
        QSize size = QWidgetItem::sizeHint();
        if (m_width != -1) {
            size.setWidth(m_width);
        }
        return size;
    }

    QSize minimumSize() const override
    {
        QSize size = QWidgetItem::minimumSize();
        if (m_width != -1) {
            size.setWidth(m_width);
        }
        return size;
    }

    QSize maximumSize() const override
    {
        QSize size = QWidgetItem::maximumSize();
        if (m_width != -1) {
            size.setWidth(m_width);
        }
        return size;
    }

    void setGeometry(const QRect &_rect) override
    {
        QRect rect = _rect;
        int width = widget()->sizeHint().width();
        if (m_itemRole == QFormLayout::LabelRole && m_formLayout->labelAlignment() & Qt::AlignRight) {
            rect.setLeft(rect.right() - width);
        }
        QWidgetItem::setGeometry(rect);
    }

private:
    QFormLayout *const m_formLayout;
    int m_width = -1;
    const QFormLayout::ItemRole m_itemRole;
};

struct GridColumnInfo {
    GridColumnInfo(QGridLayout *layout_, int column_)
        : layout(layout_)
        , column(column_)
    {
    }
    QGridLayout *layout;
    int column;
};

Q_DECLARE_TYPEINFO(GridColumnInfo, Q_PRIMITIVE_TYPE);

class KColumnResizerPrivate
{
public:
    KColumnResizerPrivate(KColumnResizer *q_ptr)
        : q(q_ptr)
        , m_updateTimer(new QTimer(q))
    {
        m_updateTimer->setSingleShot(true);
        m_updateTimer->setInterval(0);
        QObject::connect(m_updateTimer, &QTimer::timeout, q, [this]() {
            updateWidth();
        });
    }

    void scheduleWidthUpdate()
    {
        m_updateTimer->start();
    }

    void updateWidth()
    {
        int width = 0;
        for (QWidget *widget : std::as_const(m_widgets)) {
            width = qMax(widget->sizeHint().width(), width);
        }
        for (FormLayoutWidgetItem *item : std::as_const(m_formWidgetItemList)) {
            item->setWidth(width);
            item->formLayout()->update();
        }
        for (const GridColumnInfo &info : std::as_const(m_gridColumnInfoList)) {
            info.layout->setColumnMinimumWidth(info.column, width);
        }
    }

    void addWidgetsFromGridLayout(QGridLayout *layout, int column)
    {
        for (int row = 0; row < layout->rowCount(); ++row) {
            QLayoutItem *item = layout->itemAtPosition(row, column);
            if (!item) {
                continue;
            }
            QWidget *widget = item->widget();
            if (!widget) {
                continue;
            }
            q->addWidget(widget);
        }
        m_gridColumnInfoList << GridColumnInfo(layout, column);
    }

    void addWidgetsFromFormLayout(QFormLayout *layout, QFormLayout::ItemRole role)
    {
        for (int row = 0; row < layout->rowCount(); ++row) {
            QLayoutItem *item = layout->itemAt(row, role);
            if (!item) {
                continue;
            }
            QWidget *widget = item->widget();
            if (!widget) {
                continue;
            }
            // Replace the QWidgetItem with our own
            layout->removeItem(item);
            delete item;
            FormLayoutWidgetItem *newItem = new FormLayoutWidgetItem(widget, layout, role);
            layout->setItem(row, role, newItem);
            m_formWidgetItemList << newItem;

            q->addWidget(widget);
        }
    }

    KColumnResizer *q;
    QTimer *m_updateTimer;
    QSet<QWidget *> m_widgets;
    QList<FormLayoutWidgetItem *> m_formWidgetItemList;
    QList<GridColumnInfo> m_gridColumnInfoList;
};

KColumnResizer::KColumnResizer(QObject *parent)
    : QObject(parent)
    , d(new KColumnResizerPrivate(this))
{
}

KColumnResizer::~KColumnResizer() = default;

void KColumnResizer::addWidget(QWidget *widget)
{
    if (d->m_widgets.contains(widget)) {
        return;
    }
    d->m_widgets.insert(widget);
    widget->installEventFilter(this);
    d->scheduleWidthUpdate();
}

void KColumnResizer::removeWidget(QWidget *widget)
{
    if (!d->m_widgets.remove(widget)) {
        return;
    }
    widget->removeEventFilter(this);
    d->scheduleWidthUpdate();
}

bool KColumnResizer::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        d->scheduleWidthUpdate();
    }
    return false;
}

void KColumnResizer::addWidgetsFromLayout(QLayout *layout, int column)
{
    Q_ASSERT(column >= 0);
    if (column < 0) {
        qCWarning(KWidgetsAddonsLog) << "column must be >= 0";
        return;
    }
    QGridLayout *gridLayout = qobject_cast<QGridLayout *>(layout);
    if (gridLayout) {
        d->addWidgetsFromGridLayout(gridLayout, column);
        return;
    }
    QFormLayout *formLayout = qobject_cast<QFormLayout *>(layout);
    if (formLayout) {
        Q_ASSERT(column <= QFormLayout::SpanningRole);
        if (column > QFormLayout::SpanningRole) {
            qCWarning(KWidgetsAddonsLog) << "column should not be more than" << QFormLayout::SpanningRole << "for QFormLayout";
            return;
        }
        QFormLayout::ItemRole role = static_cast<QFormLayout::ItemRole>(column);
        d->addWidgetsFromFormLayout(formLayout, role);
    } else {
        qCWarning(KWidgetsAddonsLog) << "Don't know how to handle layout" << layout;
        Q_ASSERT(0);
    }
}

#include "moc_kcolumnresizer.cpp"

// vi: ts=4 sw=4 et
