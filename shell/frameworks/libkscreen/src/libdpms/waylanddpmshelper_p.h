// SPDX-FileCopyrightText: 2015 by Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "abstractdpmshelper_p.h"

#include <QMap>
#include <QObject>

class DpmsManager;

class WaylandDpmsHelper : public KScreen::AbstractDpmsHelper
{
    Q_OBJECT
public:
    WaylandDpmsHelper();
    ~WaylandDpmsHelper() override;
    void trigger(KScreen::Dpms::Mode mode, const QList<QScreen *> &screens) override;

private:
    DpmsManager *m_dpmsManager = nullptr;
};
