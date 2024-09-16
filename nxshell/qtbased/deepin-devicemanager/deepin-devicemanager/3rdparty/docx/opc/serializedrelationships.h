// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERIALIZEDRELATIONSHIPS_H
#define SERIALIZEDRELATIONSHIPS_H

#include "packuri.h"
#include "constants.h"

#include <QVector>
#include <QString>
#include <QByteArray>

namespace Docx {

class SerializedRelationship
{
public:
    SerializedRelationship();
    SerializedRelationship(const QString &baseURI, const QString &id, const QString &relType, const QString &target, const QString &targetModel = Constants::INTERNAL);
    QString baseURI() const;
    QString rId() const;
    QString relType() const;
    QString target() const;
    QString targetPartName() const;
    bool  isExternal() const;
    ~SerializedRelationship();

private:
    QString m_baseURI;
    QString m_rId;
    QString m_relType;
    QString m_target;
    QString m_targetMode;

};


class SerializedRelationships
{
public:
    SerializedRelationships();
    static SerializedRelationships* loadFromData(const QString baseURI, const QByteArray &relsXml);
    void addRelationship(const SerializedRelationship &rel);
    void addRelationships(const QVector<SerializedRelationship> &rels);
    QVector<SerializedRelationship> rels() const;
    int count() const;

    ~SerializedRelationships();

private:
    QVector<SerializedRelationship> m_rels;
};

class SerializedPart
{
public:
    SerializedPart(const PackURI &partName, const QString &contentType, const QString &reltype, const QByteArray &blob, const SerializedRelationships &srels);
    PackURI partName() const;
    QString contentType() const;
    QString relType() const;
    QByteArray blob() const;
    SerializedRelationships rels() const { return m_srels; }

private:
    PackURI m_partName;
    QString m_contentType;
    QString m_reltype;
    QByteArray m_blob;
    SerializedRelationships m_srels;
};

}

#endif // SERIALIZEDRELATIONSHIPS_H
