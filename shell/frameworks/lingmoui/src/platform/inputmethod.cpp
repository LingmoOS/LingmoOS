/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "inputmethod.h"

#include "virtualkeyboardwatcher.h"

namespace LingmoUI
{
namespace Platform
{

class LINGMOUIPLATFORM_NO_EXPORT InputMethod::Private
{
public:
    bool available = false;
    bool enabled = false;
    bool active = false;
    bool visible = false;
};

InputMethod::InputMethod(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    auto watcher = VirtualKeyboardWatcher::self();

    connect(watcher, &VirtualKeyboardWatcher::availableChanged, this, [this]() {
        d->available = VirtualKeyboardWatcher::self()->available();
        Q_EMIT availableChanged();
    });

    connect(watcher, &VirtualKeyboardWatcher::enabledChanged, this, [this]() {
        d->enabled = VirtualKeyboardWatcher::self()->enabled();
        Q_EMIT enabledChanged();
    });

    connect(watcher, &VirtualKeyboardWatcher::activeChanged, this, [this]() {
        d->active = VirtualKeyboardWatcher::self()->active();
        Q_EMIT activeChanged();
    });

    connect(watcher, &VirtualKeyboardWatcher::visibleChanged, this, [this]() {
        d->visible = VirtualKeyboardWatcher::self()->visible();
        Q_EMIT visibleChanged();
    });

    connect(watcher, &VirtualKeyboardWatcher::willShowOnActiveChanged, this, [this]() {
        Q_EMIT willShowOnActiveChanged();
    });

    d->available = watcher->available();
    d->enabled = watcher->enabled();
    d->active = watcher->active();
    d->visible = watcher->visible();
}

InputMethod::~InputMethod() = default;

bool InputMethod::available() const
{
    return d->available;
}

bool InputMethod::enabled() const
{
    return d->enabled;
}

bool InputMethod::active() const
{
    return d->active;
}

bool InputMethod::visible() const
{
    return d->visible;
}

bool InputMethod::willShowOnActive() const
{
    return VirtualKeyboardWatcher::self()->willShowOnActive();
}

}
}

#include "moc_inputmethod.cpp"
