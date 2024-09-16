// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OXMLDOCXTEXT_H
#define OXMLDOCXTEXT_H

#include "xmlchemy.h"
#include "../enums/enumtext.h"

namespace Docx {
class Run;
class Paragraph;


/*!
 * \brief 段落的属性
 */
class CT_PPr
{
public:
    CT_PPr(Paragraph *paragraph);
    void setStyle(const QString &style = QString());
    void setAlignment(WD_PARAGRAPH_ALIGNMENT align);
    void addOrAssignStyle();
    virtual ~CT_PPr();


    void loadExistStyle();
private:
    QDomElement m_style;
    QDomElement m_pStyle;
    Paragraph *m_paragraph;

};


class CT_RPr
{
public:
    CT_RPr(Run *run);
    void setStyle(const QString &style = QString());
    void setBold(bool bold);
    void setAllcaps(bool isallcaps);
    void setSmallcaps(bool issmallcpas);
    void setItalic(bool italic);
    void setDoubleStrike(bool isDoubleStrike);
    void setShadow(bool shadow = true);
    void setUnderLine(WD_UNDERLINE underline);
    virtual ~CT_RPr();    

    void loadExistStyle();
private:
    void addOrAssignStyle();

    /*!
     * \brief 设置样式子节点属性
     * \param eleName
     * \param enable
     */
    void addOrAssignStyleChildElement(const QString &eleName, bool enable);

private:
    Run *m_run;
    QDomElement m_style;
    QDomElement m_rStyle;
};

}

#endif // TEXT_H
