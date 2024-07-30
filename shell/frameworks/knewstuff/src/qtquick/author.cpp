/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "author.h"

#include "quickengine.h"

#include "core/author.h"
#include "core/provider.h"

#include <memory>

namespace KNewStuffQuick
{
// This caching will want to eventually go into the Provider level (and be more generalised)
typedef QHash<QString, std::shared_ptr<KNSCore::Author>> AllAuthorsHash;
Q_GLOBAL_STATIC(AllAuthorsHash, allAuthors)

class AuthorPrivate
{
public:
    AuthorPrivate(Author *qq)
        : q(qq)
    {
    }
    Author *const q;
    bool componentCompleted{false};
    Engine *engine{nullptr};
    QString providerId;
    QString username;

    QSharedPointer<KNSCore::Provider> provider;
    void resetConnections()
    {
        if (!componentCompleted) {
            return;
        }
        if (provider) {
            provider->disconnect(q);
        }
        if (engine) {
            provider = engine->provider(providerId);
            if (!provider) {
                provider = engine->defaultProvider();
            }
        }
        if (provider) {
            QObject::connect(provider.get(), &KNSCore::Provider::personLoaded, q, [this](const std::shared_ptr<KNSCore::Author> author) {
                allAuthors()->insert(QStringLiteral("%1 %2").arg(provider->id(), author->id()), author);
                Q_EMIT q->dataChanged();
            });
            author(); // Check and make sure...
        }
    }

    // TODO Having a shared ptr on a QSharedData class doesn't make sense
    std::shared_ptr<KNSCore::Author> author()
    {
        std::shared_ptr<KNSCore::Author> ret;
        if (provider && !username.isEmpty()) {
            ret = allAuthors()->value(QStringLiteral("%1 %2").arg(provider->id(), username));
            if (!ret.get()) {
                Q_EMIT provider->loadPerson(username);
            }
        }
        return ret;
    }
};
}

using namespace KNewStuffQuick;

Author::Author(QObject *parent)
    : QObject(parent)
    , d(new AuthorPrivate(this))
{
    connect(this, &Author::engineChanged, &Author::dataChanged);
    connect(this, &Author::providerIdChanged, &Author::dataChanged);
    connect(this, &Author::usernameChanged, &Author::dataChanged);
}

Author::~Author() = default;

void Author::classBegin()
{
}

void Author::componentComplete()
{
    d->componentCompleted = true;
    d->resetConnections();
}

Engine *Author::engine() const
{
    return d->engine;
}

void Author::setEngine(Engine *newEngine)
{
    if (d->engine != newEngine) {
        d->engine = newEngine;
        d->resetConnections();
        Q_EMIT engineChanged();
    }
}

QString Author::providerId() const
{
    return d->providerId;
}

void Author::setProviderId(const QString &providerId)
{
    if (d->providerId != providerId) {
        d->providerId = providerId;
        d->resetConnections();
        Q_EMIT providerIdChanged();
    }
}

QString Author::username() const
{
    return d->username;
}

void Author::setUsername(const QString &username)
{
    if (d->username != username) {
        d->username = username;
        d->resetConnections();
        Q_EMIT usernameChanged();
    }
}

QString Author::name() const
{
    std::shared_ptr<KNSCore::Author> author = d->author();
    if (author.get() && !author->name().isEmpty()) {
        return author->name();
    }
    return d->username;
}

QString Author::description() const
{
    std::shared_ptr<KNSCore::Author> author = d->author();
    if (author.get()) {
        return author->description();
    }
    return QString{};
}

QString Author::homepage() const
{
    std::shared_ptr<KNSCore::Author> author = d->author();
    if (author.get()) {
        return author->homepage();
    }
    return QString{};
}

QString Author::profilepage() const
{
    std::shared_ptr<KNSCore::Author> author = d->author();
    if (author.get()) {
        return author->profilepage();
    }
    return QString{};
}

QUrl Author::avatarUrl() const
{
    std::shared_ptr<KNSCore::Author> author = d->author();
    if (author.get()) {
        return author->avatarUrl();
    }
    return QUrl{};
}

#include "moc_author.cpp"
