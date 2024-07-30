/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2017 Dorian Vogel <dorianvogel@gmail.com>
 *    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#pragma once

#include "displaybrightness.h"

#include <QList>
#include <QObject>

class DDCutilDetector : public DisplayBrightnessDetector
{
    Q_OBJECT

public:
    explicit DDCutilDetector(QObject *parent = nullptr);
    ~DDCutilDetector();

    void detect() override;
    QList<DisplayBrightness *> displays() const override;
};
