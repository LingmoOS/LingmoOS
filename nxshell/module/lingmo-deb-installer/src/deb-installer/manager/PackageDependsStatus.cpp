// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PackageDependsStatus.h"
#include "utils/package_defines.h"

PackageDependsStatus PackageDependsStatus::ok()
{
    return {Pkg::DependsStatus::DependsOk, QString()};
}

PackageDependsStatus PackageDependsStatus::available(const QString &package)
{
    // 修复卸载p7zip导致lingmo-wine-helper被卸载的问题，Available 添加packageName
    return {Pkg::DependsStatus::DependsAvailable, package};
}

PackageDependsStatus PackageDependsStatus::_break(const QString &package)
{
    return {Pkg::DependsStatus::DependsBreak, package};
}

PackageDependsStatus PackageDependsStatus::_prohibit(const QString &package)
{
    return {Pkg::DependsStatus::DependsBreak, package};
}

PackageDependsStatus::PackageDependsStatus()
    : PackageDependsStatus(Pkg::DependsStatus::DependsOk, QString())
{
}

PackageDependsStatus::PackageDependsStatus(const int status, const QString &package)
    : status(status)
    , package(package)
{
}

PackageDependsStatus &PackageDependsStatus::operator=(const PackageDependsStatus &other)
{
    status = other.status;
    package = other.package;

    return *this;
}

PackageDependsStatus PackageDependsStatus::max(const PackageDependsStatus &other)
{
    if (other.status > status)
        *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::maxEq(const PackageDependsStatus &other)
{
    if (other.status >= status)
        *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::min(const PackageDependsStatus &other)
{
    if (other.status < status)
        *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::minEq(const PackageDependsStatus &other)
{
    if (other.status <= status)
        *this = other;

    return *this;
}

bool PackageDependsStatus::isBreak() const
{
    return status == Pkg::DependsStatus::DependsBreak;
}

bool PackageDependsStatus::isAuthCancel() const
{
    return status == Pkg::DependsStatus::DependsAuthCancel;
}

bool PackageDependsStatus::isAvailable() const
{
    return status == Pkg::DependsStatus::DependsAvailable;
}

bool PackageDependsStatus::isProhibit() const
{
    return status == Pkg::DependsStatus::Prohibit;
}

bool PackageDependsStatus::canInstall() const
{
    return status == Pkg::DependsAvailable || status == Pkg::DependsOk;
}

bool PackageDependsStatus::canInstallCompatible() const
{
    return status == Pkg::CompatibleNotInstalled;
}
