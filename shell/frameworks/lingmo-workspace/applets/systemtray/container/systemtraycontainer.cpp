/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "systemtraycontainer.h"
#include "debug.h"

#include <Lingmo/Corona>
#include <LingmoQuick/AppletQuickItem>
#include <QAction>
#include <kactioncollection.h>

using namespace Qt::StringLiterals;

SystemTrayContainer::SystemTrayContainer(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Lingmo::Applet(parent, data, args)
{
}

SystemTrayContainer::~SystemTrayContainer()
{
}

void SystemTrayContainer::init()
{
    Applet::init();

    // in the first creation we immediately create the systray: so it's accessible during desktop scripting
    uint id = config().readEntry("SystrayContainmentId", 0);

    if (id == 0) {
        ensureSystrayExists();
    }
}

void SystemTrayContainer::ensureSystrayExists()
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

    uint id = config().readEntry("SystrayContainmentId", 0);
    if (id > 0) {
        for (const auto conts = c->containments(); Lingmo::Containment * candidate : conts) {
            if (candidate->id() == id) {
                m_innerContainment = candidate;
                break;
            }
        }
        qCDebug(SYSTEM_TRAY_CONTAINER) << "Containment id" << id << "that used to be a system tray was deleted";
        // id = 0;
    }

    if (!m_innerContainment) {
        m_innerContainment = c->createContainment(QStringLiteral("org.kde.lingmo.private.systemtray"), QVariantList() << u"org.kde.lingmo:force-create"_s);
        config().writeEntry("SystrayContainmentId", m_innerContainment->id());
    }

    if (!m_innerContainment) {
        return;
    }

    m_innerContainment->setParent(this);
    connect(containment(), &Lingmo::Containment::screenChanged, m_innerContainment.data(), &Lingmo::Containment::reactToScreenChange);

    if (formFactor() == Lingmo::Types::Horizontal || formFactor() == Lingmo::Types::Vertical) {
        m_innerContainment->setFormFactor(formFactor());
    } else {
        m_innerContainment->setFormFactor(Lingmo::Types::Horizontal);
    }

    m_innerContainment->setLocation(location());

    auto oldInternalSystray = m_internalSystray;
    m_internalSystray = LingmoQuick::AppletQuickItem::itemForApplet(m_innerContainment);

    if (m_internalSystray != oldInternalSystray) {
        Q_EMIT internalSystrayChanged();
    }

    setInternalAction(u"configure"_s, m_innerContainment->internalAction(u"configure"_s));
    connect(m_innerContainment.data(), &Lingmo::Containment::configureRequested, this, [this](Lingmo::Applet *applet) {
        Q_EMIT containment()->configureRequested(applet);
    });

    if (m_internalSystray) {
        // don't let internal systray manage context menus
        m_internalSystray->setAcceptedMouseButtons(Qt::NoButton);
    }

    // replace internal remove action with ours
    m_innerContainment->setInternalAction(u"remove"_s, internalAction(u"remove"_s));

    // Sync the display hints
    m_innerContainment->setContainmentDisplayHints(containmentDisplayHints() | Lingmo::Types::ContainmentDrawsPlasmoidHeading
                                                   | Lingmo::Types::ContainmentForcesSquarePlasmoids);
    connect(cont, &Lingmo::Containment::containmentDisplayHintsChanged, this, [this]() {
        m_innerContainment->setContainmentDisplayHints(containmentDisplayHints() | Lingmo::Types::ContainmentDrawsPlasmoidHeading
                                                       | Lingmo::Types::ContainmentForcesSquarePlasmoids);
    });
}

void SystemTrayContainer::constraintsEvent(Lingmo::Applet::Constraints constraints)
{
    if (constraints & Lingmo::Applet::LocationConstraint) {
        if (m_innerContainment) {
            m_innerContainment->setLocation(location());
        }
    }

    if (constraints & Lingmo::Applet::FormFactorConstraint) {
        if (m_innerContainment) {
            if (formFactor() == Lingmo::Types::Horizontal || formFactor() == Lingmo::Types::Vertical) {
                m_innerContainment->setFormFactor(formFactor());
            } else {
                m_innerContainment->setFormFactor(Lingmo::Types::Horizontal);
            }
        }
    }

    if (constraints & Lingmo::Applet::UiReadyConstraint) {
        ensureSystrayExists();
    }
}

QQuickItem *SystemTrayContainer::internalSystray()
{
    return m_internalSystray;
}

K_PLUGIN_CLASS(SystemTrayContainer)

#include "systemtraycontainer.moc"
