// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REL_H
#define REL_H

#include "packuri.h"
#include "part.h"

#include <QString>
#include <QMap>
#include <QByteArray>


namespace Docx {

class Relationship
{
public:
    Relationship(const QString &rId, const QString &reltypet, const QString &targetRef, Part *target, const QString &baseURI, bool external = false);
    QString rId() const { return m_rId; }
    QString relType() const { return m_reltype; }
    Part * target() const { return m_target; }
    QString targetRef() const;
    QString baseURI() const { return m_baseURI; }
    bool isExternal() const { return m_isexternal; }
    virtual ~Relationship();

private:
    QString m_rId;
    QString m_reltype;
    QString m_targetRef;
    Part *m_target;
    QString m_baseURI;
    bool m_isexternal;

};

class Relationships
{
public:
    Relationships(const QString &baseURI);
    Relationship *addRelationship(const QString &reltype, const QString &targetRef, Part *target, const QString &rId, bool external = false);
    Part *partWithReltype(const QString &reltype);
    int count() const;
    QMap<QString, Relationship *> rels() const;
    QByteArray blob() const;
    Relationship * getOrAddPart(const QString &reltype, Part *target, const QString &baseUri);
    Relationship * getOrAddExtPart(const QString &reltype, const QString &targetref);
    virtual ~Relationships();

private:
    Relationship *getMatching(const QString &reltype, Part *target);
    Relationship *getMatchingExt(const QString &reltype, const QString &targetref);
    QString nextrId();

private:
    QString m_baseURI;    
    QMap<QString, Part *> m_targetPartsByrId;
    QMap<QString, Relationship *> m_rels;
};

}

#endif // REL_H
