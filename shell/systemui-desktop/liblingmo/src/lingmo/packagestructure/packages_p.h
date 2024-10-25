/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBS_LINGMO_PACKAGES_P_H
#define LIBS_LINGMO_PACKAGES_P_H

#include <KPackage/Package>
#include <KPackage/PackageStructure>

namespace Lingmo
{
class GenericPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;
    void initPackage(KPackage::Package *package) override;
};

} // namespace Lingmo

#endif // LIBS_LINGMO_PACKAGES_P_H
