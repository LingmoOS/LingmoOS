/*
 *   SPDX-FileCopyrightText: 2014-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "kactivitymanagerd_plugin_export.h"

// Qt
#include <QDBusVariant>
#include <QObject>
#include <QString>
#include <QStringList>

// Utils
#include <utils/d_ptr.h>

/**
 * Module
 */
class KACTIVITYMANAGERD_PLUGIN_EXPORT Module : public QObject
{
    Q_OBJECT

public:
    explicit Module(const QString &name, QObject *parent = nullptr);
    ~Module() override;

    static QObject *get(const QString &name);
    static QHash<QString, QObject *> &get();

    virtual bool isFeatureOperational(const QStringList &feature) const;
    virtual bool isFeatureEnabled(const QStringList &feature) const;
    virtual void setFeatureEnabled(const QStringList &feature, bool value);
    virtual QStringList listFeatures(const QStringList &feature) const;

    virtual QDBusVariant featureValue(const QStringList &property) const;
    virtual void setFeatureValue(const QStringList &property, const QDBusVariant &value);

private:
    D_PTR;
};
