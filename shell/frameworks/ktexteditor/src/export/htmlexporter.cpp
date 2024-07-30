/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "htmlexporter.h"

#include <ktexteditor/document.h>

#include <QTextDocument>

static QString toHtmlRgbaString(const QColor &color)
{
    if (color.alpha() == 0xFF) {
        return color.name();
    }

    QString rgba = QStringLiteral("rgba(");
    rgba.append(QString::number(color.red()));
    rgba.append(QLatin1Char(','));
    rgba.append(QString::number(color.green()));
    rgba.append(QLatin1Char(','));
    rgba.append(QString::number(color.blue()));
    rgba.append(QLatin1Char(','));
    // this must be alphaF
    rgba.append(QString::number(color.alphaF()));
    rgba.append(QLatin1Char(')'));
    return rgba;
}

HTMLExporter::HTMLExporter(KTextEditor::View *view, QTextStream &output, const bool encapsulate)
    : AbstractExporter(view, output, encapsulate)
{
    if (m_encapsulate) {
        // let's write the HTML header :
        m_output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        m_output << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">\n";
        m_output << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
        m_output << "<head>\n";
        m_output << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
        m_output << "<meta name=\"Generator\" content=\"Kate, the KDE Advanced Text Editor\" />\n";
        // for the title, we write the name of the file (/usr/local/emmanuel/myfile.cpp -> myfile.cpp)
        m_output << "<title>" << view->document()->documentName() << "</title>\n";
        m_output << "</head>\n";

        // tell in comment which highlighting was used!
        m_output << "<!-- Highlighting: \"" << view->document()->highlightingMode() << "\" -->\n";

        m_output << "<body>\n";
    }

    if (!m_defaultAttribute) {
        m_output << "<pre>\n";
    } else {
        m_output << QStringLiteral("<pre style='%1%2%3%4'>")
                        .arg(m_defaultAttribute->fontBold() ? QStringLiteral("font-weight:bold;") : QString())
                        .arg(m_defaultAttribute->fontItalic() ? QStringLiteral("font-style:italic;") : QString())
                        .arg(QLatin1String("color:") + toHtmlRgbaString(m_defaultAttribute->foreground().color()) + QLatin1Char(';'))
                        .arg(QLatin1String("background-color:") + toHtmlRgbaString(m_defaultAttribute->background().color()) + QLatin1Char(';'))
                 << '\n';
    }
    m_output.flush();
}

HTMLExporter::~HTMLExporter()
{
    m_output << "</pre>\n";

    if (m_encapsulate) {
        m_output << "</body>\n";
        m_output << "</html>\n";
    }
    m_output.flush();
}

void HTMLExporter::openLine()
{
}

void HTMLExporter::closeLine(const bool lastLine)
{
    if (!lastLine) {
        // we are inside a <pre>, so a \n is a new line
        m_output << "\n";
        m_output.flush();
    }
}

void HTMLExporter::exportText(const QString &text, const KTextEditor::Attribute::Ptr &attrib)
{
    if (!attrib || !attrib->hasAnyProperty() || attrib == m_defaultAttribute) {
        m_output << text.toHtmlEscaped();
        return;
    }

    if (attrib->fontBold()) {
        m_output << "<b>";
    }
    if (attrib->fontItalic()) {
        m_output << "<i>";
    }

    bool writeForeground = attrib->hasProperty(QTextCharFormat::ForegroundBrush)
        && (!m_defaultAttribute || attrib->foreground().color() != m_defaultAttribute->foreground().color());
    bool writeBackground = attrib->hasProperty(QTextCharFormat::BackgroundBrush)
        && (!m_defaultAttribute || attrib->background().color() != m_defaultAttribute->background().color());

    if (writeForeground || writeBackground) {
        m_output << QStringLiteral("<span style='%1%2'>")
                        .arg(writeForeground ? QString(QLatin1String("color:") + toHtmlRgbaString(attrib->foreground().color()) + QLatin1Char(';')) : QString())
                        .arg(writeBackground ? QString(QLatin1String("background:") + toHtmlRgbaString(attrib->background().color()) + QLatin1Char(';'))
                                             : QString());
    }

    m_output << text.toHtmlEscaped();

    if (writeBackground || writeForeground) {
        m_output << "</span>";
    }
    if (attrib->fontItalic()) {
        m_output << "</i>";
    }
    if (attrib->fontBold()) {
        m_output << "</b>";
    }
    m_output.flush();
}
