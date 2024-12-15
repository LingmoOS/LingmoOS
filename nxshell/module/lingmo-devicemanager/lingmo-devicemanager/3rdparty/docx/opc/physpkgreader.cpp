// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "physpkgreader.h"
#include "packuri.h"
#include "constants.h"

#include <private/qzipreader_p.h>
#include <QByteArray>

namespace Docx {


PhysPkgReader::PhysPkgReader(const QString &filePath) :
    m_reader(new QZipReader(filePath))
{

}

PhysPkgReader::PhysPkgReader(QIODevice *device) :
    m_reader(new QZipReader(device))
{

}

// Return the '[Content_Types].xml' blob from the package
QByteArray PhysPkgReader::contentTypesData()
{
    return blobForm(Constants::CONTENT_TYPES_URI);
}

QByteArray PhysPkgReader::blobForm(const QString &packuri)
{
    return m_reader->fileData(packuri);
}

QByteArray PhysPkgReader::relsFrom(const PackURI &sourceUri)
{
    QString relstr = sourceUri.relsUriStr();
    return blobForm(relstr);
}

PhysPkgReader::~PhysPkgReader()
{

}

}
