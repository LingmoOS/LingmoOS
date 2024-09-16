/* This file is part of the KDE libraries
    Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

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
#include "kglobalaccel_interface.h"
#include "globalshortcutsregistry.h"

class KGlobalAccelInterface::Private
{
public:
    Private(GlobalShortcutsRegistry *owner)
        : owner(owner) {}
    GlobalShortcutsRegistry *owner;
};

KGlobalAccelInterface::KGlobalAccelInterface(QObject *owner)
    : QObject(owner)
    , d(new Private(qobject_cast<GlobalShortcutsRegistry*>(owner)))
{
}

KGlobalAccelInterface::~KGlobalAccelInterface()
{
}

void KGlobalAccelInterface::setRegistry(GlobalShortcutsRegistry *registry)
{
    setParent(registry);
    d->owner = registry;
}

void KGlobalAccelInterface::syncWindowingSystem()
{
}

bool KGlobalAccelInterface::keyPressed(int keyQt)
{
    return d->owner->keyPressed(keyQt);
}

void KGlobalAccelInterface::grabKeys()
{
    d->owner->grabKeys();
}

void KGlobalAccelInterface::ungrabKeys()
{
    d->owner->ungrabKeys();
}
