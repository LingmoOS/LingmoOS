/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "part.h"
#include "part_p.h"

#include "kparts_logging.h"
#include "partmanager.h"

// The events have randomly generated numbers using `shuf -i1000-65535 -n 1`
#include "guiactivateevent.h"
#include "partactivateevent.h"

#include <KXMLGUIFactory>

using namespace KParts;

Part::Part(QObject *parent, const KPluginMetaData &data)
    : Part(*new PartPrivate(this, data), parent)
{
}

Part::Part(PartPrivate &dd, QObject *parent)
    : QObject(parent)
    , PartBase(dd)
{
    Q_D(Part);

    PartBase::setPartObject(this);
    if (d->m_metaData.isValid()) {
        KXMLGUIClient::setComponentName(d->m_metaData.pluginId(), d->m_metaData.name());
    }
}

Part::~Part()
{
    Q_D(Part);

    // qCDebug(KPARTSLOG) << this;

    if (d->m_widget) {
        // We need to disconnect first, to avoid calling it !
        disconnect(d->m_widget.data(), &QWidget::destroyed, this, &Part::slotWidgetDestroyed);
    }

    if (d->m_manager) {
        d->m_manager->removePart(this);
    }

    if (d->m_widget && d->m_autoDeleteWidget) {
        // qCDebug(KPARTSLOG) << "deleting widget" << d->m_widget << d->m_widget->objectName();
        delete static_cast<QWidget *>(d->m_widget);
    }
}

QWidget *Part::widget()
{
    Q_D(Part);

    return d->m_widget;
}

void Part::setAutoDeleteWidget(bool autoDeleteWidget)
{
    Q_D(Part);
    d->m_autoDeleteWidget = autoDeleteWidget;
}

void Part::setAutoDeletePart(bool autoDeletePart)
{
    Q_D(Part);
    d->m_autoDeletePart = autoDeletePart;
}

KPluginMetaData Part::metaData() const
{
    Q_D(const Part);

    return d->m_metaData;
}

void Part::setManager(PartManager *manager)
{
    Q_D(Part);

    d->m_manager = manager;
}

PartManager *Part::manager() const
{
    Q_D(const Part);

    return d->m_manager;
}

Part *Part::hitTest(QWidget *widget, const QPoint &)
{
    Q_D(Part);

    if ((QWidget *)d->m_widget != widget) {
        return nullptr;
    }

    return this;
}

void Part::setWidget(QWidget *widget)
{
    Q_D(Part);
    d->m_widget = widget;
    connect(d->m_widget.data(), &QWidget::destroyed, this, &Part::slotWidgetDestroyed, Qt::UniqueConnection);
}

void Part::customEvent(QEvent *ev)
{
    if (PartActivateEvent::test(ev)) {
        partActivateEvent(static_cast<PartActivateEvent *>(ev));
        return;
    }

    if (GUIActivateEvent::test(ev)) {
        guiActivateEvent(static_cast<GUIActivateEvent *>(ev));
        return;
    }

    QObject::customEvent(ev);
}

void Part::partActivateEvent(PartActivateEvent *)
{
}

void Part::guiActivateEvent(GUIActivateEvent *)
{
}

QWidget *Part::hostContainer(const QString &containerName)
{
    if (!factory()) {
        return nullptr;
    }

    return factory()->container(containerName, this);
}

void Part::slotWidgetDestroyed()
{
    Q_D(Part);

    d->m_widget = nullptr;
    if (d->m_autoDeletePart) {
        // qCDebug(KPARTSLOG) << "deleting part" << objectName();
        this->deleteLater();
    }
}

#include "moc_part.cpp"
