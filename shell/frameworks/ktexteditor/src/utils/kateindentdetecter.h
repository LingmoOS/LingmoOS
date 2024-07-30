/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KATE_INDENT_DETECTER_H
#define KATE_INDENT_DETECTER_H

namespace KTextEditor
{
class DocumentPrivate;
};

/**
 * File indentation detecter. Mostly ported from VSCode to here
 */
class KateIndentDetecter
{
public:
    struct Result {
        /**
         * If indentation is based on spaces (`insertSpaces` = true), then what is the number of spaces that make an indent?
         */
        int indentWidth = 4;
        /**
         * Is indentation based on spaces?
         */
        bool indentUsingSpaces = true;
    };

    KateIndentDetecter(KTextEditor::DocumentPrivate *doc);

    Result detect(int defaultTabSize, bool defaultInsertSpaces);

private:
    KTextEditor::DocumentPrivate *m_doc;
};

#endif
