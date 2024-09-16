// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYSINFO_H
#define DSYSINFO_H

#include <dtkcore_global.h>

#include <QLocale>

DCORE_BEGIN_NAMESPACE

class DSysInfoPrivate;
class LIBDTKCORESHARED_EXPORT DSysInfo
{
    Q_GADGET
public:
    enum ProductType {
        UnknownType = 0,
        Deepin,
        ArchLinux,
        CentOS,
        Debian,
        Fedora,
        LinuxMint,
        Manjaro,
        openSUSE,
        SailfishOS,
        Ubuntu,
        Uos,
        Gentoo,
        NixOS
    };

    enum DeepinType {
        UnknownDeepin = 0,
        DeepinDesktop,
        DeepinProfessional,
        DeepinServer,
        DeepinPersonal,
        DeepinMilitary
    };

    enum LogoType {
        Normal = 0,
        Light,
        Symbolic,
        Transparent
    };

    enum OrgType {
        Distribution, //!<@~english distribution itself
        Distributor, //!<@~english distributer of the current distribution
        Manufacturer //!<@~english manufacturer of the current distribution or device
    };

    enum UosType {
        UosTypeUnknown,
        UosDesktop,
        UosServer,
        UosDevice,
        UosSmart,

        UosTypeCount // must at last
    };

    enum UosEdition {
        UosEditionUnknown,
        UosProfessional,
        UosHome,
        UosCommunity,
        UosMilitary,
        UosEnterprise,
        UosEnterpriseC,
        UosEuler,
        UosMilitaryS, // for Server
        UosDeviceEdition,
        UosEducation,

        UosEditionCount // must at last
    };

    enum UosArch {
        UosArchUnknown,
        UosAMD64 = 1 << 0,
        UosARM64 = 1 << 1,
        UosMIPS64 = 1 << 2,
        UosSW64 = 1 << 3
    };

    //! @~english enum.Arch
    enum Arch {
        ARM64,      /*!< @~english arm64 */
        ARM64_BE,   /*!< @~english arm64-be */
        ARM,        /*!< @~english arm */
        ARM_BE,     /*!< @~english arm-be */
        ALPHA,      /*!< @~english alpha */
        SW_64,      /*!< @~english sw_64 */
        ARC,        /*!< @~english arc */
        ARC_BE,     /*!< @~english arc-be */
        CRIS,       /*!< @~english cris */
        X86_64,     /*!< @~english x86-64 */
        X86,        /*!< @~english x86 */
        IA64,       /*!< @~english ia64 */
        LOONGARCH64,/*!< @~english loongarch64 */
        M68K,       /*!< @~english m68k */
        MIPS64_LE,  /*!< @~english mips64-le */
        MIPS64,     /*!< @~english mips64 */
        MIPS_LE,    /*!< @~english mips-le */
        MIPS,       /*!< @~english mips */
        NIOS2,      /*!< @~english nios2 */
        PARISC64,   /*!< @~english parisc64 */
        PARISC,     /*!< @~english parisc */
        PPC64_LE,   /*!< @~english ppc64-le */
        PPC64,      /*!< @~english ppc64 */
        PPC,        /*!< @~english ppc */
        PPC_LE,     /*!< @~english ppc-le */
        RISCV32,    /*!< @~english riscv32 */
        RISCV64,    /*!< @~english riscv64 */
        S390X,      /*!< @~english s390x */
        S390,       /*!< @~english s390 */
        SH64,       /*!< @~english sh64 */
        SH,         /*!< @~english sh */
        SPARC64,    /*!< @~english sparc64 */
        SPARC,      /*!< @~english sparc */
        TILEGX,     /*!< @~english tilegx */

        NUM_ARCHES, /*!< @~english number of defined Arch types */
    };
    Q_ENUM(Arch) // Q_GADGET

#ifdef Q_OS_LINUX
    static bool isDeepin();
    static bool isDDE();
    static DeepinType deepinType();
    static QString deepinTypeDisplayName(const QLocale &locale = QLocale::system());
    static QString deepinVersion();
    static QString deepinEdition();
    static QString deepinCopyright();

    // uos version interface
    static UosType uosType();
    static UosEdition uosEditionType();
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    Q_DECL_DEPRECATED_X("Use arch() instead") static UosArch uosArch();
#endif
    static QString uosProductTypeName(const QLocale &locale = QLocale::system());
    static QString uosSystemName(const QLocale &locale = QLocale::system());
    static QString uosEditionName(const QLocale &locale = QLocale::system());

    static QString spVersion(); // SP1...SP99
    static QString udpateVersion(); // update1...update9
    static QString majorVersion();
    static QString minorVersion();
    static QString buildVersion(); // xyzs
#endif

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    Q_DECL_DEPRECATED_X("Use distributionInfoPath() instead") static QString deepinDistributionInfoPath();
    Q_DECL_DEPRECATED_X("Use distributionOrgName() instead") static QString deepinDistributorName();
    Q_DECL_DEPRECATED_X("Use distributionOrgWebsite() instead") static QPair<QString, QString> deepinDistributorWebsite();
    Q_DECL_DEPRECATED_X("Use distributionOrgLogo() instead") static QString deepinDistributorLogo(LogoType type = Normal, const QString & fallback = QString());
#endif
    static QString distributionInfoPath();
    static QString distributionInfoSectionName(OrgType type);

    static QString distributionOrgName(OrgType type = Distribution, const QLocale &locale = QLocale::system());
    static QPair<QString, QString> distributionOrgWebsite(OrgType type = Distribution);
    static QString distributionOrgLogo(OrgType orgType = Distribution, LogoType type = Normal, const QString & fallback = QString());

    static QString operatingSystemName();
    static ProductType productType();
    static QString productTypeString();
    static QString productVersion();
    static bool isCommunityEdition();

    static QString computerName();
    static QString cpuModelName();
    static qint64 memoryInstalledSize();
    static qint64 memoryTotalSize();
    static qint64 systemDiskSize();

    static QDateTime bootTime();
    static QDateTime shutdownTime();
    static qint64 uptime();
    static Arch arch();
};

DCORE_END_NAMESPACE

#endif // DSYSINFO_H
