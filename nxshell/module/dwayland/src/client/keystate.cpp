/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "keystate.h"
#include "wayland_pointer_p.h"
#include <QDebug>
#include <QPointer>
#include <wayland-client-protocol.h>
#include <wayland-keystate-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Keystate::Private
{
public:
    Private()
    {
    }

    WaylandPointer<org_kde_kwin_keystate, org_kde_kwin_keystate_destroy> keystate;

    static void org_kde_kwin_keystate_stateChanged(void *data, struct org_kde_kwin_keystate * /*keystate*/, uint32_t k, uint32_t s)
    {
        auto q = static_cast<Keystate *>(data);
        q->stateChanged(Key(k), State(s));
    }

    static const org_kde_kwin_keystate_listener s_listener;
};

const org_kde_kwin_keystate_listener Keystate::Private::s_listener = {org_kde_kwin_keystate_stateChanged};

Keystate::Keystate(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
}

Keystate::~Keystate() = default;

void Keystate::fetchStates()
{
    org_kde_kwin_keystate_fetchStates(d->keystate);
}

void Keystate::setup(org_kde_kwin_keystate *keystate)
{
    d->keystate.setup(keystate);
    org_kde_kwin_keystate_add_listener(keystate, &Keystate::Private::s_listener, this);
}

void Keystate::destroy()
{
    d->keystate.destroy();
}

void Keystate::setEventQueue(KWayland::Client::EventQueue * /*queue*/)
{
}

}
}
