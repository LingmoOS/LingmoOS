/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_MODEL_P_H
#define SYNDICATION_RDF_MODEL_P_H

#include "literal.h"
#include "model.h"
#include "nodevisitor.h"
#include "property.h"
#include "rdfvocab.h"
#include "resource.h"
#include "sequence.h"
#include "statement.h"

#include <QHash>
#include <QList>
#include <QString>

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT Model::ModelPrivate
{
public:
    long id;
    static long idCounter;
    LiteralPtr nullLiteral;
    PropertyPtr nullProperty;
    ResourcePtr nullResource;
    StatementPtr nullStatement;
    QHash<QString, StatementPtr> statements;
    QHash<QString, QList<StatementPtr>> stmtsBySubject;

    QHash<int, NodePtr> nodes;
    QHash<QString, ResourcePtr> resources;
    QHash<QString, PropertyPtr> properties;
    QHash<QString, SequencePtr> sequences;
    bool initialized;

    class AddToHashesVisitor;

    ModelPrivate()
        : id(idCounter++)
    {
        addToHashesVisitor = new AddToHashesVisitor(this);
        initialized = false;
    }

    ~ModelPrivate()
    {
        delete addToHashesVisitor;
    }

    bool operator==(const ModelPrivate &other) const
    {
        return id == other.id;
    }

    class AddToHashesVisitor : public NodeVisitor
    {
    public:
        explicit AddToHashesVisitor(ModelPrivate *parent)
            : p(parent)
        {
        }

        bool visitResource(ResourcePtr res) override
        {
            visitNode(res);
            p->resources[res->uri()] = res;
            return true;
        }

        bool visitSequence(SequencePtr seq) override
        {
            visitResource(seq);
            p->sequences[seq->uri()] = seq;
            return true;
        }

        bool visitProperty(PropertyPtr prop) override
        {
            visitResource(prop);
            p->properties[prop->uri()] = prop;
            return true;
        }

        bool visitNode(NodePtr node) override
        {
            p->nodes[node->id()] = node;
            return true;
        }

        ModelPrivate *p;
    };

    AddToHashesVisitor *addToHashesVisitor;

    bool resourceHasProperty(const Resource *resource, PropertyPtr property) const;

    StatementPtr resourceProperty(const Resource *resource, PropertyPtr property) const;

    QList<StatementPtr> resourceProperties(const Resource *resource, PropertyPtr property) const;

    NodePtr nodeByID(uint id) const;

    ResourcePtr resourceByID(uint id) const;

    PropertyPtr propertyByID(uint id) const;

    LiteralPtr literalByID(uint id) const;

    void addToHashes(NodePtr node)
    {
        addToHashesVisitor->visit(node);
    }

    void addToHashes(StatementPtr stmt, const QString &key)
    {
        statements[key] = stmt;
        stmtsBySubject[stmt->subject()->uri()].append(stmt);
    }

    void removeFromHashes(const QString &key)
    {
        StatementPtr stmt = statements[key];
        if (stmt) {
            stmtsBySubject[stmt->subject()->uri()].removeAll(stmt);
        }
        statements.remove(key);
    }

    void init(const QSharedPointer<ModelPrivate> &sharedThis)
    {
        if (!initialized) {
            Model m;
            m.d = sharedThis;
            nullLiteral = LiteralPtr(new Literal());
            nullLiteral->setModel(m);
            nullProperty = PropertyPtr(new Property());
            nullProperty->setModel(m);
            nullResource = ResourcePtr(new Resource());
            nullResource->setModel(m);
            nullStatement = StatementPtr(new Statement());
            initialized = true;
        }
    }

private:
    Q_DISABLE_COPY(ModelPrivate)
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_MODEL_P_H
