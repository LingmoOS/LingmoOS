/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KNOTIFYEVENTLIST_H
#define KNOTIFYEVENTLIST_H

#include "knotifyconfigelement.h"

#include <QTreeWidget>

class KNotifyConfigElement;
class KNotifyEventListItem;
class KConfig;

/**
    @author Olivier Goffart <ogoffart at kde.org>
*/
class KNotifyEventList : public QTreeWidget
{
    Q_OBJECT
public:
    explicit KNotifyEventList(QWidget *parent);
    ~KNotifyEventList() override;
    void fill(const QString &appname, bool loadDefaults = false);
    void save();
    void updateCurrentItem();
    void updateAllItems();
    QSize sizeHint() const override;

    void selectEvent(const QString &eventId);
    bool disableAllSounds();

private:
    KConfig *config;
    QList<KNotifyEventListItem *> m_elements;

    class KNotifyEventListDelegate;

private Q_SLOTS:
    void slotSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

Q_SIGNALS:
    void eventSelected(KNotifyConfigElement *);
};

class KNotifyEventListItem : public QTreeWidgetItem
{
public:
    KNotifyEventListItem(QTreeWidget *parent, const QString &eventName, const QString &name, const QString &description, KConfig *confir);
    ~KNotifyEventListItem() override;
    void save();

    KNotifyConfigElement *configElement()
    {
        return &m_config;
    }

    void update();

private:
    KNotifyConfigElement m_config;
};

#endif
