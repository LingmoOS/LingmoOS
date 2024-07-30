/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigpropertymap.h"

#include <KCoreConfigSkeleton>
#include <QJSValue>
#include <QPointer>

#include <functional>

class KConfigPropertyMapPrivate
{
public:
    KConfigPropertyMapPrivate(KConfigPropertyMap *map)
        : q(map)
    {
    }

    enum LoadConfigOption {
        DontEmitValueChanged,
        EmitValueChanged,
    };

    void loadConfig(LoadConfigOption option);
    void writeConfig();
    void writeConfigValue(const QString &key, const QVariant &value);

    KConfigPropertyMap *q;
    QPointer<KCoreConfigSkeleton> config;
    bool updatingConfigValue = false;
    bool notify = false;
};

KConfigPropertyMap::KConfigPropertyMap(KCoreConfigSkeleton *config, QObject *parent)
    : QQmlPropertyMap(this, parent)
    , d(new KConfigPropertyMapPrivate(this))
{
    Q_ASSERT(config);
    d->config = config;

    // Reload the config only if the change signal has *not* been emitted by ourselves updating the config
    connect(config, &KCoreConfigSkeleton::configChanged, this, [this]() {
        if (!d->updatingConfigValue) {
            d->loadConfig(KConfigPropertyMapPrivate::EmitValueChanged);
        }
    });
    connect(this, &KConfigPropertyMap::valueChanged, this, [this](const QString &key, const QVariant &value) {
        d->writeConfigValue(key, value);
    });

    d->loadConfig(KConfigPropertyMapPrivate::DontEmitValueChanged);
}

KConfigPropertyMap::~KConfigPropertyMap() = default;

bool KConfigPropertyMap::isNotify() const
{
    return d->notify;
}

void KConfigPropertyMap::setNotify(bool notify)
{
    d->notify = notify;
}

void KConfigPropertyMap::writeConfig()
{
    d->writeConfig();
}

QVariant KConfigPropertyMap::updateValue(const QString &key, const QVariant &input)
{
    Q_UNUSED(key);
    if (input.userType() == qMetaTypeId<QJSValue>()) {
        return input.value<QJSValue>().toVariant();
    }
    return input;
}

bool KConfigPropertyMap::isImmutable(const QString &key) const
{
    KConfigSkeletonItem *item = d->config.data()->findItem(key);
    if (item) {
        return item->isImmutable();
    }

    return false;
}

void KConfigPropertyMapPrivate::loadConfig(KConfigPropertyMapPrivate::LoadConfigOption option)
{
    if (!config) {
        return;
    }

    const auto &items = config.data()->items();
    for (KConfigSkeletonItem *item : items) {
        q->insert(item->key() + QStringLiteral("Default"), item->getDefault());
        q->insert(item->key(), item->property());
        if (option == EmitValueChanged) {
            Q_EMIT q->valueChanged(item->key(), item->property());
        }
    }
}

void KConfigPropertyMapPrivate::writeConfig()
{
    if (!config) {
        return;
    }

    const auto lstItems = config.data()->items();
    for (KConfigSkeletonItem *item : lstItems) {
        item->setWriteFlags(notify ? KConfigBase::Notify : KConfigBase::Normal);
        item->setProperty(q->value(item->key()));
    }
    // Internally sync the config. This way we ensure the config file is written, even if the process crashed
    config.data()->save();
}

void KConfigPropertyMapPrivate::writeConfigValue(const QString &key, const QVariant &value)
{
    KConfigSkeletonItem *item = config.data()->findItem(key);
    if (item) {
        updatingConfigValue = true;
        item->setWriteFlags(notify ? KConfigBase::Notify : KConfigBase::Normal);
        item->setProperty(value);
        updatingConfigValue = false;
    }
}

#include "moc_kconfigpropertymap.cpp"
