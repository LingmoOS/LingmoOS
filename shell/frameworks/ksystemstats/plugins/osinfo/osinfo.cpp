/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "osinfo.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KCoreAddons>
#include <KOSRelease>

#include <systemstats/SensorContainer.h>
#include <systemstats/SensorObject.h>
#include <systemstats/SensorProperty.h>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#endif
#include <time.h>

// Uppercase the first letter of each word.
QString upperCaseFirst(const QString &input)
{
    auto parts = input.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (auto &part : parts) {
        part[0] = part[0].toUpper();
    }
    return parts.join(QLatin1Char(' '));
}

// Helper to simplify async dbus calls.
template <typename T>
QDBusPendingCallWatcher *dbusCall(
    const QDBusConnection &bus,
    const QString &service,
    const QString &object,
    const QString &interface,
    const QString &method,
    const QVariantList &arguments,
    std::function<void(const QDBusPendingReply<T>&)> callback
)
{
    auto message = QDBusMessage::createMethodCall(service, object, interface, method);
    message.setArguments(arguments);
    auto watcher = new QDBusPendingCallWatcher{bus.asyncCall(message)};
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [callback](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<T> reply = watcher->reply();
        callback(reply);
        watcher->deleteLater();
    });
    return watcher;
}

class OSInfoPrivate
{
public:
    OSInfoPrivate(OSInfoPlugin *qq);
    virtual ~OSInfoPrivate() = default;

    virtual void update();
    virtual void init();

    OSInfoPlugin *q;

    KSysGuard::SensorContainer *container = nullptr;

    KSysGuard::SensorObject *kernelObject = nullptr;
    KSysGuard::SensorProperty *kernelNameProperty = nullptr;
    KSysGuard::SensorProperty *kernelVersionProperty = nullptr;
    KSysGuard::SensorProperty *kernelPrettyNameProperty = nullptr;

    KSysGuard::SensorObject *systemObject = nullptr;
    KSysGuard::SensorProperty *hostnameProperty = nullptr;
    KSysGuard::SensorProperty *osNameProperty = nullptr;
    KSysGuard::SensorProperty *osVersionProperty = nullptr;
    KSysGuard::SensorProperty *osPrettyNameProperty = nullptr;
    KSysGuard::SensorProperty *osLogoProperty = nullptr;
    KSysGuard::SensorProperty *osUrlProperty = nullptr;
    KSysGuard::SensorProperty *uptimeProperty = nullptr;

    KSysGuard::SensorObject *plasmaObject = nullptr;
    KSysGuard::SensorProperty *qtVersionProperty = nullptr;
    KSysGuard::SensorProperty *kfVersionProperty = nullptr;
    KSysGuard::SensorProperty *plasmaVersionProperty = nullptr;
    KSysGuard::SensorProperty *windowSystemProperty = nullptr;
};

class LinuxPrivate : public OSInfoPrivate
{
public:
    LinuxPrivate(OSInfoPlugin *qq) : OSInfoPrivate(qq) { }

    void init() override;
};

OSInfoPrivate::OSInfoPrivate(OSInfoPlugin *qq)
    : q(qq)
{
    container = new KSysGuard::SensorContainer(QStringLiteral("os"), i18nc("@title", "Operating System"), q);

    kernelObject = new KSysGuard::SensorObject(QStringLiteral("kernel"), i18nc("@title", "Kernel"), container);
    kernelNameProperty = new KSysGuard::SensorProperty(QStringLiteral("name"), i18nc("@title", "Kernel Name"), kernelObject);
    kernelVersionProperty = new KSysGuard::SensorProperty(QStringLiteral("version"), i18nc("@title", "Kernel Version"), kernelObject);
    kernelPrettyNameProperty = new KSysGuard::SensorProperty(QStringLiteral("prettyName"), i18nc("@title", "Kernel Name and Version"), kernelObject);
    kernelPrettyNameProperty->setShortName(i18nc("@title Kernel Name and Version", "Kernel"));

    systemObject = new KSysGuard::SensorObject(QStringLiteral("system"), i18nc("@title", "System"), container);
    hostnameProperty = new KSysGuard::SensorProperty(QStringLiteral("hostname"), i18nc("@title", "Hostname"), systemObject);
    osNameProperty = new KSysGuard::SensorProperty(QStringLiteral("name"), i18nc("@title", "Operating System Name"), systemObject);
    osVersionProperty = new KSysGuard::SensorProperty(QStringLiteral("version"), i18nc("@title", "Operating System Version"), systemObject);
    osPrettyNameProperty = new KSysGuard::SensorProperty(QStringLiteral("prettyName"), i18nc("@title", "Operating System Name and Version"), systemObject);
    osPrettyNameProperty->setShortName(i18nc("@title Operating System Name and Version", "OS"));
    osLogoProperty = new KSysGuard::SensorProperty(QStringLiteral("logo"), i18nc("@title", "Operating System Logo"), systemObject);
    osUrlProperty = new KSysGuard::SensorProperty(QStringLiteral("url"), i18nc("@title", "Operating System URL"), systemObject);
    uptimeProperty = new KSysGuard::SensorProperty(QStringLiteral("uptime"), i18nc("@title", "Uptime"), systemObject);
    uptimeProperty->setUnit(KSysGuard::UnitTime);

    plasmaObject = new KSysGuard::SensorObject(QStringLiteral("plasma"), i18nc("@title", "KDE Plasma"), container);
    qtVersionProperty = new KSysGuard::SensorProperty(QStringLiteral("qtVersion"), i18nc("@title", "Qt Version"), plasmaObject);
    kfVersionProperty = new KSysGuard::SensorProperty(QStringLiteral("kfVersion"), i18nc("@title", "KDE Frameworks Version"), plasmaObject);
    plasmaVersionProperty = new KSysGuard::SensorProperty(QStringLiteral("plasmaVersion"), i18nc("@title", "KDE Plasma Version"), plasmaObject);
    windowSystemProperty = new KSysGuard::SensorProperty(QStringLiteral("windowsystem"), i18nc("@title", "Window System"), plasmaObject);
}

OSInfoPlugin::~OSInfoPlugin() = default;

void OSInfoPrivate::init()
{
    auto kernelName = upperCaseFirst(QSysInfo::kernelType());
    kernelNameProperty->setValue(kernelName);
    kernelVersionProperty->setValue(QSysInfo::kernelVersion());
    kernelPrettyNameProperty->setValue(QString{kernelName % QLatin1Char(' ') % QSysInfo::kernelVersion()});
    hostnameProperty->setValue(QSysInfo::machineHostName());

    KOSRelease os;
    osNameProperty->setValue(os.name());
    osVersionProperty->setValue(os.version());
    osPrettyNameProperty->setValue(os.prettyName());
    osLogoProperty->setValue(os.logo());
    osUrlProperty->setValue(os.homeUrl());

    qtVersionProperty->setValue(QString::fromLatin1(qVersion()));
    kfVersionProperty->setValue(KCoreAddons::versionString());
    windowSystemProperty->setValue(qgetenv("XDG_SESSION_TYPE").compare("x11", Qt::CaseInsensitive) == 0 ? QStringLiteral("X11") : QStringLiteral("Wayland"));

    dbusCall<QVariant>(
        QDBusConnection::sessionBus(),
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/MainApplication"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("Get"),
        { QStringLiteral("org.qtproject.Qt.QCoreApplication"), QStringLiteral("applicationVersion") },
        [this](const QDBusPendingReply<QVariant> &reply) {
            if (reply.isError()) {
                qWarning() << "Could not determine Plasma version, got: " << reply.error().message();
                plasmaVersionProperty->setValue(i18nc("@info", "Unknown"));
            } else {
                plasmaVersionProperty->setValue(reply.value());
            }
        }
    );
}

void OSInfoPrivate::update()
{
#if defined Q_OS_LINUX
    struct sysinfo info;
    sysinfo(&info);
    // can't send a long over the bus
    uptimeProperty->setValue(QVariant::fromValue<qlonglong>(info.uptime));
#elif defined Q_OS_FREEBSD
    timespec time;
    clock_gettime(CLOCK_UPTIME, &time);
    uptimeProperty->setValue(QVariant::fromValue<qlonglong>(time.tv_sec));
#endif
}

void LinuxPrivate::init()
{
    OSInfoPrivate::init();

    // Override some properties with values from hostnamed, if available.
    dbusCall<QVariantMap>(
        QDBusConnection::systemBus(),
        QStringLiteral("org.freedesktop.hostname1"),
        QStringLiteral("/org/freedesktop/hostname1"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("GetAll"),
        { QStringLiteral("org.freedesktop.hostname1") },
        [this](const QDBusPendingReply<QVariantMap> &reply) {
            if (reply.isError()) {
                qWarning() << "Could not contact hostnamed, got: " << reply.error().message();
            } else {
                auto properties = reply.value();
                auto kernelName = properties.value(QStringLiteral("KernelName"), kernelNameProperty->value()).toString();
                kernelNameProperty->setValue(kernelName);
                auto kernelVersion = properties.value(QStringLiteral("KernelRelease"), kernelVersionProperty->value()).toString();
                kernelVersionProperty->setValue(kernelVersion);
                kernelPrettyNameProperty->setValue(QString{kernelName % QLatin1Char(' ') % kernelVersion});

                auto prettyHostName = properties.value(QStringLiteral("PrettyHostname"), QString{}).toString();
                if (!prettyHostName.isEmpty()) {
                    hostnameProperty->setValue(prettyHostName);
                } else {
                    hostnameProperty->setValue(properties.value(QStringLiteral("Hostname"), hostnameProperty->value()));
                }
            }
        }
    );
}

OSInfoPlugin::OSInfoPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
#ifdef Q_OS_LINUX
    d = std::make_unique<LinuxPrivate>(this);
#else
    d = std::make_unique<OSInfoPrivate>(this);
#endif
    d->init();
}

void OSInfoPlugin::update()
{
    d->update();
}

K_PLUGIN_CLASS_WITH_JSON(OSInfoPlugin, "metadata.json")

#include "osinfo.moc"

#include "moc_osinfo.cpp"
