/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateabstractinputmode.h"
#include "kateviewinternal.h"

KateAbstractInputMode::KateAbstractInputMode(KateViewInternal *viewInternal)
    : m_viewInternal(viewInternal)
    , m_view(viewInternal->view())
{
}

KateLayoutCache *KateAbstractInputMode::layoutCache() const
{
    return m_viewInternal->cache();
}

void KateAbstractInputMode::updateCursor(const KTextEditor::Cursor c)
{
    m_viewInternal->updateCursor(c);
}

int KateAbstractInputMode::linesDisplayed() const
{
    return m_viewInternal->linesDisplayed();
}

void KateAbstractInputMode::scrollViewLines(int offset)
{
    return m_viewInternal->scrollViewLines(offset);
}
