/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXPORTERPLUGINVIEW_H
#define EXPORTERPLUGINVIEW_H

#include <KTextEditor/View>

#include <QTextStream>

class KateExporter
{
public:
    explicit KateExporter(KTextEditor::View *view)
        : m_view(view)
    {
    }

    void exportToClipboard();
    void exportToFile(const QString &file);

private:
    /// TODO: maybe make this scriptable for additional exporters?
    void exportData(const bool useSelction, QTextStream &output);

private:
    KTextEditor::View *m_view;
};

#endif
