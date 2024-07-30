/*******************************************************************
 * systeminformation.cpp
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include <config-drkonqi.h>

#include "systeminformation.h"

#if HAVE_UNAME
#include <errno.h>
#include <sys/utsname.h>
#endif

#include "drkonqi_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <KCoreAddons>
#include <KOSRelease>
#include <KProcess>
#include <KSharedConfig>
#include <KWindowSystem>
#include <QStandardPaths>
#include <kcoreaddons_version.h>

static const QString OS_UNSPECIFIED = QStringLiteral("unspecified");
static const QString PLATFORM_UNSPECIFIED = QStringLiteral("unspecified");

// This function maps the operating system to an OS value that is
// accepted by bugs.kde.org. If the values change on the server
// side, they need to be updated here as well.
static QString fetchBasicOperatingSystem()
{
    // krazy:excludeall=cpp
    // Get the base OS string (bugzillaOS)
#if defined(Q_OS_LINUX)
    return QStringLiteral("Linux");
#elif defined(Q_OS_FREEBSD)
    return QStringLiteral("FreeBSD");
#elif defined(Q_OS_NETBSD)
    return QStringLiteral("NetBSD");
#elif defined(Q_OS_OPENBSD)
    return QStringLiteral("OpenBSD");
#elif defined(Q_OS_AIX)
    return QStringLiteral("AIX");
#elif defined(Q_OS_HPUX)
    return QStringLiteral("HP-UX");
#elif defined(Q_OS_IRIX)
    return QStringLiteral("IRIX");
#elif defined(Q_OS_OSF)
    return QStringLiteral("Tru64");
#elif defined(Q_OS_SOLARIS)
    return QStringLiteral("Solaris");
#elif defined(Q_OS_CYGWIN)
    return QStringLiteral("Cygwin");
#elif defined(Q_OS_DARWIN)
    return QStringLiteral("OS X");
#elif defined(Q_OS_WIN32)
    return QStringLiteral("MS Windows");
#else
    return OS_UNSPECIFIED;
#endif
}

SystemInformation::Config::Config()
    : basicOperatingSystem(fetchBasicOperatingSystem())
    , lsbReleasePath(QStandardPaths::findExecutable(QStringLiteral("lsb_release")))
    , osReleasePath(/* Use KOSRelease default */)
{
}

SystemInformation::SystemInformation(Config infoConfig, QObject *parent)
    : QObject(parent)
    , m_bugzillaOperatingSystem(infoConfig.basicOperatingSystem)
    , m_bugzillaPlatform(PLATFORM_UNSPECIFIED)
    , m_complete(false)
    , m_infoConfig(infoConfig)
{
    // NOTE: order matters. These require m_bugzillaOperatingSystem to be set!
    m_operatingSystem = fetchOSDetailInformation();
    tryToSetBugzillaPlatform();

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("SystemInformation"));
    m_compiledSources = config.readEntry("CompiledSources", false);
}

SystemInformation::~SystemInformation()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("SystemInformation"));
    config.writeEntry("CompiledSources", m_compiledSources);
    config.sync();
}

void SystemInformation::tryToSetBugzillaPlatform()
{
    QString platform = PLATFORM_UNSPECIFIED;
    // first, try to guess bugzilla platfrom from the internal OS information
    // this should work for BSDs, solaris and windows.
    platform = guessBugzillaPlatform(m_bugzillaOperatingSystem);

    // if the internal information is not enough, refer to external information
    if (platform == PLATFORM_UNSPECIFIED) {
        tryToSetBugzillaPlatformFromExternalInfo();
    } else {
        setBugzillaPlatform(platform);
    }
}

void SystemInformation::tryToSetBugzillaPlatformFromExternalInfo()
{
    // Run lsb_release async
    QString lsb_release = m_infoConfig.lsbReleasePath;
    if (!lsb_release.isEmpty()) {
        qCDebug(DRKONQI_LOG) << "found lsb_release";
        auto *process = new KProcess();
        process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        process->setEnv(QStringLiteral("LC_ALL"), QStringLiteral("C.UTF-8"));
        *process << lsb_release << QStringLiteral("-sd");
        connect(process, static_cast<void (KProcess::*)(int, QProcess::ExitStatus)>(&KProcess::finished), this, &SystemInformation::lsbReleaseFinished);
        process->start();
    } else {
        // when lsb_release is unavailable, turn to /etc/os-release
        const QString &osReleaseInfo = fetchOSReleaseInformation();
        const QString &platform = guessBugzillaPlatform(osReleaseInfo);
        setBugzillaPlatform(platform);
        m_complete = true;
    }
}

void SystemInformation::lsbReleaseFinished()
{
    auto *process = qobject_cast<KProcess *>(sender());
    Q_ASSERT(process);
    m_distributionPrettyName = QString::fromLocal8Bit(process->readAllStandardOutput().trimmed());
    process->deleteLater();

    // Guess distro string
    QString platform = guessBugzillaPlatform(m_distributionPrettyName);

    // if lsb_release doesn't work well, turn to the /etc/os-release file
    if (platform == PLATFORM_UNSPECIFIED) {
        const QString &osReleaseInfo = fetchOSReleaseInformation();
        platform = guessBugzillaPlatform(osReleaseInfo);
    }

    setBugzillaPlatform(platform);
    m_complete = true;
}

// this function maps the distribution information to an "Hardware Platform"    .
// value that is accepted by bugs.kde.org.  If the values change on the server    .
// side, they need to be updated here as well                                   .
QString SystemInformation::guessBugzillaPlatform(const QString &distroInfo) const
{
    static QHash<QString, QString> platforms{{QStringLiteral("suse"), QStringLiteral("openSUSE RPMs")},
                                             {QStringLiteral("mint"), QStringLiteral("Mint (Ubuntu Based)")},
                                             {QStringLiteral("lmde"), QStringLiteral("Mint (Debian Based)")},
                                             {QStringLiteral("ubuntu"), QStringLiteral("Ubuntu Packages")},
                                             {QStringLiteral("fedora"), QStringLiteral("Fedora RPMs")},
                                             {QStringLiteral("redhat"), QStringLiteral("RedHat RPMs")},
                                             {QStringLiteral("gentoo"), QStringLiteral("Gentoo Packages")},
                                             {QStringLiteral("mandriva"), QStringLiteral("Mandriva RPMs")},
                                             {QStringLiteral("mageia"), QStringLiteral("Mageia RPMs")},
                                             {QStringLiteral("slack"), QStringLiteral("Slackware Packages")},
                                             {QStringLiteral("pclinuxos"), QStringLiteral("PCLinuxOS")},
                                             {QStringLiteral("pardus"), QStringLiteral("Pardus Packages")},
                                             {QStringLiteral("freebsd"), QStringLiteral("FreeBSD Ports")},
                                             {QStringLiteral("netbsd"), QStringLiteral("NetBSD pkgsrc")},
                                             {QStringLiteral("openbsd"), QStringLiteral("OpenBSD Packages")},
                                             {QStringLiteral("solaris"), QStringLiteral("Solaris Packages")},
                                             {QStringLiteral("chakra"), QStringLiteral("Chakra")},
                                             {QStringLiteral("ms windows"), QStringLiteral("MS Windows")},
                                             {QStringLiteral("arch"), QStringLiteral("Archlinux Packages")},
                                             {QStringLiteral("kde neon"), QStringLiteral("Neon Packages")}};
    for (auto it = platforms.constBegin(); it != platforms.constEnd(); ++it) {
        if (distroInfo.contains(it.key(), Qt::CaseInsensitive)) {
            return it.value();
        }
    }

    // Debian has multiple platforms.
    if (distroInfo.contains(QLatin1String("debian"), Qt::CaseInsensitive)) {
        if (distroInfo.contains(QLatin1String("unstable"), Qt::CaseInsensitive)) {
            return QStringLiteral("Debian unstable");
        } else if (distroInfo.contains(QLatin1String("testing"), Qt::CaseInsensitive)) {
            return QStringLiteral("Debian testing");
        } else {
            return QStringLiteral("Debian stable");
        }
    }

    return PLATFORM_UNSPECIFIED;
}

QString SystemInformation::fetchOSDetailInformation() const
{
    // Get complete OS string (and fallback to base string)
    QString operatingSystem = m_bugzillaOperatingSystem;

#if HAVE_UNAME
    struct utsname buf;

    auto unameFunc = &uname;
    if (m_infoConfig.unameFunc) {
        unameFunc = (decltype(&uname))m_infoConfig.unameFunc;
    }

    if ((*unameFunc)(&buf) == -1) {
        qCDebug(DRKONQI_LOG) << "call to uname failed" << errno;
    } else {
        operatingSystem = QString::fromLocal8Bit(buf.sysname) + QLatin1Char(' ') + QString::fromLocal8Bit(buf.release) + QLatin1Char(' ')
            + QString::fromLocal8Bit(buf.machine);
    }
#endif

    return operatingSystem;
}

QString SystemInformation::fetchOSReleaseInformation()
{
    KOSRelease os(m_infoConfig.osReleasePath);
    return m_distributionPrettyName = os.prettyName();
}

QString SystemInformation::operatingSystem() const
{
    return m_operatingSystem;
}

QString SystemInformation::bugzillaOperatingSystem() const
{
    return m_bugzillaOperatingSystem;
}

QString SystemInformation::bugzillaPlatform() const
{
    return m_bugzillaPlatform;
}

void SystemInformation::setBugzillaPlatform(const QString &platform)
{
    m_bugzillaPlatform = platform;
    Q_EMIT bugzillaPlatformChanged();
}

QString SystemInformation::distributionPrettyName() const
{
    return m_distributionPrettyName;
}

bool SystemInformation::compiledSources() const
{
    return m_compiledSources;
}

void SystemInformation::setCompiledSources(bool compiled)
{
    m_compiledSources = compiled;
    Q_EMIT compiledSourcesChanged();
}

QString SystemInformation::qtVersion() const
{
    return QString::fromLatin1(qVersion());
}

QString SystemInformation::frameworksVersion() const
{
    return KCoreAddons::versionString();
}

bool SystemInformation::complete() const
{
    return m_complete;
}

QString SystemInformation::windowSystem() const
{
    switch (KWindowSystem::platform()) {
    case KWindowSystem::Platform::Unknown:
        return QStringLiteral("Unknown");
    case KWindowSystem::Platform::X11:
        return QStringLiteral("X11");
    case KWindowSystem::Platform::Wayland:
        return QStringLiteral("Wayland");
    }
    return QStringLiteral("Unknown");
}

#include "moc_systeminformation.cpp"
