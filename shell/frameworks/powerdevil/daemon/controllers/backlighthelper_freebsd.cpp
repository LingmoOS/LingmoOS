/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2012 Alberto Villa <avilla@FreeBSD.org>
 *    SPDX-FileCopyrightText: 2023 Serenity Cyber Security LLC <license@futurecrew.ru>
 *                       Author: Gleb Popov <arrowd@FreeBSD.org>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#include "backlighthelper_freebsd.h"

#include <powerdevil_debug.h>

#include <QDebug>
#include <QDir>
#include <QProcess>

#include <KLocalizedString>

#include <algorithm>
#include <climits>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/utsname.h>

#define BACKLIGHT_PATH "/dev/backlight/"
// man backlight states that the brightness value is always between 0 and 100
#define BACKLIGHT_MAX_BRIGHTNESS 100
#define HAS_SYSCTL(n) (sysctlbyname(n, nullptr, nullptr, nullptr, 0) == 0)

static QByteArray runFreeBSDBacklight(const QStringList &args)
{
    QProcess backlight;
    backlight.setProgram(QStringLiteral("backlight"));
    backlight.setArguments(args);
    backlight.start(QIODevice::ReadOnly);
    backlight.waitForFinished();
    return backlight.readAllStandardOutput();
}

BacklightHelper::BacklightHelper(QObject *parent)
    : QObject(parent)
{
    initUsingFreeBSDBacklight();

    if (m_devices.isEmpty()) {
        initUsingSysctl();

        if (m_sysctlDevice.isEmpty() || m_sysctlBrightnessLevels.isEmpty()) {
            qCWarning(POWERDEVIL) << "no kernel backlight interface found";
            return;
        }
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

int BacklightHelper::readFromDevice(const QString &device) const
{
    bool ok;
    int value = runFreeBSDBacklight({QStringLiteral("-qf"), device}).toInt(&ok);
    return ok ? value : -1;
}

bool BacklightHelper::writeToDevice(const QString &device, int brightness) const
{
    return 0 == QProcess::execute(QStringLiteral("backlight"), {QStringLiteral("-f"), device, QString::number(brightness)});
}

void BacklightHelper::initUsingSysctl()
{
    /*
     * lcd0 is, in theory, the correct device, but some vendors have custom ACPI implementations
     * which cannot be interpreted. In that case, devices should be reported as "out", but
     * FreeBSD doesn't care (yet), so they can appear as any other type. Let's search for the first
     * device with brightness management, then.
     */
    /*
     * The acpi_video interface is not standartized and will not work for some vendors due to
     * the sysctl being named hw.acpi.video_<vendor>.*
     */
    QStringList types;
    types << QStringLiteral("lcd") << QStringLiteral("out") << QStringLiteral("crt") << QStringLiteral("tv") << QStringLiteral("ext");
    for (const QString &type : types) {
        for (int i = 0; m_sysctlDevice.isEmpty(); i++) {
            QString device = QStringLiteral("%1%2").arg(type, QString::number(i));
            // We don't care about the value, we only want the sysctl to be there.
            if (!HAS_SYSCTL(qPrintable(QStringLiteral("hw.acpi.video.%1.active").arg(device)))) {
                break;
            }
            if (HAS_SYSCTL(qPrintable(QStringLiteral("hw.acpi.video.%1.brightness").arg(device)))
                && HAS_SYSCTL(qPrintable(QStringLiteral("hw.acpi.video.%1.levels").arg(device)))) {
                m_sysctlDevice = device;
                break;
            }
        }
    }

    if (m_sysctlDevice.isEmpty()) {
        return;
    }

    size_t len;
    if (sysctlbyname(qPrintable(QStringLiteral("hw.acpi.video.%1.levels").arg(m_sysctlDevice)), nullptr, &len, nullptr, 0) != 0 || len == 0) {
        return;
    }
    int *levels = (int *)malloc(len);
    if (!levels) {
        return;
    }
    if (sysctlbyname(qPrintable(QStringLiteral("hw.acpi.video.%1.levels").arg(m_sysctlDevice)), levels, &len, nullptr, 0) != 0) {
        free(levels);
        return;
    }
    // acpi_video(4) supports only some predefined brightness levels.
    int nlevels = len / sizeof(int);
    for (int i = 0; i < nlevels; i++) {
        m_sysctlBrightnessLevels << levels[i];
    }
    free(levels);
    // Sorting helps when finding max value and when scanning for the nearest level in setbrightness().
    std::sort(m_sysctlBrightnessLevels.begin(), m_sysctlBrightnessLevels.end());
}

void BacklightHelper::initUsingFreeBSDBacklight()
{
    QDir ledsDir(QStringLiteral(BACKLIGHT_PATH));
    ledsDir.setFilter(QDir::NoDotAndDotDot | QDir::System);
    ledsDir.setNameFilters({QStringLiteral("backlight*")});

    QStringList devices = ledsDir.entryList();

    for (auto &devName : devices) {
        auto devPath = QStringLiteral(BACKLIGHT_PATH) + devName;
        auto output = runFreeBSDBacklight({QStringLiteral("-if"), devPath});
        // % backlight -if /dev/backlight/backlight0
        // Backlight name: intel_backlight
        // Backlight hardware type: Panel
        if (output.contains(": Panel")) {
            m_devices.append(qMakePair(devPath, 100));
        }
    }
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

    int brightness = -1;

    if (!m_sysctlDevice.isEmpty()) {
        size_t len = sizeof(int);
        if (sysctlbyname(qPrintable(QStringLiteral("hw.acpi.video.%1.brightness").arg(m_sysctlDevice)), &brightness, &len, nullptr, 0) != 0) {
            return -1;
        }
    } else {
        brightness = readFromDevice(m_devices.constFirst().first);
    }

    return brightness;
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
    if (!m_sysctlDevice.isEmpty()) {
        int actual_level = -1;
        int d1 = 101;
        // Search for the nearest level.
        for (int level : m_sysctlBrightnessLevels) {
            int d2 = qAbs(level - brightness);
            /*
             * The list is sorted, so we break when it starts diverging. There may be repeated values,
             * so we keep going on equal gap (e.g., value = 7.5, levels = 0 0 10 ...: we don't break at
             * the second '0' so we can get to the '10'). This also means that the value will always
             * round off to the bigger level when in the middle (e.g., value = 5, levels = 0 10 ...:
             * value rounds off to 10).
             */
            if (d2 > d1) {
                break;
            }
            actual_level = level;
            d1 = d2;
        }
        size_t len = sizeof(int);
        return sysctlbyname(qPrintable(QStringLiteral("hw.acpi.video.%1.brightness").arg(m_sysctlDevice)), nullptr, nullptr, &actual_level, len) == 0;
    }

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
    return ActionReply::HelperErrorReply();
}

ActionReply BacklightHelper::brightnessmax(const QVariantMap &args)
{
    Q_UNUSED(args);

    if (!m_isSupported) {
        return ActionReply::HelperErrorReply();
    }

    int max_brightness = !m_sysctlBrightnessLevels.isEmpty() ? m_sysctlBrightnessLevels.last() : BACKLIGHT_MAX_BRIGHTNESS;

    if (max_brightness <= 0) {
        return ActionReply::HelperErrorReply();
    }

    ActionReply reply;
    reply.addData(QStringLiteral("brightnessmax"), max_brightness);
    // qCDebug(POWERDEVIL) << "data contains:" << reply.data()["brightnessmax"];

    return reply;
}

KAUTH_HELPER_MAIN("org.kde.powerdevil.backlighthelper", BacklightHelper)
