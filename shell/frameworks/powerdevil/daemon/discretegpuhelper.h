/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#pragma once

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#include <QObject>
#include <kauth_version.h>

using namespace KAuth;

class DiscreteGpuHelper : public QObject
{
    Q_OBJECT
public:
    explicit DiscreteGpuHelper(QObject *parent = nullptr);

public Q_SLOTS:
    ActionReply hasdualgpu(const QVariantMap &args);
};
