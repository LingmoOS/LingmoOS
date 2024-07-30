/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "model.h"
#include "model_p.h"

namespace Syndication
{
namespace RDF
{
long Model::ModelPrivate::idCounter = 0;

Model::Model()
    : d(new ModelPrivate)
{
}

Model::Model(const Model &other)
{
    *this = other;
}

Model::~Model()
{
}

Model &Model::operator=(const Model &other)
{
    d = other.d;
    return *this;
}

bool Model::operator==(const Model &other) const
{
    return *d == *(other.d);
}

PropertyPtr Model::createProperty(const QString &uri)
{
    PropertyPtr prop;

    if (d->properties.contains(uri)) {
        prop = d->properties[uri];
    } else {
        prop = PropertyPtr(new Property(uri));
        prop->setModel(*this);
        // if there is a resource object with the same uri, replace
        // the resource object by the new property object and reuse the id
        if (d->resources.contains(uri)) {
            prop->setId(d->resources[uri]->id());
        }
        d->addToHashes(prop);
    }

    return prop;
}

ResourcePtr Model::createResource(const QString &uri)
{
    ResourcePtr res;

    if (d->resources.contains(uri)) {
        res = d->resources[uri];
    } else {
        res = ResourcePtr(new Resource(uri));
        res->setModel(*this);
        d->addToHashes(res);
    }

    return res;
}

SequencePtr Model::createSequence(const QString &uri)
{
    SequencePtr seq;

    if (d->sequences.contains(uri)) {
        seq = d->sequences[uri];
    } else {
        seq = SequencePtr(new Sequence(uri));
        seq->setModel(*this);
        // if there is a resource object with the same uri, replace
        // the resource object by the new sequence object and reuse the id
        if (d->resources.contains(uri)) {
            seq->setId(d->resources[uri]->id());
        }

        d->addToHashes(seq);
    }

    return seq;
}

LiteralPtr Model::createLiteral(const QString &text)
{
    LiteralPtr lit(new Literal(text));

    d->addToHashes(lit);
    return lit;
}

void Model::removeStatement(StatementPtr statement)
{
    removeStatement(statement->subject(), statement->predicate(), statement->object());
}

void Model::removeStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object)
{
    /* clang-format off */
    QString key = QStringLiteral("%1-%2-%3").arg(QString::number(subject->id()),
                                                 QString::number(predicate->id()),
                                                 QString::number(object->id()));
    /* clang-format on */

    d->removeFromHashes(key);
}

StatementPtr Model::addStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object)
{
    d->init(d);
    ResourcePtr subjInternal = subject;

    if (!d->nodes.contains(subjInternal->id())) {
        subjInternal = ResourcePtr(subject->clone());
        subjInternal->setModel(*this);
        d->addToHashes(subjInternal);
    }

    PropertyPtr predInternal = predicate;

    if (!d->nodes.contains(predInternal->id())) {
        predInternal = PropertyPtr(predicate->clone());
        predInternal->setModel(*this);
        d->addToHashes(predInternal);
    }

    NodePtr objInternal = object;

    if (!d->nodes.contains(objInternal->id())) {
        objInternal = NodePtr(object->clone());
        objInternal->setModel(*this);
        d->addToHashes(objInternal);
    }

    // TODO: avoid duplicated stmts with literal objects!

    /* clang-format off */
    QString key = QStringLiteral("%1-%2-%3").arg(QString::number(subjInternal->id()),
                                                 QString::number(predInternal->id()),
                                                 QString::number(objInternal->id()));
    /* clang-format on */

    StatementPtr stmt;

    if (!d->statements.contains(key)) {
        stmt = StatementPtr(new Statement(subjInternal, predInternal, objInternal));
        d->addToHashes(stmt, key);
    } else {
        stmt = d->statements[key];
    }

    return stmt;
}

bool Model::isEmpty() const
{
    return d->statements.isEmpty();
}

bool Model::resourceHasProperty(const Resource *resource, PropertyPtr property) const
{
    return d->resourceHasProperty(resource, property);
}

bool Model::ModelPrivate::resourceHasProperty(const Resource *resource, PropertyPtr property) const
{
    // resource unknown
    if (!resources.contains(resource->uri())) {
        return false;
    }

    const QList<StatementPtr> &stmts = stmtsBySubject[resource->uri()];

    return std::any_of(stmts.cbegin(), stmts.cend(), [&property](const StatementPtr &p) {
        return *(p->predicate()) == *property;
    });
}

StatementPtr Model::resourceProperty(const Resource *resource, PropertyPtr property) const
{
    return d->resourceProperty(resource, property);
}

StatementPtr Model::ModelPrivate::resourceProperty(const Resource *resource, PropertyPtr property) const
{
    const QList<StatementPtr> &stmts = stmtsBySubject[resource->uri()];
    auto it = std::find_if(stmts.cbegin(), stmts.cend(), [&property](const StatementPtr &p) {
        return *(p->predicate()) == *property;
    });
    return it != stmts.cend() ? *it : nullStatement;
}

QList<StatementPtr> Model::resourceProperties(const Resource *resource, PropertyPtr property) const
{
    return d->resourceProperties(resource, property);
}

QList<StatementPtr> Model::ModelPrivate::resourceProperties(const Resource *resource, PropertyPtr property) const
{
    QList<StatementPtr> res;

    const QList<StatementPtr> &stmts = stmtsBySubject[resource->uri()];
    for (const auto &p : stmts) {
        if (*(p->predicate()) == *property) {
            res.append(p);
        }
    }

    return res;
}

QList<StatementPtr> Model::statements() const
{
    return d->statements.values();
}

QString Model::debugInfo() const
{
    QString info;
    for (const StatementPtr &stmtPtr : std::as_const(d->statements)) {
        info += QStringLiteral("<%1> <%2> ").arg(stmtPtr->subject()->uri(), stmtPtr->predicate()->uri());

        if (stmtPtr->object()->isLiteral()) {
            info += QStringLiteral("\"%1\"\n").arg(stmtPtr->asString());
        } else {
            info += QStringLiteral("<%1>\n").arg(stmtPtr->asResource()->uri());
        }
    }

    return info;
}

QList<ResourcePtr> Model::resourcesWithType(ResourcePtr type) const
{
    QList<ResourcePtr> list;

    for (const StatementPtr &stmtPtr : std::as_const(d->statements)) {
        if (*(stmtPtr->predicate()) == *(RDFVocab::self()->type()) //
            && stmtPtr->object()->isResource() && *(dynamic_cast<Resource *>(stmtPtr->object().data())) == *type) {
            list.append(stmtPtr->subject());
        }
    }

    return list;
}

NodePtr Model::nodeByID(uint id) const
{
    return d->nodeByID(id);
}

NodePtr Model::ModelPrivate::nodeByID(uint _id) const
{
    if (!nodes.contains(_id)) {
        return nullLiteral;
    } else {
        return nodes.value(_id);
    }
}

ResourcePtr Model::resourceByID(uint id) const
{
    return d->resourceByID(id);
}

ResourcePtr Model::ModelPrivate::resourceByID(uint _id) const
{
    if (!nodes.contains(_id)) {
        return nullResource;
    } else {
        NodePtr node = nodes.value(_id);
        if (node->isResource()) {
            return node.staticCast<Resource>();
        } else {
            return nullResource;
        }
    }
}

PropertyPtr Model::propertyByID(uint id) const
{
    return d->propertyByID(id);
}

PropertyPtr Model::ModelPrivate::propertyByID(uint _id) const
{
    if (!nodes.contains(_id)) {
        return nullProperty;
    } else {
        NodePtr node = nodes.value(_id);
        if (node->isProperty()) {
            return node.staticCast<Property>();
        } else {
            return nullProperty;
        }
    }
}

LiteralPtr Model::literalByID(uint id) const
{
    return d->literalByID(id);
}

LiteralPtr Model::ModelPrivate::literalByID(uint _id) const
{
    if (!nodes.contains(_id)) {
        return nullLiteral;
    } else {
        NodePtr node = nodes.value(_id);
        if (node->isLiteral()) {
            return node.staticCast<Literal>();
        } else {
            return nullLiteral;
        }
    }
}

} // namespace RDF
} // namespace Syndication
