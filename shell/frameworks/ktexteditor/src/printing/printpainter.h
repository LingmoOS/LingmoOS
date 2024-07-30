/*
    SPDX-FileCopyrightText: 2002-2010 Anders Lund <anders@alweb.dk>

    Rewritten based on code of:
    SPDX-FileCopyrightText: 2002 Michael Goffioul <kdeprint@swing.be>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_PRINT_PAINTER_H
#define KATE_PRINT_PAINTER_H

#include <QColor>
#include <QFont>
#include <QString>

namespace KTextEditor
{
class DocumentPrivate;
class ViewPrivate;
}

class QPrinter;
class QPainter;

class KateRenderer;

namespace Kate
{
class TextFolding;
}
namespace KTextEditor
{
class Range;
}

namespace KatePrinter
{
class PageLayout;

class PrintPainter
{
public:
    PrintPainter(KTextEditor::DocumentPrivate *doc, KTextEditor::ViewPrivate *view);
    ~PrintPainter();

    PrintPainter(const PrintPainter &) = delete;
    PrintPainter &operator=(const PrintPainter &) = delete;

    void paint(QPrinter *printer) const;

    // Attributes
    void setColorScheme(const QString &scheme);

    void setPrintGuide(const bool on)
    {
        m_printGuide = on;
    }
    void setPrintLineNumbers(const bool on)
    {
        m_printLineNumbers = on;
    }
    void setDontPrintFoldedCode(bool p)
    {
        m_dontPrintFoldedCode = p;
    }
    void setUseHeader(const bool on)
    {
        m_useHeader = on;
    }
    void setUseFooter(const bool on)
    {
        m_useFooter = on;
    }
    void setUseBackground(const bool on)
    {
        m_useBackground = on;
    }
    void setUseBox(const bool on);
    void setBoxMargin(const int margin)
    {
        m_boxMargin = margin;
    }
    void setBoxWidth(const int width);
    void setBoxColor(const QColor &color);
    void setHeadersFont(const QFont &font)
    {
        m_fhFont = font;
    }
    void setTextFont(const QFont &font);

    void setHeaderBackground(const QColor &color);
    void setHeaderForeground(const QColor &color);
    void setUseHeaderBackground(const bool on)
    {
        m_useHeaderBackground = on;
    }

    void setFooterBackground(const QColor &color);
    void setFooterForeground(const QColor &color);
    void setUseFooterBackground(const bool on)
    {
        m_useFooterBackground = on;
    }

    void setHeaderFormat(const QStringList &list)
    {
        m_headerFormat = list;
    }
    void setFooterFormat(const QStringList &list)
    {
        m_footerFormat = list;
    }

private:
    void paintLineNumber(QPainter &painter, const uint number, const PageLayout &pl) const;
    void paintLine(QPainter &painter, const uint line, uint &y, uint &remainder, const PageLayout &pl) const;
    void paintNewPage(QPainter &painter, const uint currentPage, uint &y, const PageLayout &pl) const;

    void paintBackground(QPainter &painter, const uint y, const PageLayout &pl) const;
    void paintBox(QPainter &painter, uint &y, const PageLayout &pl) const;
    void paintGuide(QPainter &painter, uint &y, const PageLayout &pl) const;

    void paintHeader(QPainter &painter, const uint currentPage, uint &y, const PageLayout &pl) const;
    void paintFooter(QPainter &painter, const uint currentPage, const PageLayout &pl) const;
    void configure(const QPrinter *printer, PageLayout &layout) const;

    void updateCache();

private:
    KTextEditor::ViewPrivate *m_view;
    KTextEditor::DocumentPrivate *m_doc;

    bool m_printGuide;
    bool m_printLineNumbers;
    bool m_dontPrintFoldedCode;
    bool m_useHeader;
    bool m_useFooter;
    bool m_useBackground;
    bool m_useBox;
    bool m_useHeaderBackground;
    bool m_useFooterBackground;

    int m_boxMargin;
    int m_boxWidth;
    QColor m_boxColor;

    QColor m_headerBackground;
    QColor m_headerForeground;
    QColor m_footerBackground;
    QColor m_footerForeground;

    QFont m_fhFont;

    QStringList m_headerFormat;
    QStringList m_footerFormat;

    /* Internal vars */
    KateRenderer *m_renderer;

    int m_fontHeight;
    uint m_lineNumberWidth;
};

}

#endif
