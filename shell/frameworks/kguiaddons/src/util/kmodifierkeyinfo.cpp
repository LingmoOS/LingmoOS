/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmodifierkeyinfo.h"
#include "kmodifierkeyinfoprovider_p.h"
#include <kguiaddons_debug.h>

#include <QGuiApplication>

#include "config.h"

#if WITH_WAYLAND
#include "kmodifierkeyinfoprovider_wayland.h"
#endif

#if WITH_X11
#include "kmodifierkeyinfoprovider_xcb.h"
#endif

KModifierKeyInfoProvider *createProvider()
{
#if WITH_WAYLAND
    if (qGuiApp->platformName() == QLatin1String("wayland")) {
        return new KModifierKeyInfoProviderWayland;
    }
#endif

#if WITH_X11
    if (qGuiApp->platformName() == QLatin1String("xcb")) {
        return new KModifierKeyInfoProviderXcb;
    }
#endif

    qCWarning(KGUIADDONS_LOG) << "No modifierkeyinfo backend for platform" << qGuiApp->platformName();
    return new KModifierKeyInfoProvider;
}

KModifierKeyInfo::KModifierKeyInfo(QObject *parent)
    : QObject(parent)
    , p(createProvider())
{
    connect(p.data(), &KModifierKeyInfoProvider::keyPressed, this, &KModifierKeyInfo::keyPressed);
    connect(p.data(), &KModifierKeyInfoProvider::keyLatched, this, &KModifierKeyInfo::keyLatched);
    connect(p.data(), &KModifierKeyInfoProvider::keyLocked, this, &KModifierKeyInfo::keyLocked);
    connect(p.data(), &KModifierKeyInfoProvider::buttonPressed, this, &KModifierKeyInfo::buttonPressed);
    connect(p.data(), &KModifierKeyInfoProvider::keyAdded, this, &KModifierKeyInfo::keyAdded);
    connect(p.data(), &KModifierKeyInfoProvider::keyRemoved, this, &KModifierKeyInfo::keyRemoved);
}

KModifierKeyInfo::~KModifierKeyInfo()
{
}

bool KModifierKeyInfo::knowsKey(Qt::Key key) const
{
    return p->knowsKey(key);
}

const QList<Qt::Key> KModifierKeyInfo::knownKeys() const
{
    return p->knownKeys();
}

bool KModifierKeyInfo::isKeyPressed(Qt::Key key) const
{
    return p->isKeyPressed(key);
}

bool KModifierKeyInfo::isKeyLatched(Qt::Key key) const
{
    return p->isKeyLatched(key);
}

bool KModifierKeyInfo::setKeyLatched(Qt::Key key, bool latched)
{
    return p->setKeyLatched(key, latched);
}

bool KModifierKeyInfo::isKeyLocked(Qt::Key key) const
{
    return p->isKeyLocked(key);
}

bool KModifierKeyInfo::setKeyLocked(Qt::Key key, bool locked)
{
    return p->setKeyLocked(key, locked);
}

bool KModifierKeyInfo::isButtonPressed(Qt::MouseButton button) const
{
    return p->isButtonPressed(button);
}

#include "moc_kmodifierkeyinfo.cpp"
