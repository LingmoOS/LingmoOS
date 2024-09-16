// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roundpopupwidget.h"

#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>
#include <QEvent>
#include <QResizeEvent>
#include "constants.h"

static constexpr int MARGIN = 5;
static constexpr int RADIUS = 18;
static constexpr int BlurMaskAlpha = 120;

RoundPopupWidget::RoundPopupWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_pContentWidget(nullptr)
    , m_mainLayout(new QVBoxLayout())
    , m_savedParent(this)
{
    initUI();
}

void RoundPopupWidget::initUI()
{
    setBlurRectXRadius(RADIUS);
    setBlurRectYRadius(RADIUS);
    setMaskColor(DBlurEffectWidget::LightColor);
    setMaskAlpha(BlurMaskAlpha);
    m_mainLayout->setMargin(0);
    m_mainLayout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    setLayout(m_mainLayout);
}

void RoundPopupWidget::setContent(QWidget *widget)
{
    if (widget != m_pContentWidget && m_pContentWidget) {
        m_mainLayout->removeWidget(m_pContentWidget);
        m_pContentWidget->setParent(m_savedParent);
        m_pContentWidget->hide();
        m_savedParent = this;
    }

    if (!widget)
        return;

    resize(widget->size() + QSize(MARGIN * 2, MARGIN * 2));
    if (widget->parentWidget() != this)
        m_savedParent = widget->parentWidget();
    m_mainLayout->addWidget(widget);
    m_pContentWidget = widget;
    m_pContentWidget->installEventFilter(this);
    widget->show();
}

QWidget *RoundPopupWidget::getContent() const
{
    return m_pContentWidget;
}

void RoundPopupWidget::hideEvent(QHideEvent *event)
{
    setContent(nullptr);
    QWidget::hideEvent(event);
}

bool RoundPopupWidget::eventFilter(QObject *watched, QEvent *event)
{
    switch(event->type()) {
    case QEvent::Resize: {
        if (watched != m_pContentWidget)
            break;
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        QSize newSize = resizeEvent->size() + QSize(MARGIN * 2, MARGIN * 2);
        // Recalculate the geometry
        QRect oldGeometry = geometry();
        QPoint anchor = parentWidget()->mapToGlobal({oldGeometry.left() + oldGeometry.width() / 2 - 1, oldGeometry.bottom()});
        // Bound to screen and margin. If we can't show the popup fully, assume it's a scroll area, we just set a max height.
        QRect boundary = topLevelWidget()->frameGeometry();
        boundary.adjust((boundary.width() + DDESESSIONCC::PASSWDLINEEIDT_WIDTH) / 2 + Popup::HorizontalMargin, Popup::VerticalMargin, -Popup::HorizontalMargin, 0);
        boundary.setBottom(anchor.y());
        QSize properSize = newSize.boundedTo(boundary.size());
        QPoint newTopLeft(anchor.x() - properSize.width() / 2 + 1, anchor.y() - properSize.height() + 1);
        QRect newGeometry(newTopLeft, properSize);
        if (!boundary.contains(newGeometry)) {
            if (boundary.contains(newGeometry.bottomLeft())) {
                newGeometry.moveBottomRight(boundary.bottomRight());
            } else {
                newGeometry.moveBottomLeft(boundary.bottomLeft());
            }
        }
        newGeometry.moveTopLeft(parentWidget()->mapFromGlobal(newGeometry.topLeft()));
        setGeometry(newGeometry);
        break;
    }
    default:
        break;
    }
    return DBlurEffectWidget::eventFilter(watched, event);
}
