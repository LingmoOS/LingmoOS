/*
 *   SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

// Qt
#include <QDBusVariant>
#include <QObject>
#include <QString>

// Utils
#include <utils/d_ptr.h>

// Local
#include "Module.h"

/**
 * Features object provides one interface for clients
 * to access other objects' features
 */
class Features : public Module
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Features")

public:
    explicit Features(QObject *parent = nullptr);
    ~Features() override;

public Q_SLOTS:
    /**
     * Is the feature backend available?
     */
    bool IsFeatureOperational(const QString &feature) const;

    QStringList ListFeatures(const QString &module) const;

    QDBusVariant GetValue(const QString &property) const;

    void SetValue(const QString &property, const QDBusVariant &value);

private:
    D_PTR;
};
