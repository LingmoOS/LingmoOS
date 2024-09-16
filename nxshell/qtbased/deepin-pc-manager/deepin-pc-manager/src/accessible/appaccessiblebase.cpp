// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appaccessiblebase.h"

AppAccessibleBase::AppAccessibleBase(QWidget *widget)
    : QAccessibleWidget(widget)
    , m_widget(widget)
{
}

void *AppAccessibleBase::interface_cast(QAccessible::InterfaceType t)
{
    switch (t) {
    case QAccessible::ActionInterface:
        return static_cast<QAccessibleActionInterface *>(this);
    case QAccessible::TextInterface:
        return static_cast<QAccessibleTextInterface *>(this);
    default:
        return nullptr;
    }
}

QString AppAccessibleBase::text(QAccessible::Text text) const
{
    if (!m_widget) {
        return QString();
    }

    switch (text) {
    case QAccessible::Name:
        return m_widget->accessibleName();
    case QAccessible::Description:
        return m_widget->accessibleDescription();
    default:
        return QString();
    }
}

QString AppAccessibleBase::text(int startOffset, int endOffset) const
{
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);

    return QString();
}

void AppAccessibleBase::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    Q_UNUSED(selectionIndex)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)

    return;
}

int AppAccessibleBase::selectionCount() const
{
    return 0;
}

void AppAccessibleBase::addSelection(int startOffset, int endOffset)
{
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)

    return;
}

void AppAccessibleBase::removeSelection(int selectionIndex)
{
    Q_UNUSED(selectionIndex)

    return;
}

void AppAccessibleBase::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    Q_UNUSED(selectionIndex)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)

    return;
}

int AppAccessibleBase::cursorPosition() const
{
    return 0;
}

void AppAccessibleBase::setCursorPosition(int position)
{
    Q_UNUSED(position)
}

int AppAccessibleBase::characterCount() const
{
    return 0;
}

QRect AppAccessibleBase::characterRect(int offset) const
{
    Q_UNUSED(offset)

    return QRect();
}

int AppAccessibleBase::offsetAtPoint(const QPoint &point) const
{
    Q_UNUSED(point)

    return 0;
}

void AppAccessibleBase::scrollToSubstring(int startIndex, int endIndex)
{
    Q_UNUSED(startIndex)
    Q_UNUSED(endIndex)

    return;
}

QString AppAccessibleBase::attributes(int offset, int *startOffset, int *endOffset) const
{
    Q_UNUSED(offset)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)

    return QString();
}
