// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packagewriter.h"
#include "physpkgwriter.h"
#include "rel.h"
#include "part.h"
#include "contenttypemap.h"
#include "constants.h"

#include <QList>

namespace Docx {

PackageWriter::PackageWriter(const QString &pkgFile)
{
    m_writer = new PhysPkgWriter(pkgFile);
}

//PackageWriter::PackageWriter(const QString &pkgFile, const Relationships *pkgRels, const QList<Part *> &parts)
//    : PackageWriter(pkgFile)
//{

//}

void PackageWriter::writePkgRels(const Relationships *pkgRels)
{
    m_writer->write(QStringLiteral("_rels/.rels"), pkgRels->blob());
}

void PackageWriter::writeContentTypes(const QList<Part *> &parts)
{
    ContentTypeMap contTypeMap;
    QListIterator<Part *> iter(parts);
    while (iter.hasNext()) {
      Part *p = iter.next();
      contTypeMap.addContentType(p->partName(), p->contentType());
    }
    m_writer->write(Constants::CONTENT_TYPES_URI, contTypeMap.blob());
}

void PackageWriter::writeParts(const QList<Part *> &parts)
{
    QListIterator<Part *> iter(parts);
    while (iter.hasNext()) {
        Part *p = iter.next();
        m_writer->write(p->partName(), p->blob());
        Relationships *rles = p->rels();
        if (rles->count() > 0)
            m_writer->write(p->partName().relsUriStr(), rles->blob());
    }
}

PackageWriter::~PackageWriter()
{
    delete m_writer;
}
}
