/*
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATERENDERRANGE_H
#define KATERENDERRANGE_H

#include <ktexteditor/attribute.h>
#include <ktexteditor/range.h>

#include <utility>
#include <vector>

class NormalRenderRange
{
public:
    void addRange(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute);

    KTextEditor::Cursor nextBoundary() const;
    bool advanceTo(const KTextEditor::Cursor pos);
    KTextEditor::Attribute::Ptr currentAttribute() const;

private:
    std::vector<std::pair<KTextEditor::Range, KTextEditor::Attribute::Ptr>> m_ranges;
    KTextEditor::Cursor m_nextBoundary;
    KTextEditor::Attribute::Ptr m_currentAttribute;
    size_t m_currentRange = 0;
};

class RenderRangeVector
{
public:
    KTextEditor::Cursor nextBoundary() const;
    void advanceTo(const KTextEditor::Cursor pos);
    bool hasAttribute() const;
    KTextEditor::Attribute::Ptr generateAttribute() const;
    NormalRenderRange &pushNewRange();
    bool isEmpty() const
    {
        return m_ranges.empty();
    }
    void reserve(size_t size)
    {
        m_ranges.reserve(size);
    }

private:
    std::vector<NormalRenderRange> m_ranges;
    KTextEditor::Cursor m_currentPos;
};

#endif
