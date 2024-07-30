/*
    SPDX-FileCopyrightText: 2017 Grzegorz Szymaszek <gszymaszek@short.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _EDITORCONFIG_H_
#define _EDITORCONFIG_H_

#include <QLatin1String>

#include <editorconfig/editorconfig.h>
#include <editorconfig/editorconfig_handle.h>

class KateDocumentConfig;

namespace KTextEditor
{
class DocumentPrivate;
}

class EditorConfig
{
public:
    explicit EditorConfig(KTextEditor::DocumentPrivate *document);
    ~EditorConfig();
    EditorConfig(const EditorConfig &) = delete;
    EditorConfig &operator=(const EditorConfig &) = delete;
    /**
     * Runs EditorConfig parser and sets proper parent DocumentPrivate
     * configuration. Implemented options: charset, end_of_line, indent_size,
     * indent_style, insert_final_newline, max_line_length, tab_width,
     * trim_trailing_whitespace.
     *
     * @see https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties
     *
     * @return 0 if EditorConfig library reported successful file parsing,
     * positive integer if a parsing error occurred (the return value would be
     * the line number of parsing error), negative integer if another error
     * occurred. In other words, returns value returned by editorconfig_parse.
     */
    int parse();

private:
    KTextEditor::DocumentPrivate *m_document;
    editorconfig_handle m_handle;
};

#endif
