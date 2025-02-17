/*
SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include "kcmoduledata.h"

class AppearanceSettings;

class KScreenLockerData : public KCModuleData
{
    Q_OBJECT

public:
    explicit KScreenLockerData(QObject *parent, const QVariantList &args);

    bool isDefaults() const override;

private:
    AppearanceSettings *m_appearanceSettings = nullptr;
};
