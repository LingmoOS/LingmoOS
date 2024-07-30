/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KMODIFIERKEYINFOPROVIDERWAYLAND_H
#define KMODIFIERKEYINFOPROVIDERWAYLAND_H

#include <kmodifierkeyinfoprovider_p.h>

class KeyState;

class KModifierKeyInfoProviderWayland : public KModifierKeyInfoProvider
{
    Q_OBJECT
public:
    KModifierKeyInfoProviderWayland();
    ~KModifierKeyInfoProviderWayland();

    bool setKeyLatched(Qt::Key key, bool latched) override;
    bool setKeyLocked(Qt::Key key, bool locked) override;

private:
    KeyState *m_keystate;
};

#endif
