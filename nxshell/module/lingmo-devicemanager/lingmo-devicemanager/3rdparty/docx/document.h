// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "docx_global.h"
#include "length.h"

#include <QString>
#include <QImage>

class QIODevice;

namespace Docx {
class Paragraph;
class Table;
class DocumentPart;
class Package;
class InlineShape;

class DOCX_EXPORT Document
{
public:
    Document();
    explicit Document(const QString& name);    
    explicit Document(QIODevice* device);

    Paragraph *addParagraph(const QString &text = QString(), const QString &style = QString());

    Paragraph *addHeading(const QString &text = QString(), int level = 1);

    Table *addTable(int rows, int cols);

    InlineShape *addPicture(const QString &imgPath, const Length &width = Length(), const Length &height = Length());
    InlineShape *addPicture(const QImage &img, const Length &width = Length(), const Length &height = Length());
    Paragraph *addPageBreak();
    QList<Paragraph*> paragraphs();
    QList<Table*> tables();
    virtual ~Document();

    void save(const QString& path);

private:
    void open(const QString& name);
    void open(QIODevice* device);

private:
    DocumentPart *m_docPart;
    Package *m_package;

};

}


#endif // DOCUMENT_H
