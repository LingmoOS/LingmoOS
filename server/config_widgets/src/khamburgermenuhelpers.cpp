/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "khamburgermenuhelpers_p.h"

#include "khamburgermenu.h"

#include <QEvent>
#include <QMenu>
#include <QToolButton>
#include <QWidget>
#include <QWindow>

ListenerContainer::ListenerContainer(KHamburgerMenuPrivate *hamburgerMenuPrivate)
    : QObject{hamburgerMenuPrivate}
    , m_listeners{std::vector<std::unique_ptr<QObject>>(4)}
{
}

ListenerContainer::~ListenerContainer()
{
}

bool AddOrRemoveActionListener::eventFilter(QObject * /*watched*/, QEvent *event)
{
    if (event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionRemoved) {
        static_cast<KHamburgerMenuPrivate *>(parent())->notifyMenuResetNeeded();
    }
    return false;
}

bool ButtonPressListener::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress) {
        auto hamburgerMenuPrivate = static_cast<KHamburgerMenuPrivate *>(parent());
        auto q = static_cast<KHamburgerMenu *>(hamburgerMenuPrivate->q_ptr);
        Q_EMIT q->aboutToShowMenu();
        hamburgerMenuPrivate->resetMenu(); // This menu never has a parent which can be
        // problematic because it can lead to situations in which the QMenu itself is
        // treated like its own window.
        // To avoid this we set a sane transientParent() now even if it already has one
        // because the menu might be opened from another window this time.
        const auto watchedButton = qobject_cast<QToolButton *>(watched);
        if (!watchedButton) {
            return false;
        }
        auto menu = watchedButton->menu();
        if (!menu) {
            return false;
        }
        // ensure polished so the style can change the surfaceformat of the window which is
        // not possible once the window has been created
        menu->ensurePolished();
        menu->winId(); // trigger being a native widget already, to ensure windowHandle created
        // generic code if not known if the available parent widget is a native widget or not
        auto parentWindowHandle = watchedButton->windowHandle();
        if (!parentWindowHandle) {
            parentWindowHandle = watchedButton->nativeParentWidget()->windowHandle();
        }
        menu->windowHandle()->setTransientParent(parentWindowHandle);
    }
    return false;
}

bool VisibleActionsChangeListener::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
        if (!event->spontaneous()) {
            static_cast<KHamburgerMenuPrivate *>(parent())->notifyMenuResetNeeded();
        }
    } else if (event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionRemoved) {
        Q_ASSERT_X(qobject_cast<QWidget *>(watched), "VisibileActionsChangeListener", "The watched QObject is expected to be a QWidget.");
        if (static_cast<QWidget *>(watched)->isVisible()) {
            static_cast<KHamburgerMenuPrivate *>(parent())->notifyMenuResetNeeded();
        }
    }
    return false;
}

bool VisibilityChangesListener::eventFilter(QObject * /*watched*/, QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
        if (!event->spontaneous()) {
            static_cast<KHamburgerMenuPrivate *>(parent())->updateVisibility();
        }
    }
    return false;
}

bool isWidgetActuallyVisible(const QWidget *widget)
{
    Q_CHECK_PTR(widget);
    bool actuallyVisible = widget->isVisible();
    const QWidget *ancestorWidget = widget->parentWidget();
    while (actuallyVisible && ancestorWidget) {
        actuallyVisible = ancestorWidget->isVisible();
        ancestorWidget = ancestorWidget->parentWidget();
    }
    return actuallyVisible;
}

bool listContainsWidget(const std::forward_list<QPointer<const QWidget>> &list, const QWidget *widget)
{
    for (const auto &item : list) {
        if (widget == item) {
            return true;
        }
    }
    return false;
}
