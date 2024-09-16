// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCXOPCOXML_H
#define DOCXOPCOXML_H

#include "constants.h"

#include <QDomDocument>

namespace Docx {

class BaseOxmlElement : public QDomDocument
{
public:
    BaseOxmlElement();

    virtual ~BaseOxmlElement();

private:

};

class CT_Default : public QDomElement
{
public:
    CT_Default();
    CT_Default(const QDomElement &x);

    QString contentType();
    QString extension();
    static CT_Default newDefault(const QString &ext, const QString &content_type);

    virtual ~CT_Default();

private:

};


class CT_Override : public QDomElement
{
public:
    CT_Override();
    CT_Override(const QDomElement &x);

    QString contentType();
    QString partname();
    static CT_Override newOverride(const QString &partname, const QString &content_type);


    virtual ~CT_Override();

private:

};

class CT_Relationship : public QDomElement
{
public:
    CT_Relationship();
    CT_Relationship(const QDomElement &x);
    static CT_Relationship newRelationship(const QString &rId, const QString &reltype
                                   , const QString &target, bool target_mode);
    QString rId();
    QString relType();
    QString targetRef();
    QString targetMode();

    virtual ~CT_Relationship();

private:
};

class CT_Relationships : public QDomElement
{
public:
    CT_Relationships();
    CT_Relationships(const QDomElement &x);
    static CT_Relationships newRelationships();
    void RelationshipLst();

    virtual ~CT_Relationships();

private:

};

}
#endif // OXML_H
