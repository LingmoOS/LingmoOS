// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shape.h"
#include "./oxml/oxmlshape.h"
#include "text.h"
#include "./parts/imagepart.h"

namespace Docx {

InlineShape::InlineShape(CT_Inline *inlinev)
    : m_inline(inlinev)
{

}

Length InlineShape::width() const
{
    QString value = m_inline->m_extent.attribute(QStringLiteral("cx"), "0");
    return Length(value.toInt());
}

void InlineShape::setWidth(const Length &width)
{
    m_inline->m_extent.setAttribute(QStringLiteral("cx"), QString::number(width.emu()));
}

Length InlineShape::height() const
{
    QString value = m_inline->m_extent.attribute(QStringLiteral("cy"), "0");
    return Length(value.toInt());
}

void InlineShape::setHeight(const Length &height)
{
    m_inline->m_extent.setAttribute(QStringLiteral("cy"), QString::number(height.emu()));
}

InlineShape::~InlineShape()
{

}



}
