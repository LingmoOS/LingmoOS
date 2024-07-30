/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_RENDERER_H
#define KATE_RENDERER_H

#include "kateconfig.h"
#include "ktexteditor/range.h"

#include <QFlags>
#include <QFont>
#include <QFontMetricsF>
#include <QTextLine>

namespace KTextEditor
{
class DocumentPrivate;
class ViewPrivate;
class Attribute;
}
class KateRendererConfig;
namespace Kate
{
class TextFolding;
class TextLine;
}

class KateTextLayout;
class KateLineLayout;
typedef QExplicitlySharedDataPointer<KTextEditor::Attribute> AttributePtr;

namespace KTextEditor
{
enum class caretStyles {
    Line,
    Block,
    Underline,
    Half,
};
}

/**
 * Handles all of the work of rendering the text
 * (used for the views and printing)
 *
 **/
class KateRenderer
{
public:
    /**
     * Style of Caret
     *
     * The caret is displayed as a vertical bar (Line), a filled box
     * (Block), a horizontal bar (Underline), or a half-height filled
     * box (Half). The default is Line.
     *
     *     Line           Block          Underline           Half
     *
     * ##     _         #########              _                _
     * ##  __| |        #####| |#           __| |            __| |
     * ## / _' |        ##/ _' |#          / _' |           / _' |
     * ##| (_| |        #| (#| |#         | (_| |         #| (#| |#
     * ## \__,_|        ##\__,_|#          \__,_|         ##\__,_|#
     * ##               #########        #########        #########
     */

    /**
     * Constructor
     * @param doc document to render
     * @param folding folding information
     * @param view view which is output (0 for example for rendering to print)
     */
    explicit KateRenderer(KTextEditor::DocumentPrivate *doc, Kate::TextFolding &folding, KTextEditor::ViewPrivate *view = nullptr);

    KateRenderer(const KateRenderer &) = delete;
    KateRenderer &operator=(const KateRenderer &) = delete;

    /**
     * Returns the document to which this renderer is bound
     */
    KTextEditor::DocumentPrivate *doc() const
    {
        return m_doc;
    }

    /**
     * Returns the folding info to which this renderer is bound
     * @return folding info
     */
    Kate::TextFolding &folding() const
    {
        return m_folding;
    }

    /**
     * Returns the view to which this renderer is bound
     */
    KTextEditor::ViewPrivate *view() const
    {
        return m_view;
    }

    /**
     * update the highlighting attributes
     * (for example after an hl change or after hl config changed)
     */
    void updateAttributes();

    /**
     * Determine whether the caret (text cursor) will be drawn.
     * @return should it be drawn?
     */
    inline bool drawCaret() const
    {
        return m_drawCaret;
    }

    /**
     * Set whether the caret (text cursor) will be drawn.
     * @param drawCaret should caret be drawn?
     */
    void setDrawCaret(bool drawCaret);

    /**
     * The style of the caret (text cursor) to be painted.
     * @return caretStyle
     */
    inline KTextEditor::caretStyles caretStyle() const
    {
        return m_caretStyle;
    }

    /**
     * Set the style of caret to be painted.
     * @param style style to set
     */
    void setCaretStyle(KTextEditor::caretStyles style);

    /**
     * Set a \a brush with which to override drawing of the caret.  Set to QColor() to clear.
     */
    void setCaretOverrideColor(const QColor &color);

    /**
     * @returns whether tabs should be shown (ie. a small mark
     * drawn to identify a tab)
     * @return tabs should be shown
     */
    inline bool showTabs() const
    {
        return m_showTabs;
    }

    /**
     * Set whether a mark should be painted to help identifying tabs.
     * @param showTabs show the tabs?
     */
    void setShowTabs(bool showTabs);

    /**
     * Set which spaces should be rendered
     */
    void setShowSpaces(KateDocumentConfig::WhitespaceRendering showSpaces);

    /**
     * @returns whether which spaces should be rendered
     */
    inline KateDocumentConfig::WhitespaceRendering showSpaces() const
    {
        return m_showSpaces;
    }

    /**
     * Update marker size shown.
     */
    void updateMarkerSize();

    /**
     * @returns whether non-printable spaces should be shown
     */
    inline bool showNonPrintableSpaces() const
    {
        return m_showNonPrintableSpaces;
    }

    /**
     * Set whether box should be drawn around non-printable spaces
     */
    void setShowNonPrintableSpaces(const bool showNonPrintableSpaces);

    /**
     * Sets the width of the tab. Helps performance.
     * @param tabWidth new tab width
     */
    void setTabWidth(int tabWidth);

    /**
     * @returns whether indent lines should be shown
     * @return indent lines should be shown
     */
    bool showIndentLines() const;

    /**
     * Set whether a guide should be painted to help identifying indent lines.
     * @param showLines show the indent lines?
     */
    void setShowIndentLines(bool showLines);

    /**
     * Sets the width of the tab. Helps performance.
     * @param indentWidth new indent width
     */
    void setIndentWidth(int indentWidth);

    /**
     * Show the view's selection?
     * @return show sels?
     */
    inline bool showSelections() const
    {
        return m_showSelections;
    }

    /**
     * Set whether the view's selections should be shown.
     * The default is true.
     * @param showSelections show the selections?
     */
    void setShowSelections(bool showSelections);

    /**
     * Change to a different font (soon to be font set?)
     */
    void increaseFontSizes(qreal step = 1.0) const;
    void decreaseFontSizes(qreal step = 1.0) const;
    void resetFontSizes() const;

    /**
     * Access currently used font.
     * @return current font
     */
    const QFont &currentFont() const
    {
        return m_font;
    }

    /**
     * Access currently used font metrics.
     * @return current font metrics
     */
    const QFontMetricsF &currentFontMetrics() const
    {
        return m_fontMetrics;
    }

    /**
     * @return whether the renderer is configured to paint in a
     * printer-friendly fashion.
     */
    bool isPrinterFriendly() const;

    /**
     * Configure this renderer to paint in a printer-friendly fashion.
     *
     * Sets the other options appropriately if true.
     */
    void setPrinterFriendly(bool printerFriendly);

    /**
     * Text width & height calculation functions...
     */
    void layoutLine(KateLineLayout *line, int maxwidth = -1, bool cacheLayout = false) const;

    /**
     * This is a smaller QString::isRightToLeft(). It's also marked as internal to kate
     * instead of internal to Qt, so we can modify. This method searches for the first
     * strong character in the paragraph and then returns its direction. In case of a
     * line without any strong characters, the direction is forced to be LTR.
     *
     * Back in KDE 4.1 this method counted chars, which lead to unwanted side effects.
     * (see https://bugs.kde.org/show_bug.cgi?id=178594). As this function is internal
     * the way it work will probably change between releases. Be warned!
     */
    static bool isLineRightToLeft(QStringView str);

    /**
     * The ultimate decoration creation function.
     *
     * \param selectionsOnly return decorations for selections and/or dynamic highlighting.
     */
    QList<QTextLayout::FormatRange> decorationsForLine(const Kate::TextLine &textLine, int line, bool selectionsOnly = false) const;

    // Width calculators
    qreal spaceWidth() const;

    /**
     * Returns the x position of cursor \p col on the line \p range.
     */
    int cursorToX(const KateTextLayout &range, int col, bool returnPastLine = false) const;
    /// \overload
    int cursorToX(const KateTextLayout &range, const KTextEditor::Cursor pos, bool returnPastLine = false) const;

    /**
     * Returns the real cursor which is occupied by the specified x value, or that closest to it.
     * If \p returnPastLine is true, the column will be extrapolated out with the assumption
     * that the extra characters are spaces.
     */
    KTextEditor::Cursor xToCursor(const KateTextLayout &range, int x, bool returnPastLine = false) const;

    // Font height
    uint fontHeight() const;

    // Line height
    int lineHeight() const;

    // Document height
    uint documentHeight() const;

    // Selection boundaries
    bool getSelectionBounds(int line, int lineLength, int &start, int &end) const;

    /**
     * Flags to customize the paintTextLine function behavior
     */
    enum PaintTextLineFlag {
        /**
         * Skip drawing the dashed underline at the start of a folded block of text?
         */
        SkipDrawFirstInvisibleLineUnderlined = 0x1,
        /**
         * Skip drawing the line selection
         * This is useful when we are drawing the draggable pixmap for drag event
         */
        SkipDrawLineSelection = 0x2
    };
    Q_DECLARE_FLAGS(PaintTextLineFlags, PaintTextLineFlag)

    /**
     * This is the ultimate function to perform painting of a text line.
     *
     * The text line is painted from the upper limit of (0,0).  To move that,
     * apply a transform to your painter.
     *
     * @param paint           painter to use
     * @param range           layout to use in painting this line
     * @param textClipRect    clip rect for text to not paint lines outside the visible area.
     * @param xStart          starting width in pixels.
     * @param xEnd            ending width in pixels.
     * @param cursor          position of the caret, if placed on the current line.
     * @param flags           flags for customizing the drawing of the line
     */
    void paintTextLine(QPainter &paint,
                       KateLineLayout *range,
                       int xStart,
                       int xEnd,
                       const QRectF &textClipRect = QRectF(),
                       const KTextEditor::Cursor *cursor = nullptr,
                       PaintTextLineFlags flags = PaintTextLineFlags());

    /**
     * Paint the background of a line
     *
     * Split off from the main @ref paintTextLine method to make it smaller. As it's being
     * called only once per line it shouldn't noticably affect performance and it
     * helps readability a LOT.
     *
     * @param paint           painter to use
     * @param layout          layout to use in painting this line
     * @param currentViewLine if one of the view lines is the current line, set
     *                        this to the index; otherwise -1.
     * @param xStart          starting width in pixels.
     * @param xEnd            ending width in pixels.
     */
    void paintTextLineBackground(QPainter &paint, KateLineLayout *layout, int currentViewLine, int xStart, int xEnd);

    void paintTextBackground(QPainter &paint, KateLineLayout *layout, const QList<QTextLayout::FormatRange> &selRanges, const QBrush &br, int xStart) const;

    /**
     * This takes an in index, and returns all the attributes for it.
     * For example, if you have a ktextline, and want the KTextEditor::Attribute
     * for a given position, do:
     *
     *   attribute(myktextline->attribute(position));
     */
    const AttributePtr &attribute(uint pos) const;
    AttributePtr specificAttribute(int context) const;

    /**
     * Paints a range of text into @a d. This function is mainly used to paint the pixmap
     * when dragging text.
     *
     * Please note that this will not paint the selection background but only the text.
     *
     * @param d                 the paint device
     * @param startLine         start line
     * @param xStart            start pos on @a startLine in pixels
     * @param endLine           end line
     * @param xEnd              end pos on @a endLine in pixels
     * @param scale             the amount of scaling to apply. Default is 1.0, negative values are not supported
     */
    void paintSelection(QPaintDevice *d, int startLine, int xStart, int endLine, int xEnd, qreal scale = 1.0);

private:
    /**
     * Paint a trailing space on position (x, y).
     */
    void paintSpaces(QPainter &paint, const QPointF *points, const int count) const;
    /**
     * Paint a tab stop marker on position (x, y).
     */
    void paintTabstop(QPainter &paint, qreal x, qreal y) const;

    /**
     * Paint a non-breaking space marker on position (x, y).
     */
    void paintNonBreakSpace(QPainter &paint, qreal x, qreal y) const;

    /**
     * Paint non printable spaces bounding box
     */
    void paintNonPrintableSpaces(QPainter &paint, qreal x, qreal y, const QChar &chr);

    /** Paint a SciTE-like indent marker. */
    void paintIndentMarker(QPainter &paint, uint x, int line);

    static void assignSelectionBrushesFromAttribute(QTextLayout::FormatRange &target, const KTextEditor::Attribute &attribute);

    void paintCaret(KTextEditor::Cursor cursor, KateLineLayout *range, QPainter &paint, int xStart, int xEnd);

    // update font height
    void updateFontHeight();

    bool hasCustomLineHeight() const;

    KTextEditor::DocumentPrivate *const m_doc;
    Kate::TextFolding &m_folding;
    KTextEditor::ViewPrivate *const m_view;

    // cache of config values
    int m_tabWidth;
    int m_indentWidth;
    int m_fontHeight;
    float m_fontAscent;

    // if we are at bracket, this will have the X for the opener
    int m_currentBracketX = -1;

    // The bracket range, if we are at a bracket
    KTextEditor::Range m_currentBracketRange = KTextEditor::Range::invalid();

    // some internal flags
    KTextEditor::caretStyles m_caretStyle;
    bool m_drawCaret;
    bool m_showSelections;
    bool m_showTabs;
    KateDocumentConfig::WhitespaceRendering m_showSpaces = KateDocumentConfig::None;
    float m_markerSize;
    bool m_showNonPrintableSpaces;
    bool m_printerFriendly;
    QColor m_caretOverrideColor;

    QList<AttributePtr> m_attributes;

    /**
     * Configuration
     */
public:
    void updateConfig();

    inline KateRendererConfig *config() const
    {
        return m_config.get();
    }

private:
    std::unique_ptr<KateRendererConfig> const m_config;

    /**
     * cached font, was perhaps adjusted for current DPIs
     */
    QFont m_font;

    /**
     * cached font metrics
     */
    QFontMetricsF m_fontMetrics;
};

#endif
