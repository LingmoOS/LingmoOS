// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPART_H
#define IMAGEPART_H

#include "../opc/part.h"
#include "../image/image.h"
#include "../length.h"

namespace Docx {
class ImagePart : public Part
{
public:
    ImagePart(const PackURI &partName, const QString &contentType, const QByteArray &blob = QByteArray(), Image *image = nullptr, const QByteArray &hash = QByteArray());    QString fileName() const;
    Length defaultCx() const;
    Length defaultCy() const;
    static ImagePart *load(const PackURI &partName, const QString &contentType, const QByteArray &blob = QByteArray(), Image *image = nullptr);
    static ImagePart *fromImage(const PackURI &partName, Image *image, const QByteArray &hash = QByteArray());
    void afterUnmarshal();
    Image * image() const;
    QByteArray blob() const;
    QByteArray hash() const;

    ~ImagePart();   

private:
    Image *m_image;    
    QByteArray m_hash;        


};
}

#endif // IMAGEPART_H
