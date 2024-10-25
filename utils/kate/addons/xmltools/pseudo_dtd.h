/***************************************************************************
   pseudoDtd.cpp
   copyright            : (C) 2001-2002 by Daniel Naber
   email                : daniel.naber@t-online.de
***************************************************************************/

/***************************************************************************
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or ( at your option ) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

#pragma once

#include <QProgressDialog>
#include <map>
#include <qdom.h>

/**
 * This class contains the attributes for one element.
 * To get ALL attributes, concatenate the two lists.
 */
class ElementAttributes
{
public:
    QStringList optionalAttributes;
    QStringList requiredAttributes;
};

class PseudoDTD
{
public:
    PseudoDTD();
    ~PseudoDTD();

    void analyzeDTD(QString &metaDtdUrl, QString &metaDtd);

    QStringList allowedElements(const QString &parentElement);
    QStringList allowedAttributes(const QString &parentElement);
    QStringList attributeValues(const QString &element, const QString &attribute);
    QStringList entities(const QString &start);
    QStringList requiredAttributes(const QString &parentElement) const;

protected:
    bool parseElements(QDomDocument *doc, QProgressDialog *progress);
    bool parseAttributes(QDomDocument *doc, QProgressDialog *progress);
    bool parseAttributeValues(QDomDocument *doc, QProgressDialog *progress);
    bool parseEntities(QDomDocument *doc, QProgressDialog *progress);

    bool m_sgmlSupport;

    // Entities, e.g. <"nbsp", "160">
    std::map<QString, QString> m_entityList;
    // Elements, e.g. <"a", ( "b", "i", "em", "strong" )>
    std::map<QString, QStringList> m_elementsList;
    // Attributes e.g. <"a", ( "href", "lang", "title" )>
    std::map<QString, ElementAttributes> m_attributesList;
    // Attribute values e.g. <"td", <"align", ( "left", "right", "justify" )>>
    std::map<QString, std::map<QString, QStringList>> m_attributevaluesList;
};

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
