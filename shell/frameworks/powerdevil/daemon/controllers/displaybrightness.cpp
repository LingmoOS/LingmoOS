/*
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "displaybrightness.h"

DisplayBrightnessDetector::DisplayBrightnessDetector(QObject *parent)
    : QObject(parent)
{
}

DisplayBrightness::DisplayBrightness(QObject *parent)
    : QObject(parent)
{
}

bool DisplayBrightness::isInternal() const
{
    return false;
}

std::optional<QByteArray> DisplayBrightness::edidData() const
{
    return std::nullopt;
}

#include "moc_displaybrightness.cpp"
