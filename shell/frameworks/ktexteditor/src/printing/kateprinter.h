/*
    SPDX-FileCopyrightText: 2002-2010 Anders Lund <anders@alweb.dk>

    Rewritten based on code of:
    SPDX-FileCopyrightText: 2002 Michael Goffioul <kdeprint@swing.be>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_PRINTER_H
#define KATE_PRINTER_H

namespace KTextEditor
{
class DocumentPrivate;
class ViewPrivate;
}

namespace KatePrinter
{
/**
 * Launches print dialog for specified @view
 * @returns true if document was successfully printed
 */
bool print(KTextEditor::ViewPrivate *view);

/**
 * Launches print preview dialog for specified @view
 * @returns true if document was printed
 */
bool printPreview(KTextEditor::ViewPrivate *view);

/**
 * Overloaded print function for document
 * Useful when there is no view for the document. Consequently this function
 * cannot print only selected portion of document.
 */
bool print(KTextEditor::DocumentPrivate *doc);

/**
 * Overloaded print function for document
 * Useful when there is no view for the document. Consequently this function
 * cannot print only selected portion of document.
 */
bool printPreview(KTextEditor::DocumentPrivate *doc);
}

#endif
