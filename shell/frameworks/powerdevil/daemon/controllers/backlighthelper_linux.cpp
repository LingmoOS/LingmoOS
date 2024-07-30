/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2010-2011 Lukas Tinkl <ltinkl@redhat.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#include "backlighthelper_linux.h"

#include <powerdevil_debug.h>

#include <QDebug>
#include <QDir>

#include <KLocalizedString>

#include <algorithm>
#include <climits>
#include <sys/utsname.h>

using namespace Qt::StringLiterals;

inline constexpr QLatin1StringView BACKLIGHT_SYSFS_PATH("/sys/class/backlight/");
inline constexpr QLatin1StringView LED_SYSFS_PATH("/sys/class/leds/");

BacklightHelper::BacklightHelper(QObject *parent)
    : QObject(parent)
{
    init();
}

void BacklightHelper::init()
{
    initUsingBacklightType();

    if (m_devices.isEmpty()) {
        qCWarning(POWERDEVIL) << "no kernel backlight interface found";
        return;
    }

    m_anim.setEasingCurve(QEasingCurve::InOutQuad);
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        // When animating to zero, it emits a value change to 0 before starting the animation...
        if (m_anim.state() == QAbstractAnimation::Running) {
            writeBrightness(value.toInt());
        }
    });

    m_isSupported = true;
}

int BacklightHelper::readFromDevice(const QString &device, const QString &property) const
{
    int value = -1;

    QFile file(QString(device + u'/' + property));
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(POWERDEVIL) << "reading from device " << device << "/" << property << " failed with error code " << file.error() << file.errorString();
        return value;
    }

    QTextStream stream(&file);
    stream >> value;
    file.close();

    return value;
}

bool BacklightHelper::writeToDevice(const QString &device, int brightness) const
{
    QFile file(device + QLatin1String("/brightness"));
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(POWERDEVIL) << "writing to device " << device << "/brightness failed with error code " << file.error() << file.errorString();
        return false;
    }

    const int bytesWritten = file.write(QByteArray::number(brightness));
    if (bytesWritten == -1) {
        qCWarning(POWERDEVIL) << "writing to device " << device << "/brightness failed with error code " << file.error() << file.errorString();
        return false;
    }

    return true;
}

QStringList BacklightHelper::getBacklightTypeDevices() const
{
    QDir ledsDir(LED_SYSFS_PATH);
    ledsDir.setFilter(QDir::Dirs | QDir::NoDot | QDir::NoDotDot | QDir::NoDotAndDotDot | QDir::Readable);
    ledsDir.setNameFilters({QStringLiteral("*lcd*"), QStringLiteral("*wled*")});

    QStringList ledInterfaces = ledsDir.entryList();

    if (!ledInterfaces.isEmpty()) {
        QStringList output;
        for (const QString &interface : ledInterfaces) {
            output.append(LED_SYSFS_PATH + interface);
        }
        return output;
    }

    QDir backlightDir(BACKLIGHT_SYSFS_PATH);
    backlightDir.setFilter(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot | QDir::NoDotAndDotDot | QDir::Readable);
    backlightDir.setSorting(QDir::Name | QDir::Reversed); // Reverse is needed to priorize acpi_video1 over 0

    const QStringList interfaces = backlightDir.entryList();

    QFile file;
    QByteArray buffer;
    QStringList firmware, platform, rawEnabled, rawAll;

    for (const QString &interface : interfaces) {
        file.setFileName(BACKLIGHT_SYSFS_PATH + interface + u"/type");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        buffer = file.readLine().trimmed();
        if (buffer == "firmware") {
            firmware.append(BACKLIGHT_SYSFS_PATH + interface);
        } else if (buffer == "platform") {
            platform.append(BACKLIGHT_SYSFS_PATH + interface);
        } else if (buffer == "raw") {
            QFile enabled(BACKLIGHT_SYSFS_PATH + interface + u"/device/enabled");
            rawAll.append(BACKLIGHT_SYSFS_PATH + interface);
            if (enabled.open(QIODevice::ReadOnly | QIODevice::Text) && enabled.readLine().trimmed() == "enabled") {
                // this backlight device is connected to a display, so append
                // it to rawEnabled list
                rawEnabled.append(BACKLIGHT_SYSFS_PATH + interface);
            }
        } else {
            qCWarning(POWERDEVIL) << "Interface type not handled" << buffer;
        }

        file.close();
    }

    if (!firmware.isEmpty())
        return firmware;

    if (!platform.isEmpty())
        return platform;

    if (!rawEnabled.isEmpty())
        return rawEnabled;

    if (!rawAll.isEmpty())
        return rawAll;

    return {};
}

void BacklightHelper::initUsingBacklightType()
{
    m_devices.clear();
    QStringList devices = getBacklightTypeDevices();

    for (const QString &interface : devices) {
        int max_brightness = readFromDevice(interface, u"max_brightness"_s);
        m_devices.append(qMakePair(interface, max_brightness));
    }

    return;
}

ActionReply BacklightHelper::brightness(const QVariantMap &args)
{
    Q_UNUSED(args);
    const int brightness = readBrightness();

    if (brightness == -1) {
        return ActionReply::HelperErrorReply();
    }

    ActionReply reply;
    reply.addData(QStringLiteral("brightness"), brightness);
    return reply;
}

int BacklightHelper::readBrightness() const
{
    if (!m_isSupported) {
        return -1;
    }

    return readFromDevice(m_devices.constFirst().first, QLatin1String("brightness"));
}

ActionReply BacklightHelper::setbrightness(const QVariantMap &args)
{
    if (!m_isSupported) {
        return ActionReply::HelperErrorReply();
    }

    const int brightness = args.value(QStringLiteral("brightness")).toInt();
    const int animationDuration = args.value(QStringLiteral("animationDuration")).toInt();

    m_anim.stop();

    if (animationDuration <= 0) {
        writeBrightness(brightness);
        return ActionReply::SuccessReply();
    }

    m_anim.setDuration(animationDuration);
    m_anim.setStartValue(readBrightness());
    m_anim.setEndValue(brightness);
    m_anim.start();

    return ActionReply::SuccessReply();
}

bool BacklightHelper::writeBrightness(int brightness) const
{
    if (!m_devices.isEmpty()) {
        const int first_maxbrightness = std::max(1, m_devices.constFirst().second);
        for (const auto &device : m_devices) {
            // Some monitor brightness values are ridiculously high, and can easily overflow during computation
            const qint64 new_brightness_64 = static_cast<qint64>(brightness) * static_cast<qint64>(device.second) / static_cast<qint64>(first_maxbrightness);
            // cautiously truncate it back
            const int new_brightness = static_cast<int>(std::min(static_cast<qint64>(std::numeric_limits<int>::max()), new_brightness_64));
            writeToDevice(device.first, new_brightness);
        }
    }

    return true;
}

ActionReply BacklightHelper::syspath(const QVariantMap &args)
{
    Q_UNUSED(args);

    ActionReply reply;

    if (!m_isSupported || m_devices.isEmpty()) {
        reply = ActionReply::HelperErrorReply();
        return reply;
    }

    reply.addData(QStringLiteral("syspath"), m_devices.constFirst().first);

    return reply;
}

ActionReply BacklightHelper::brightnessmax(const QVariantMap &args)
{
    Q_UNUSED(args);

    ActionReply reply;

    if (!m_isSupported) {
        reply = ActionReply::HelperErrorReply();
        return -1;
    }

    // maximum brightness
    int max_brightness = readFromDevice(m_devices.constFirst().first, QLatin1String("max_brightness"));

    if (max_brightness <= 0) {
        reply = ActionReply::HelperErrorReply();
        return reply;
    }

    reply.addData(QStringLiteral("brightnessmax"), max_brightness);
    // qCDebug(POWERDEVIL) << "data contains:" << reply.data()["brightnessmax"];

    return reply;
}

KAUTH_HELPER_MAIN("org.kde.powerdevil.backlighthelper", BacklightHelper)

#include "moc_backlighthelper_linux.cpp"
