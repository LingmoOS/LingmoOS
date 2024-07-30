/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HTMLEXPORTER_H
#define HTMLEXPORTER_H

#include "abstractexporter.h"

/// TODO: add abstract interface for future exporters
class HTMLExporter : public AbstractExporter
{
public:
    HTMLExporter(KTextEditor::View *view, QTextStream &output, const bool withHeaderFooter = false);
    ~HTMLExporter() override;

    void openLine() override;
    void closeLine(const bool lastLine) override;
    void exportText(const QString &text, const KTextEditor::Attribute::Ptr &attrib) override;
};

#endif
