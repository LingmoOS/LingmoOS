// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGEWRITER_H
#define PACKAGEWRITER_H

#include <QString>

namespace Docx {
class PhysPkgWriter;
class Relationships;
class Part;

class PackageWriter
{
public:
    PackageWriter(const QString &pkgFile);
//    PackageWriter(const QString &pkgFile, const Relationships *pkgRels, const QList<Part *> &parts);
    void writePkgRels(const Relationships *pkgRels);
    void writeContentTypes(const QList<Part *> &parts);
    void writeParts(const QList<Part *> &parts);
    ~PackageWriter();

private:
    PhysPkgWriter *m_writer;
};
}

#endif // PACKAGEWRITER_H
