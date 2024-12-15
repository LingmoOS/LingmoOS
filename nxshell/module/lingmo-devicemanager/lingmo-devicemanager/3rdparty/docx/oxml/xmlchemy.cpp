// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xmlchemy.h"

namespace Docx {



BaseChildElement::BaseChildElement(const QString &nsptagname, const QString &successors)
    : m_nsptagname(nsptagname), m_successors(successors)
{

}

BaseChildElement::~BaseChildElement()
{

}



ZeroOrOne::ZeroOrOne(const QString &nsptagname, const QString &successors)
    : BaseChildElement(nsptagname, successors)
{

}

ZeroOrOne::~ZeroOrOne()
{

}


// OxmlElementBase

OxmlElementBase::OxmlElementBase(): m_element()
{

}

OxmlElementBase::OxmlElementBase(QDomElement *x)
    : m_element(x)
{

}

//void OxmlElementBase::insertElementBefore(QDomElement *elm, const QString &tagname)
//{
////    QDomElement child = this->firstChildElement(tagname);
////    if (child.isElement()) {
////        this->insertBefore(*elm, child);
////    } else
////        this->appendChild(*elm);

//}

OxmlElementBase::~OxmlElementBase()
{

}

}

