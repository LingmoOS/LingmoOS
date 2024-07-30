/*
    krichtextedit
    SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2008 Thomas McGuire <thomas.mcguire@gmx.net>
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KRICHTEXTEDIT_P_H
#define KRICHTEXTEDIT_P_H

#include "ktextedit_p.h"
#include "nestedlisthelper_p.h"

class KRichTextEditPrivate : public KTextEditPrivate
{
    Q_DECLARE_PUBLIC(KRichTextEdit)

public:
    explicit KRichTextEditPrivate(KRichTextEdit *qq)
        : KTextEditPrivate(qq)
        , nestedListHelper(new NestedListHelper(qq))
    {
    }

    ~KRichTextEditPrivate() override
    {
        delete nestedListHelper;
    }

    //
    // Normal functions
    //

    // If the text under the cursor is a link, the cursor's selection is set to
    // the complete link text. Otherwise selects the current word if there is no
    // selection.
    void selectLinkText() const;

    void init();

    // Switches to rich text mode and emits the mode changed signal if the
    // mode really changed.
    void activateRichText();

    // Applies formatting to the current word if there is no selection.
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    void setTextCursor(QTextCursor &cursor);

    // Data members
    KRichTextEdit::Mode mMode = KRichTextEdit::Plain;

    NestedListHelper *nestedListHelper;
};

#endif
