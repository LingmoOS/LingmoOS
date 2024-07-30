/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBS_PLASMA_PACKAGES_P_H
#define LIBS_PLASMA_PACKAGES_P_H

#include <KPackage/Package>
#include <KPackage/PackageStructure>

namespace Plasma
{
class GenericPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;
    void initPackage(KPackage::Package *package) override;
};

} // namespace Plasma

#endif // LIBS_PLASMA_PACKAGES_P_H
