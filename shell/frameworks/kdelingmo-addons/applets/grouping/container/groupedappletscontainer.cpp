/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "groupedappletscontainer.h"
#include "debug.h"

#include <QDebug>

#include <KActionCollection>
#include <Lingmo/Corona>
#include <LingmoQuick/AppletQuickItem>
#include <QAction>

GroupedAppletsContainer::GroupedAppletsContainer(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Lingmo::Applet(parent, data, args)
{
}

GroupedAppletsContainer::~GroupedAppletsContainer()
{
    if (destroyed()) {
        m_innerContainment->destroy();
    }
}

void GroupedAppletsContainer::init()
{
    Applet::init();

    // in the first creation we immediately create the systray: so it's accessible during desktop scripting
    uint id = config().readEntry("ContainmentId", 0);

    if (id == 0) {
        ensureSystrayExists();
    }
}

void GroupedAppletsContainer::ensureSystrayExists()
{
    if (m_innerContainment) {
        return;
    }

    Lingmo::Containment *cont = containment();
    if (!cont) {
        return;
    }

    Lingmo::Corona *c = cont->corona();
    if (!c) {
        return;
    }

    uint id = config().readEntry("ContainmentId", 0);
    if (id > 0) {
        const auto containments = c->containments();
        for (Lingmo::Containment *candidate : containments) {
            if (candidate->id() == id) {
                m_innerContainment = candidate;
                break;
            }
        }
        qCDebug(GROUPING_DEBUG) << "Containment id" << id << "that used to be a grouped containment that was deleted";
        // id = 0;
    }

    if (!m_innerContainment) {
        m_innerContainment = c->createContainment(QStringLiteral("org.kde.lingmo.private.grouping"), //
                                                  QVariantList() << QStringLiteral("org.kde.lingmo:force-create"));
        config().writeEntry("ContainmentId", m_innerContainment->id());
    }

    if (!m_innerContainment) {
        return;
    }

    m_innerContainment->setParent(this);
    connect(containment(), &Lingmo::Containment::screenChanged, m_innerContainment.data(), &Lingmo::Containment::reactToScreenChange);
    m_innerContainment->setFormFactor(formFactor());
    m_innerContainment->setLocation(location());

    m_internalContainmentItem = LingmoQuick::AppletQuickItem::itemForApplet(m_innerContainment);
    Q_EMIT internalContainmentItemChanged();

    setInternalAction(QStringLiteral("configure"), m_innerContainment->internalAction(QStringLiteral("configure")));
    connect(m_innerContainment.data(), &Lingmo::Containment::configureRequested, this, [this](Lingmo::Applet *applet) {
        Q_EMIT containment()->configureRequested(applet);
    });

    if (m_internalContainmentItem) {
        // don't let internal systray manage context menus
        m_internalContainmentItem->setAcceptedMouseButtons(Qt::NoButton);
    }

    // replace internal remove action with ours
    m_innerContainment->setInternalAction(QStringLiteral("remove"), internalAction(QStringLiteral("remove")));
}

void GroupedAppletsContainer::constraintsEvent(Lingmo::Applet::Constraints constraints)
{
    if (constraints & Lingmo::Applet::LocationConstraint) {
        if (m_innerContainment) {
            m_innerContainment->setLocation(location());
        }
    }
    if (constraints & Lingmo::Applet::FormFactorConstraint) {
        if (m_innerContainment) {
            m_innerContainment->setFormFactor(formFactor());
        }
    }

    if (constraints & Lingmo::Applet::UiReadyConstraint) {
        ensureSystrayExists();
    }
}

QQuickItem *GroupedAppletsContainer::internalContainmentItem()
{
    return m_internalContainmentItem;
}

K_PLUGIN_CLASS(GroupedAppletsContainer)

#include "groupedappletscontainer.moc"
