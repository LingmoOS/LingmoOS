/*
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katerenderrange.h"

#include <KColorUtils>

void mergeAttributes(KTextEditor::Attribute::Ptr base, KTextEditor::Attribute::Ptr add)
{
    if (!add) {
        return;
    }

    bool hadBg = base->hasProperty(KTextEditor::Attribute::BackgroundBrush);
    bool hasBg = add->hasProperty(KTextEditor::Attribute::BackgroundBrush);

    bool hadFg = base->hasProperty(KTextEditor::Attribute::ForegroundBrush);
    bool hasFg = add->hasProperty(KTextEditor::Attribute::ForegroundBrush);

    if (((!hadBg || !hasBg) && (!hadFg || !hasFg))) {
        // Nothing to blend
        *base += *add;
        return;
    }

    // We eventually have to blend

    QBrush baseBgBrush;
    QBrush baseFgBrush;

    if (hadBg) {
        baseBgBrush = base->background();
    }

    if (hadFg) {
        baseFgBrush = base->foreground();
    }

    *base += *add;

    if (hadBg && hasBg) {
        QBrush bg = add->background();
        if (!bg.isOpaque()) {
            QColor mixWithColor = bg.color();
            mixWithColor.setAlpha(255);
            bg.setColor(KColorUtils::mix(baseBgBrush.color(), mixWithColor, bg.color().alphaF()));
            base->setBackground(bg);
        }
    }
    if (hadFg && hasFg) {
        QBrush fg = add->foreground();
        if (!fg.isOpaque()) {
            QColor mixWithColor = fg.color();
            mixWithColor.setAlpha(255);
            fg.setColor(KColorUtils::mix(baseFgBrush.color(), mixWithColor, fg.color().alphaF()));
            base->setForeground(fg);
        }
    }
}

void NormalRenderRange::addRange(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute)
{
    m_ranges.emplace_back(range, std::move(attribute));
}

KTextEditor::Cursor NormalRenderRange::nextBoundary() const
{
    return m_nextBoundary;
}

bool NormalRenderRange::advanceTo(const KTextEditor::Cursor pos)
{
    size_t index = m_currentRange;
    while (index < m_ranges.size()) {
        const auto &p = m_ranges[index];
        const auto &r = p.first;
        if (r.end() <= pos) {
            ++index;
        } else {
            bool ret = index != m_currentRange;
            m_currentRange = index;

            if (r.start() > pos) {
                m_nextBoundary = r.start();
            } else {
                m_nextBoundary = r.end();
            }
            if (r.contains(pos)) {
                m_currentAttribute = p.second;
            } else {
                m_currentAttribute.reset();
            }

            return ret;
        }
    }

    m_nextBoundary = KTextEditor::Cursor(INT_MAX, INT_MAX);
    m_currentAttribute.reset();
    return false;
}

KTextEditor::Attribute::Ptr NormalRenderRange::currentAttribute() const
{
    return m_currentAttribute;
}

KTextEditor::Cursor RenderRangeVector::nextBoundary() const
{
    KTextEditor::Cursor ret = m_currentPos;
    bool first = true;
    for (auto &r : m_ranges) {
        if (first) {
            ret = r.nextBoundary();
            first = false;

        } else {
            KTextEditor::Cursor nb = r.nextBoundary();
            if (ret > nb) {
                ret = nb;
            }
        }
    }
    return ret;
}

NormalRenderRange &RenderRangeVector::pushNewRange()
{
    m_ranges.push_back(NormalRenderRange());
    return m_ranges.back();
}

void RenderRangeVector::advanceTo(const KTextEditor::Cursor pos)
{
    for (auto &r : m_ranges) {
        r.advanceTo(pos);
    }
}

bool RenderRangeVector::hasAttribute() const
{
    for (auto &r : m_ranges) {
        if (r.currentAttribute()) {
            return true;
        }
    }

    return false;
}

KTextEditor::Attribute::Ptr RenderRangeVector::generateAttribute() const
{
    KTextEditor::Attribute::Ptr a;
    bool ownsAttribute = false;

    for (auto &r : m_ranges) {
        if (KTextEditor::Attribute::Ptr a2 = r.currentAttribute()) {
            if (!a) {
                a = a2;
            } else {
                if (!ownsAttribute) {
                    // Make an own copy of the attribute..
                    ownsAttribute = true;
                    a = new KTextEditor::Attribute(*a);
                }
                mergeAttributes(a, a2);
            }
        }
    }

    return a;
}
