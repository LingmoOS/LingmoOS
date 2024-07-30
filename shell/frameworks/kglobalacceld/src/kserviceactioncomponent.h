/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICEACTIONCOMPONENT_H
#define KSERVICEACTIONCOMPONENT_H

#include "component.h"

#include <KService>

#include <memory>

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KServiceActionComponent : public Component
{
    Q_OBJECT

public:
    ~KServiceActionComponent() override;

    void loadFromService();
    void emitGlobalShortcutPressed(const GlobalShortcut &shortcut) override;
    void writeSettings(KConfigGroup &config) const override;
    void loadSettings(KConfigGroup &config) override;
    bool cleanUp() override;

private:
    friend class ::GlobalShortcutsRegistry;
    //! Constructs a KServiceActionComponent. This is a private constuctor, to create
    //! a KServiceActionComponent, use GlobalShortcutsRegistry::self()->createServiceActionComponent().
    KServiceActionComponent(KService::Ptr service);

    KService::Ptr m_service;
};

#endif /* #ifndef KSERVICEACTIONCOMPONENT_H */
