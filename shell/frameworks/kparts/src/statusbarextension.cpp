/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statusbarextension.h"

#include "kparts_logging.h"

#include "guiactivateevent.h"
#include "readonlypart.h"

#include <KMainWindow>

#include <QObject>
#include <QPointer>
#include <QStatusBar>

using namespace KParts;

///////////////////////////////////////////////////////////////////
// Helper Classes
///////////////////////////////////////////////////////////////////

class KParts::StatusBarItem
{
public:
    StatusBarItem() // for QValueList
        : m_widget(nullptr)
        , m_visible(false)
    {
    }
    StatusBarItem(QWidget *widget, int stretch, bool permanent)
        : m_widget(widget)
        , m_stretch(stretch)
        , m_permanent(permanent)
        , m_visible(false)
    {
    }

    QWidget *widget() const
    {
        return m_widget;
    }

    void ensureItemShown(QStatusBar *sb)
    {
        if (m_widget && !m_visible) {
            if (m_permanent) {
                sb->addPermanentWidget(m_widget, m_stretch);
            } else {
                sb->addWidget(m_widget, m_stretch);
            }
            m_visible = true;
            m_widget->show();
        }
    }
    void ensureItemHidden(QStatusBar *sb)
    {
        if (m_widget && m_visible) {
            sb->removeWidget(m_widget);
            m_visible = false;
            m_widget->hide();
        }
    }

private:
    QPointer<QWidget> m_widget;
    int m_stretch;
    bool m_permanent;
    bool m_visible; // true when the item has been added to the statusbar
};

class KParts::StatusBarExtensionPrivate
{
public:
    StatusBarExtensionPrivate(StatusBarExtension *qq)
        : q(qq)
        , m_statusBar(nullptr)
        , m_activated(true)
    {
    }

    StatusBarExtension *q;
    QList<StatusBarItem> m_statusBarItems; // Our statusbar items
    QStatusBar *m_statusBar;
    bool m_activated;
};

///////////////////////////////////////////////////////////////////

StatusBarExtension::StatusBarExtension(KParts::Part *parent)
    : QObject(parent)
    , d(new StatusBarExtensionPrivate(this))
{
    parent->installEventFilter(this);
}

StatusBarExtension::StatusBarExtension(KParts::ReadOnlyPart *parent)
    : QObject(parent)
    , d(new StatusBarExtensionPrivate(this))
{
    parent->installEventFilter(this);
}

StatusBarExtension::~StatusBarExtension()
{
    QStatusBar *sb = d->m_statusBar;
    for (int i = d->m_statusBarItems.count() - 1; i >= 0; --i) {
        if (d->m_statusBarItems[i].widget()) {
            if (sb) {
                d->m_statusBarItems[i].ensureItemHidden(sb);
            }
            d->m_statusBarItems[i].widget()->deleteLater();
        }
    }
}

StatusBarExtension *StatusBarExtension::childObject(QObject *obj)
{
    return obj->findChild<KParts::StatusBarExtension *>(QString(), Qt::FindDirectChildrenOnly);
}

bool StatusBarExtension::eventFilter(QObject *watched, QEvent *ev)
{
    if (!GUIActivateEvent::test(ev) || !::qobject_cast<KParts::Part *>(watched)) {
        return QObject::eventFilter(watched, ev);
    }

    QStatusBar *sb = statusBar();
    if (!sb) {
        return QObject::eventFilter(watched, ev);
    }

    GUIActivateEvent *gae = static_cast<GUIActivateEvent *>(ev);
    d->m_activated = gae->activated();

    if (d->m_activated) {
        for (auto &item : d->m_statusBarItems) {
            item.ensureItemShown(sb);
        }
    } else {
        for (auto &item : d->m_statusBarItems) {
            item.ensureItemHidden(sb);
        }
    }

    return false;
}

QStatusBar *StatusBarExtension::statusBar() const
{
    if (!d->m_statusBar) {
        KParts::Part *part = qobject_cast<KParts::Part *>(parent());
        QWidget *w = part ? part->widget() : nullptr;
        KMainWindow *mw = w ? qobject_cast<KMainWindow *>(w->topLevelWidget()) : nullptr;
        if (mw) {
            d->m_statusBar = mw->statusBar();
        }
    }
    return d->m_statusBar;
}

void StatusBarExtension::setStatusBar(QStatusBar *status)
{
    d->m_statusBar = status;
}

void StatusBarExtension::addStatusBarItem(QWidget *widget, int stretch, bool permanent)
{
    d->m_statusBarItems.append(StatusBarItem(widget, stretch, permanent));
    StatusBarItem &it = d->m_statusBarItems.last();
    QStatusBar *sb = statusBar();
    if (sb && d->m_activated) {
        it.ensureItemShown(sb);
    }
}

void StatusBarExtension::removeStatusBarItem(QWidget *widget)
{
    QStatusBar *sb = statusBar();
    QList<StatusBarItem>::iterator it = d->m_statusBarItems.begin();
    for (; it != d->m_statusBarItems.end(); ++it) {
        if ((*it).widget() == widget) {
            if (sb) {
                (*it).ensureItemHidden(sb);
            }
            d->m_statusBarItems.erase(it);
            return;
        }
    }

    qCWarning(KPARTSLOG) << "StatusBarExtension::removeStatusBarItem. Widget not found :" << widget;
}

#include "moc_statusbarextension.cpp"
