/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "providersmodel.h"

#include "enginebase.h"

namespace KNSCore
{
class ProvidersModelPrivate
{
    ProvidersModel *const q;

public:
    explicit ProvidersModelPrivate(ProvidersModel *qq)
        : q(qq)
    {
    }

    EngineBase *getEngine() const;
    void setEngine(EngineBase *engine);

    EngineBase *engine = nullptr;
    QStringList knownProviders;
};

EngineBase *ProvidersModelPrivate::getEngine() const
{
    return engine;
}

void ProvidersModelPrivate::setEngine(EngineBase *engine)
{
    q->setEngine(engine);
}

ProvidersModel::ProvidersModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new ProvidersModelPrivate(this))
{
}

ProvidersModel::~ProvidersModel() = default;

QHash<int, QByteArray> KNSCore::ProvidersModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {IdRole, "id"},
        {NameRole, "name"},
        {VersionRole, "version"},
        {WebsiteRole, "website"},
        {HostRole, "host"},
        {ContactEmailRole, "contactEmail"},
        {SupportsSslRole, "supportsSsl"},
        {IconRole, "icon"},
        {ObjectRole, "object"},
    };
    return roles;
}

int KNSCore::ProvidersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->knownProviders.count();
}

QVariant KNSCore::ProvidersModel::data(const QModelIndex &index, int role) const
{
    if (checkIndex(index) && d->engine) {
        QSharedPointer<Provider> provider = d->engine->provider(d->knownProviders.value(index.row()));
        if (provider) {
            switch (role) {
            case IdRole:
                return provider->id();
            case NameRole:
                return provider->name();
            case VersionRole:
                return provider->version();
            case WebsiteRole:
                return provider->website();
            case HostRole:
                return provider->host();
            case ContactEmailRole:
                return provider->contactEmail();
            case SupportsSslRole:
                return provider->supportsSsl();
            case IconRole:
                return provider->icon();
            case ObjectRole:
                return QVariant::fromValue(provider.data());
            }
        }
    }
    return QVariant();
}

QObject *KNSCore::ProvidersModel::engine() const
{
    return d->engine;
}

void KNSCore::ProvidersModel::setEngine(QObject *engine)
{
    if (d->engine != engine) {
        if (d->engine) {
            d->engine->disconnect(this);
        }
        d->engine = qobject_cast<EngineBase *>(engine);
        Q_EMIT engineChanged();
        if (d->engine) {
            connect(d->engine, &EngineBase::providersChanged, this, [this]() {
                beginResetModel();
                d->knownProviders = d->engine->providerIDs();
                endResetModel();
            });
            beginResetModel();
            d->knownProviders = d->engine->providerIDs();
            endResetModel();
        }
    }
}

}

#include "moc_providersmodel.cpp"
