/*
    SPDX-FileCopyrightText: 2002-2010 Anders Lund <anders@alweb.dk>

    Rewritten based on code of:
    SPDX-FileCopyrightText: 2002 Michael Goffioul <kdeprint@swing.be>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "printpainter.h"

#include "katebuffer.h"
#include "katedocument.h"
#include "katehighlight.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "katetextfolding.h"
#include "katetextlayout.h"
#include "kateview.h"

#include <KLocalizedString>
#include <KUser>

#include <QPainter>
#include <QPrinter>

using namespace KatePrinter;

class KatePrinter::PageLayout
{
public:
    PageLayout()
        : headerTagList()
        , footerTagList()
        , selectionRange()
    {
    }

    uint pageWidth = 0;
    uint pageHeight = 0;
    uint headerWidth = 0;
    uint maxWidth = 0;
    uint maxHeight = 0;
    int xstart = 0; // beginning point for painting lines
    int innerMargin = 0;

    bool selectionOnly = false;

    uint firstline = 0;
    uint lastline = 0;

    // Header/Footer Page
    uint headerHeight = 0;
    QStringList headerTagList;
    uint footerHeight = 0;
    QStringList footerTagList;

    KTextEditor::Range selectionRange;
};

PrintPainter::PrintPainter(KTextEditor::DocumentPrivate *doc, KTextEditor::ViewPrivate *view)
    : m_view(view)
    , m_doc(doc)
    , m_printGuide(false)
    , m_printLineNumbers(false)
    , m_useHeader(false)
    , m_useFooter(false)
    , m_useBackground(false)
    , m_useBox(false)
    , m_useHeaderBackground(false)
    , m_useFooterBackground(false)
    , m_boxMargin(0)
    , m_boxWidth(1)
    , m_boxColor(Qt::black)
    , m_headerBackground(Qt::lightGray)
    , m_headerForeground(Qt::black)
    , m_footerBackground(Qt::lightGray)
    , m_footerForeground(Qt::black)
    , m_fhFont()
    , m_headerFormat()
    , m_footerFormat()
{
    m_renderer = new KateRenderer(m_doc, m_view->renderer()->folding(), m_view);
    m_renderer->setPrinterFriendly(true);

    updateCache();
}

PrintPainter::~PrintPainter()
{
    delete m_renderer;
}

void PrintPainter::setTextFont(const QFont &font)
{
    m_view->rendererConfig()->setFont(font);
}

void PrintPainter::setUseBox(const bool on)
{
    m_useBox = on;
    setBoxWidth(m_boxWidth); // reset the width
}

void PrintPainter::setBoxWidth(const int width)
{
    if (m_useBox) {
        m_boxWidth = width;
        if (width < 1) {
            m_boxWidth = 1;
        }
    } else {
        m_boxWidth = 0;
    }
}

void PrintPainter::setBoxColor(const QColor &color)
{
    if (color.isValid()) {
        m_boxColor = color;
    }
}

void PrintPainter::setHeaderBackground(const QColor &color)
{
    if (color.isValid()) {
        m_headerBackground = color;
    }
}

void PrintPainter::setHeaderForeground(const QColor &color)
{
    if (color.isValid()) {
        m_headerForeground = color;
    }
}

void PrintPainter::setFooterBackground(const QColor &color)
{
    if (color.isValid()) {
        m_footerBackground = color;
    }
}
void PrintPainter::setFooterForeground(const QColor &color)
{
    if (color.isValid()) {
        m_footerForeground = color;
    }
}

void PrintPainter::setColorScheme(const QString &scheme)
{
    // directly set that for the renderer
    m_view->rendererConfig()->setSchema(scheme);

    // changed renderer requires cache updates
    updateCache();
}

void PrintPainter::updateCache()
{
    m_fontHeight = m_renderer->fontHeight();

    // figure out the horizontal space required
    QString s = QStringLiteral("%1 ").arg(m_doc->lines());
    s.fill(QLatin1Char('5'), -1); // some non-fixed fonts haven't equally wide numbers
    // FIXME calculate which is actually the widest...
    m_lineNumberWidth = m_renderer->currentFontMetrics().boundingRect(s).width();
}

void PrintPainter::paint(QPrinter *printer) const
{
    QPainter painter(printer);
    PageLayout pl;

    configure(printer, pl);

    uint lineCount = pl.firstline;
    uint y = 0;
    uint currentPage = (printer->fromPage() == 0) ? 1 : printer->fromPage();
    bool pageStarted = true;
    uint remainder = 0;

    auto &f = m_view->renderer()->folding();

    // On to draw something :-)
    while (lineCount <= pl.lastline) {
        if (y + m_fontHeight > pl.maxHeight) {
            if ((int)currentPage == printer->toPage()) { // we've reached the page break of last page to be printed
                break;
            }
            printer->newPage();
            painter.resetTransform();
            currentPage++;
            pageStarted = true;
            y = 0;
        }

        if (pageStarted) {
            qCDebug(LOG_KTE) << "Starting new page," << lineCount << "lines up to now.";
            paintNewPage(painter, currentPage, y, pl);
            pageStarted = false;
            painter.translate(pl.xstart, y);
        }

        const bool skipLine = m_dontPrintFoldedCode && !f.isLineVisible(lineCount);

        if (!skipLine && m_printLineNumbers /*&& ! startCol*/) { // don't repeat!
            paintLineNumber(painter, lineCount, pl);
        }

        if (!skipLine) {
            paintLine(painter, lineCount, y, remainder, pl);
        }

        if (!remainder) {
            lineCount++;
        }
    }

    painter.end();
}

void PrintPainter::configure(const QPrinter *printer, PageLayout &pl) const
{
    pl.pageHeight = printer->height();
    pl.pageWidth = printer->width();
    pl.headerWidth = printer->width();
    pl.innerMargin = m_useBox ? m_boxMargin : 6;
    pl.maxWidth = printer->width();
    pl.maxHeight = (m_useBox ? printer->height() - pl.innerMargin : printer->height());
    pl.selectionOnly = (printer->printRange() == QPrinter::Selection);
    pl.lastline = m_doc->lastLine();

    if (m_view && pl.selectionOnly) {
        // set a line range from the first selected line to the last
        pl.selectionRange = m_view->selectionRange();
        pl.firstline = pl.selectionRange.start().line();
        pl.lastline = pl.selectionRange.end().line();
    }

    if (m_printLineNumbers) {
        // a small space between the line numbers and the text
        int _adj = m_renderer->currentFontMetrics().horizontalAdvance(QStringLiteral("5"));
        // adjust available width and set horizontal start point for data
        pl.maxWidth -= m_lineNumberWidth + _adj;
        pl.xstart += m_lineNumberWidth + _adj;
    }

    if (m_useHeader || m_useFooter) {
        // Set up a tag map
        // This retrieves all tags, used or not, but
        // none of these operations should be expensive,
        // and searching each tag in the format strings is avoided.
        QDateTime dt = QDateTime::currentDateTime();
        std::map<QString, QString> tags;

        KUser u(KUser::UseRealUserID);
        tags[QStringLiteral("u")] = u.loginName();

        tags[QStringLiteral("d")] = QLocale().toString(dt, QLocale::ShortFormat);
        tags[QStringLiteral("D")] = QLocale().toString(dt, QLocale::LongFormat);
        tags[QStringLiteral("h")] = QLocale().toString(dt.time(), QLocale::ShortFormat);
        tags[QStringLiteral("y")] = QLocale().toString(dt.date(), QLocale::ShortFormat);
        tags[QStringLiteral("Y")] = QLocale().toString(dt.date(), QLocale::LongFormat);
        tags[QStringLiteral("f")] = m_doc->url().fileName();
        tags[QStringLiteral("U")] = m_doc->url().toString();
        if (pl.selectionOnly) {
            QString s(i18n("(Selection of) "));
            tags[QStringLiteral("f")].prepend(s);
            tags[QStringLiteral("U")].prepend(s);
        }

        static const QRegularExpression reTags(QStringLiteral("%([dDfUhuyY])")); // TODO check for "%%<TAG>"

        if (m_useHeader) {
            pl.headerHeight = QFontMetrics(m_fhFont).height();
            if (m_useBox || m_useHeaderBackground) {
                pl.headerHeight += pl.innerMargin * 2;
            } else {
                pl.headerHeight += 1 + QFontMetrics(m_fhFont).leading();
            }

            pl.headerTagList = m_headerFormat;
            QMutableStringListIterator it(pl.headerTagList);
            while (it.hasNext()) {
                QString tag = it.next();
                QRegularExpressionMatch match;
                int pos = tag.indexOf(reTags, 0, &match);
                QString rep;
                while (pos > -1) {
                    rep = tags[match.captured(1)];
                    tag.replace((uint)pos, 2, rep);
                    pos += rep.length();
                    pos = tag.indexOf(reTags, pos, &match);
                }
                it.setValue(tag);
            }
        }

        if (m_useFooter) {
            pl.footerHeight = QFontMetrics(m_fhFont).height();
            if (m_useBox || m_useFooterBackground) {
                pl.footerHeight += 2 * pl.innerMargin;
            } else {
                pl.footerHeight += 1; // line only
            }

            pl.footerTagList = m_footerFormat;
            QMutableStringListIterator it(pl.footerTagList);
            while (it.hasNext()) {
                QString tag = it.next();
                QRegularExpressionMatch match;
                int pos = tag.indexOf(reTags, 0, &match);
                QString rep;
                while (pos > -1) {
                    rep = tags[match.captured(1)];
                    tag.replace((uint)pos, 2, rep);
                    pos += rep.length();
                    pos = tag.indexOf(reTags, pos, &match);
                }
                it.setValue(tag);
            }

            pl.maxHeight -= pl.footerHeight;
        }
    } // if ( useHeader || useFooter )

    if (m_useBackground) {
        if (!m_useBox) {
            pl.xstart += pl.innerMargin;
            pl.maxWidth -= pl.innerMargin * 2;
        }
    }

    if (m_useBox) {
        // set maxwidth to something sensible
        pl.maxWidth -= (m_boxWidth + pl.innerMargin) * 2;
        pl.xstart += m_boxWidth + pl.innerMargin;
        // maxheight too..
        pl.maxHeight -= m_boxWidth;
    }

    int pageHeight = pl.maxHeight;
    if (m_useHeader) {
        pageHeight -= pl.headerHeight + pl.innerMargin;
    }
    if (m_useFooter) {
        pageHeight -= pl.footerHeight + pl.innerMargin;
    }

    const int linesPerPage = pageHeight / m_fontHeight;

    if (printer->fromPage() > 0) {
        pl.firstline = (printer->fromPage() - 1) * linesPerPage;
    }

    // now that we know the vertical amount of space needed,
    // it is possible to calculate the total number of pages
    // if needed, that is if any header/footer tag contains "%P".
    if (!pl.headerTagList.filter(QStringLiteral("%P")).isEmpty() || !pl.footerTagList.filter(QStringLiteral("%P")).isEmpty()) {
        qCDebug(LOG_KTE) << "'%P' found! calculating number of pages...";

        // calculate total layouted lines in the document
        int totalLines = 0;
        // TODO: right now ignores selection printing
        for (unsigned int i = pl.firstline; i <= pl.lastline; ++i) {
            KateLineLayout rangeptr(*m_renderer);
            rangeptr.setLine(i);
            m_renderer->layoutLine(&rangeptr, (int)pl.maxWidth, false);
            totalLines += rangeptr.viewLineCount();
        }

        const int totalPages = (totalLines / linesPerPage) + ((totalLines % linesPerPage) > 0 ? 1 : 0);

        // TODO: add space for guide if required
        //         if ( useGuide )
        //           _lt += (guideHeight + (fontHeight /2)) / fontHeight;

        // substitute both tag lists
        QString re(QStringLiteral("%P"));
        QStringList::Iterator it;

        for (it = pl.headerTagList.begin(); it != pl.headerTagList.end(); ++it) {
            it->replace(re, QString::number(totalPages));
        }

        for (it = pl.footerTagList.begin(); it != pl.footerTagList.end(); ++it) {
            (*it).replace(re, QString::number(totalPages));
        }
    }
}

void PrintPainter::paintNewPage(QPainter &painter, const uint currentPage, uint &y, const PageLayout &pl) const
{
    if (m_useHeader) {
        paintHeader(painter, currentPage, y, pl);
    }

    if (m_useFooter) {
        paintFooter(painter, currentPage, pl);
    }

    if (m_useBackground) {
        paintBackground(painter, y, pl);
    }

    if (m_useBox) {
        paintBox(painter, y, pl);
    }

    if (m_printGuide && currentPage == 1) {
        paintGuide(painter, y, pl);
    }
}

void PrintPainter::paintHeader(QPainter &painter, const uint currentPage, uint &y, const PageLayout &pl) const
{
    painter.save();
    painter.setPen(QPen(m_headerForeground, 0.5));
    painter.setFont(m_fhFont);

    if (m_useHeaderBackground) {
        painter.fillRect(0, 0, pl.headerWidth, pl.headerHeight, m_headerBackground);
    }

    if (pl.headerTagList.count() == 3) {
        int valign = (m_useBox || m_useHeaderBackground || m_useBackground) ? Qt::AlignVCenter : Qt::AlignTop;
        int align = valign | Qt::AlignLeft;
        int marg = (m_useBox || m_useHeaderBackground) ? pl.innerMargin : 0;
        if (m_useBox) {
            marg += m_boxWidth;
        }

        QString s;
        for (int i = 0; i < 3; i++) {
            s = pl.headerTagList[i];
            if (s.indexOf(QLatin1String("%p")) != -1) {
                s.replace(QLatin1String("%p"), QString::number(currentPage));
            }

            painter.drawText(marg, 0, pl.headerWidth - (marg * 2), pl.headerHeight, align, s);
            align = valign | (i == 0 ? Qt::AlignHCenter : Qt::AlignRight);
        }
    }

    if (!(m_useHeaderBackground || m_useBox || m_useBackground)) { // draw a 1 px (!?) line to separate header from contents
        painter.drawLine(0, pl.headerHeight - 1, pl.headerWidth, pl.headerHeight - 1);
        // y += 1; now included in headerHeight
    }

    painter.restore();

    y += pl.headerHeight + pl.innerMargin;
}

void PrintPainter::paintFooter(QPainter &painter, const uint currentPage, const PageLayout &pl) const
{
    painter.save();
    painter.setPen(QPen(m_footerForeground, 0.5));
    painter.setFont(m_fhFont);

    if (!(m_useFooterBackground || m_useBox || m_useBackground)) { // draw a 1 px (!?) line to separate footer from contents
        painter.drawLine(0, pl.pageHeight - pl.footerHeight - 1, pl.headerWidth, pl.pageHeight - pl.footerHeight - 1);
    }
    if (m_useFooterBackground) {
        painter.fillRect(0, pl.pageHeight - pl.footerHeight, pl.headerWidth, pl.footerHeight, m_footerBackground);
    }

    if (pl.footerTagList.count() == 3) {
        int align = Qt::AlignVCenter | Qt::AlignLeft;
        int marg = (m_useBox || m_useFooterBackground) ? pl.innerMargin : 0;
        if (m_useBox) {
            marg += m_boxWidth;
        }

        QString s;
        for (int i = 0; i < 3; i++) {
            s = pl.footerTagList[i];
            if (s.indexOf(QLatin1String("%p")) != -1) {
                s.replace(QLatin1String("%p"), QString::number(currentPage));
            }
            painter.drawText(marg, pl.pageHeight - pl.footerHeight, pl.headerWidth - (marg * 2), pl.footerHeight, align, s);
            align = Qt::AlignVCenter | (i == 0 ? Qt::AlignHCenter : Qt::AlignRight);
        }
    }
    painter.restore();
}

void PrintPainter::paintGuide(QPainter &painter, uint &y, const PageLayout &pl) const
{
    // FIXME - this may span more pages...
    // draw a box unless we have boxes, in which case we end with a box line
    int _ystart = y;
    QString _hlName = m_doc->highlight()->name();

    // list of highlight attributes for the legend
    const auto _attributes = m_doc->highlight()->attributesForDefinition(m_view->rendererConfig()->schema());
    const QColor _defaultPen = _attributes.at(0)->foreground().color();

    painter.save();
    painter.setPen(_defaultPen);

    int _marg = 0;
    if (m_useBox) {
        _marg += (2 * m_boxWidth) + (2 * pl.innerMargin);
    } else {
        if (m_useBackground) {
            _marg += 2 * pl.innerMargin;
        }
        _marg += 1;
        y += 1 + pl.innerMargin;
    }

    // draw a title string
    QFont _titleFont = m_renderer->currentFont();
    _titleFont.setBold(true);
    painter.setFont(_titleFont);
    QRect _r;
    painter.drawText(QRect(_marg, y, pl.pageWidth - (2 * _marg), pl.maxHeight - y),
                     Qt::AlignTop | Qt::AlignHCenter,
                     i18n("Typographical Conventions for %1", _hlName),
                     &_r);
    const int _w = pl.pageWidth - (_marg * 2) - (pl.innerMargin * 2);
    const int _x = _marg + pl.innerMargin;
    y += _r.height() + pl.innerMargin;
    painter.drawLine(_x, y, _x + _w, y);
    y += 1 + pl.innerMargin;

    int _widest(0);
    for (const KTextEditor::Attribute::Ptr &attribute : std::as_const(_attributes)) {
        const QString _name = attribute->name().section(QLatin1Char(':'), 1, 1);
        _widest = qMax(QFontMetrics(attribute->font()).boundingRect(_name).width(), _widest);
    }

    const int _guideCols = _w / (_widest + pl.innerMargin);

    // draw attrib names using their styles
    const int _cw = _w / _guideCols;
    int _i = 0;

    _titleFont.setUnderline(true);
    QString _currentHlName;
    for (const KTextEditor::Attribute::Ptr &attribute : std::as_const(_attributes)) {
        QString _hl = attribute->name().section(QLatin1Char(':'), 0, 0);
        QString _name = attribute->name().section(QLatin1Char(':'), 1, 1);
        if (_hl != _hlName && _hl != _currentHlName) {
            _currentHlName = _hl;
            if (_i % _guideCols) {
                y += m_fontHeight;
            }
            y += pl.innerMargin;
            painter.setFont(_titleFont);
            painter.setPen(_defaultPen);
            painter.drawText(_x, y, _w, m_fontHeight, Qt::AlignTop, _hl + QLatin1Char(' ') + i18n("text"));
            y += m_fontHeight;
            _i = 0;
        }

        painter.setPen(attribute->foreground().color());
        painter.setFont(attribute->font());

        if (attribute->hasProperty(QTextFormat::BackgroundBrush)) {
            QRect _rect = QFontMetrics(attribute->font()).boundingRect(_name);
            _rect.moveTo(_x + ((_i % _guideCols) * _cw), y);
            painter.fillRect(_rect, attribute->background());
        }

        painter.drawText((_x + ((_i % _guideCols) * _cw)), y, _cw, m_fontHeight, Qt::AlignTop, _name);

        _i++;
        if (_i && !(_i % _guideCols)) {
            y += m_fontHeight;
        }
    }

    if (_i % _guideCols) {
        y += m_fontHeight; // last row not full
    }
    // draw a box around the legend
    painter.setPen(_defaultPen);

    if (m_useBox) {
        painter.fillRect(0, y + pl.innerMargin, pl.headerWidth, m_boxWidth, m_boxColor);
    } else {
        _marg -= 1;
        painter.drawRect(_marg, _ystart, pl.pageWidth - (2 * _marg), y - _ystart + pl.innerMargin);
    }

    painter.restore();

    y += (m_useBox ? m_boxWidth : 1) + (pl.innerMargin * 2);
}

void PrintPainter::paintBox(QPainter &painter, uint &y, const PageLayout &pl) const
{
    painter.save();
    painter.setPen(QPen(m_boxColor, m_boxWidth));
    painter.drawRect(0, 0, pl.pageWidth, pl.pageHeight);

    if (m_useHeader) {
        painter.drawLine(0, pl.headerHeight, pl.headerWidth, pl.headerHeight);
    } else {
        y += pl.innerMargin;
    }

    if (m_useFooter) { // drawline is not trustable, grr.
        painter.fillRect(0, pl.maxHeight + pl.innerMargin, pl.headerWidth, m_boxWidth, m_boxColor);
    }

    painter.restore();
}

void PrintPainter::paintBackground(QPainter &painter, const uint y, const PageLayout &pl) const
{
    // If we have a box, or the header/footer has backgrounds, we want to paint
    // to the border of those. Otherwise just the contents area.
    int _y = y;
    int _h = pl.maxHeight - y;
    if (m_useBox) {
        _y -= pl.innerMargin;
        _h += 2 * pl.innerMargin;
    } else {
        if (m_useHeaderBackground) {
            _y -= pl.innerMargin;
            _h += pl.innerMargin;
        }
        if (m_useFooterBackground) {
            _h += pl.innerMargin;
        }
    }
    painter.fillRect(0, _y, pl.pageWidth, _h, m_view->rendererConfig()->backgroundColor());
}

void PrintPainter::paintLine(QPainter &painter, const uint line, uint &y, uint &remainder, const PageLayout &pl) const
{
    // HA! this is where we print [part of] a line ;]]
    KateLineLayout rangeptr(*m_renderer);
    rangeptr.setLine(line);
    m_renderer->layoutLine(&rangeptr, (int)pl.maxWidth, false);

    // selectionOnly: clip non-selection parts and adjust painter position if needed
    int _xadjust = 0;
    if (pl.selectionOnly) {
        if (m_view && m_view->blockSelection()) {
            int _x = m_renderer->cursorToX(rangeptr.viewLine(0), pl.selectionRange.start());
            int _x1 = m_renderer->cursorToX(rangeptr.viewLine(rangeptr.viewLineCount() - 1), pl.selectionRange.end());
            _xadjust = _x;
            painter.translate(-_xadjust, 0);
            painter.setClipRegion(QRegion(_x, 0, _x1 - _x, rangeptr.viewLineCount() * m_fontHeight));

        } else if (line == pl.firstline || line == pl.lastline) {
            QRegion region(0, 0, pl.maxWidth, rangeptr.viewLineCount() * m_fontHeight);

            if (line == pl.firstline) {
                region = region.subtracted(QRegion(0, 0, m_renderer->cursorToX(rangeptr.viewLine(0), pl.selectionRange.start()), m_fontHeight));
            }

            if (line == pl.lastline) {
                int _x = m_renderer->cursorToX(rangeptr.viewLine(rangeptr.viewLineCount() - 1), pl.selectionRange.end());
                region = region.subtracted(QRegion(_x, 0, pl.maxWidth - _x, m_fontHeight));
            }

            painter.setClipRegion(region);
        }
    }

    // If the line is too long (too many 'viewlines') to fit the remaining vertical space,
    // clip and adjust the painter position as necessary
    int _lines = rangeptr.viewLineCount(); // number of "sublines" to paint.

    int proceedLines = _lines;
    if (remainder) {
        proceedLines = qMin((pl.maxHeight - y) / m_fontHeight, remainder);

        painter.translate(0, -(_lines - int(remainder)) * m_fontHeight + 1);
        painter.setClipRect(0,
                            (_lines - int(remainder)) * m_fontHeight + 1,
                            pl.maxWidth,
                            proceedLines * m_fontHeight); // ### drop the crosspatch in printerfriendly mode???
        remainder -= proceedLines;
    } else if (y + m_fontHeight * _lines > pl.maxHeight) {
        remainder = _lines - ((pl.maxHeight - y) / m_fontHeight);
        painter.setClipRect(0, 0, pl.maxWidth, (_lines - int(remainder)) * m_fontHeight + 1); // ### drop the crosspatch in printerfriendly mode???
    } else if (!pl.selectionOnly) {
        painter.setClipRegion(QRegion());
        painter.setClipping(false);
    }

    KateRenderer::PaintTextLineFlags flags;
    if (!m_dontPrintFoldedCode) {
        flags.setFlag(KateRenderer::PaintTextLineFlag::SkipDrawFirstInvisibleLineUnderlined);
    }

    m_renderer->paintTextLine(painter, &rangeptr, 0, (int)pl.maxWidth, QRectF{}, nullptr, flags);

    painter.setClipping(false);
    painter.translate(_xadjust, (m_fontHeight * (_lines - remainder)));

    y += m_fontHeight * proceedLines;
}

void PrintPainter::paintLineNumber(QPainter &painter, const uint number, const PageLayout &pl) const
{
    const int left = ((m_useBox || m_useBackground) ? pl.innerMargin : 0) - pl.xstart;

    painter.save();
    painter.setFont(m_renderer->currentFont());
    painter.setPen(m_view->rendererConfig()->lineNumberColor());
    painter.drawText(left, 0, m_lineNumberWidth, m_fontHeight, Qt::AlignRight | Qt::AlignVCenter, QString::number(number + 1));
    painter.restore();
}

// END PrintPainter
