/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "khamburgermenuhelpers_p.h"

#include "khamburgermenu.h"

#include <QEvent>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
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

void ButtonPressListener::prepareHamburgerButtonForPress(QObject *button)
{
    Q_ASSERT(qobject_cast<QToolButton *>(button));

    auto hamburgerMenuPrivate = static_cast<KHamburgerMenuPrivate *>(parent());
    auto q = static_cast<KHamburgerMenu *>(hamburgerMenuPrivate->q_ptr);
    Q_EMIT q->aboutToShowMenu();
    hamburgerMenuPrivate->resetMenu(); // This menu never has a parent which can be
    // problematic because it can lead to situations in which the QMenu itself is
    // treated like its own window.
    // To avoid this we set a sane transientParent() now even if it already has one
    // because the menu might be opened from another window this time.
    const auto watchedButton = static_cast<QToolButton *>(button);
    auto menu = watchedButton->menu();
    if (!menu) {
        return;
    }
    prepareParentlessMenuForShowing(menu, watchedButton);
}

bool ButtonPressListener::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress) {
        prepareHamburgerButtonForPress(watched);
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

bool isMenuBarVisible(const QMenuBar *menuBar)
{
    return menuBar && (menuBar->isVisible() && !menuBar->isNativeMenuBar());
}

bool isWidgetActuallyVisible(const QWidget *widget)
{
    Q_CHECK_PTR(widget);
    if (widget->width() < 1 || widget->height() < 1) {
        return false;
    }

    bool actuallyVisible = widget->isVisible();
    const QWidget *ancestorWidget = widget->parentWidget();
    while (actuallyVisible && ancestorWidget) {
        actuallyVisible = ancestorWidget->isVisible();
        ancestorWidget = ancestorWidget->parentWidget();
    }
    return actuallyVisible;
}

void prepareParentlessMenuForShowing(QMenu *menu, const QWidget *surrogateParent)
{
    Q_CHECK_PTR(menu);
    // ensure polished so the style can change the surfaceformat of the window which is
    // not possible once the window has been created
    menu->ensurePolished();
    menu->winId(); // trigger being a native widget already, to ensure windowHandle created
    // generic code if not known if the available parent widget is a native widget or not

    if (surrogateParent) {
        auto parentWindowHandle = surrogateParent->windowHandle();
        if (!parentWindowHandle) {
            parentWindowHandle = surrogateParent->nativeParentWidget()->windowHandle();
        }
        menu->windowHandle()->setTransientParent(parentWindowHandle);
        return;
    }

    menu->windowHandle()->setTransientParent(qGuiApp->focusWindow());
    // Worst case: The menu's transientParent is now still nullptr in which case it might open as
    // its own window.
}

void setToolButtonVisible(QWidget *toolButton, bool visible)
{
    toolButton->setVisible(visible);
    // setVisible() unfortunately has no effect for QWidgetActions on toolbars,
    // so we work around this by using setMaximumSize().
    if (qobject_cast<QToolBar *>(toolButton->parent())) {
        if (visible) {
            toolButton->setMaximumSize(QSize(9999999, 9999999));
            toolButton->setFocusPolicy(Qt::TabFocus);
        } else {
            toolButton->setMaximumSize(QSize(0, 0));
            toolButton->setFocusPolicy(Qt::NoFocus); // We don't want focus on invisible items.
        }
    }
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

#include "moc_khamburgermenuhelpers_p.cpp"
