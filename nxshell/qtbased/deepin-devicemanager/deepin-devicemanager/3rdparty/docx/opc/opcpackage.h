// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPCPACKAGE_H
#define OPCPACKAGE_H

#include <QString>
#include <QMap>

namespace Docx {

class PackageReader;
class Package;
class Part;
class DocumentPart;
class Relationships;
class OpcPackage
{
public:
    OpcPackage();
    DocumentPart *mainDocument();
    Part *partByRelated(const QString &reltype);
    QList<Part *> parts() const;
    virtual ~OpcPackage();

protected:
    void partsbyRels(const Relationships *rels, QList<Part *> *parts) const;

protected:
    Relationships *m_rels;

};

class Unmarshaller
{
public:
    Unmarshaller();
    static void unmarshal(PackageReader *pkgReader, Package *package);
    static QMap<QString, Part *> unmarshalParts(PackageReader *pkgReader, Package *package);
    static void unmarshalRelationships(PackageReader *pkgReader, Package *package, const QMap<QString, Part *> &parts);
    virtual ~Unmarshaller();

};
}
#endif // OPCPACKAGE_H
