/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resource.h"
#include "model.h"
#include "model_p.h"
#include "nodevisitor.h"
#include "property.h"
#include "statement.h"

#include <QList>
#include <QString>
#include <QUuid>
#include <QWeakPointer>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT Resource::ResourcePrivate
{
public:
    QString uri;
    QWeakPointer<Model::ModelPrivate> model;
    bool isAnon;
    unsigned int id;

    bool operator==(const ResourcePrivate &other) const
    {
        if (!isAnon && !other.isAnon) {
            return uri == other.uri;
        } else {
            return id == other.id;
        }
    }
};

Resource::Resource(const Resource &other)
    : Node(other)
{
    *this = other;
}

Resource::Resource()
    : d()
{
}

Resource::Resource(const QString &uri)
    : d(new ResourcePrivate)
{
    if (uri.isNull()) {
        d->uri = QUuid().createUuid().toString();
        d->isAnon = true;
    } else {
        d->uri = uri;
        d->isAnon = false;
    }

    d->id = idCounter++;
}

Resource::~Resource()
{
}

Resource &Resource::operator=(const Resource &other)
{
    d = other.d;
    return *this;
}

bool Resource::operator==(const Resource &other) const
{
    if (!d || !other.d) {
        return d == other.d;
    }
    return *d == *(other.d);
}

bool Resource::hasProperty(PropertyPtr property) const
{
    if (!d) {
        return false;
    }
    const QSharedPointer<Model::ModelPrivate> m = d->model.toStrongRef();
    if (!m) {
        return false;
    }
    return m->resourceHasProperty(this, property);
}

StatementPtr Resource::property(PropertyPtr property) const
{
    StatementPtr ptr(new Statement());
    if (!d) {
        return ptr;
    }
    const QSharedPointer<Model::ModelPrivate> m = d->model.toStrongRef();
    if (!m) {
        return ptr;
    }
    return m->resourceProperty(this, property);
}

QList<StatementPtr> Resource::properties(PropertyPtr property) const
{
    if (!d) {
        return QList<StatementPtr>();
    }
    const QSharedPointer<Model::ModelPrivate> m = d->model.toStrongRef();
    if (!m) {
        return QList<StatementPtr>();
    }

    return m->resourceProperties(this, property);
}

Resource *Resource::clone() const
{
    return new Resource(*this);
}

void Resource::accept(NodeVisitor *visitor, NodePtr ptr)
{
    ResourcePtr rptr = ptr.staticCast<Resource>();
    if (!visitor->visitResource(rptr)) {
        Node::accept(visitor, ptr);
    }
}

unsigned int Resource::id() const
{
    return d ? d->id : 0;
}

bool Resource::isNull() const
{
    return !d;
}

Model Resource::model() const
{
    if (!d) {
        return Model();
    }

    const QSharedPointer<Model::ModelPrivate> mp = d->model.toStrongRef();

    Model m;

    if (mp) {
        m.d = mp;
    }

    return m;
}

bool Resource::isResource() const
{
    return true;
}

bool Resource::isProperty() const
{
    return false;
}

bool Resource::isLiteral() const
{
    return false;
}

bool Resource::isAnon() const
{
    return d ? d->isAnon : false;
}

bool Resource::isSequence() const
{
    return false;
}

void Resource::setModel(const Model &model)
{
    if (d) {
        d->model = model.d;
    }
}

void Resource::setId(unsigned int id)
{
    if (d) {
        d->id = id;
    }
}

QString Resource::text() const
{
    return QString();
}

QString Resource::uri() const
{
    return d ? d->uri : QString();
}

} // namespace RDF
} // namespace Syndication
