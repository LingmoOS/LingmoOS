/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KHAMBURGERMENUHELPERS_P_H
#define KHAMBURGERMENUHELPERS_P_H

#include "khamburgermenu_p.h"

#include <QObject>

#include <memory>
#include <vector>

class QFont;
class QWidget;

/**
 * @brief Makes sure there are no redundant event listeners.
 *
 * Functionally identical event listeners are needed throughout khamburgermenu.cpp.
 * This class makes sure only one of each is created when needed and then reused.
 * This also simplifies the removal of event listeners.
 * \internal
 */
class ListenerContainer : private QObject
{
public:
    explicit ListenerContainer(KHamburgerMenuPrivate *hamburgerMenuPrivate);
    ~ListenerContainer() override;

    /**
     * @return an object of class @p Listener with the same parent as ListenerContainer.
     */
    template<class Listener>
    Listener *get()
    {
        for (auto &i : m_listeners) {
            if (auto existingListener = qobject_cast<Listener *>(i.get())) {
                return existingListener;
            }
        }

        KHamburgerMenuPrivate *hamburgerMenuPrivate = static_cast<KHamburgerMenuPrivate *>(parent());
        m_listeners.push_back(std::unique_ptr<QObject>(new Listener(hamburgerMenuPrivate)));
        return static_cast<Listener *>(m_listeners.back().get());
    }

protected:
    std::vector<std::unique_ptr<QObject>> m_listeners;
};

/**
 * When an action is added or removed, calls KHamburgerMenuPrivate::notifyMenuResetNeeded().
 * \internal
 */
class AddOrRemoveActionListener : public QObject
{
    Q_OBJECT

protected:
    inline AddOrRemoveActionListener(QObject *parent)
        : QObject{parent} {};

    bool eventFilter(QObject * /*watched*/, QEvent *event) override;

    friend class ListenerContainer;
};

/**
 * When the button is pressed, emits KHamburgerMenu::aboutToShowMenu(), then calls
 * KHamburgerMenuPrivate::resetMenu() (which will only actually reset the menu if
 * a menu reset is needed).
 * \internal
 */
class ButtonPressListener : public QObject
{
    Q_OBJECT

protected:
    inline ButtonPressListener(QObject *parent)
        : QObject{parent} {};

    bool eventFilter(QObject *watched, QEvent *event) override;

    friend class ListenerContainer;
};

/**
 * When either
 * - the visibility of the widget changes or
 * - actions are added or removed from the widget while it isVisible()
 * calls KHamburgerMenuPrivate::notifyMenuResetNeeded().
 * \internal
 */
class VisibleActionsChangeListener : public QObject
{
    Q_OBJECT

protected:
    inline VisibleActionsChangeListener(QObject *parent)
        : QObject{parent} {};

    /**
     * Listen for events that potentially lead to a change in user-visible actions.
     * Examples: Adding an action or hiding a toolbar.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    friend class ListenerContainer;
};

/**
 * When the visibility of the widget changes calls KHamburgerMenuPrivate::updateVisibility().
 * \internal
 */
class VisibilityChangesListener : public QObject
{
    Q_OBJECT

protected:
    inline VisibilityChangesListener(QObject *parent)
        : QObject{parent} {};

    bool eventFilter(QObject * /*watched*/, QEvent *event) override;

    friend class ListenerContainer;
};

/**
 * Is the widget and all of its ancestors visible?
 */
bool isWidgetActuallyVisible(const QWidget *widget);

/**
 * Does the @p list contain the @p widget?
 */
bool listContainsWidget(const std::forward_list<QPointer<const QWidget>> &list, const QWidget *widget);

#endif // KHAMBURGERMENUHELPERS_P_H
