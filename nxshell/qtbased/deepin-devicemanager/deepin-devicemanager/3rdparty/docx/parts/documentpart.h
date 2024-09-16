// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUMENTPART_H
#define DOCUMENTPART_H

#include "../docx_global.h"
#include "../opc/part.h"

#include <QPair>
#include <QDomDocument>
#include <QByteArray>
#include <QImage>

namespace Docx {
//class Body;
class CT_Default;
class Paragraph;
class Run;
class Table;
class InlineShapes;
class ImagePart;
class ImageParts;
class InlineShape;

class DocumentPart : public Part
{
public:
    DocumentPart(const QString &partName, const QString &contentType, const QByteArray &blob = QByteArray(), Package *package = nullptr);
    Paragraph* addParagraph(const QString &text, const QString &style = QStringLiteral(""));
    static DocumentPart *load(const PackURI &partName, const QString &contentType, const QByteArray &blob = QByteArray(), Package *package = nullptr);
    Table *addTable(int rows, int cols, const QString &style = QString::fromLatin1("TableGrid"));
    void afterUnmarshal();
    QDomDocument * element() const;
    QByteArray blob() const;
    QPair<ImagePart *, QString> getOrAddImagePart(const PackURI &imageDescriptor);
    QPair<ImagePart *, QString> getOrAddImagePart(const QImage &img);
    QPair<ImagePart *, QString> getOrAddImagePart(ImagePart *imagPart);
    QList<Paragraph*> paragraphs();
    QList<Table*> tables();
    int nextId();
    virtual ~DocumentPart();

private:
    void findAttributes(const QDomNodeList &eles, const QString &attr, QList<QString> *nums);
    QDomNode lastsectPr() const;

private:
    //Body *m_body;
    QDomDocument *m_dom;
    InlineShapes *m_inlineshapes;

    // add
    QList<Paragraph *> m_addParagraphs;
    QList<Table *> m_addtables;

    // load
    QList<Paragraph *> m_ps;
    QList<Table *> m_tables;

    friend class Paragraph;
    friend class Run;
    friend class InlineShapes;
    friend class Table;

};

class InlineShapes
{
public:
    InlineShapes(DocumentPart *part);
    ~InlineShapes();

    InlineShape *addPicture(const QString &imagePath, Run *run);
    InlineShape *addPicture(const QImage &img, Run *run);

private:
    InlineShape *addPicture(const QPair<ImagePart *, QString> &imagePartAndId, Run *run);

private:
    DocumentPart *m_part;
    QDomDocument *m_dom;
    friend class InlineShape;
};

}

#endif // DOCUMENTPART_H
