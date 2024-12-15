// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OXMLSHAPE_H
#define OXMLSHAPE_H

#include <QString>
#include <QDomDocument>

namespace Docx {


// <pic:pic>
class CT_Picture
{
public:
    CT_Picture(QDomDocument *dom, const QString &picId, const QString &fileName, const QString &rId, int cx, int cy);
    QDomElement element() const;
    ~CT_Picture();

private:
    void createElement(const QString &picId, const QString &fileName, const QString &rId, int cx, int cy);

private:    
    QDomDocument *m_dom;
    QDomElement m_picEle;

};

// <w:inline>
class CT_Inline
{
public:
    CT_Inline(QDomDocument *dom, double cx, double cy, const QString &shapeId, CT_Picture *pic);
    QDomElement element() const;
    virtual ~CT_Inline();

private:
    void createInlineXml(double cx, double cy, const QString &shapeId);

private:
    QDomElement m_inlineEle;
    CT_Picture *m_pic;
    QDomDocument *m_dom;
    QDomElement m_graphicData;
    QDomElement m_extent;
    friend class InlineShape;
};
}
#endif // OXMLSHAPE_H
