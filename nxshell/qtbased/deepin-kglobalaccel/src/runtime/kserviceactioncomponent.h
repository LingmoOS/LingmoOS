#ifndef KSERVICEACTIONCOMPONENT_H
#define KSERVICEACTIONCOMPONENT_H
/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>
   Copyright (C) 2016 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "component.h"

#include <KDesktopFile>

namespace KdeDGlobalAccel {

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KServiceActionComponent : public Component
{
    Q_OBJECT

public:

    //! Creates a new component. The component will be registered with @p
    //! registry if specified and registered with dbus.
    KServiceActionComponent(
            const QString &serviceStorageId,
            const QString &friendlyName,
            GlobalShortcutsRegistry *registry = nullptr);

    ~KServiceActionComponent() Q_DECL_OVERRIDE;

    void loadFromService();
    void emitGlobalShortcutPressed(const GlobalShortcut &shortcut) Q_DECL_OVERRIDE;

    bool cleanUp() Q_DECL_OVERRIDE;

private:
    QString m_serviceStorageId;
    KDesktopFile m_desktopFile;
};



}


#endif /* #ifndef COMPONENT_H */
