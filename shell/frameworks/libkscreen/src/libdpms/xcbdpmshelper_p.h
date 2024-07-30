//
// SPDX-FileCopyrightText: 2015 by Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "abstractdpmshelper_p.h"

#include <QScopedPointer>

class XcbDpmsHelper : public KScreen::AbstractDpmsHelper
{
public:
    XcbDpmsHelper();
    ~XcbDpmsHelper() override;

    void trigger(KScreen::Dpms::Mode mode, const QList<QScreen *> &screens) override;
};
