/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2003-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2013 Andrey Matveyakin <a.matveyakin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katerenderer.h"

#include "inlinenotedata.h"
#include "katebuffer.h"
#include "katedocument.h"
#include "kateextendedattribute.h"
#include "katehighlight.h"
#include "katerenderrange.h"
#include "katetextlayout.h"
#include "kateview.h"

#include "ktexteditor/attribute.h"
#include "ktexteditor/inlinenote.h"
#include "ktexteditor/inlinenoteprovider.h"

#include "katepartdebug.h"

#include <QBrush>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QStack>
#include <QtMath> // qCeil

static const QChar tabChar(QLatin1Char('\t'));
static const QChar spaceChar(QLatin1Char(' '));
static const QChar nbSpaceChar(0xa0); // non-breaking space

KateRenderer::KateRenderer(KTextEditor::DocumentPrivate *doc, Kate::TextFolding &folding, KTextEditor::ViewPrivate *view)
    : m_doc(doc)
    , m_folding(folding)
    , m_view(view)
    , m_tabWidth(m_doc->config()->tabWidth())
    , m_indentWidth(m_doc->config()->indentationWidth())
    , m_caretStyle(KTextEditor::caretStyles::Line)
    , m_drawCaret(true)
    , m_showSelections(true)
    , m_showTabs(true)
    , m_showSpaces(KateDocumentConfig::Trailing)
    , m_showNonPrintableSpaces(false)
    , m_printerFriendly(false)
    , m_config(new KateRendererConfig(this))
    , m_font(m_config->baseFont())
    , m_fontMetrics(m_font)
{
    updateAttributes();

    // initialize with a sane font height
    updateFontHeight();

    // make the proper calculation for markerSize
    updateMarkerSize();
}

void KateRenderer::updateAttributes()
{
    m_attributes = m_doc->highlight()->attributes(config()->schema());
}

const KTextEditor::Attribute::Ptr &KateRenderer::attribute(uint pos) const
{
    if (pos < (uint)m_attributes.count()) {
        return m_attributes[pos];
    }

    return m_attributes[0];
}

KTextEditor::Attribute::Ptr KateRenderer::specificAttribute(int context) const
{
    if (context >= 0 && context < m_attributes.count()) {
        return m_attributes[context];
    }

    return m_attributes[0];
}

void KateRenderer::setDrawCaret(bool drawCaret)
{
    m_drawCaret = drawCaret;
}

void KateRenderer::setCaretStyle(KTextEditor::caretStyles style)
{
    m_caretStyle = style;
}

void KateRenderer::setShowTabs(bool showTabs)
{
    m_showTabs = showTabs;
}

void KateRenderer::setShowSpaces(KateDocumentConfig::WhitespaceRendering showSpaces)
{
    m_showSpaces = showSpaces;
}

void KateRenderer::setShowNonPrintableSpaces(const bool on)
{
    m_showNonPrintableSpaces = on;
}

void KateRenderer::setTabWidth(int tabWidth)
{
    m_tabWidth = tabWidth;
}

bool KateRenderer::showIndentLines() const
{
    return m_config->showIndentationLines();
}

void KateRenderer::setShowIndentLines(bool showIndentLines)
{
    // invalidate our "active indent line" cached stuff
    m_currentBracketRange = KTextEditor::Range::invalid();
    m_currentBracketX = -1;

    m_config->setShowIndentationLines(showIndentLines);
}

void KateRenderer::setIndentWidth(int indentWidth)
{
    m_indentWidth = indentWidth;
}

void KateRenderer::setShowSelections(bool showSelections)
{
    m_showSelections = showSelections;
}

void KateRenderer::increaseFontSizes(qreal step) const
{
    QFont f(config()->baseFont());
    f.setPointSizeF(f.pointSizeF() + step);
    config()->setFont(f);
}

void KateRenderer::resetFontSizes() const
{
    QFont f(KateRendererConfig::global()->baseFont());
    config()->setFont(f);
}

void KateRenderer::decreaseFontSizes(qreal step) const
{
    QFont f(config()->baseFont());
    if ((f.pointSizeF() - step) > 0) {
        f.setPointSizeF(f.pointSizeF() - step);
    }
    config()->setFont(f);
}

bool KateRenderer::isPrinterFriendly() const
{
    return m_printerFriendly;
}

void KateRenderer::setPrinterFriendly(bool printerFriendly)
{
    m_printerFriendly = printerFriendly;
    setShowTabs(false);
    setShowSpaces(KateDocumentConfig::None);
    setShowSelections(false);
    setDrawCaret(false);
}

void KateRenderer::paintTextLineBackground(QPainter &paint, KateLineLayout *layout, int currentViewLine, int xStart, int xEnd)
{
    if (isPrinterFriendly()) {
        return;
    }

    // Normal background color
    QColor backgroundColor(config()->backgroundColor());

    // paint the current line background if we're on the current line
    QColor currentLineColor = config()->highlightedLineColor();

    // Check for mark background
    int markRed = 0;
    int markGreen = 0;
    int markBlue = 0;
    int markCount = 0;

    // Retrieve marks for this line
    uint mrk = m_doc->mark(layout->line());
    if (mrk) {
        for (uint bit = 0; bit < 32; bit++) {
            KTextEditor::Document::MarkTypes markType = (KTextEditor::Document::MarkTypes)(1U << bit);
            if (mrk & markType) {
                QColor markColor = config()->lineMarkerColor(markType);

                if (markColor.isValid()) {
                    markCount++;
                    markRed += markColor.red();
                    markGreen += markColor.green();
                    markBlue += markColor.blue();
                }
            }
        } // for
    } // Marks

    if (markCount) {
        markRed /= markCount;
        markGreen /= markCount;
        markBlue /= markCount;
        backgroundColor.setRgb(int((backgroundColor.red() * 0.9) + (markRed * 0.1)),
                               int((backgroundColor.green() * 0.9) + (markGreen * 0.1)),
                               int((backgroundColor.blue() * 0.9) + (markBlue * 0.1)),
                               backgroundColor.alpha());
    }

    // Draw line background
    paint.fillRect(0, 0, xEnd - xStart, lineHeight() * layout->viewLineCount(), backgroundColor);

    // paint the current line background if we're on the current line
    const bool currentLineHasSelection = m_view && m_view->selection() && m_view->selectionRange().overlapsLine(layout->line());
    if (currentViewLine != -1 && !currentLineHasSelection) {
        if (markCount) {
            markRed /= markCount;
            markGreen /= markCount;
            markBlue /= markCount;
            currentLineColor.setRgb(int((currentLineColor.red() * 0.9) + (markRed * 0.1)),
                                    int((currentLineColor.green() * 0.9) + (markGreen * 0.1)),
                                    int((currentLineColor.blue() * 0.9) + (markBlue * 0.1)),
                                    currentLineColor.alpha());
        }

        paint.fillRect(0, lineHeight() * currentViewLine, xEnd - xStart, lineHeight(), currentLineColor);
    }
}

void KateRenderer::paintTabstop(QPainter &paint, qreal x, qreal y) const
{
    QPen penBackup(paint.pen());
    QPen pen(config()->tabMarkerColor());
    pen.setWidthF(qMax(1.0, spaceWidth() / 10.0));
    paint.setPen(pen);

    int dist = spaceWidth() * 0.3;
    QPoint points[8];
    points[0] = QPoint(x - dist, y - dist);
    points[1] = QPoint(x, y);
    points[2] = QPoint(x, y);
    points[3] = QPoint(x - dist, y + dist);
    x += spaceWidth() / 3.0;
    points[4] = QPoint(x - dist, y - dist);
    points[5] = QPoint(x, y);
    points[6] = QPoint(x, y);
    points[7] = QPoint(x - dist, y + dist);
    paint.drawLines(points, 4);
    paint.setPen(penBackup);
}

void KateRenderer::paintSpaces(QPainter &paint, const QPointF *points, const int count) const
{
    if (count == 0) {
        return;
    }
    QPen penBackup(paint.pen());
    QPen pen(config()->tabMarkerColor());

    pen.setWidthF(m_markerSize);
    pen.setCapStyle(Qt::RoundCap);
    paint.setPen(pen);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.drawPoints(points, count);
    paint.setPen(penBackup);
    paint.setRenderHint(QPainter::Antialiasing, false);
}

void KateRenderer::paintNonBreakSpace(QPainter &paint, qreal x, qreal y) const
{
    QPen penBackup(paint.pen());
    QPen pen(config()->tabMarkerColor());
    pen.setWidthF(qMax(1.0, spaceWidth() / 10.0));
    paint.setPen(pen);

    const int height = fontHeight();
    const int width = spaceWidth();

    QPoint points[6];
    points[0] = QPoint(x + width / 10, y + height / 4);
    points[1] = QPoint(x + width / 10, y + height / 3);
    points[2] = QPoint(x + width / 10, y + height / 3);
    points[3] = QPoint(x + width - width / 10, y + height / 3);
    points[4] = QPoint(x + width - width / 10, y + height / 3);
    points[5] = QPoint(x + width - width / 10, y + height / 4);
    paint.drawLines(points, 3);
    paint.setPen(penBackup);
}

void KateRenderer::paintNonPrintableSpaces(QPainter &paint, qreal x, qreal y, const QChar &chr)
{
    paint.save();
    QPen pen(config()->spellingMistakeLineColor());
    pen.setWidthF(qMax(1.0, spaceWidth() * 0.1));
    paint.setPen(pen);

    const int height = fontHeight();
    const int width = m_fontMetrics.boundingRect(chr).width();
    const int offset = spaceWidth() * 0.1;

    QPoint points[8];
    points[0] = QPoint(x - offset, y + offset);
    points[1] = QPoint(x + width + offset, y + offset);
    points[2] = QPoint(x + width + offset, y + offset);
    points[3] = QPoint(x + width + offset, y - height - offset);
    points[4] = QPoint(x + width + offset, y - height - offset);
    points[5] = QPoint(x - offset, y - height - offset);
    points[6] = QPoint(x - offset, y - height - offset);
    points[7] = QPoint(x - offset, y + offset);
    paint.drawLines(points, 4);
    paint.restore();
}

/**
 * Helper function that checks if our cursor is at a bracket
 * and calculates X position for opening/closing brackets. We
 * then use this data to color the indentation line differently.
 * @p view is current view
 * @p range is the current range from @ref paintTextLine
 * @p spaceWidth width of space char
 * @p c is the position of cursor
 * @p bracketXPos will be X position of close bracket or -1 if not found
 */
static KTextEditor::Range cursorAtBracket(KTextEditor::ViewPrivate *view, const KateLineLayout *range, int spaceWidth, KTextEditor::Cursor c, int &bracketXPos)
{
    bracketXPos = -1;
    if (range->line() != c.line()) {
        return KTextEditor::Range::invalid();
    }

    auto *doc = view->doc();
    // Avoid work if we are below tabwidth
    if (c.column() < doc->config()->tabWidth()) {
        return KTextEditor::Range::invalid();
    }

    // We match these brackets only
    static constexpr QChar brackets[] = {QLatin1Char('{'), QLatin1Char('}'), QLatin1Char('('), QLatin1Char(')')};
    // look for character in front
    QChar right = doc->characterAt(c);
    auto it = std::find(std::begin(brackets), std::end(brackets), right);

    KTextEditor::Range ret = KTextEditor::Range::invalid();
    bool found = false;
    if (it != std::end(brackets)) {
        found = true;
    } else {
        // look at previous character
        QChar left = doc->characterAt({c.line(), c.column() - 1});
        it = std::find(std::begin(brackets), std::end(brackets), left);
        if (it != std::end(brackets)) {
            found = true;
        }
    }

    // We have a bracket
    if (found) {
        ret = doc->findMatchingBracket(c, 500);
        if (!ret.isValid()) {
            return ret;
        }
        bracketXPos = (ret.end().column() * spaceWidth) + 1;
    }

    return ret;
}

void KateRenderer::paintIndentMarker(QPainter &paint, uint x, int line)
{
    const QPen penBackup(paint.pen());
    static const QList<qreal> dashPattern = QList<qreal>() << 1 << 1;
    QPen myPen;

    const bool onBracket = m_currentBracketX == (int)x;
    if (onBracket && m_currentBracketRange.containsLine(line)) {
        QColor c = view()->theme().textColor(KSyntaxHighlighting::Theme::Normal);
        c.setAlphaF(0.7);
        myPen.setColor(c);
    } else {
        myPen.setColor(config()->indentationLineColor());
        myPen.setDashPattern(dashPattern);
    }

    paint.setPen(myPen);

    QPainter::RenderHints renderHints = paint.renderHints();
    paint.setRenderHints(renderHints, false);

    paint.drawLine(x + 2, 0, x + 2, lineHeight());

    paint.setRenderHints(renderHints, true);

    paint.setPen(penBackup);
}

static bool rangeLessThanForRenderer(const Kate::TextRange *a, const Kate::TextRange *b)
{
    // compare Z-Depth first
    // smaller Z-Depths should win!
    if (a->zDepth() > b->zDepth()) {
        return true;
    } else if (a->zDepth() < b->zDepth()) {
        return false;
    }

    // end of a > end of b?
    if (a->end().toCursor() > b->end().toCursor()) {
        return true;
    }

    // if ends are equal, start of a < start of b?
    if (a->end().toCursor() == b->end().toCursor()) {
        return a->start().toCursor() < b->start().toCursor();
    }

    return false;
}

QList<QTextLayout::FormatRange> KateRenderer::decorationsForLine(const Kate::TextLine &textLine, int line, bool selectionsOnly) const
{
    // limit number of attributes we can highlight in reasonable time
    const int limitOfRanges = 1024;
    auto rangesWithAttributes = m_doc->buffer().rangesForLine(line, m_printerFriendly ? nullptr : m_view, true);
    if (rangesWithAttributes.size() > limitOfRanges) {
        rangesWithAttributes.clear();
    }

    // Don't compute the highlighting if there isn't going to be any highlighting
    const auto &al = textLine.attributesList();
    if (!(selectionsOnly || !al.empty() || !rangesWithAttributes.empty())) {
        return QList<QTextLayout::FormatRange>();
    }

    // Add the inbuilt highlighting to the list, limit with limitOfRanges
    RenderRangeVector renderRanges;
    if (!al.empty()) {
        auto &currentRange = renderRanges.pushNewRange();
        for (int i = 0; i < std::min<int>(al.size(), limitOfRanges); ++i) {
            if (al[i].length > 0 && al[i].attributeValue > 0) {
                currentRange.addRange(KTextEditor::Range(KTextEditor::Cursor(line, al[i].offset), al[i].length), specificAttribute(al[i].attributeValue));
            }
        }
    }

    // check for dynamic hl stuff
    const QSet<Kate::TextRange *> *rangesMouseIn = m_view ? m_view->rangesMouseIn() : nullptr;
    const QSet<Kate::TextRange *> *rangesCaretIn = m_view ? m_view->rangesCaretIn() : nullptr;
    bool anyDynamicHlsActive = m_view && (!rangesMouseIn->empty() || !rangesCaretIn->empty());

    // sort all ranges, we want that the most specific ranges win during rendering, multiple equal ranges are kind of random, still better than old smart
    // rangs behavior ;)
    std::sort(rangesWithAttributes.begin(), rangesWithAttributes.end(), rangeLessThanForRenderer);

    renderRanges.reserve(rangesWithAttributes.size());
    // loop over all ranges
    for (int i = 0; i < rangesWithAttributes.size(); ++i) {
        // real range
        Kate::TextRange *kateRange = rangesWithAttributes[i];

        // calculate attribute, default: normal attribute
        KTextEditor::Attribute::Ptr attribute = kateRange->attribute();
        if (anyDynamicHlsActive) {
            // check mouse in
            if (KTextEditor::Attribute::Ptr attributeMouseIn = attribute->dynamicAttribute(KTextEditor::Attribute::ActivateMouseIn)) {
                if (rangesMouseIn->contains(kateRange)) {
                    attribute = attributeMouseIn;
                }
            }

            // check caret in
            if (KTextEditor::Attribute::Ptr attributeCaretIn = attribute->dynamicAttribute(KTextEditor::Attribute::ActivateCaretIn)) {
                if (rangesCaretIn->contains(kateRange)) {
                    attribute = attributeCaretIn;
                }
            }
        }

        // span range
        renderRanges.pushNewRange().addRange(*kateRange, std::move(attribute));
    }

    // Add selection highlighting if we're creating the selection decorations
    if ((m_view && selectionsOnly && showSelections() && m_view->selection()) || (m_view && m_view->blockSelection())) {
        auto &currentRange = renderRanges.pushNewRange();

        // Set up the selection background attribute TODO: move this elsewhere, eg. into the config?
        static KTextEditor::Attribute::Ptr backgroundAttribute;
        if (!backgroundAttribute) {
            backgroundAttribute = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
        }

        if (!hasCustomLineHeight()) {
            backgroundAttribute->setBackground(config()->selectionColor());
        }
        backgroundAttribute->setForeground(attribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->selectedForeground().color());

        // Create a range for the current selection
        if (m_view->blockSelection() && m_view->selectionRange().overlapsLine(line)) {
            currentRange.addRange(m_doc->rangeOnLine(m_view->selectionRange(), line), backgroundAttribute);
        } else {
            currentRange.addRange(m_view->selectionRange(), backgroundAttribute);
        }
    }

    // no render ranges, nothing to do, else we loop below endless!
    if (renderRanges.isEmpty()) {
        return QList<QTextLayout::FormatRange>();
    }

    // Calculate the range which we need to iterate in order to get the highlighting for just this line
    KTextEditor::Cursor currentPosition;
    KTextEditor::Cursor endPosition;
    if (m_view && selectionsOnly) {
        if (m_view->blockSelection()) {
            KTextEditor::Range subRange = m_doc->rangeOnLine(m_view->selectionRange(), line);
            currentPosition = subRange.start();
            endPosition = subRange.end();
        } else {
            KTextEditor::Range rangeNeeded = m_view->selectionRange() & KTextEditor::Range(line, 0, line + 1, 0);

            currentPosition = qMax(KTextEditor::Cursor(line, 0), rangeNeeded.start());
            endPosition = qMin(KTextEditor::Cursor(line + 1, 0), rangeNeeded.end());
        }
    } else {
        currentPosition = KTextEditor::Cursor(line, 0);
        endPosition = KTextEditor::Cursor(line + 1, 0);
    }

    // Background formats have lower priority so they get overriden by selection
    const bool shoudlClearBackground = m_view && hasCustomLineHeight() && m_view->selection();
    const KTextEditor::Range selectionRange = shoudlClearBackground ? m_view->selectionRange() : KTextEditor::Range::invalid();

    // Main iterative loop.  This walks through each set of highlighting ranges, and stops each
    // time the highlighting changes.  It then creates the corresponding QTextLayout::FormatRanges.
    QList<QTextLayout::FormatRange> newHighlight;
    while (currentPosition < endPosition) {
        renderRanges.advanceTo(currentPosition);

        if (!renderRanges.hasAttribute()) {
            // No attribute, don't need to create a FormatRange for this text range
            currentPosition = renderRanges.nextBoundary();
            continue;
        }

        KTextEditor::Cursor nextPosition = renderRanges.nextBoundary();

        // Create the format range and populate with the correct start, length and format info
        QTextLayout::FormatRange fr;
        fr.start = currentPosition.column();

        if (nextPosition < endPosition || endPosition.line() <= line) {
            fr.length = nextPosition.column() - currentPosition.column();

        } else {
            // before we did here +1 to force background drawing at the end of the line when it's warranted
            // we now skip this, we don't draw e.g. full line backgrounds
            fr.length = textLine.length() - currentPosition.column();
        }

        KTextEditor::Attribute::Ptr a = renderRanges.generateAttribute();
        if (a) {
            fr.format = *a;

            if (selectionsOnly) {
                assignSelectionBrushesFromAttribute(fr, *a);
            }
        }

        // Clear background if this position overlaps selection
        if (shoudlClearBackground && selectionRange.contains(currentPosition) && fr.format.hasProperty(QTextFormat::BackgroundBrush)) {
            fr.format.clearBackground();
        }

        newHighlight.append(fr);

        currentPosition = nextPosition;
    }

    // ensure bold & italic fonts work, even if the main font is e.g. light or something like that
    for (auto &formatRange : newHighlight) {
        if (formatRange.format.fontWeight() == QFont::Bold || formatRange.format.fontItalic()) {
            formatRange.format.setFontStyleName(QString());
        }
    }

    return newHighlight;
}

void KateRenderer::assignSelectionBrushesFromAttribute(QTextLayout::FormatRange &target, const KTextEditor::Attribute &attribute)
{
    if (attribute.hasProperty(SelectedForeground)) {
        target.format.setForeground(attribute.selectedForeground());
    }
    if (attribute.hasProperty(SelectedBackground)) {
        target.format.setBackground(attribute.selectedBackground());
    }
}

void KateRenderer::paintTextBackground(QPainter &paint,
                                       KateLineLayout *layout,
                                       const QList<QTextLayout::FormatRange> &selRanges,
                                       const QBrush &brush,
                                       int xStart) const
{
    const bool rtl = layout->isRightToLeft();

    for (const auto &sel : selRanges) {
        const int s = sel.start;
        const int e = sel.start + sel.length;
        QBrush br;

        // Prefer using the brush supplied by user
        if (brush != Qt::NoBrush) {
            br = brush;
        } else if (sel.format.background() != Qt::NoBrush) {
            // Otherwise use the brush in format
            br = sel.format.background();
        } else {
            // skip if no brush to fill with
            continue;
        }

        const int startViewLine = layout->viewLineForColumn(s);
        const int endViewLine = layout->viewLineForColumn(e);
        if (startViewLine == endViewLine) {
            KateTextLayout l = layout->viewLine(startViewLine);
            // subtract xStart so that the selection is shown where it belongs
            // we don't do it in the else case because this only matters when dynWrap==false
            // and when dynWrap==false, we always have 1 QTextLine per layout
            const int startX = cursorToX(l, s) - xStart;
            const int endX = cursorToX(l, e) - xStart;
            const int y = startViewLine * lineHeight();
            QRect r(startX, y, (endX - startX), lineHeight());
            paint.fillRect(r, br);
        } else {
            QPainterPath p;
            for (int l = startViewLine; l <= endViewLine; ++l) {
                auto kateLayout = layout->viewLine(l);
                int sx = 0;
                int width = rtl ? kateLayout.lineLayout().width() : kateLayout.lineLayout().naturalTextWidth();

                if (l == startViewLine) {
                    if (rtl) {
                        // For rtl, Rect starts at 0 and ends at selection start
                        sx = 0;
                        width = kateLayout.lineLayout().cursorToX(s);
                    } else {
                        sx = kateLayout.lineLayout().cursorToX(s);
                    }
                } else if (l == endViewLine) {
                    if (rtl) {
                        // Drawing will start at selection end, and end at the view border
                        sx = kateLayout.lineLayout().cursorToX(e);
                    } else {
                        width = kateLayout.lineLayout().cursorToX(e);
                    }
                }

                const int y = l * lineHeight();
                QRect r(sx, y, width - sx, lineHeight());
                p.addRect(r);
            }
            paint.fillPath(p, br);
        }
    }
}

void KateRenderer::paintTextLine(QPainter &paint,
                                 KateLineLayout *range,
                                 int xStart,
                                 int xEnd,
                                 const QRectF &textClipRect,
                                 const KTextEditor::Cursor *cursor,
                                 PaintTextLineFlags flags)
{
    Q_ASSERT(range->isValid());

    //   qCDebug(LOG_KTE)<<"KateRenderer::paintTextLine";

    // font data
    const QFontMetricsF &fm = m_fontMetrics;

    int currentViewLine = -1;
    if (cursor && cursor->line() == range->line()) {
        currentViewLine = range->viewLineForColumn(cursor->column());
    }

    paintTextLineBackground(paint, range, currentViewLine, xStart, xEnd);

    // Draws the dashed underline at the start of a folded block of text.
    if (!(flags & SkipDrawFirstInvisibleLineUnderlined) && range->startsInvisibleBlock()) {
        QPen pen(config()->foldingColor());
        pen.setCosmetic(true);
        pen.setStyle(Qt::DashLine);
        pen.setDashOffset(xStart);
        pen.setWidth(2);
        paint.setPen(pen);
        paint.drawLine(0, (lineHeight() * range->viewLineCount()) - 2, xEnd - xStart, (lineHeight() * range->viewLineCount()) - 2);
    }

    if (range->layout()) {
        bool drawSelection =
            m_view && m_view->selection() && showSelections() && m_view->selectionRange().overlapsLine(range->line()) && !flags.testFlag(SkipDrawLineSelection);
        // Draw selection in block selection mode. We need 2 kinds of selections that QTextLayout::draw can't render:
        //   - past-end-of-line selection and
        //   - 0-column-wide selection (used to indicate where text will be typed)
        if (drawSelection && m_view->blockSelection()) {
            int selectionStartColumn = m_doc->fromVirtualColumn(range->line(), m_doc->toVirtualColumn(m_view->selectionRange().start()));
            int selectionEndColumn = m_doc->fromVirtualColumn(range->line(), m_doc->toVirtualColumn(m_view->selectionRange().end()));
            QBrush selectionBrush = config()->selectionColor();
            if (selectionStartColumn != selectionEndColumn) {
                KateTextLayout lastLine = range->viewLine(range->viewLineCount() - 1);
                if (selectionEndColumn > lastLine.startCol()) {
                    int selectionStartX = (selectionStartColumn > lastLine.startCol()) ? cursorToX(lastLine, selectionStartColumn, true) : 0;
                    int selectionEndX = cursorToX(lastLine, selectionEndColumn, true);
                    paint.fillRect(QRect(selectionStartX - xStart, (int)lastLine.lineLayout().y(), selectionEndX - selectionStartX, lineHeight()),
                                   selectionBrush);
                }
            } else {
                const int selectStickWidth = 2;
                KateTextLayout selectionLine = range->viewLine(range->viewLineForColumn(selectionStartColumn));
                int selectionX = cursorToX(selectionLine, selectionStartColumn, true);
                paint.fillRect(QRect(selectionX - xStart, (int)selectionLine.lineLayout().y(), selectStickWidth, lineHeight()), selectionBrush);
            }
        }

        QList<QTextLayout::FormatRange> additionalFormats;
        if (range->length() > 0) {
            // We may have changed the pen, be absolutely sure it gets set back to
            // normal foreground color before drawing text for text that does not
            // set the pen color
            paint.setPen(attribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->foreground().color());
            // Draw the text :)

            if (range->layout()->textOption().textDirection() == Qt::RightToLeft) {
                // If the text is RTL, we draw text background ourselves
                auto decos = decorationsForLine(range->textLine(), range->line(), false);
                auto sr = view()->selectionRange();
                auto c = config()->selectionColor();
                int line = range->line();
                // Remove "selection" format from decorations
                // "selection" will get painted below
                decos.erase(std::remove_if(decos.begin(),
                                           decos.end(),
                                           [sr, c, line](const QTextLayout::FormatRange &fr) {
                                               return sr.overlapsLine(line) && sr.overlapsColumn(fr.start) && fr.format.background().color() == c;
                                           }),
                            decos.end());
                paintTextBackground(paint, range, decos, Qt::NoBrush, xStart);
            }

            if (drawSelection) {
                additionalFormats = decorationsForLine(range->textLine(), range->line(), true);
                if (hasCustomLineHeight()) {
                    paintTextBackground(paint, range, additionalFormats, config()->selectionColor(), xStart);
                }
                // DONT apply clipping, it breaks rendering when there are selections
                range->layout()->draw(&paint, QPoint(-xStart, 0), additionalFormats);

            } else {
                range->layout()->draw(&paint, QPoint(-xStart, 0), QList<QTextLayout::FormatRange>{}, textClipRect);
            }
        }

        // Check if we are at a bracket and color the indentation
        // line differently
        const bool indentLinesEnabled = showIndentLines();
        if (cursor && indentLinesEnabled) {
            auto cur = *cursor;
            cur.setColumn(cur.column() - 1);
            if (!m_currentBracketRange.boundaryAtCursor(*cursor) && m_currentBracketRange.end() != cur && m_currentBracketRange.start() != cur) {
                m_currentBracketRange = cursorAtBracket(view(), range, spaceWidth(), *cursor, m_currentBracketX);
            }
        }

        // Loop each individual line for additional text decoration etc.
        for (int i = 0; i < range->viewLineCount(); ++i) {
            KateTextLayout line = range->viewLine(i);

            // Draw indent lines
            if (!m_printerFriendly && (indentLinesEnabled && i == 0)) {
                const qreal w = spaceWidth();
                const int lastIndentColumn = range->textLine().indentDepth(m_tabWidth);
                for (int x = m_indentWidth; x < lastIndentColumn; x += m_indentWidth) {
                    auto xPos = x * w + 1 - xStart;
                    if (xPos >= 0) {
                        paintIndentMarker(paint, xPos, range->line());
                    }
                }
            }

            // draw an open box to mark non-breaking spaces
            const QString &text = range->textLine().text();
            int y = lineHeight() * i + m_fontAscent - fm.strikeOutPos();
            int nbSpaceIndex = text.indexOf(nbSpaceChar, line.lineLayout().xToCursor(xStart));

            while (nbSpaceIndex != -1 && nbSpaceIndex < line.endCol()) {
                int x = line.lineLayout().cursorToX(nbSpaceIndex);
                if (x > xEnd) {
                    break;
                }
                paintNonBreakSpace(paint, x - xStart, y);
                nbSpaceIndex = text.indexOf(nbSpaceChar, nbSpaceIndex + 1);
            }

            // draw tab stop indicators
            if (showTabs()) {
                int tabIndex = text.indexOf(tabChar, line.lineLayout().xToCursor(xStart));
                while (tabIndex != -1 && tabIndex < line.endCol()) {
                    int x = line.lineLayout().cursorToX(tabIndex);
                    if (x > xEnd) {
                        break;
                    }
                    paintTabstop(paint, x - xStart + spaceWidth() / 2.0, y);
                    tabIndex = text.indexOf(tabChar, tabIndex + 1);
                }
            }

            // draw trailing spaces
            if (showSpaces() != KateDocumentConfig::None) {
                int spaceIndex = line.endCol() - 1;
                const int trailingPos = showSpaces() == KateDocumentConfig::All ? 0 : qMax(range->textLine().lastChar(), 0);

                if (spaceIndex >= trailingPos) {
                    QVarLengthArray<int, 32> spacePositions;
                    // Adjust to visible contents
                    const auto dir = range->layout()->textOption().textDirection();
                    const bool isRTL = dir == Qt::RightToLeft && m_view->dynWordWrap();
                    int start = isRTL ? xEnd : xStart;
                    int end = isRTL ? xStart : xEnd;

                    spaceIndex = std::min(line.lineLayout().xToCursor(end), spaceIndex);
                    int visibleStart = line.lineLayout().xToCursor(start);

                    for (; spaceIndex >= line.startCol(); --spaceIndex) {
                        if (!text.at(spaceIndex).isSpace()) {
                            if (showSpaces() == KateDocumentConfig::Trailing) {
                                break;
                            } else {
                                continue;
                            }
                        }
                        if (text.at(spaceIndex) != QLatin1Char('\t') || !showTabs()) {
                            spacePositions << spaceIndex;
                        }

                        if (spaceIndex < visibleStart) {
                            break;
                        }
                    }

                    QPointF prev;
                    QVarLengthArray<QPointF, 32> spacePoints;
                    const auto spaceWidth = this->spaceWidth();
                    // reverse because we want to look at the spaces at the beginning of line first
                    for (auto rit = spacePositions.rbegin(); rit != spacePositions.rend(); ++rit) {
                        const int spaceIdx = *rit;
                        qreal x = line.lineLayout().cursorToX(spaceIdx) - xStart;
                        int dir = 1; // 1 == ltr, -1 == rtl
                        if (range->layout()->textOption().alignment() == Qt::AlignRight) {
                            dir = -1;
                            if (spaceIdx > 0) {
                                QChar c = text.at(spaceIdx - 1);
                                // line is LTR aligned, but is the char ltr or rtl?
                                if (!isLineRightToLeft(QStringView(&c, 1))) {
                                    dir = 1;
                                }
                            }
                        } else {
                            if (spaceIdx > 0) {
                                // line is LTR aligned, but is the char ltr or rtl?
                                QChar c = text.at(spaceIdx - 1);
                                if (isLineRightToLeft(QStringView(&c, 1))) {
                                    dir = -1;
                                }
                            }
                        }

                        x += dir * (spaceWidth / 2.0);

                        const QPointF currentPoint(x, y);
                        if (!prev.isNull() && currentPoint == prev) {
                            break;
                        }
                        spacePoints << currentPoint;
                        prev = QPointF(x, y);
                    }
                    if (!spacePoints.isEmpty()) {
                        paintSpaces(paint, spacePoints.constData(), spacePoints.size());
                    }
                }
            }

            if (showNonPrintableSpaces()) {
                const int y = lineHeight() * i + m_fontAscent;

                static const QRegularExpression nonPrintableSpacesRegExp(
                    QStringLiteral("[\\x{0000}-\\x{0008}\\x{000A}-\\x{001F}\\x{2000}-\\x{200F}\\x{2028}-\\x{202F}\\x{205F}-\\x{2064}\\x{206A}-\\x{206F}]"));
                QRegularExpressionMatchIterator i = nonPrintableSpacesRegExp.globalMatch(text, line.lineLayout().xToCursor(xStart));

                while (i.hasNext()) {
                    const int charIndex = i.next().capturedStart();

                    const int x = line.lineLayout().cursorToX(charIndex);
                    if (x > xEnd) {
                        break;
                    }

                    paintNonPrintableSpaces(paint, x - xStart, y, text[charIndex]);
                }
            }

            // draw word-wrap-honor-indent filling
            if ((i > 0) && range->shiftX && (range->shiftX > xStart)) {
                // fill background first with selection if we had selection from the previous line
                if (drawSelection && !m_view->blockSelection() && m_view->selectionRange().start() < line.start()
                    && m_view->selectionRange().end() >= line.start()) {
                    paint.fillRect(0, lineHeight() * i, range->shiftX - xStart, lineHeight(), QBrush(config()->selectionColor()));
                }

                // paint the normal filling for the word wrap markers
                paint.fillRect(0, lineHeight() * i, range->shiftX - xStart, lineHeight(), QBrush(config()->wordWrapMarkerColor(), Qt::Dense4Pattern));
            }
        }

        // Draw carets
        if (m_view && cursor && drawCaret()) {
            const auto &secCursors = view()->secondaryCursors();
            // Find carets on this line
            auto mIt = std::lower_bound(secCursors.begin(), secCursors.end(), range->line(), [](const KTextEditor::ViewPrivate::SecondaryCursor &l, int line) {
                return l.pos->line() < line;
            });
            if (mIt != secCursors.end() && mIt->cursor().line() == range->line()) {
                for (; mIt != secCursors.end(); ++mIt) {
                    auto cursor = mIt->cursor();
                    if (cursor.line() == range->line()) {
                        paintCaret(cursor, range, paint, xStart, xEnd);
                    } else {
                        break;
                    }
                }
            }
            paintCaret(*cursor, range, paint, xStart, xEnd);
        }
    }

    // show word wrap marker if desirable
    if ((!isPrinterFriendly()) && config()->wordWrapMarker()) {
        const QPainter::RenderHints backupRenderHints = paint.renderHints();
        paint.setPen(config()->wordWrapMarkerColor());
        int _x = qreal(m_doc->config()->wordWrapAt()) * fm.horizontalAdvance(QLatin1Char('x')) - xStart;
        paint.drawLine(_x, 0, _x, lineHeight());
        paint.setRenderHints(backupRenderHints);
    }

    // Draw inline notes
    if (!isPrinterFriendly()) {
        const auto inlineNotes = m_view->inlineNotes(range->line());
        for (const auto &inlineNoteData : inlineNotes) {
            KTextEditor::InlineNote inlineNote(inlineNoteData);
            const int column = inlineNote.position().column();
            const int viewLine = range->viewLineForColumn(column);
            // We only consider a line "rtl" if dynamic wrap is enabled. If it is disabled, our
            // text is always on the left side of the view
            const auto dir = range->layout()->textOption().textDirection();
            const bool isRTL = dir == Qt::RightToLeft && m_view->dynWordWrap();

            // Determine the position where to paint the note.
            // We start by getting the x coordinate of cursor placed to the column.
            // If the text is ltr or rtl + dyn wrap, get the X from column
            qreal x;
            if (dir == Qt::LeftToRight || (dir == Qt::RightToLeft && m_view->dynWordWrap())) {
                x = range->viewLine(viewLine).lineLayout().cursorToX(column) - xStart;
            } else /* rtl + dynWordWrap == false */ {
                // if text is rtl and dynamic wrap is false, the x offsets are in the opposite
                // direction i.e., [0] == biggest offset, [1] = next
                x = range->viewLine(viewLine).lineLayout().cursorToX(range->length() - column) - xStart;
            }
            int textLength = range->length();
            if (column == 0 || column < textLength) {
                // If the note is inside text or at the beginning, then there is a hole in the text where the
                // note should be painted and the cursor gets placed at the right side of it. So we have to
                // subtract the width of the note to get to left side of the hole.
                x -= inlineNote.width();
            } else {
                // If the note is outside the text, then the X coordinate is located at the end of the line.
                // Add appropriate amount of spaces to reach the required column.
                const auto spaceToAdd = spaceWidth() * (column - textLength);
                x += isRTL ? -spaceToAdd : spaceToAdd;
            }

            qreal y = lineHeight() * viewLine;

            // Paint the note
            paint.save();
            paint.translate(x, y);
            inlineNote.provider()->paintInlineNote(inlineNote, paint, isRTL ? Qt::RightToLeft : Qt::LeftToRight);
            paint.restore();
        }
    }
}

static void drawCursor(const QTextLayout &layout, QPainter *p, const QPointF &pos, int cursorPosition, int width, const int height)
{
    cursorPosition = qBound(0, cursorPosition, layout.text().length());
    const QTextLine l = layout.lineForTextPosition(cursorPosition);
    Q_ASSERT(l.isValid());
    if (!l.isValid()) {
        return;
    }
    const QPainter::CompositionMode origCompositionMode = p->compositionMode();
    if (p->paintEngine()->hasFeature(QPaintEngine::RasterOpModes)) {
        p->setCompositionMode(QPainter::RasterOp_NotDestination);
    }

    const QPointF position = pos + layout.position();
    const qreal x = position.x() + l.cursorToX(cursorPosition);
    const qreal y = l.lineNumber() * height;
    p->fillRect(QRectF(x, y, (qreal)width, (qreal)height), p->pen().brush());
    p->setCompositionMode(origCompositionMode);
}

void KateRenderer::paintCaret(KTextEditor::Cursor cursor, KateLineLayout *range, QPainter &paint, int xStart, int xEnd)
{
    if (range->includesCursor(cursor)) {
        int caretWidth;
        int lineWidth = 2;
        QColor color;
        QTextLine line = range->layout()->lineForTextPosition(qMin(cursor.column(), range->length()));

        // Determine the caret's style
        KTextEditor::caretStyles style = caretStyle();

        // Make the caret the desired width
        if (style == KTextEditor::caretStyles::Line) {
            caretWidth = lineWidth;
        } else if (line.isValid() && cursor.column() < range->length()) {
            caretWidth = int(line.cursorToX(cursor.column() + 1) - line.cursorToX(cursor.column()));
            if (caretWidth < 0) {
                caretWidth = -caretWidth;
            }
        } else {
            caretWidth = spaceWidth();
        }

        // Determine the color
        if (m_caretOverrideColor.isValid()) {
            // Could actually use the real highlighting system for this...
            // would be slower, but more accurate for corner cases
            color = m_caretOverrideColor;
        } else {
            // search for the FormatRange that includes the cursor
            const auto formatRanges = range->layout()->formats();
            for (const QTextLayout::FormatRange &r : formatRanges) {
                if ((r.start <= cursor.column()) && ((r.start + r.length) > cursor.column())) {
                    // check for Qt::NoBrush, as the returned color is black() and no invalid QColor
                    QBrush foregroundBrush = r.format.foreground();
                    if (foregroundBrush != Qt::NoBrush) {
                        color = r.format.foreground().color();
                    }
                    break;
                }
            }
            // still no color found, fall back to default style
            if (!color.isValid()) {
                color = attribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->foreground().color();
            }
        }

        paint.save();
        switch (style) {
        case KTextEditor::caretStyles::Line:
            paint.setPen(QPen(color, caretWidth));
            break;
        case KTextEditor::caretStyles::Block:
            // use a gray caret so it's possible to see the character
            color.setAlpha(128);
            paint.setPen(QPen(color, caretWidth));
            break;
        case KTextEditor::caretStyles::Underline:
            break;
        case KTextEditor::caretStyles::Half:
            color.setAlpha(128);
            paint.setPen(QPen(color, caretWidth));
            break;
        }

        if (cursor.column() <= range->length()) {
            // Ensure correct cursor placement for RTL text
            if (range->layout()->textOption().textDirection() == Qt::RightToLeft) {
                xStart += caretWidth;
            }
            qreal width = 0;
            if (cursor.column() < range->length()) {
                const auto inlineNotes = m_view->inlineNotes(range->line());
                for (const auto &inlineNoteData : inlineNotes) {
                    KTextEditor::InlineNote inlineNote(inlineNoteData);
                    if (inlineNote.position().column() == cursor.column()) {
                        width = inlineNote.width() + (caretStyle() == KTextEditor::caretStyles::Line ? 2.0 : 0.0);
                    }
                }
            }
            drawCursor(*range->layout(), &paint, QPoint(-xStart - width, 0), cursor.column(), caretWidth, lineHeight());
        } else {
            // Off the end of the line... must be block mode. Draw the caret ourselves.
            const KateTextLayout &lastLine = range->viewLine(range->viewLineCount() - 1);
            int x = cursorToX(lastLine, KTextEditor::Cursor(range->line(), cursor.column()), true);
            if ((x >= xStart) && (x <= xEnd)) {
                paint.fillRect(x - xStart, (int)lastLine.lineLayout().y(), caretWidth, lineHeight(), color);
            }
        }

        paint.restore();
    }
}

uint KateRenderer::fontHeight() const
{
    return m_fontHeight;
}

uint KateRenderer::documentHeight() const
{
    return m_doc->lines() * lineHeight();
}

int KateRenderer::lineHeight() const
{
    return fontHeight();
}

bool KateRenderer::getSelectionBounds(int line, int lineLength, int &start, int &end) const
{
    bool hasSel = false;

    if (m_view->selection() && !m_view->blockSelection()) {
        if (m_view->lineIsSelection(line)) {
            start = m_view->selectionRange().start().column();
            end = m_view->selectionRange().end().column();
            hasSel = true;
        } else if (line == m_view->selectionRange().start().line()) {
            start = m_view->selectionRange().start().column();
            end = lineLength;
            hasSel = true;
        } else if (m_view->selectionRange().containsLine(line)) {
            start = 0;
            end = lineLength;
            hasSel = true;
        } else if (line == m_view->selectionRange().end().line()) {
            start = 0;
            end = m_view->selectionRange().end().column();
            hasSel = true;
        }
    } else if (m_view->lineHasSelected(line)) {
        start = m_view->selectionRange().start().column();
        end = m_view->selectionRange().end().column();
        hasSel = true;
    }

    if (start > end) {
        int temp = end;
        end = start;
        start = temp;
    }

    return hasSel;
}

void KateRenderer::updateConfig()
{
    // update the attribute list pointer
    updateAttributes();

    // update font height, do this before we update the view!
    updateFontHeight();

    // trigger view update, if any!
    if (m_view) {
        m_view->updateRendererConfig();
    }
}

bool KateRenderer::hasCustomLineHeight() const
{
    return !qFuzzyCompare(config()->lineHeightMultiplier(), 1.0);
}

void KateRenderer::updateFontHeight()
{
    // cache font + metrics
    m_font = config()->baseFont();
    m_fontMetrics = QFontMetricsF(m_font);

    // ensure minimal height of one pixel to not fall in the div by 0 trap somewhere
    //
    // use a line spacing that matches the code in qt to layout/paint text
    //
    // see bug 403868
    // https://github.com/qt/qtbase/blob/5.12/src/gui/text/qtextlayout.cpp (line 2270 at the moment) where the text height is set as:
    //
    // qreal height = maxY + fontHeight - minY;
    //
    // with fontHeight:
    //
    // qreal fontHeight = font.ascent() + font.descent();
    m_fontHeight = qMax(1, qCeil(m_fontMetrics.ascent() + m_fontMetrics.descent()));
    m_fontAscent = m_fontMetrics.ascent();

    if (hasCustomLineHeight()) {
        const auto oldFontHeight = m_fontHeight;
        const qreal newFontHeight = qreal(m_fontHeight) * config()->lineHeightMultiplier();
        m_fontHeight = newFontHeight;

        qreal diff = std::abs(oldFontHeight - newFontHeight);
        m_fontAscent += (diff / 2);
    }
}

void KateRenderer::updateMarkerSize()
{
    // marker size will be calculated over the value defined
    // on dialog

    m_markerSize = spaceWidth() / (3.5 - (m_doc->config()->markerSize() * 0.5));
}

qreal KateRenderer::spaceWidth() const
{
    return m_fontMetrics.horizontalAdvance(spaceChar);
}

void KateRenderer::layoutLine(KateLineLayout *lineLayout, int maxwidth, bool cacheLayout) const
{
    // if maxwidth == -1 we have no wrap

    Kate::TextLine textLine = lineLayout->textLine();

    QTextLayout *l = lineLayout->layout();
    if (!l) {
        l = new QTextLayout(textLine.text(), m_font);
    } else {
        l->setText(textLine.text());
        l->setFont(m_font);
    }

    l->setCacheEnabled(cacheLayout);

    // Initial setup of the QTextLayout.

    // Tab width
    QTextOption opt;
    opt.setFlags(QTextOption::IncludeTrailingSpaces);
    opt.setTabStopDistance(m_tabWidth * m_fontMetrics.horizontalAdvance(spaceChar));
    if (m_view && m_view->config()->dynWrapAnywhere()) {
        opt.setWrapMode(QTextOption::WrapAnywhere);
    } else {
        opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    }

    // Find the first strong character in the string.
    // If it is an RTL character, set the base layout direction of the string to RTL.
    //
    // See https://www.unicode.org/reports/tr9/#The_Paragraph_Level (Sections P2 & P3).
    // Qt's text renderer ("scribe") version 4.2 assumes a "higher-level protocol"
    // (such as KatePart) will specify the paragraph level, so it does not apply P2 & P3
    // by itself. If this ever change in Qt, the next code block could be removed.
    // -----
    // Only force RTL direction if dynWordWrap is on. Otherwise the view has infinite width
    // and the lines will never be forced RTL no matter what direction we set. The layout
    // can't force a line to the right if it doesn't know where the "right" is
    if (isLineRightToLeft(textLine.text()) || (view()->dynWordWrap() && view()->forceRTLDirection())) {
        opt.setAlignment(Qt::AlignRight);
        opt.setTextDirection(Qt::RightToLeft);
        // Must turn off this flag otherwise cursor placement
        // is totally broken.
        if (view()->config()->dynWordWrap()) {
            auto flags = opt.flags();
            flags.setFlag(QTextOption::IncludeTrailingSpaces, false);
            opt.setFlags(flags);
        }
    } else {
        opt.setAlignment(Qt::AlignLeft);
        opt.setTextDirection(Qt::LeftToRight);
    }

    l->setTextOption(opt);

    // Syntax highlighting, inbuilt and arbitrary
    QList<QTextLayout::FormatRange> decorations = decorationsForLine(textLine, lineLayout->line());

    // Qt works badly if you have RTL text and formats set on that text.
    // It will shape the text according to the given format ranges which
    // produces incorrect results as a letter in RTL can have a different
    // shape depending upon where in the word it resides. The resulting output
    // looks like: وقار vs وق ار, i.e, the ligature قا is broken into ق ا which
    // is really bad for readability
    if (opt.textDirection() == Qt::RightToLeft) {
        // We can fix this to a large extent here by using QGlyphRun etc, but for now
        // we only fix this for formats which have a background color and a foreground
        // color that is same as "dsNormal". Reasoning is that, it is unlikely that RTL
        // text will have a lot of cases where you have partially colored ligatures. BG
        // formats are different, you can easily have a format that covers a ligature partially
        // as a result of "Search" or "multiple cursor selection"
        QColor c = view()->theme().textColor(KSyntaxHighlighting::Theme::Normal);
        decorations.erase(std::remove_if(decorations.begin(),
                                         decorations.end(),
                                         [c](const QTextLayout::FormatRange &fr) {
                                             return fr.format.hasProperty(QTextFormat::BackgroundBrush)
                                                 && (fr.format.property(QTextFormat::ForegroundBrush).value<QBrush>().color() == c
                                                     || fr.format.foreground() == Qt::NoBrush);
                                         }),
                          decorations.end());
    }

    int firstLineOffset = 0;

    if (!isPrinterFriendly() && m_view) {
        const auto inlineNotes = m_view->inlineNotes(lineLayout->line());
        for (const KateInlineNoteData &noteData : inlineNotes) {
            const KTextEditor::InlineNote inlineNote(noteData);
            const int column = inlineNote.position().column();
            int width = inlineNote.width();

            // Make space for every inline note.
            // If it is on column 0 (at the beginning of the line), we must offset the first line.
            // If it is inside the text, we use absolute letter spacing to create space for it between the two letters.
            // If it is outside of the text, we don't have to make space for it.
            if (column == 0) {
                firstLineOffset = width;
            } else if (column < l->text().length()) {
                QTextCharFormat text_char_format;
                const qreal caretWidth = caretStyle() == KTextEditor::caretStyles::Line ? 2.0 : 0.0;
                text_char_format.setFontLetterSpacing(width + caretWidth);
                text_char_format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                decorations.append(QTextLayout::FormatRange{.start=column - 1, .length=1, .format=text_char_format});
            }
        }
    }
    l->setFormats(decorations);

    // Begin layouting
    l->beginLayout();

    int height = 0;
    int shiftX = 0;

    bool needShiftX = (maxwidth != -1) && m_view && (m_view->config()->dynWordWrapAlignIndent() > 0);

    while (true) {
        QTextLine line = l->createLine();
        if (!line.isValid()) {
            break;
        }

        if (maxwidth > 0) {
            line.setLineWidth(maxwidth);
        } else {
            line.setLineWidth(INT_MAX);
        }

        // we include the leading, this must match the ::updateFontHeight code!
        line.setLeadingIncluded(true);

        line.setPosition(QPoint(line.lineNumber() ? shiftX : firstLineOffset, height - line.ascent() + m_fontAscent));

        if (needShiftX && line.width() > 0) {
            needShiftX = false;
            // Determine x offset for subsequent-lines-of-paragraph indenting
            int pos = textLine.nextNonSpaceChar(0);

            if (pos > 0) {
                shiftX = (int)line.cursorToX(pos);
            }

            // check for too deep shift value and limit if necessary
            if (m_view && shiftX > ((double)maxwidth / 100 * m_view->config()->dynWordWrapAlignIndent())) {
                shiftX = 0;
            }

            // if shiftX > 0, the maxwidth has to adapted
            maxwidth -= shiftX;

            lineLayout->shiftX = shiftX;
        }

        height += lineHeight();
    }

    l->endLayout();

    lineLayout->setLayout(l);
}

// 1) QString::isRightToLeft() sux
// 2) QString::isRightToLeft() is marked as internal (WTF?)
// 3) QString::isRightToLeft() does not seem to work on my setup
// 4) isStringRightToLeft() should behave much better than QString::isRightToLeft() therefore:
// 5) isStringRightToLeft() kicks ass
bool KateRenderer::isLineRightToLeft(QStringView str)
{
    // borrowed from QString::updateProperties()
    for (auto c : str) {
        switch (c.direction()) {
        case QChar::DirL:
        case QChar::DirLRO:
        case QChar::DirLRE:
            return false;

        case QChar::DirR:
        case QChar::DirAL:
        case QChar::DirRLO:
        case QChar::DirRLE:
            return true;

        default:
            break;
        }
    }

    return false;
#if 0
    // or should we use the direction of the widget?
    QWidget *display = qobject_cast<QWidget *>(view()->parent());
    if (!display) {
        return false;
    }
    return display->layoutDirection() == Qt::RightToLeft;
#endif
}

int KateRenderer::cursorToX(const KateTextLayout &range, int col, bool returnPastLine) const
{
    return cursorToX(range, KTextEditor::Cursor(range.line(), col), returnPastLine);
}

int KateRenderer::cursorToX(const KateTextLayout &range, const KTextEditor::Cursor pos, bool returnPastLine) const
{
    Q_ASSERT(range.isValid());

    int x;
    if (range.lineLayout().width() > 0) {
        x = (int)range.lineLayout().cursorToX(pos.column());
    } else {
        x = 0;
    }

    int over = pos.column() - range.endCol();
    if (returnPastLine && over > 0) {
        x += over * spaceWidth();
    }

    return x;
}

KTextEditor::Cursor KateRenderer::xToCursor(const KateTextLayout &range, int x, bool returnPastLine) const
{
    Q_ASSERT(range.isValid());
    KTextEditor::Cursor ret(range.line(), range.lineLayout().xToCursor(x));

    // Do not wrap to the next line. (bug #423253)
    if (range.wrap() && ret.column() >= range.endCol() && range.length() > 0) {
        ret.setColumn(range.endCol() - 1);
    }
    // TODO wrong for RTL lines?
    if (returnPastLine && range.endCol(true) == -1 && x > range.width() + range.xOffset()) {
        ret.setColumn(ret.column() + round((x - (range.width() + range.xOffset())) / spaceWidth()));
    }

    return ret;
}

void KateRenderer::setCaretOverrideColor(const QColor &color)
{
    m_caretOverrideColor = color;
}

void KateRenderer::paintSelection(QPaintDevice *d, int startLine, int xStart, int endLine, int xEnd, qreal scale)
{
    if (!d || scale < 0.0) {
        return;
    }

    const int lineHeight = std::max(1, this->lineHeight());
    QPainter paint(d);
    paint.scale(scale, scale);

    // clip out non selected parts of start / end line
    {
        QRect mainRect(0, 0, d->width(), d->height());
        QRegion main(mainRect);
        // start line
        QRect startRect(0, 0, xStart, lineHeight);
        QRegion startRegion(startRect);
        // end line
        QRect endRect(mainRect.bottomLeft().x() + xEnd, mainRect.bottomRight().y() - lineHeight, mainRect.width() - xEnd, lineHeight);
        QRegion drawRegion = main.subtracted(startRegion).subtracted(QRegion(endRect));
        paint.setClipRegion(drawRegion);
    }

    for (int line = startLine; line <= endLine; ++line) {
        // get real line, skip if invalid!
        if (line < 0 || line >= doc()->lines()) {
            continue;
        }

        // compute layout WITHOUT cache to not poison it + render it
        KateLineLayout lineLayout(*this);
        lineLayout.setLine(line, -1);
        layoutLine(&lineLayout, -1 /* no wrap */, false /* no layout cache */);
        KateRenderer::PaintTextLineFlags flags;
        flags.setFlag(KateRenderer::SkipDrawFirstInvisibleLineUnderlined);
        flags.setFlag(KateRenderer::SkipDrawLineSelection);
        paintTextLine(paint, &lineLayout, 0, 0, QRectF{}, nullptr, flags);

        // translate for next line
        paint.translate(0, lineHeight);
    }
}
