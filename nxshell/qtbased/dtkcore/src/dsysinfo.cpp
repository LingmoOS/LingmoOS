// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsysinfo.h"
#include "ddesktopentry.h"

#include <QFile>
#include <QLocale>
#include <QStorageInfo>
#include <QProcess>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <QRegularExpression>
#include <QLoggingCategory>
#include <qmath.h>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

#define OS_VERSION_FILE     DSYSINFO_PREFIX"/etc/os-version"
#define LSB_RELEASE_FILE    DSYSINFO_PREFIX"/etc/lsb-release"
#define OS_RELEASE_FILE     DSYSINFO_PREFIX"/etc/os-release"
#define DEEPIN_VERSION_FILE DSYSINFO_PREFIX"/etc/deepin-version"

static inline bool inTest()
{
    return !QLatin1String(DSYSINFO_PREFIX).isEmpty();
}

DCORE_BEGIN_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logSysInfo, "dtk.dsysinfo")
#else
Q_LOGGING_CATEGORY(logSysInfo, "dtk.dsysinfo", QtInfoMsg)
#endif

class Q_DECL_HIDDEN DSysInfoPrivate
{
public:
    DSysInfoPrivate();

#ifdef Q_OS_LINUX
    void ensureDeepinInfo();
    bool ensureOsVersion();
    void ensureDistributionInfo();
    bool splitA_BC_DMode();
#endif
    void ensureReleaseInfo();
    void ensureComputerInfo();
    QMap<QString, QString> parseInfoFile(QFile &file);
    QMap<QString, QString> parseInfoContent(const QString &content);
#ifdef Q_OS_LINUX
    DSysInfo::DeepinType deepinType = DSysInfo::DeepinType(-1);
    QMap<QString, QString> deepinTypeMap; //Type Name with Language
    QString deepinVersion;
    QString deepinEdition;
    QString deepinCopyright;

    QString majorVersion;
    QString minorVersion;
    struct MinVersion {
        enum Type {
            A_BC_D, /*!< Professional Edition*/
            X_Y_Z,  /*!< Home Edition*/
            A_B_C  /*!< Community Edition*/
        };
        MinVersion()
            : A(0)
            , B(0)
            , BC(0)
            , C(0)
            , D(0)
            , X(0)
            , Y(0)
            , Z(0)
        {
        }

        uint A, B, BC, C, D; // A-BC-D
        uint X, Y, Z;
        Type type;
    };
    struct OSBuild {
        OSBuild():A(0), B(0), C(0), D(0), xyz(100){
        }
        uint A, B, C, D, E, xyz; // ABCDE.xyz
    };

    MinVersion minVersion;
    OSBuild osBuild;
#endif

    QScopedPointer<DDesktopEntry> distributionInfo;

    DSysInfo::ProductType productType = DSysInfo::ProductType(-1);
    QString prettyName;
    QString productTypeString;
    QString productVersion;

    QString computerName;
    QString cpuModelName;
    qint64 memoryAvailableSize = -1;
    qint64 memoryInstalledSize = -1;
    qint64 diskSize = 0;
};

DSysInfoPrivate::DSysInfoPrivate()
{

}

#ifdef Q_OS_LINUX
void DSysInfoPrivate::ensureDistributionInfo()
{
    if (distributionInfo)
        return;

    const QString distributionInfoFile(DSysInfo::distributionInfoPath());
    // Generic DDE distribution info
    distributionInfo.reset(new DDesktopEntry(distributionInfoFile));
}

bool DSysInfoPrivate::splitA_BC_DMode()
{
    // A-BC-D
    bool ok = false;
    uint minv = minorVersion.toUInt(&ok);
    if (ok) {
        minVersion.D = minv % 10;
    } else if (minorVersion.length() > 0) {
        const QString D = minorVersion.right(1);
        if (D.contains(QRegularExpression("[0-9A-Z]"))) {
            // 0-9...A-Z
            minVersion.D = 10 + static_cast<uint>(D.data()->toLatin1() - 'A');
        } else {
            qWarning() << "invalid minorVersion";
            minVersion.D = 0;
        }
    }
    uint minVer = minorVersion.left(3).toUInt();
    minVersion.BC = minVer % 100;
    minVer /= 100;
    minVersion.A = minVer % 10;
    minVersion.type = MinVersion::A_BC_D;
    return ok;
}

void DSysInfoPrivate::ensureDeepinInfo()
{
    if (static_cast<int>(deepinType) > 0 && !inTest())
        return;

    if (inTest())
        deepinTypeMap.clear(); // clear cache for test

    QFile file(DEEPIN_VERSION_FILE);

    if (!file.open(QFile::ReadOnly)) {
        deepinType = DSysInfo::UnknownDeepin;

        return;
    }

    char buf[1024];
    int buf_length = 0;

    Q_FOREVER {
        buf_length = file.readLine(buf, sizeof(buf));

        if (buf_length < 0)
            break;

        const QByteArray line(buf, buf_length);
        const QByteArrayList &list = line.split('=');

        if (list.count() != 2) {
            continue;
        }

        const auto key_value = qMakePair(list.first().trimmed(), list.last().trimmed());

        if (key_value.first == "Version") {
            deepinVersion = key_value.second;
        } else if (line.startsWith("Type")) {
            if (key_value.first == "Type") {
                deepinTypeMap[QString()] = QString::fromLatin1(key_value.second);
            } else if (key_value.first.at(4) == '[' && key_value.first.at(key_value.first.size() - 1) == ']') {
                const QByteArray &language = key_value.first.mid(5, key_value.first.size() - 6);

                if (!language.isEmpty()) {
                    deepinTypeMap[QString::fromLatin1(language)] = QString::fromUtf8(key_value.second);
                }
            }
        } else if (key_value.first == "Edition") {
            deepinEdition = QString::fromUtf8(key_value.second);
        } else if (key_value.first == "Copyright") {
            deepinCopyright = QString::fromUtf8(key_value.second);
        }

        // deepinTypeMap may not parse finished(multi language) but !deepinTypeMap.isEmpty()
//        if (!deepinTypeMap.isEmpty() && !deepinEdition.isEmpty() && !deepinCopyright.isEmpty()) {
//            break;
//        }
    }

    file.close();

    const QString &deepin_type = deepinTypeMap[QString()];

    if (deepin_type.isEmpty()) {
        deepinType = DSysInfo::UnknownDeepin;
    } else if (deepin_type == "Desktop") {
        deepinType = DSysInfo::DeepinDesktop;
    } else if (deepin_type == "Professional") {
        deepinType = DSysInfo::DeepinProfessional;
    } else if (deepin_type == "Server") {
        deepinType = DSysInfo::DeepinServer;
    } else if (deepin_type == "Personal") {
        deepinType = DSysInfo::DeepinPersonal;
    } else if (deepin_type == "Military") {
        deepinType = DSysInfo::DeepinMilitary;
    } else {
        deepinType = DSysInfo::UnknownDeepin;
    }
}

bool DSysInfoPrivate::ensureOsVersion()
{
    if (osBuild.A > 0 && !inTest())
        return true;

    DDesktopEntry entry(OS_VERSION_FILE);
    bool ok = false;

#define D_ASSET_EXIT(con, msg) do { \
    if (!(con)) { \
        qWarning() << __func__ << msg; \
        return false; \
    } \
} while (false)

    D_ASSET_EXIT(entry.status() == DDesktopEntry::NoError, entry.status());

    // 先获取版本信息
    // ABCDE.xyz.abc
    QString osb = entry.stringValue("OsBuild", "Version");
    QStringList osbs = osb.split(".");
    ok = (osbs.size() >= 2 && osbs.value(0).size() == 5);
    D_ASSET_EXIT(ok, "OsBuild version invalid!");

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QStringList &left = osbs.value(0).split(QString(), Qt::SkipEmptyParts);
#else
    const QStringList &left = osbs.value(0).split(QString(), QString::SkipEmptyParts);
#endif
    D_ASSET_EXIT(left.size() == 5, "OsBuild version(ls) invalid!");

    int idx = 0;
    osBuild.A = left.value(idx++, "0").toUInt(&ok);
    D_ASSET_EXIT(ok, "OsBuild version(A) invalid!");
    osBuild.B = left.value(idx++, "0").toUInt(&ok);
    D_ASSET_EXIT(ok, "OsBuild version(B) invalid!");
    osBuild.C = left.value(idx++, "0").toUInt(&ok);
    if (!ok) {
        auto c = left.value(idx-1, "0").toLatin1();
        D_ASSET_EXIT(c.size()>0, "OsBuild version(C) invalid!");
        osBuild.C = uint(c.at(0));
    }
    osBuild.D = left.value(idx++, "0").toUInt(&ok);
    D_ASSET_EXIT(ok, "OsBuild version(D) invalid!");
    osBuild.E = left.value(idx++, "0").toUInt(&ok);
    D_ASSET_EXIT(ok, "OsBuild version(E) invalid!");

    // xyz
    osBuild.xyz = osbs.value(1).trimmed().toUInt(&ok);

    majorVersion = entry.stringValue("MajorVersion", "Version");
    minorVersion = entry.stringValue("MinorVersion", "Version");

    switch (osBuild.D) {
    case 7: {
        // Home Edition uses the form of "full version number coding -x.y.z"
        const QStringList &versionList = minorVersion.split('.');
        if (versionList.isEmpty()) {
            // If the reading fails, return it directly to empty
            qWarning() << "no minorVersion";
            return false;
        } else if (versionList.length() == 2) {
            // Z is 0
            minVersion.X = versionList.first().toUInt();
            minVersion.Y = versionList.last().toUInt();
            minVersion.Z = 0;
        } else if (versionList.length() == 3) {
            // X.Y.Z exists
            minVersion.X = versionList.at(0).toUInt();
            minVersion.Y = versionList.at(1).toUInt();
            minVersion.Z = versionList.at(2).toUInt();
        }
        minVersion.type = MinVersion::X_Y_Z;
    } break;

    case 3: {
        // The community version uses the form of "full version number coding A.B.C"
        bool a_bc_dMode = false;
        const QStringList &versionList = minorVersion.split('.');
        if (versionList.isEmpty()) {
            // If the reading fails, return it directly to empty
            qWarning() << "no minorVersion";
            return false;
        } else if (versionList.length() == 1) {
            QString modeVersion = versionList.first();
            if (modeVersion.length() == 2) {
                //A.B.C mode and B c are 0
                minVersion.A = modeVersion.toUInt();
                minVersion.B = 0;
                minVersion.C = 0;
            } else {
                // A_BC_D mode
                splitA_BC_DMode();
                a_bc_dMode = true;
            }
        } else if (versionList.length() == 2) {
            // C=0
            minVersion.A = versionList.first().toUInt();
            minVersion.B = versionList.last().toUInt();
            minVersion.C = 0;
        } else if (versionList.length() == 3) {
            // A.B.C exists
            minVersion.A = versionList.at(0).toUInt();
            minVersion.B = versionList.at(1).toUInt();
            minVersion.C = versionList.at(2).toUInt();
        }

        if (!a_bc_dMode)
            minVersion.type = MinVersion::A_B_C;
    } break;
    default: {
        // A-BC-D
        ok = splitA_BC_DMode();
    } break;
    }
    return ok;
}

static QString unquote(const QByteArray &value)
{
    if (value.at(0) == '"' || value.at(0) == '\'') {
        return QString::fromLatin1(value.mid(1, value.size() - 2));
    }

    return QString::fromLatin1(value);
}

static bool readEtcFile(DSysInfoPrivate *info, const char *filename,
                        const QByteArray &idKey, const QByteArray &versionKey, const QByteArray &prettyNameKey)
{

    QFile file(QString::fromLatin1(filename));

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    quint8 valid_data_count = 0;
    char buf[1024];


    if (inTest()) {
        // for test clear cache
        info->productTypeString.clear();
        info->productType = DSysInfo::UnknownType;
    }

    while (valid_data_count < 3) {
        int buf_length = file.readLine(buf, sizeof(buf));

        if (buf_length < 0)
            break;

        const QByteArray line(buf, buf_length - 1);

        if (info->productTypeString.isEmpty() && line.startsWith(idKey)) {
            const QByteArray value(line.constData() + idKey.size());
            info->productTypeString = unquote(value);
            ++valid_data_count;
            continue;
        }

        if (info->prettyName.isEmpty() && line.startsWith(prettyNameKey)) {
            const QByteArray value(line.constData() + prettyNameKey.size());
            info->prettyName = unquote(value);
            ++valid_data_count;
            continue;
        }

        if (info->productVersion.isEmpty() && line.startsWith(versionKey)) {
            const QByteArray value(line.constData() + versionKey.size());
            info->productVersion = unquote(value);
            ++valid_data_count;
            continue;
        }
    }

    file.close();

    return valid_data_count != 0;
}

static bool readOsRelease(DSysInfoPrivate *info)
{
    if (!readEtcFile(info, OS_RELEASE_FILE, "ID=", "VERSION_ID=", "PRETTY_NAME="))
        return readEtcFile(info, DSYSINFO_PREFIX"/usr/lib/os-release", "ID=", "VERSION_ID=", "PRETTY_NAME=");

    return true;
}

static bool readLsbRelease(DSysInfoPrivate *info)
{
    return readEtcFile(info, LSB_RELEASE_FILE, "DISTRIB_ID=", "DISTRIB_RELEASE=", "DISTRIB_DESCRIPTION=");
}
#endif

void DSysInfoPrivate::ensureReleaseInfo()
{
    if (productType > 0 && !inTest()) {
        return;
    }

#ifdef Q_OS_LINUX
    readOsRelease(this);
    readLsbRelease(this);

    if (productTypeString.isEmpty()) {
        productType = DSysInfo::UnknownType;
    } else {
        switch (productTypeString.at(0).unicode()) {
        case 'd':
        case 'D':
            if (productTypeString.compare("deepin", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Deepin;
            } else if (productTypeString.compare("debian", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Debian;
            }
            break;
        case 'a':
        case 'A':
            if (productTypeString.compare("arch", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::ArchLinux;
            break;
        case 'c':
        case 'C':
            if (productTypeString.compare("centos", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::CentOS;
            break;
        case 'f':
        case 'F':
            if (productTypeString.compare("fedora", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::Fedora;
            break;
        case 'g':
        case 'G':
            if (productTypeString.compare("gentoo", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::Gentoo;
            break;
        case 'l':
        case 'L':
            if (productTypeString.compare("linuxmint", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::LinuxMint;
            break;
        case 'm':
        case 'M':
            if (productTypeString.compare("manjaro", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::Manjaro;
            break;
        case 'n':
        case 'N':
            if (productTypeString.compare("nixos", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::NixOS;
            break;
        case 'o':
        case 'O':
            if (productTypeString.compare("opensuse", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::openSUSE;
            break;
        case 's':
        case 'S':
            if (productTypeString.compare("sailfishos", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::SailfishOS;
            break;
        case 'u':
        case 'U':
            if (productTypeString.compare("ubuntu", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Ubuntu;
            } else if (productTypeString.compare("uos", Qt::CaseInsensitive) == 0 || productTypeString.compare("UnionTech OS", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Uos;
            }
            break;
        default:
            productType = DSysInfo::UnknownType;
            break;
        }
    }
#endif
}

void DSysInfoPrivate::ensureComputerInfo()
{
#ifdef Q_OS_LINUX

#endif
}

QMap<QString, QString> DSysInfoPrivate::parseInfoFile(QFile &file)
{
    char buf[1024];
    qint64 lineLength = 0;
    QMap<QString, QString> map;
    do {
        lineLength = file.readLine(buf, sizeof(buf));
        QString s(buf);
        if (s.contains(':')) {
            QStringList list = s.split(':');
            if (list.size() == 2) {
                map.insert(list.first().trimmed(), list.back().trimmed());
            }
        }
    } while (lineLength >= 0);
    return map;
}

QMap<QString, QString> DSysInfoPrivate::parseInfoContent(const QString &content)
{
    QMap<QString, QString> map;
    QStringList lineContents = content.split("\n");
    for (auto lineContent : lineContents) {
        if (lineContent.contains(':')) {
            QStringList list = lineContent.split(':');
            if (list.size() == 2) {
                map.insert(list.first().trimmed(), list.back().trimmed());
            }
        }
    }
    return map;
}

Q_GLOBAL_STATIC(DSysInfoPrivate, siGlobal)

QString DSysInfo::operatingSystemName()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->prettyName;
}

#ifdef Q_OS_LINUX
/*!
  \brief Check current distro is Deepin or not.
  \note Uos will also return true.
 */
bool DSysInfo::isDeepin()
{
    siGlobal->ensureReleaseInfo();

    return productType() == Deepin || productType() == Uos;
}

bool DSysInfo::isDDE()
{
    if (!DSysInfo::isDeepin()) {
        const QByteArray &xsd = qgetenv("XDG_SESSION_DESKTOP");
        return !xsd.compare("deepin", Qt::CaseInsensitive) ||
                !xsd.compare("DDE", Qt::CaseInsensitive);
    }

    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinType != UnknownDeepin;
}

DSysInfo::DeepinType DSysInfo::deepinType()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinType;
}

QString DSysInfo::deepinTypeDisplayName(const QLocale &locale)
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinTypeMap.value(locale.name(), siGlobal->deepinTypeMap.value(QString()));
}

QString DSysInfo::deepinVersion()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinVersion;
}

QString DSysInfo::deepinEdition()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinEdition;
}

QString DSysInfo::deepinCopyright()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinCopyright;
}

/*!
@~english
  \brief
  Display system type [1: desktop] [2: server] [3: special devices]
  \note 根据 osBuild.B 判断
 */
DSysInfo::UosType DSysInfo::uosType()
{
    if (!DSysInfo::isDeepin() && !inTest())
        return UosTypeUnknown;

    siGlobal->ensureOsVersion();

    UosType ost = UosTypeUnknown;
    if ((siGlobal->osBuild.B > UosTypeUnknown && siGlobal->osBuild.B < UosTypeCount)) {
        ost = static_cast<UosType>(siGlobal->osBuild.B);
    }

    return ost;
}

/*!
@~english
  \brief
  Editions: professional version/personal version/community version ...
  \note According to osbuild.b && osbuild.d
 */
DSysInfo::UosEdition DSysInfo::uosEditionType()
{
    siGlobal->ensureOsVersion();
    UosEdition ospt = UosEditionUnknown;
    if (siGlobal->osBuild.B == UosDesktop) {
        switch (siGlobal->osBuild.D) {
        case 1:
            return UosProfessional;
        case 2:
        case 7:
            //The new version of the family version (7) and the old version of the personal version (2) The same as the home does not modify the old logic (7) to ensure the adaptation of the old version
            return UosHome;
        case 3:
            return UosCommunity;
        case 4:
            return UosMilitary;
        case 5:
            return UosDeviceEdition;
        case 6:
            return UosEducation;
        default:
            break;
        }
    } else if (siGlobal->osBuild.B == UosServer) {
        switch (siGlobal->osBuild.D) {
        case 1:
            return UosEnterprise;
        case 2:
            return UosEnterpriseC;
        case 3:
            return UosEuler;
        case 4:
            return UosMilitaryS;
        case 5:
            return UosDeviceEdition;
        default:
            break;
        }
    } else if (siGlobal->osBuild.B == UosDevice){
        ospt = UosEnterprise; // os-version 1.4 if B==Device then et=Enterprise
    }

    return ospt;
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
@~english
   \brief Architecture information (using bit flags of a byte)
  【0x8 sw64】【0x4 mips64】【0x2 arm64】【0x1 amd64】
 */
DSysInfo::UosArch DSysInfo::uosArch()
{
    siGlobal->ensureOsVersion();

    return static_cast<UosArch>(siGlobal->osBuild.E);
}
#endif

static QString getUosVersionValue(const QString &key, const QLocale &locale)
{
    DDesktopEntry entry(OS_VERSION_FILE);
    QString localKey = QString("%1[%2]").arg(key, locale.name());

    return entry.stringValue(localKey, "Version", entry.stringValue(key, "Version"));
}

/*!
@~english
  \brief Version name
  ProductType[xx] The corresponding value of the item, if you can't find the value of the corresponding language, use the value of the productType (desktop/server/device)
  \a locale Current system language
 */
QString DSysInfo::uosProductTypeName(const QLocale &locale)
{
    return getUosVersionValue("ProductType", locale);
}

/*!
@~english
  \brief DSysInfo::osSystemName Version name

  The corresponding value corresponding to SystemName [xx] item, if you can't find the default language of the corresponding language, use the value of SystemName uniontech os
  \a locale Current system language
 */
QString DSysInfo::uosSystemName(const QLocale &locale)
{
    return getUosVersionValue("SystemName", locale);
}

/*!
@~english
  \brief DSysInfo::osEditionName Version name
   EditionName[xx] The corresponding value of the item, if you can't find the value of the corresponding language, use the value of EditionName (Professional/Home/Community ...)
  \a locale Current system language
 */
QString DSysInfo::uosEditionName(const QLocale &locale)
{
    return getUosVersionValue("EditionName", locale);
}

/*!
@~english
  \brief DSysInfo::spVersion Period version name
  BC, A.B.C in the small version number a-bc-d
  Return to SP1-SPXX, if the official version returns empty
  In the x.y.z mode, it will not support returning this version number for the time being
  \ note minversion.bc == 00: The official version minversion.bc | minversion.b == 01-99: SP1 ... .sp99
 */
QString DSysInfo::spVersion()
{
    siGlobal->ensureOsVersion();
    switch (siGlobal->minVersion.type) {
    case DSysInfoPrivate::MinVersion::A_BC_D: {
        if (siGlobal->minVersion.BC > 0) {
            return QString("SP%1").arg(siGlobal->minVersion.BC);
        } else {
            return QString(); // 00 正式版
        }
    }

    case DSysInfoPrivate::MinVersion::A_B_C: {
        if (siGlobal->minVersion.B > 0) {
            return QStringLiteral("SP%1").arg(siGlobal->minVersion.B);
        } else {
            return {};
        }
    }

    case DSysInfoPrivate::MinVersion::X_Y_Z:
        qWarning() << "Getting the SP version in this mode is not supported.";
        return {};
    }
    return QString();
}

/*!
@~english
  \brief DSysInfo::udpateVersion Update version name
  minor version number D in A-BC-D mode、C in A.B.C mode
  Return to Update1 ... Update9, if the official version returns to empty
 In the x.y.z mode, it will not support returning this version number for the time being
  \note minVersion.D == 0：official version    minVersion.D | minVersion.C == 1-9：update1… update9,updateA...updateZ
 */
QString DSysInfo::udpateVersion()
{
    siGlobal->ensureOsVersion();
    switch (siGlobal->minVersion.type) {
    case DSysInfoPrivate::MinVersion::A_BC_D: {
        if (siGlobal->minVersion.D > 0) {
            uint uv = siGlobal->minVersion.D;
            if (uv < 10) {
                return QString("update%1").arg(uv);
            } else if (uv < 36) {
                return QString("update").append(QChar(uv - 10 + 'A'));
            } else {
                qWarning() << "invalid update versoin";
                break;
            }
        } else {
            break; // 0 正式版
        }
    }

    case DSysInfoPrivate::MinVersion::A_B_C: {
        if (siGlobal->minVersion.C > 0) {
            return QStringLiteral("update%1").arg(siGlobal->minVersion.C);
        } else {
            break;
        }
    }

    case DSysInfoPrivate::MinVersion::X_Y_Z:
        qWarning() << "Getting the update version in this mode is not supported.";
        break;
    }

    return {};
}

/*!
@~english
  \brief Main edition number
  Main edition number 【20】【23】【25】【26】【29】【30】
  \note Return to Majorversion value
 */
QString DSysInfo::majorVersion()
{
    siGlobal->ensureOsVersion();
    return siGlobal->majorVersion;
}

/*!
@~english
  \brief DSysInfo::minorVersion minor version
 *【ABCD】 ·[0-9]{4}
 *【A.B.C】 or【X.Y.Z】
  @return the value of minorversion
 */
QString DSysInfo::minorVersion()
{
    siGlobal->ensureOsVersion();
    return siGlobal->minorVersion;
}

/*!
@~english
  \brief DSysInfo::buildVersion Small version number
  System mirror batch number, in order of time (non-retreat) increase from 100-999
  \note Return  osbuild.xyz value
 */
QString DSysInfo::buildVersion()
{
    DDesktopEntry entry(OS_VERSION_FILE);
    QString osb = entry.stringValue("OsBuild", "Version");
    return osb.mid(6).trimmed();
}
#endif

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QString DSysInfo::deepinDistributionInfoPath()
{
    return distributionInfoPath();
}
#endif

QString DSysInfo::distributionInfoPath()
{
#ifdef Q_OS_LINUX
    // return "/usr/share/deepin/distribution.info";
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, "deepin/distribution.info");
#else
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath("deepin-distribution.info");
#endif // Q_OS_LINUX
}

QString DSysInfo::distributionInfoSectionName(DSysInfo::OrgType type)
{
    switch (type) {
    case Distribution:
        return "Distribution";
    case Distributor:
        return "Distributor";
    case Manufacturer:
        return "Manufacturer";
    }

    return QString();
}

/*!
@~english
  \return the organization name.

  use \a type as Distribution to get the name of current deepin distribution itself.

  \sa deepinDistributionInfoPath()
 */
QString DSysInfo::distributionOrgName(DSysInfo::OrgType type, const QLocale &locale)
{
#ifdef Q_OS_LINUX
    siGlobal->ensureDistributionInfo();
#endif

    QString fallback = type == Distribution ? QStringLiteral("Deepin") : QString();

    return siGlobal->distributionInfo->localizedValue("Name", locale, distributionInfoSectionName(type), fallback);
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QString DSysInfo::deepinDistributorName()
{
    return distributionOrgName(Distributor);
}
#endif
/*!
@~english
  \return the organization website name and url.

  use \a type as Distribution to get the name of current deepin distribution itself.

  \sa deepinDistributionInfoPath()
 */
QPair<QString, QString> DSysInfo::distributionOrgWebsite(DSysInfo::OrgType type)
{
#ifdef Q_OS_LINUX
    siGlobal->ensureDistributionInfo();
#endif

    QString fallbackSiteName = type == Distribution ? QStringLiteral("www.deepin.org") : QString();
    QString fallbackSiteUrl = type == Distribution ? QStringLiteral("https://www.deepin.org") : QString();

    return {
        siGlobal->distributionInfo->stringValue("WebsiteName", distributionInfoSectionName(type), fallbackSiteName),
        siGlobal->distributionInfo->stringValue("Website", distributionInfoSectionName(type), fallbackSiteUrl),
    };
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QPair<QString, QString> DSysInfo::deepinDistributorWebsite()
{
    return distributionOrgWebsite(Distributor);
}
#endif

/*!
@~english
  \return the obtained organization logo path, or the given \a fallback one if there are no such logo.

  use \a type as Distribution to get the logo of current deepin distribution itself.

  \sa deepinDistributionInfoPath()
 */
QString DSysInfo::distributionOrgLogo(DSysInfo::OrgType orgType, DSysInfo::LogoType type, const QString &fallback)
{
    DDesktopEntry distributionInfo(distributionInfoPath());
    QString orgSectionName = distributionInfoSectionName(orgType);

    switch (type) {
    case Normal:
        return distributionInfo.stringValue("Logo", orgSectionName, fallback);
    case Light:
        return distributionInfo.stringValue("LogoLight", orgSectionName, fallback);
    case Symbolic:
        return distributionInfo.stringValue("LogoSymbolic", orgSectionName, fallback);
    case Transparent:
        return distributionInfo.stringValue("LogoTransparent", orgSectionName, fallback);
    }

    return QString();
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
QString DSysInfo::deepinDistributorLogo(DSysInfo::LogoType type, const QString &fallback)
{
    return distributionOrgLogo(Distributor, type, fallback);
}
#endif

DSysInfo::ProductType DSysInfo::productType()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->productType;
}

QString DSysInfo::productTypeString()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->productTypeString;
}

QString DSysInfo::productVersion()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->productVersion;
}

/*!
@~english
  \brief Check if current edition is a community edition

  Developer can use this way to check if we need enable or disable features
  for community or enterprise edition.

  Current rule:
   - Professional, Server, Personal edition (DeepinType) will be treat as Enterprise edition.
   - Uos (ProductType) will be treat as Enterprise edition.

  \return true if it's on a community edition distro/installation
 */
bool DSysInfo::isCommunityEdition()
{
#ifdef Q_OS_LINUX
    DeepinType type = deepinType();
    QList<DeepinType> enterpriseTypes {
        DeepinProfessional, DeepinServer, DeepinPersonal
    };

    if (enterpriseTypes.contains(type)) {
        return false;
    }

    if (productType() == Uos) {
        return false;
    }
#endif // Q_OS_LINUX

    return true;
}

QString DSysInfo::computerName()
{
#ifdef Q_OS_LINUX
    struct utsname u;
    if (uname(&u) == 0)
        siGlobal->computerName = QString::fromLatin1(u.nodename);

    return siGlobal->computerName;
#endif
    return QString();
}

QString DSysInfo::cpuModelName()
{
    if (!siGlobal->cpuModelName.isEmpty())
        return siGlobal->cpuModelName;

#ifdef Q_OS_LINUX
    static QFile file("/proc/cpuinfo");

    if (file.open(QFile::ReadOnly)) {
        QMap<QString, QString> map = siGlobal->parseInfoFile(file);
        if (map.contains("Processor")) {
            // arm-cpuinfo hw_kirin-cpuinfo
            siGlobal->cpuModelName = map.value("Processor");
        } else if (map.contains("model name")) {
            // cpuinfo
            siGlobal->cpuModelName = map.value("model name");
        } else if (map.contains("cpu model")) {
            // loonson3-cpuinfo sw-cpuinfo
            siGlobal->cpuModelName = map.value("cpu model");
        } else if (map.contains("Hardware")) {
            // "HardWare" field contains cpu info on huawei kirin machine (e.g. klv or klu)
            siGlobal->cpuModelName = map.value("Hardware");
        }

        file.close();
    }

    // Get the cpu info by executing lscpu command
    if (siGlobal->cpuModelName.isEmpty()) {
        const auto &lscpu_command = QStandardPaths::findExecutable("lscpu");
        if (lscpu_command.isEmpty()) {
            qWarning() << "lscpu not found";
            return QString();
        }
        QProcess lscpu;
        QStringList env = QProcess::systemEnvironment();
        env << "LC_ALL=C"; // Add an environment variable
        lscpu.setEnvironment(env);
        lscpu.setProgram(lscpu_command);
        lscpu.start();
        if (lscpu.waitForFinished(3000)) {
            const QMap<QString, QString> map = siGlobal->parseInfoContent(lscpu.readAll());
            if (map.contains("Model name")) {
                siGlobal->cpuModelName = map.value("Model name");
            }
        } else {
            qWarning() << "lscpu:" << lscpu.errorString();
        }
    }

    return siGlobal->cpuModelName;
#endif
    return QString();
}

/*!
@~english
  \return the installed memory size
 */
qint64 DSysInfo::memoryInstalledSize()
{
#ifdef Q_OS_LINUX
    // Getting Memory Installed Size
    // TODO: way to not dept on lshw?
    if (siGlobal->memoryInstalledSize >= 0) {
        return siGlobal->memoryInstalledSize;
    }
    if (!QStandardPaths::findExecutable("lshw").isEmpty()) {
        QProcess lshw;

        lshw.start("lshw", {"-c", "memory", "-json", "-sanitize"}, QIODevice::ReadOnly);

        if (!lshw.waitForFinished()) {
            return -1;
        }

        const QByteArray &lshwInfoJson = lshw.readAllStandardOutput();

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(lshwInfoJson, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(logSysInfo(), "parse failed, expect json doc from lshw command");
            return -1;
        }

        if (!doc.isArray()) {
            qCWarning(logSysInfo(), "parse failed, expect array");
            return -1;
        }

        QJsonArray lshwResultArray = doc.array();
        for (const QJsonValue value : lshwResultArray) {
            QJsonObject obj = value.toObject();
            if (obj.contains("id") && obj.value("id").toString() == "memory") {
                siGlobal->memoryInstalledSize = obj.value("size").toDouble(); // TODO: check "units" is "bytes" ?
                break;
            }
        }
    }

    Q_ASSERT(siGlobal->memoryInstalledSize > 0);

    return siGlobal->memoryInstalledSize;
#else
    return -1;
#endif
}

/*!
@~english
  \return the total available to use memory size
 */
qint64 DSysInfo::memoryTotalSize()
{
#ifdef Q_OS_LINUX
    siGlobal->memoryAvailableSize = get_phys_pages() * sysconf(_SC_PAGESIZE);
    return siGlobal->memoryAvailableSize;
#endif
    return -1;
}

qint64 DSysInfo::systemDiskSize()
{
#ifdef Q_OS_LINUX
    // Getting Disk Size
    QString deviceName;
    QProcess lsblk;

    lsblk.start("lsblk", {"-Jlpb", "-oNAME,KNAME,PKNAME,SIZE,MOUNTPOINT"}, QIODevice::ReadOnly);

    if (!lsblk.waitForFinished()) {
        return -1;
    }

    const QByteArray &diskStatusJson = lsblk.readAllStandardOutput();
    QJsonDocument diskStatus = QJsonDocument::fromJson(diskStatusJson);
    QJsonValue diskStatusJsonValue = diskStatus.object().value("blockdevices");
    QMap<QString, QPair<QString, qulonglong>> deviceParentAndSizeMap;

    if (!diskStatusJsonValue.isUndefined()) {
        QJsonArray diskStatusArray = diskStatusJsonValue.toArray();
        QString keyName;

        for (const QJsonValue oneValue : diskStatusArray) {
            QString name = oneValue.toObject().value("name").toString();
            QString kname = oneValue.toObject().value("kname").toString();
            QString pkname = oneValue.toObject().value("pkname").toString();
            qulonglong size = oneValue.toObject().value("size").toVariant().toULongLong();
            QString deviceNameMP = oneValue.toObject().value("mountpoint").toString();

            if ("/" ==  deviceNameMP){
                deviceName = name;
            }

            if (keyName.isNull() && deviceName == name) {
                keyName = kname;
            }

            deviceParentAndSizeMap[kname] = QPair<QString, qulonglong>(pkname, size);
        }

        while (!deviceParentAndSizeMap[keyName].first.isNull()) {
            keyName = deviceParentAndSizeMap[keyName].first;
        }

        siGlobal->diskSize = deviceParentAndSizeMap[keyName].second;
    }

    return siGlobal->diskSize;

#endif

    return -1;
}

/*! @~english DSysInfo::bootTime
 * @~english \sa DSysInfo::uptime
 * @~english \return the boot time(currentDateTime - uptime)
*/
QDateTime DSysInfo::bootTime()
{
    qint64 ut = uptime();
    return ut > 0 ? QDateTime::currentDateTime().addSecs(-ut) : QDateTime();
}

/*! @~english DSysInfo::shutdownTime
 * @~english \return the last shutdown time
*/
QDateTime DSysInfo::shutdownTime()
{
    QDateTime dt;
#if defined Q_OS_LINUX
    QProcess lastx;
    lastx.start("last", {"-x", "-F" }, QIODevice::ReadOnly);
    if (!lastx.waitForFinished()) {
        qWarning() << lastx.errorString();
        return QDateTime();
    }

    while (lastx.canReadLine()) {
        const QByteArray data = lastx.readLine(1024);
        //shutdown system down  4.19.0-amd64-des Fri Sep 30 17:53:17 2022 - Sat Oct  8 08:32:47 2022 (7+14:39)
        if (data.startsWith("shutdown")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            QString timeFmt = QString(data).split(' ', Qt::SkipEmptyParts).mid(4, 5).join(' ');
#else
            QString timeFmt = QString(data).split(' ', QString::SkipEmptyParts).mid(4, 5).join(' ');
#endif
            dt = QDateTime::fromString(timeFmt);
            break;
        }
    }
#else

#endif
    return dt;
}

/*! @~english DSysInfo::uptime
 * @~english \return the up time (/proc/uptime)
*/
qint64 DSysInfo::uptime()
{
#if defined Q_OS_LINUX
    QFile file("/proc/uptime");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << file.errorString();
        return -1;
    }

    QByteArray upTime = file.readAll();
    bool ok = false;
    qint64 sec = qCeil(upTime.split(' ').value(0).toDouble(&ok)); // [0]: uptime [1]: idletime

    return ok ? sec : -1;
#elif defined Q_OS_WIN64
     return GetTickCount64();
#elif defined Q_OS_WIN32
    return GetTickCount();
#else
    return -1;
#endif
}

/*! @~english DSysInfo::arch
 * @~english \return the architecture of processor
*/
DSysInfo::Arch DSysInfo::arch()
{
#if defined(__x86_64__)
    return X86_64;
#elif defined(__i386__)
    return X86;
#elif defined(__powerpc64__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return PPC64;
#  else
    return PPC64_LE;
#  endif
#elif defined(__powerpc__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return PPC;
#  else
    return PPC_LE;
#  endif
#elif defined(__ia64__)
    return IA64;
#elif defined(__hppa64__)
    return PARISC64;
#elif defined(__hppa__)
    return PARISC;
#elif defined(__s390x__)
    return S390X;
#elif defined(__s390__)
    return S390;
#elif defined(__sparc__) && defined (__arch64__)
    return SPARC64;
#elif defined(__sparc__)
    return SPARC;
#elif defined(__mips64) && defined(__LP64__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return MIPS64;
#  else
    return MIPS64_LE;
#  endif
#elif defined(__mips64)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return MIPS64;
#  else
    return MIPS64_LE;
#  endif
#elif defined(__mips__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return MIPS;
#  else
    return MIPS_LE;
#  endif
#elif defined(__alpha__)
    return ALPHA;
#elif defined(__aarch64__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return ARM64_BE;
#  else
    return ARM64;
#  endif
#elif defined(__arm__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return ARM_BE;
#  else
    return ARM;
#  endif
#elif defined(__sh64__)
    return SH64;
#elif defined(__sh__)
    return SH;
#elif defined(__loongarch64)
    return LOONGARCH64;
#elif defined(__m68k__)
    return M68K;
#elif defined(__tilegx__)
    return TILEGX;
#elif defined(__cris__)
    return CRIS;
#elif defined(__nios2__)
    return NIOS2;
#elif defined(__riscv)
#  if __SIZEOF_POINTER__ == 4
    return RISCV32;
#  elif __SIZEOF_POINTER__ == 8
    return RISCV64;
#  else
#    error "Unrecognized riscv architecture variant"
#  endif
#elif defined(__arc__)
#  if __BYTE_ORDER == __BIG_ENDIAN
    return ARC_BE;
#  else
    return ARC;
#  endif
#elif defined(__sw_64__)
    return SW_64;
#else
#  error "Please register your architecture here!"
#endif
}

DCORE_END_NAMESPACE
