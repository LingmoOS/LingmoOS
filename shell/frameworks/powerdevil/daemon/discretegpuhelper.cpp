/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#include "discretegpuhelper.h"

#include <powerdevil_debug.h>

#include <QFile>

inline static constexpr QLatin1String SWITCHEROO_SYSFS_PATH("/sys/kernel/debug/vgaswitcheroo/switch");

DiscreteGpuHelper::DiscreteGpuHelper(QObject *parent)
    : QObject(parent)
{
}

ActionReply DiscreteGpuHelper::hasdualgpu(const QVariantMap &args)
{
    Q_UNUSED(args);

    ActionReply reply;
    reply.addData(QStringLiteral("hasdualgpu"), QFile::exists(SWITCHEROO_SYSFS_PATH));
    // qCDebug(POWERDEVIL) << "data contains:" << reply.data()["hasdualgpu"];

    return reply;
}

KAUTH_HELPER_MAIN("org.kde.powerdevil.discretegpuhelper", DiscreteGpuHelper)

#include "moc_discretegpuhelper.cpp"
