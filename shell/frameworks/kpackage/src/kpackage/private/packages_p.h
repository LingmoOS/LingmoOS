/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kpackage/packagestructure.h"

class GenericPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override;
};

class GenericQMLPackage : public GenericPackage
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override;
};
