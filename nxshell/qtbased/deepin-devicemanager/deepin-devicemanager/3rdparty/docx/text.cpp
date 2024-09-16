// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"
#include "./oxml/oxmltext.h"
#include "./oxml/oxmlshape.h"
#include "./parts/documentpart.h"
#include "./shape.h"

#include <QDomDocument>
#include <QLoggingCategory>
#include <math.h>

namespace Docx {


Paragraph::Paragraph(DocumentPart *part, const QDomElement &element)
    : m_part(part)
{
    m_dom = part->m_dom;
    m_pEle = new QDomElement(element);
    m_style = new CT_PPr(this);

    // load exist run
    QDomNodeList reles = m_pEle->elementsByTagName(QStringLiteral("w:r"));
    if (!reles.isEmpty()) {
        for (int i = 0; i < reles.count(); i++) {
            QDomElement rele = reles.at(i).toElement();
            Run *run = new Run(m_part, m_pEle, rele);
            m_runs.append(run);
        }
    }
}

/*!
 * \brief 添加文本块
 * \param text
 * \param style
 * \return
 */
Run *Paragraph::addRun(const QString &text, const QString &style)    
{
    Run *run = new Run(m_part, m_pEle);
    if (!text.isEmpty())
        run->addText(text);
    if (!style.isEmpty())
        setStyle(style);

    m_runs.append(run);
    return run;
}

void Paragraph::addText(const QString &text)
{
    addRun(text);
}

/*!
 * \brief 所有字体
 * \return
 */
QString Paragraph::text() const
{
    QStringList str;
    for (const Run *r : m_runs) {
        str.append(r->text());
    }
    return str.join("");
}


/*!
 * \brief 设置样式
 * \param style
 */
void Paragraph::setStyle(const QString &style)
{
    m_style->setStyle(style);
}

/*!
 * \brief 对齐方式
 * \param align
 */
void Paragraph::setAlignment(WD_PARAGRAPH_ALIGNMENT align)
{
    m_style->setAlignment(align);
}

/*!
 * \brief 在本段落前添加段落
 * \param text
 * \param style
 * \return
 */
Paragraph *Paragraph::insertParagraphBefore(const QString &text, const QString &style)
{
    QDomElement pEle = m_dom->createElement(QStringLiteral("w:p"));
    Paragraph *p = new Paragraph(m_part, pEle);
    p->addRun(text, style);
    QDomNode parent = m_pEle->parentNode();

    parent.insertBefore(pEle, *m_pEle);
    return p;
}

Paragraph::~Paragraph()
{
    qDeleteAll(m_runs);
    delete m_pEle;
}

// End Paragraph


Run::Run(DocumentPart *part, QDomElement *parent)
    : m_part(part), m_parent(parent)
{
    m_dom = part->m_dom;
    m_rEle = m_dom->createElement(QStringLiteral("w:r"));
    m_parent->appendChild(m_rEle);
    QDomElement pPr = m_dom->createElement(QStringLiteral("w:pPr"));
    QDomElement jc = m_dom->createElement(QStringLiteral("w:jc"));
    jc.setAttribute("w:val","left");
    pPr.appendChild(jc);
    m_parent->appendChild(pPr);

    m_style = new CT_RPr(this);
}

Run::Run(DocumentPart *part, QDomElement *parent, const QDomElement &ele)
    : m_part(part), m_parent(parent)
{
    m_dom = part->m_dom;
    m_rEle = QDomElement(ele);
    m_style = new CT_RPr(this);
}

/*!
 * \brief 添加 Tab
 */
void Run::addTab()
{
    QDomElement tabEle = m_dom->createElement(QStringLiteral("w:tab"));
    m_rEle.appendChild(tabEle);
}

void Run::addBreak(WD_BREAK breakType)
{
    QDomElement brele = m_dom->createElement(QStringLiteral("w:br"));
    QString eletype, eleclear;
    switch (breakType) {
    case WD_BREAK::LINE: break;
    case WD_BREAK::PAGE: eletype = QStringLiteral("page"); break;
    case WD_BREAK::COLUMN: eletype = QStringLiteral("column"); break;
    case WD_BREAK::LINE_CLEAR_LEFT: eletype = QStringLiteral("textWrapping"); eleclear = QStringLiteral("left"); break;
    case WD_BREAK::LINE_CLEAR_RIGHT: eletype = QStringLiteral("textWrapping"); eleclear = QStringLiteral("right"); break;
    case WD_BREAK::LINE_CLEAR_ALL: eletype = QStringLiteral("textWrapping"); eleclear = QStringLiteral("all"); break;
    default: break;
    }
    m_rEle.appendChild(brele);
    if (!eletype.isEmpty())
        brele.setAttribute(QStringLiteral("w:type"), eletype);

    if (!eleclear.isEmpty())
        brele.setAttribute(QStringLiteral("w:clear"), eleclear);
}

/*!
 * \brief 添加文字
 * \param text
 */
void Run::addText(const QString &text)
{
    QDomElement tEle = m_dom->createElement(QStringLiteral("w:t"));
    tEle.appendChild(m_dom->createTextNode(text));
    m_rEle.appendChild(tEle);
    //m_rEle
}

QString Run::text() const
{
    return m_rEle.text();
}

/*!
 * \brief 返回 InlineShape
 * \param path
 * \param width
 * \param height
 * \return
 */
InlineShape *Run::addPicture(const QString &path, const Length &width, const Length &height)
{
    InlineShapes *ships = m_part->m_inlineshapes;
    InlineShape *picture = ships->addPicture(path, this);
    return scalePicture(picture, width, height);

}

InlineShape *Run::scalePicture(InlineShape *picture, const Length &width, const Length &height)
{
    if (!width.isEmpty() || !height.isEmpty()) {
        int lwidth = width.emu();
        int lheight = height.emu();

        int native_width = picture->width().emu();
        int native_height = picture->height().emu();
        if (width.isEmpty()) {
            float scaling_factor = float(lheight) / float(native_height);
            lwidth = int(round(native_width * scaling_factor));
        }
        if (height.isEmpty()) {
            float scaling_factor = float(lwidth) / float(native_width);
            lheight = int(round(native_height * scaling_factor));
        }
        picture->setWidth(lwidth);
        picture->setHeight(lheight);
    }    
    return picture;
}

InlineShape *Run::addPicture(const QImage &img, const Length &width, const Length &height)
{
    InlineShapes *ships = m_part->m_inlineshapes;
    InlineShape *picture = ships->addPicture(img, this);
    return scalePicture(picture, width, height);
}

/*!
 * \brief 设置样式
 * \param style
 */
void Run::setStyle(const QString &style)
{
    m_style->setStyle(style);
}

Run::~Run()
{
    delete m_style;
}

/*!
 * \brief 全部大写
 * \param isallcaps
 */
void Run::setAllcaps(bool isallcaps)
{
    m_style->setAllcaps(isallcaps);
}

/*!
 * \brief 小一号的大写
 * \param smallcpas
 */
void Run::setSmallcaps(bool smallcpas)
{
    m_style->setSmallcaps(smallcpas);
}

/*!
 * \brief 加粗
 * \param isbold
 */
void Run::setBold(bool isbold)
{
    m_style->setBold(isbold);
}

/*!
 * \brief 倾斜
 * \param isItalic
 */
void Run::setItalic(bool isItalic)
{
    m_style->setItalic(isItalic);
}

/*!
 * \brief 双划线
 * \param isDoubleStrike
 */
void Run::setDoubleStrike(bool isDoubleStrike)
{
    m_style->setDoubleStrike(isDoubleStrike);
}

/*!
 * \brief 阴影
 * \param shadow
 */
void Run::setShadow(bool shadow)
{
    m_style->setShadow(shadow);
}

/*!
 * \brief 下划线
 * \param underline
 */
void Run::setUnderLine(WD_UNDERLINE underline)
{
    m_style->setUnderLine(underline);
}

void Run::addDrawing(CT_Inline *imline)
{
    QDomElement drawing = m_dom->createElement(QStringLiteral("w:drawing"));

    m_rEle.appendChild(drawing);
    drawing.appendChild(imline->element());
}



}


