// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakewindowlayer.h"
#include <QEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

FakeWindowLayer::FakeWindowLayer(QWidget *parent)
    : QWidget(parent)
    , m_content(nullptr)
    , m_savedParent(nullptr)
    , m_savedFocus(nullptr)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_WState_Hidden); // ensure invisible to parent
}

void FakeWindowLayer::setContent(QWidget *toSet)
{
    if (content() && content() != toSet) {
        content()->setParent(m_savedParent);
        content()->hide();
    }
    if (!toSet) {
        m_content = nullptr;
        return;
    }
    m_savedFocus = QApplication::focusWidget();
    if (toSet->parentWidget() != this) {
        // In case that setContent is invoked many times
        m_savedParent = toSet->parentWidget();
        toSet->setParent(this);
    }
    m_content = toSet;
    m_content->show();
    m_content->setFocus();
}

QWidget *FakeWindowLayer::content() const
{
    return m_content;
}

void FakeWindowLayer::mouseReleaseEvent(QMouseEvent *event)
{
    hide();
    QWidget::mouseReleaseEvent(event);
}

void FakeWindowLayer::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        hide();
        break;
    default:
        break;
    }
    QWidget::keyReleaseEvent(event);
}

void FakeWindowLayer::hideEvent(QHideEvent *event)
{
    setContent(nullptr);
    if (m_savedFocus) {
        m_savedFocus->setFocus();
    }
    QWidget::hideEvent(event);
}

void FakeWindowLayer::setVisible(bool visible)
{
    if (content())
        QWidget::setVisible(visible);
    else
        QWidget::setVisible(false);
}
