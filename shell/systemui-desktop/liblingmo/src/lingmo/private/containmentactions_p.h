/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMO_CONTAINMENTACTIONSPRIVATE_H
#define LINGMO_CONTAINMENTACTIONSPRIVATE_H

#include <KPluginMetaData>

namespace Lingmo
{

class ContainmentActionsPrivate
{
public:
    ContainmentActionsPrivate(const QVariant &arg, ContainmentActions * /*containmentActions*/)
        : containmentActionsDescription(arg.value<KPluginMetaData>())
    {
    }

    const KPluginMetaData containmentActionsDescription;
    Containment *containment = nullptr;
};

} // namespace Lingmo
#endif // LINGMO_CONTAINMENTACTIONSPRIVATE_H
