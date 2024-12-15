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
    QString packageName;  // depends package name
    QString version;      // depends pacakge version
};
using DependsPair = QPair<QList<DependInfo>, QList<DependInfo>>;

enum PackageType {
    UnknownPackage,
    Deb,  // ensure deb priority
    Uab,
    Ddim,
    Proxy,  // both deb and uab
};

enum PackageReadability {
    PkgReadable,
    PkgNotInLocal,
    PkgNoPermission,
};

enum AppendFailReason {
    PackageInvalid,
    PackageNotLocal,
    PackageNotInstallable,  // can not install, wrong arch or no permission etc.
    PackageAlreadyExists,   // same name and version package intalled
    PackageNotDdim,
};

enum PackageInstallStatus {
    NotInstalled,
    InstalledSameVersion,     // current version installed
    InstalledEarlierVersion,  // earlier version installed
    InstalledLaterVersion,    // later version installed
};

enum DependsStatus {
    DependsOk,            // ready to install
    DependsAvailable,     // alailable but need download depends
    DependsBreak,         // depends are unavaliable, confilcit, or no depends packge etc.
    DependsVerifyFailed,  // signature verify failed
    DependsAuthCancel,    // pre depends (wine, linglong) auth check failed
    ArchBreak,            // arch check failed, e.g.: amd64 package cannot install in arm system
    Prohibit,             // The application is restricted by the domain management and cannot be installed

    CompatibleNotInstalled,  // Package depends break, but can install to compatible rootfs
    CompatibleIntalled,      // Package depends ok, but installed in (compatible mode / current system)
};

enum PackageOperationStatus {
    Prepare,    // prepare install/uninstall
    Operating,  // installing/uninstalling
    Success,
    Failed,
    Waiting,       // waitng for next intall/uninstall
    VerifyFailed,  // runtime signature verfiy failed (hierarchical verify)
};

// Signature fail error code
enum ErrorCode {
    NoError,
    UnknownError,
    NoDigitalSignature = 101,
    DigitalSignatureError,
    ConfigAuthCancel = 127,     // Authentication failed
    ApplocationProhibit = 404,  // the current package is in the blacklist and is prohibited from installation
};

// Install / Uninstall process return code
enum ExitCode {
    ExitNoError = 0,      // success
    ExitAuthError = 126,  // authentication check cancel or failed
    ExitUnknownError = -1,
};

}  // namespace Pkg

Q_DECLARE_METATYPE(Pkg::DependsPair)
Q_DECLARE_METATYPE(Pkg::PackageType)

#endif  // PACKAGE_DEFINES_H
