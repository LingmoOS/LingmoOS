/*
 * SPDX-FileCopyrightText: 2024 Rafael Sadowski <rafael@rsadowski.de>
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "chargethresholdhelper.h"

#include <powerdevil_debug.h>

#include <KAuth/HelperSupport>

#include <algorithm>

#include <sys/sysctl.h>
#include <sys/types.h>

#include <QDir>
#include <QFile>

ChargeThresholdHelper::ChargeThresholdHelper(QObject *parent)
    : QObject(parent)
{
}

static bool isThresholdSupported()
{
    int mode;
    size_t len = sizeof(mode);
    // If HW_BATTERY_CHARGEMODE is present there is hardware support.
    int mib[] = {CTL_HW, HW_BATTERY, HW_BATTERY_CHARGEMODE};
    return (sysctl(mib, 3, &mode, &len, NULL, 0) != -1);
}

static int getBatteryCharge(const int type)
{
    int percentage = -1;
    size_t len = sizeof(percentage);
    int mib[] = {CTL_HW, HW_BATTERY, type};
    sysctl(mib, 3, &percentage, &len, NULL, 0);
    return percentage;
}

static bool setBatteryCharge(const int type, int percentage)
{
    size_t len = sizeof(percentage);
    int mib[] = {CTL_HW, HW_BATTERY, type};
    return (sysctl(mib, 3, NULL, NULL, &percentage, len) != -1);
}

ActionReply ChargeThresholdHelper::getthreshold(const QVariantMap &args)
{
    Q_UNUSED(args);
    if (!isThresholdSupported()) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Control battery charging is not supported by the kernel for this hardware"));
        return reply;
    }

    const int startThreshold = getBatteryCharge(HW_BATTERY_CHARGESTART);
    const int stopThreshold = getBatteryCharge(HW_BATTERY_CHARGESTOP);

    ActionReply reply;
    reply.setData({
        {QStringLiteral("chargeStartThreshold"), startThreshold},
        {QStringLiteral("chargeStopThreshold"), stopThreshold},
    });
    return reply;
}

ActionReply ChargeThresholdHelper::setthreshold(const QVariantMap &args)
{
    bool hasStartThreshold;
    const int startThreshold = args.value(QStringLiteral("chargeStartThreshold"), -1).toInt(&hasStartThreshold);
    hasStartThreshold &= startThreshold != -1;

    bool hasStopThreshold;
    const int stopThreshold = args.value(QStringLiteral("chargeStopThreshold"), -1).toInt(&hasStopThreshold);
    hasStopThreshold &= stopThreshold != -1;

    if ((hasStartThreshold && (startThreshold < 0 || startThreshold > 100)) || (hasStopThreshold && (stopThreshold < 0 || stopThreshold > 100))
        || (hasStartThreshold && hasStopThreshold && startThreshold > stopThreshold) || (!hasStartThreshold && !hasStopThreshold)) {
        auto reply = ActionReply::HelperErrorReply(); // is there an "invalid arguments" error?
        reply.setErrorDescription(QStringLiteral("Invalid thresholds provided"));
        return reply;
    }
    if (hasStartThreshold && !setBatteryCharge(HW_BATTERY_CHARGESTART, startThreshold)) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Failed to write start charge threshold"));
        return reply;
    }

    if (hasStopThreshold && !setBatteryCharge(HW_BATTERY_CHARGESTOP, stopThreshold)) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Failed to write stop charge threshold"));
        return reply;
    }

    return ActionReply();
}

ActionReply ChargeThresholdHelper::getconservationmode(const QVariantMap &args)
{
    Q_UNUSED(args);
    auto reply = ActionReply::HelperErrorReply();
    reply.setErrorDescription(QStringLiteral("Battery conservation mode is not supported on OpenBSD"));
    return reply;
}

ActionReply ChargeThresholdHelper::setconservationmode(const QVariantMap &args)
{
    Q_UNUSED(args);
    auto reply = ActionReply::HelperErrorReply();
    reply.setErrorDescription(QStringLiteral("Battery conservation mode is not supported on OpenBSD"));
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.powerdevil.chargethresholdhelper", ChargeThresholdHelper)

#include "moc_chargethresholdhelper.cpp"
