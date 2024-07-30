/*
 * SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>
 * SPDX-FileCopyrightText: 2023 Fabian Arndt <fabian.arndt@root-core.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "chargethresholdhelper.h"

#include <powerdevil_debug.h>

#include <KAuth/HelperSupport>

#include <algorithm>

#include <QDir>
#include <QFile>

using namespace Qt::StringLiterals;

static const QString s_powerSupplySysFsPath = QStringLiteral("/sys/class/power_supply");
static const QString s_conservationModeSysFsPath = QStringLiteral("/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode");

static const QString s_chargeStartThreshold = QStringLiteral("charge_control_start_threshold");
static const QString s_chargeEndThreshold = QStringLiteral("charge_control_end_threshold");

// kept for thinkpad driver retro compat for kernels < 5.9
static const QString s_oldChargeStartThreshold = QStringLiteral("charge_start_threshold");
static const QString s_oldChargeStopThreshold = QStringLiteral("charge_stop_threshold");

ChargeThresholdHelper::ChargeThresholdHelper(QObject *parent)
    : QObject(parent)
{
}

static QStringList getBatteries()
{
    const QStringList power_supplies = QDir(s_powerSupplySysFsPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList batteries;

    for (const QString &psu : power_supplies) {
        QDir psuDir(QString(s_powerSupplySysFsPath + u'/' + psu));
        QFile file(psuDir.filePath(u"type"_s));
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QString psu_type;
        QTextStream stream(&file);
        stream >> psu_type;

        if (psu_type.trimmed() != QLatin1String("Battery")) {
            continue; // Not a battery, skip
        }

        if (!psuDir.exists(s_chargeStartThreshold) && !psuDir.exists(s_oldChargeStartThreshold) && !psuDir.exists(s_chargeEndThreshold)
            && !psuDir.exists(s_oldChargeStopThreshold)) {
            continue; // No charge thresholds, skip
        }

        batteries.append(psu);
    }

    return batteries;
}

static QList<int> getThresholds(const QString &which)
{
    QList<int> thresholds;

    const QStringList batteries = getBatteries();
    for (const QString &battery : batteries) {
        QFile file(s_powerSupplySysFsPath + QLatin1Char('/') + battery + QLatin1Char('/') + which);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        int threshold = -1;
        QTextStream stream(&file);
        stream >> threshold;

        if (threshold < 0 || threshold > 100) {
            qWarning() << file.fileName() << "contains invalid threshold" << threshold;
            continue;
        }

        thresholds.append(threshold);
    }

    return thresholds;
}

static bool setThresholds(const QString &which, int threshold)
{
    const QStringList batteries = getBatteries();
    for (const QString &battery : batteries) {
        QFile file(s_powerSupplySysFsPath + QLatin1Char('/') + battery + QLatin1Char('/') + which);
        // TODO should we check the current value before writing the new one or is it clever
        // enough not to shred some chip by writing the same thing again?
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to open" << file.fileName() << "for writing";
            return false;
        }

        if (file.write(QByteArray::number(threshold)) == -1) {
            qWarning() << "Failed to write threshold into" << file.fileName();
            return false;
        }
    }

    return true;
}

ActionReply ChargeThresholdHelper::getthreshold(const QVariantMap &args)
{
    Q_UNUSED(args);

    auto startThresholds = getThresholds(s_chargeStartThreshold);
    auto stopThresholds = getThresholds(s_chargeEndThreshold);

    if (startThresholds.isEmpty() && stopThresholds.isEmpty()) {
        startThresholds = getThresholds(s_oldChargeStartThreshold);
        stopThresholds = getThresholds(s_oldChargeStopThreshold);
    }

    if (startThresholds.isEmpty() && stopThresholds.isEmpty()) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Charge thresholds are not supported by the kernel for this hardware"));
        return reply;
    }

    if (!startThresholds.isEmpty() && !stopThresholds.isEmpty() && startThresholds.count() != stopThresholds.count()) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Charge thresholds are not supported by the kernel for this hardware"));
        return reply;
    }

    // In the rare case there are multiple batteries with varying charge thresholds, try to use something sensible
    const int startThreshold = !startThresholds.empty() ? *std::max_element(startThresholds.begin(), startThresholds.end()) : -1;
    const int stopThreshold = !stopThresholds.empty() ? *std::min_element(stopThresholds.begin(), stopThresholds.end()) : -1;

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

    if (hasStartThreshold && !(setThresholds(s_chargeStartThreshold, startThreshold) || setThresholds(s_oldChargeStartThreshold, startThreshold))) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Failed to write start charge threshold"));
        return reply;
    }

    if (hasStopThreshold && !(setThresholds(s_chargeEndThreshold, stopThreshold) || setThresholds(s_oldChargeStopThreshold, stopThreshold))) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Failed to write stop charge threshold"));
        return reply;
    }

    return ActionReply();
}

// This handles Lenovo's "battery conservation mode" that limits the threshold to a fixed value
// This value differs from model to model (most 55-60%, others ~80%) and is unknown (last checked in Kernel 6.3)
ActionReply ChargeThresholdHelper::getconservationmode(const QVariantMap &args)
{
    Q_UNUSED(args);

    QFile file(s_conservationModeSysFsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Battery conservation mode is not supported"));
        return reply;
    }

    int state = 0;
    QTextStream stream(&file);
    stream >> state;

    if (state < 0 || state > 1) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Battery conservation mode is in an invalid state"));
        return reply;
    }

    ActionReply reply;
    reply.setData({
        {QStringLiteral("batteryConservationModeEnabled"), !!state}
    });
    return reply;
}

ActionReply ChargeThresholdHelper::setconservationmode(const QVariantMap &args)
{
    const bool enabled = args.value(QStringLiteral("batteryConservationModeEnabled"), 0).toBool();
    QFile file(s_conservationModeSysFsPath);

    if (!file.open(QIODevice::WriteOnly)) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Failed to open battery conservation mode file"));
        return reply;
    }

    if (file.write(QByteArray::number(enabled)) == -1) {
        auto reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Failed to set battery conservation mode"));
        return reply;
    }

    return ActionReply();
}

KAUTH_HELPER_MAIN("org.kde.powerdevil.chargethresholdhelper", ChargeThresholdHelper)

#include "moc_chargethresholdhelper.cpp"
