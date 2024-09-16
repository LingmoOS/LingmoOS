// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGE_DEFINES_H
#define PACKAGE_DEFINES_H

#include <QString>
#include <QPair>
#include <QList>
#include <QMetaType>

/**
  @brief define package common struct / enum , both deb and uab package.
 */
namespace Pkg {

struct DependInfo
{
    QString packageName;  // 依赖的包名称
    QString version;      // 依赖的包的版本
};
typedef QPair<QList<DependInfo>, QList<DependInfo>> DependsPair;

enum AppendFailReason {
    PackageInvalid,
    PackageNotLocal,
    PackageNotInstallable,  // can not install, wrong arch or no permission etc.
    PackageAlreadyExists,   // same name and version package intalled
    PackageNotDdim,
};

}  // namespace Pkg

Q_DECLARE_METATYPE(Pkg::DependsPair)

#endif  // PACKAGE_DEFINES_H
