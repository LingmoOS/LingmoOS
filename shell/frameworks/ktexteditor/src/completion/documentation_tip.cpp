/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "documentation_tip.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QScreen>

DocTip::DocTip(QWidget *parent)
    : QFrame(parent)
    , m_textView(new QTextBrowser(this))
{
    setFocusPolicy(Qt::NoFocus);

    m_textView->setFrameStyle(QFrame::Box | QFrame::Raised);

    setFixedWidth(250);
    setFixedHeight(150);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    setContentsMargins({});
    layout->addWidget(&m_stack);
    m_stack.addWidget(m_textView);
}

QWidget *DocTip::currentWidget()
{
    return m_stack.currentWidget();
}

void DocTip::clearWidgets()
{
    for (auto *widget : m_widgets) {
        widget->deleteLater();
    }
    m_widgets.clear();
}

void DocTip::setText(const QString &s)
{
    m_textView->setPlainText(s);
    if (m_stack.currentWidget() != m_textView) {
        m_stack.removeWidget(m_stack.currentWidget());
        m_stack.addWidget(m_textView);
    }
    Q_ASSERT(m_stack.count() == 1);
}

void DocTip::setWidget(QWidget *widget)
{
    if (auto w = m_stack.currentWidget()) {
        if (w != m_textView) {
            m_widgets.push_back(w);
        }
        m_stack.removeWidget(w);
    }

    if (!widget) {
        return;
    }

    m_stack.addWidget(widget);

    Q_ASSERT(m_stack.count() == 1);
}

void DocTip::updatePosition(QWidget *completionWidget)
{
    auto parent = parentWidget();
    if (!parent) {
        qWarning() << Q_FUNC_INFO << "Unexpected null parent!";
        return;
    }

    const auto parentRight = parent->geometry().right();
    auto completionWidgetRight = completionWidget->geometry().right();
    constexpr int Margin = 8;

    int x = 0;
    // is the completion widget too far right?
    if ((completionWidgetRight + this->width()) > parentRight) {
        // let's hope there is enough available space to the left of completionWidget
        x = (completionWidget->x() - this->width()) - Margin;
    } else {
        // we have enough space on the right
        x = completionWidget->x() + completionWidget->width() + Margin;
    }
    move(x, completionWidget->y());
}
