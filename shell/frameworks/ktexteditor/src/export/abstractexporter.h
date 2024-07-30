/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ABSTRACTEXPORTER_H
#define ABSTRACTEXPORTER_H

#include <QTextStream>

#include <ktexteditor/attribute.h>
#include <ktexteditor/document.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>

class AbstractExporter
{
public:
    /// If \p m_encapsulate is set, you should add some kind of header in the ctor
    /// to \p m_output.
    AbstractExporter(KTextEditor::View *view, QTextStream &output, const bool encapsulate = false)
        : m_view(view)
        , m_output(output)
        , m_encapsulate(encapsulate)
        , m_defaultAttribute(nullptr)
    {
        QColor defaultBackground;
        QVariant variant = m_view->configValue(QStringLiteral("background-color"));
        if (variant.canConvert<QColor>()) {
            defaultBackground = variant.value<QColor>();
        }

        m_defaultAttribute = view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
        m_defaultAttribute->setBackground(QBrush(defaultBackground));
    }

    /// Gets called after everything got exported.
    /// Hence, if \p m_encapsulate is set, you should probably add some kind of footer here.
    virtual ~AbstractExporter()
    {
    }

    /// Begin a new line.
    virtual void openLine() = 0;

    /// Finish the current line.
    virtual void closeLine(const bool lastLine) = 0;

    /// Export \p text with given text attribute \p attrib.
    virtual void exportText(const QString &text, const KTextEditor::Attribute::Ptr &attrib) = 0;

protected:
    KTextEditor::View *m_view;
    QTextStream &m_output;
    bool m_encapsulate;
    KTextEditor::Attribute::Ptr m_defaultAttribute;
};

#endif
