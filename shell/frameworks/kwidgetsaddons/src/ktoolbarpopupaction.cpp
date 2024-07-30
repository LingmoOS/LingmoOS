/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktoolbarpopupaction.h"

#include <QMenu>
#include <QToolBar>

#include <memory>

class KToolBarPopupActionPrivate
{
public:
    KToolBarPopupActionPrivate(KToolBarPopupAction *q)
        : q(q)
        , m_popupMode(KToolBarPopupAction::MenuButtonPopup)
        , m_popupMenu(new QMenu())
    {
    }

    void applyPopupMode(QToolButton *button, KToolBarPopupAction::PopupMode mode)
    {
        if (mode == KToolBarPopupAction::NoPopup) {
            button->setMenu(nullptr);
            button->setPopupMode(QToolButton::InstantPopup);
        } else {
            // In case the menu is explicitly overridden, use it.
            // QAction::menu() is automatically applied thanks to QToolButton::setDefaultAction().
            if (!q->menu()) {
                button->setMenu(m_popupMenu.get());
            }
            button->setPopupMode(static_cast<QToolButton::ToolButtonPopupMode>(mode));
        }
    }

    KToolBarPopupAction *q;
    KToolBarPopupAction::PopupMode m_popupMode;
    std::unique_ptr<QMenu> m_popupMenu;
};

KToolBarPopupAction::KToolBarPopupAction(const QIcon &icon, const QString &text, QObject *parent)
    : QWidgetAction(parent)
    , d(new KToolBarPopupActionPrivate(this))
{
    setIcon(icon);
    setText(text);
}

KToolBarPopupAction::~KToolBarPopupAction() = default;

QWidget *KToolBarPopupAction::createWidget(QWidget *_parent)
{
    QToolBar *parent = qobject_cast<QToolBar *>(_parent);
    if (!parent) {
        return QWidgetAction::createWidget(_parent);
    }
    QToolButton *button = new QToolButton(parent);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setIconSize(parent->iconSize());
    button->setToolButtonStyle(parent->toolButtonStyle());
    button->setDefaultAction(this);
    d->applyPopupMode(button, d->m_popupMode);

    connect(parent, &QToolBar::iconSizeChanged, button, &QAbstractButton::setIconSize);
    connect(parent, &QToolBar::toolButtonStyleChanged, button, &QToolButton::setToolButtonStyle);
    connect(button, &QToolButton::triggered, parent, &QToolBar::actionTriggered);

    return button;
}

KToolBarPopupAction::PopupMode KToolBarPopupAction::popupMode() const
{
    return d->m_popupMode;
}

void KToolBarPopupAction::setPopupMode(KToolBarPopupAction::PopupMode popupMode)
{
    d->m_popupMode = popupMode;

    const auto widgets = createdWidgets();
    for (QWidget *widget : widgets) {
        if (auto *button = qobject_cast<QToolButton *>(widget)) {
            d->applyPopupMode(button, popupMode);
        }
    }
}

QMenu *KToolBarPopupAction::popupMenu() const
{
    return d->m_popupMenu.get();
}

#include "moc_ktoolbarpopupaction.cpp"
