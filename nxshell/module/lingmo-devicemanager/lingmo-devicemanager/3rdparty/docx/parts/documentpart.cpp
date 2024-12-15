// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "documentpart.h"
#include "../text.h"
#include "../table.h"
#include "../opc/oxml.h"
#include "../shape.h"
#include "../oxml/oxmlshape.h"
#include "../package.h"
#include "imagepart.h"
#include "DDLog.h"

#include <QLoggingCategory>

using namespace DDLog;

namespace Docx
{

DocumentPart::DocumentPart(const QString &partName, const QString &contentType, const QByteArray &blob, Package *package)
    : Part(partName, contentType, QByteArray(), package)
{        
    m_dom = new QDomDocument();
    m_dom->setContent(blob);
    m_inlineshapes = new InlineShapes(this);
}

Paragraph *DocumentPart::addParagraph(const QString &text, const QString &style)
{
    QDomNode n = lastsectPr();//nodes.at(nodes.count() - 1);
    QDomNode parentNode = n.parentNode();

    QDomElement pEle = m_dom->createElement(QStringLiteral("w:p"));

    Paragraph *p = new Paragraph(this, pEle);

    if (!text.isEmpty())
        p->addRun(text, style);

    parentNode.insertBefore(pEle, n);

    m_addParagraphs.append(p);
    return p;
}

DocumentPart *DocumentPart::load(const PackURI &partName, const QString &contentType, const QByteArray &blob, Package *package)
{
    return new DocumentPart(partName, contentType, blob, package);
}

Table *DocumentPart::addTable(int rows, int cols, const QString &style)
{

    QDomNode n = lastsectPr();
    QDomNode parentNode = n.parentNode();

    QDomElement pEle = m_dom->createElement(QStringLiteral("w:tbl"));
    Table *table =  new Table(this, pEle);

    parentNode.insertBefore(pEle, n);

    for (int i = 0; i < cols; i++) {
        table->addColumn();
    }
    for (int i = 0; i < rows; i++) {
        table->addRow();
    }
    table->setStyle(style);
    m_addtables.append(table);
    return table;
}
void DocumentPart::afterUnmarshal()
{
    qCInfo(appLog) << "afetrUnmarshal";
}

QDomDocument *DocumentPart::element() const
{
    return m_dom;
}

QByteArray DocumentPart::blob() const
{
    return m_dom->toByteArray();
}

QPair<ImagePart *, QString> DocumentPart::getOrAddImagePart(const PackURI &imageDescriptor)
{
    ImageParts *imgs = m_package->imageparts();
    ImagePart *imagPart = imgs->getOrAddImagePart(imageDescriptor);

    return getOrAddImagePart(imagPart);
}

/*!
 * \brief 是否有相同的图片如果有则返回现有的，如果没有则新添加
 * \param img
 * \return
 */
QPair<ImagePart *, QString> DocumentPart::getOrAddImagePart(const QImage &img)
{
    ImageParts *imgs = m_package->imageparts();
    ImagePart *imagPart = imgs->getOrAddImagePart(img);

    return getOrAddImagePart(imagPart);
}

QPair<ImagePart *, QString> DocumentPart::getOrAddImagePart(ImagePart *imagPart)
{
    QString rId = relateTo(imagPart, Constants::IMAGE, QStringLiteral("word/"));
    return QPair<ImagePart *, QString>(imagPart, rId);
}

QList<Paragraph *> DocumentPart::paragraphs()
{
    qDeleteAll(m_ps);
    m_ps.clear();
    QDomNodeList pEles = m_dom->elementsByTagName(QStringLiteral("w:p"));
    if (pEles.isEmpty())
        return m_ps;

    for (int i = 0; i < pEles.count(); i++) {
        QDomElement pele = pEles.at(i).toElement();
        Paragraph *p = new Paragraph(this, pele);
        m_ps.append(p);
    }
    return m_ps;
}

QList<Table *> DocumentPart::tables()
{
    qDeleteAll(m_tables);
    m_tables.clear();
    QDomNodeList pEles = m_dom->elementsByTagName(QStringLiteral("w:tbl"));
    if (pEles.isEmpty())
        return m_tables;

    for (int i = 0; i < pEles.count(); i++) {
        QDomElement tblEle = pEles.at(i).toElement();
        Table *p = new Table(this, tblEle);
        m_tables.append(p);
    }
    return m_tables;
}

DocumentPart::~DocumentPart()
{
    delete m_inlineshapes;
    delete m_dom;    
    qDeleteAll(m_addParagraphs);
    m_addParagraphs.clear();

    qDeleteAll(m_addtables);
    m_addtables.clear();

    qDeleteAll(m_ps);
    m_ps.clear();

    qDeleteAll(m_tables);
    m_tables.clear();
}

int DocumentPart::nextId()
{
    QDomNodeList eles = m_dom->childNodes();
    QList<QString> numbers;
    findAttributes(eles, QStringLiteral("id"), &numbers);

    int size = numbers.count() + 2;
    for (int i = 1; i < size; i++) {
        if (!numbers.contains(QString::number(i)))
            return i;
    }
    return 1;
}

/*!
 * \brief DocumentPart::findAttributes
 * \param eles
 * \param attr
 * \param nums
 */
void DocumentPart::findAttributes(const QDomNodeList &eles, const QString &attr, QList<QString> *nums)
{
    for (int i = 0; i < eles.count(); i++) {
        QString num = eles.at(i).toElement().attribute(attr);
        if (!nums->contains(num))
            nums->append(num);
        findAttributes(eles.at(i).childNodes(), attr, nums);
    }
}

QDomNode DocumentPart::lastsectPr() const
{
    QDomNodeList nodes = m_dom->elementsByTagName(QStringLiteral("w:sectPr"));

    QDomNode n = nodes.at(nodes.count() - 1);
    return n;
}

InlineShapes::InlineShapes(DocumentPart *part)
    : m_part(part)
{
    m_dom = part->m_dom;
}

InlineShapes::~InlineShapes()
{

}

InlineShape *InlineShapes::addPicture(const QString &imagePath, Run *run)
{

    QPair<ImagePart *, QString> imagePartAndId = m_part->getOrAddImagePart(imagePath);
    return addPicture(imagePartAndId, run);
}

InlineShape *InlineShapes::addPicture(const QImage &img, Run *run)
{
    QPair<ImagePart *, QString> imagePartAndId = m_part->getOrAddImagePart(img);
    return addPicture(imagePartAndId, run);
}

InlineShape *InlineShapes::addPicture(const QPair<ImagePart *, QString> &imagePartAndId, Run *run)
{
    int shapeId = m_part->nextId();
    //InlineShape *picture = new InlineShape()
    CT_Picture *pic = new CT_Picture(m_part->m_dom, QStringLiteral("0")
                                   , imagePartAndId.first->fileName()
                                   , imagePartAndId.second
                                   , imagePartAndId.first->defaultCx().emu()
                                   , imagePartAndId.first->defaultCy().emu());

    CT_Inline *lnLine = new CT_Inline(m_part->m_dom
                                      , imagePartAndId.first->defaultCx().emu()
                                      , imagePartAndId.first->defaultCy().emu()
                                      , QString::number(shapeId)
                                      , pic
                                      );
    run->addDrawing(lnLine);
    InlineShape *picture = new InlineShape(lnLine);
    return picture;
}


}


