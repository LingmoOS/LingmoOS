// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCXTEXT_H
#define DOCXTEXT_H

#include "docx_global.h"
#include "length.h"
#include "shared.h"
#include "./enums/enumtext.h"

#include <QString>
#include <QDomElement>
#include <QList>
#include <QImage>

class QDomDocument;

namespace Docx {
class Run;
class Text;
class CT_RPr;
class CT_PPr;
class DocumentPart;
class InlineShape;
class CT_Inline;

class DOCX_EXPORT Paragraph
{
public:

    Paragraph(DocumentPart *part, const QDomElement &element);

    Run* addRun(const QString &text = QString(), const QString &style = QString());

    void addText(const QString &text);

    QString text() const;

    void setStyle(const QString &style);

    void setAlignment(WD_PARAGRAPH_ALIGNMENT align);

    Paragraph* insertParagraphBefore(const QString &text, const QString &style = QString());

    virtual ~Paragraph();

private:
    DocumentPart *m_part;
    QDomDocument *m_dom;
    QDomElement *m_pEle;
    QList<Run *> m_runs;
    CT_PPr *m_style;
    friend class CT_PPr;

};

class DOCX_EXPORT Run
{
public:    
    Run(DocumentPart *part, QDomElement *parent);
    Run(DocumentPart *part, QDomElement *parent, const QDomElement &ele);
    virtual ~Run();

    void addTab();
    void addBreak(WD_BREAK breakType = WD_BREAK::PAGE);

    void addText(const QString &text);

    QString text() const;

    InlineShape* addPicture(const QString &path, const Length &width = Length(), const Length &height = Length());
    InlineShape* addPicture(const QImage &img, const Length &width = Length(), const Length &height = Length());

    void setStyle(const QString &style);

    void setAllcaps(bool isallcaps = true);

    void setSmallcaps(bool smallcpas = true);

    void setBold(bool isbold = true);

    void setItalic(bool isItalic = true);

    void setDoubleStrike(bool isDoubleStrike = true);

    void setShadow(bool shadow = true);

    void setUnderLine(WD_UNDERLINE underline);

    void addDrawing(CT_Inline *imline);

private:
    InlineShape* scalePicture(InlineShape *picture, const Length &width, const Length &height);

private:

    QString m_text;
    DocumentPart *m_part;
    QDomDocument *m_dom;
    QDomElement m_rEle;
    QDomElement *m_parent;
    CT_RPr *m_style = nullptr;
    friend class CT_RPr;

};


}

#endif // TEXT_H
