// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oxmlshape.h"

namespace Docx {

CT_Picture::CT_Picture(QDomDocument *dom, const QString &picId, const QString &fileName, const QString &rId, int cx, int cy)
    : m_dom(dom)
{
    createElement(picId, fileName, rId, cx, cy);
}

QDomElement CT_Picture::element() const
{
    return m_picEle;
}

void CT_Picture::createElement(const QString &picId, const QString &fileName, const QString &rId, int cx, int cy)
{
    m_picEle = m_dom->createElement(QStringLiteral("pic:pic"));
    // nvPicPr
    QDomElement nvPicPr = m_dom->createElement(QStringLiteral("pic:nvPicPr"));
    m_picEle.appendChild(nvPicPr);

    // nvPicPr -cNvPr
    QDomElement cNvPr = m_dom->createElement(QStringLiteral("pic:cNvPr"));
    nvPicPr.appendChild(cNvPr);
    cNvPr.setAttribute(QStringLiteral("id"), picId);
    cNvPr.setAttribute(QStringLiteral("name"), fileName);

    // nvPicPr -cNvPicPr
    QDomElement cNvPicPr = m_dom->createElement(QStringLiteral("pic:cNvPicPr"));
    nvPicPr.appendChild(cNvPicPr);

    // blipFill
    QDomElement blipFill = m_dom->createElement(QStringLiteral("pic:blipFill"));
    m_picEle.appendChild(blipFill);

    // blipFill -blip
    QDomElement blip = m_dom->createElement(QStringLiteral("a:blip"));
    blipFill.appendChild(blip);
    blip.setAttribute(QStringLiteral("r:embed"), rId);

    // blipFill -stretch
    QDomElement stretch = m_dom->createElement(QStringLiteral("a:stretch"));
    blipFill.appendChild(stretch);

    QDomElement fillRect = m_dom->createElement(QStringLiteral("a:fillRect"));
    stretch.appendChild(fillRect);

    //spPr
    QDomElement spPr = m_dom->createElement(QStringLiteral("pic:spPr"));
    m_picEle.appendChild(spPr);

    QDomElement xfrm = m_dom->createElement(QStringLiteral("a:xfrm"));
    spPr.appendChild(xfrm);

    QDomElement off = m_dom->createElement(QStringLiteral("a:off"));
    xfrm.appendChild(off);
    off.setAttribute(QStringLiteral("x"), 0);
    off.setAttribute(QStringLiteral("y"), 0);

    QDomElement ext = m_dom->createElement(QStringLiteral("a:ext"));
    xfrm.appendChild(ext);
    ext.setAttribute(QStringLiteral("cx"), cx);
    ext.setAttribute(QStringLiteral("cy"), cy);

    QDomElement prstGeom = m_dom->createElement(QStringLiteral("a:prstGeom"));
    spPr.appendChild(prstGeom);
    prstGeom.setAttribute(QStringLiteral("prst"), QStringLiteral("rect"));

}

CT_Picture::~CT_Picture()
{

}


CT_Inline::CT_Inline(QDomDocument *dom, double cx, double cy, const QString &shapeId, CT_Picture *pic)
    :  m_pic(pic), m_dom(dom)
{
    createInlineXml(cx, cy, shapeId);
}

QDomElement CT_Inline::element() const
{
    return m_inlineEle;
}

CT_Inline::~CT_Inline()
{
    delete m_pic;
}

void CT_Inline::createInlineXml(double cx, double cy, const QString &shapeId)
{
    m_inlineEle = m_dom->createElement(QStringLiteral("wp:inline"));
    m_inlineEle.setAttribute(QStringLiteral("xmlns:a"), QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"));
    m_inlineEle.setAttribute(QStringLiteral("xmlns:pic"), QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/picture"));

    m_extent = m_dom->createElement(QStringLiteral("wp:extent"));
    m_inlineEle.appendChild(m_extent);
    m_extent.setAttribute(QStringLiteral("cx"), cx);
    m_extent.setAttribute(QStringLiteral("cy"), cy);

    QDomElement docPr = m_dom->createElement(QStringLiteral("wp:docPr"));
    m_inlineEle.appendChild(docPr);
    docPr.setAttribute(QStringLiteral("id"), shapeId);
    docPr.setAttribute(QStringLiteral("name"), QString("Picture %1").arg(shapeId));

    //cNvGraphicFramePr
    QDomElement cNvGraphicFramePr = m_dom->createElement(QStringLiteral("wp:cNvGraphicFramePr"));
    m_inlineEle.appendChild(cNvGraphicFramePr);

    QDomElement graphicFrameLocks = m_dom->createElement(QStringLiteral("a:graphicFrameLocks"));
    cNvGraphicFramePr.appendChild(graphicFrameLocks);
    graphicFrameLocks.setAttribute(QStringLiteral("noChangeAspect"), 1);

    //graphic
    QDomElement graphic = m_dom->createElement(QStringLiteral("a:graphic"));
    m_inlineEle.appendChild(graphic);

    // m_graphicData
    m_graphicData = m_dom->createElement(QStringLiteral("a:graphicData"));
    graphic.appendChild(m_graphicData);
    m_graphicData.setAttribute(QStringLiteral("uri"), QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/picture"));

    m_graphicData.appendChild(m_pic->element());
}

}

