/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QVariant>

#include "types.h"

namespace KScreen
{
class AbstractBackend;
}

class BackendDBusWrapper : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KScreen.Backend")

public:
    explicit BackendDBusWrapper(KScreen::AbstractBackend *backend);
    ~BackendDBusWrapper() override;

    bool init();

    QVariantMap getConfig() const;
    QVariantMap setConfig(const QVariantMap &config);
    QByteArray getEdid(int output) const;

    inline KScreen::AbstractBackend *backend() const
    {
        return mBackend;
    }

Q_SIGNALS:
    void configChanged(const QVariantMap &config);

private Q_SLOTS:
    void backendConfigChanged(const KScreen::ConfigPtr &config);
    void doEmitConfigChanged();

private:
    KScreen::AbstractBackend *mBackend = nullptr;
    QTimer mChangeCollector;
    KScreen::ConfigPtr mCurrentConfig;
};
