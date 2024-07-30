/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008, 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012 Kåre Särs <kare.sars@iki.fi> (Minimap)
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kateviewhelpers.h"

#include "kateabstractinputmode.h"
#include "kateannotationitemdelegate.h"
#include "katecmd.h"
#include "katecommandrangeexpressionparser.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katelayoutcache.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "katesyntaxmanager.h"
#include "katetextlayout.h"
#include "katetextpreview.h"
#include "kateview.h"
#include "kateviewinternal.h"
#include <katebuffer.h>
#include <ktexteditor/annotationinterface.h>
#include <ktexteditor/attribute.h>
#include <ktexteditor/command.h>
#include <ktexteditor/movingrange.h>

#include <KActionCollection>
#include <KCharsets>
#include <KColorUtils>
#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>

#include <QAction>
#include <QActionGroup>
#include <QBoxLayout>
#include <QCursor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPen>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QToolTip>
#include <QVariant>
#include <QWhatsThis>
#include <QtAlgorithms>

#include <math.h>

// BEGIN KateMessageLayout
KateMessageLayout::KateMessageLayout(QWidget *parent)
    : QLayout(parent)
{
    qCDebug(LOG_KTE);
}

KateMessageLayout::~KateMessageLayout()
{
    while (QLayoutItem *item = takeAt(0)) {
        delete item;
    }
}

void KateMessageLayout::addItem(QLayoutItem *item)
{
    Q_ASSERT(false);
    add(item, KTextEditor::Message::CenterInView);
}

void KateMessageLayout::addWidget(QWidget *widget, KTextEditor::Message::MessagePosition pos)
{
    add(new QWidgetItem(widget), pos);
}

int KateMessageLayout::count() const
{
    return m_items.size();
}

QLayoutItem *KateMessageLayout::itemAt(int index) const
{
    return m_items.value(index).item;
}

void KateMessageLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    const int s = spacing();
    const QRect adjustedRect = rect.adjusted(s, s, -s, -s);

    for (const auto &wrapper : std::as_const(m_items)) {
        QLayoutItem *item = wrapper.item;
        auto position = wrapper.position;

        if (position == KTextEditor::Message::TopInView) {
            const QRect r(adjustedRect.width() - item->sizeHint().width(), s, item->sizeHint().width(), item->sizeHint().height());
            item->setGeometry(r);
        } else if (position == KTextEditor::Message::BottomInView) {
            const QRect r(adjustedRect.width() - item->sizeHint().width(),
                          adjustedRect.height() - item->sizeHint().height(),
                          item->sizeHint().width(),
                          item->sizeHint().height());
            item->setGeometry(r);
        } else if (position == KTextEditor::Message::CenterInView) {
            QRect r(0, 0, item->sizeHint().width(), item->sizeHint().height());
            r.moveCenter(adjustedRect.center());
            item->setGeometry(r);
        } else {
            Q_ASSERT_X(false, "setGeometry", "Only TopInView, CenterInView, and BottomInView are supported.");
        }
    }
}

QSize KateMessageLayout::sizeHint() const
{
    return QSize();
}

QLayoutItem *KateMessageLayout::takeAt(int index)
{
    if (index >= 0 && index < m_items.size()) {
        return m_items.takeAt(index).item;
    }
    return nullptr;
}

void KateMessageLayout::add(QLayoutItem *item, KTextEditor::Message::MessagePosition pos)
{
    m_items.push_back({item, pos});
}
// END KateMessageLayout

// BEGIN KateScrollBar
static const int s_lineWidth = 100;
static const int s_pixelMargin = 8;
static const int s_linePixelIncLimit = 6;

KateScrollBar::KateScrollBar(Qt::Orientation orientation, KateViewInternal *parent)
    : QScrollBar(orientation, parent->m_view)
    , m_middleMouseDown(false)
    , m_leftMouseDown(false)
    , m_view(parent->m_view)
    , m_doc(parent->doc())
    , m_viewInternal(parent)
    , m_textPreview(nullptr)
    , m_showMarks(false)
    , m_showMiniMap(false)
    , m_miniMapAll(true)
    , m_needsUpdateOnShow(false)
    , m_miniMapWidth(40)
    , m_grooveHeight(height())
{
    connect(this, &KateScrollBar::valueChanged, this, &KateScrollBar::sliderMaybeMoved);
    connect(m_doc, &KTextEditor::DocumentPrivate::marksChanged, this, &KateScrollBar::marksChanged);

    m_updateTimer.setInterval(300);
    m_updateTimer.setSingleShot(true);

    // track mouse for text preview widget
    setMouseTracking(orientation == Qt::Vertical);

    // setup text preview timer
    m_delayTextPreviewTimer.setSingleShot(true);
    m_delayTextPreviewTimer.setInterval(250);
    connect(&m_delayTextPreviewTimer, &QTimer::timeout, this, &KateScrollBar::showTextPreview);
}

void KateScrollBar::showEvent(QShowEvent *event)
{
    QScrollBar::showEvent(event);

    if (m_needsUpdateOnShow) {
        m_needsUpdateOnShow = false;
        updatePixmap();
    }
}

KateScrollBar::~KateScrollBar()
{
    delete m_textPreview;
}

void KateScrollBar::setShowMiniMap(bool b)
{
    if (b && !m_showMiniMap) {
        auto timerSlot = qOverload<>(&QTimer::start);
        connect(m_view, &KTextEditor::ViewPrivate::selectionChanged, &m_updateTimer, timerSlot, Qt::UniqueConnection);
        connect(m_doc, &KTextEditor::DocumentPrivate::textChanged, &m_updateTimer, timerSlot, Qt::UniqueConnection);
        connect(m_view, &KTextEditor::ViewPrivate::delayedUpdateOfView, &m_updateTimer, timerSlot, Qt::UniqueConnection);
        connect(&m_updateTimer, &QTimer::timeout, this, &KateScrollBar::updatePixmap, Qt::UniqueConnection);
        connect(&(m_view->textFolding()), &Kate::TextFolding::foldingRangesChanged, &m_updateTimer, timerSlot, Qt::UniqueConnection);
    } else if (!b) {
        disconnect(&m_updateTimer);
    }

    m_showMiniMap = b;

    updateGeometry();
    update();
}

QSize KateScrollBar::sizeHint() const
{
    if (m_showMiniMap) {
        return QSize(m_miniMapWidth, QScrollBar::sizeHint().height());
    }
    return QScrollBar::sizeHint();
}

int KateScrollBar::minimapYToStdY(int y)
{
    // Check if the minimap fills the whole scrollbar
    if (m_stdGroveRect.height() == m_mapGroveRect.height()) {
        return y;
    }

    // check if y is on the step up/down
    if ((y < m_stdGroveRect.top()) || (y > m_stdGroveRect.bottom())) {
        return y;
    }

    if (y < m_mapGroveRect.top()) {
        return m_stdGroveRect.top() + 1;
    }

    if (y > m_mapGroveRect.bottom()) {
        return m_stdGroveRect.bottom() - 1;
    }

    // check for div/0
    if (m_mapGroveRect.height() == 0) {
        return y;
    }

    int newY = (y - m_mapGroveRect.top()) * m_stdGroveRect.height() / m_mapGroveRect.height();
    newY += m_stdGroveRect.top();
    return newY;
}

void KateScrollBar::mousePressEvent(QMouseEvent *e)
{
    // delete text preview
    hideTextPreview();

    if (e->button() == Qt::MiddleButton) {
        m_middleMouseDown = true;
    } else if (e->button() == Qt::LeftButton) {
        m_leftMouseDown = true;
    }

    if (m_showMiniMap) {
        if (!m_sliderRect.contains(e->pos()) && m_leftMouseDown && e->pos().y() > m_mapGroveRect.top() && e->pos().y() < m_mapGroveRect.bottom()) {
            // if we show the minimap left-click jumps directly to the selected position
            int newVal = (e->pos().y() - m_mapGroveRect.top()) / (double)m_mapGroveRect.height() * (double)(maximum() + pageStep()) - pageStep() / 2;
            newVal = qBound(0, newVal, maximum());
            setSliderPosition(newVal);
        }
        const QPoint pos(6, minimapYToStdY(e->pos().y()));
        QMouseEvent eMod(QEvent::MouseButtonPress, pos, mapToGlobal(pos), e->button(), e->buttons(), e->modifiers());
        QScrollBar::mousePressEvent(&eMod);
    } else {
        QScrollBar::mousePressEvent(e);
    }

    m_toolTipPos = e->globalPosition().toPoint() - QPoint(e->pos().x(), 0);
    const int fromLine = m_viewInternal->toRealCursor(m_viewInternal->startPos()).line() + 1;
    const int lastLine = m_viewInternal->toRealCursor(m_viewInternal->endPos()).line() + 1;
    QToolTip::showText(m_toolTipPos, i18nc("from line - to line", "<center>%1<br/>&#x2014;<br/>%2</center>", fromLine, lastLine), this);

    redrawMarks();
}

void KateScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MiddleButton) {
        m_middleMouseDown = false;
    } else if (e->button() == Qt::LeftButton) {
        m_leftMouseDown = false;
    }

    redrawMarks();

    if (m_leftMouseDown || m_middleMouseDown) {
        QToolTip::hideText();
    }

    if (m_showMiniMap) {
        const QPoint pos(e->pos().x(), minimapYToStdY(e->pos().y()));
        QMouseEvent eMod(QEvent::MouseButtonRelease, pos, mapToGlobal(pos), e->button(), e->buttons(), e->modifiers());
        QScrollBar::mouseReleaseEvent(&eMod);
    } else {
        QScrollBar::mouseReleaseEvent(e);
    }
}

void KateScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    if (m_showMiniMap) {
        const QPoint pos(e->pos().x(), minimapYToStdY(e->pos().y()));
        QMouseEvent eMod(QEvent::MouseMove, pos, mapToGlobal(pos), e->button(), e->buttons(), e->modifiers());
        QScrollBar::mouseMoveEvent(&eMod);
    } else {
        QScrollBar::mouseMoveEvent(e);
    }

    if (e->buttons() & (Qt::LeftButton | Qt::MiddleButton)) {
        redrawMarks();

        // current line tool tip
        m_toolTipPos = e->globalPosition().toPoint() - QPoint(e->pos().x(), 0);
        const int fromLine = m_viewInternal->toRealCursor(m_viewInternal->startPos()).line() + 1;
        const int lastLine = m_viewInternal->toRealCursor(m_viewInternal->endPos()).line() + 1;
        QToolTip::showText(m_toolTipPos, i18nc("from line - to line", "<center>%1<br/>&#x2014;<br/>%2</center>", fromLine, lastLine), this);
    }

    showTextPreviewDelayed();
}

void KateScrollBar::leaveEvent(QEvent *event)
{
    hideTextPreview();

    QAbstractSlider::leaveEvent(event);
}

bool KateScrollBar::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)

    if (m_textPreview && event->type() == QEvent::WindowDeactivate) {
        // We need hide the scrollbar TextPreview widget
        hideTextPreview();
    }

    return false;
}

void KateScrollBar::paintEvent(QPaintEvent *e)
{
    if (m_doc->marks().size() != m_lines.size()) {
        recomputeMarksPositions();
    }
    if (m_showMiniMap) {
        miniMapPaintEvent(e);
    } else {
        normalPaintEvent(e);
    }
}

void KateScrollBar::showTextPreviewDelayed()
{
    if (!m_textPreview) {
        if (!m_delayTextPreviewTimer.isActive()) {
            m_delayTextPreviewTimer.start();
        }
    } else {
        showTextPreview();
    }
}

void KateScrollBar::showTextPreview()
{
    if (orientation() != Qt::Vertical || isSliderDown() || (minimum() == maximum()) || !m_view->config()->scrollBarPreview()) {
        return;
    }

    // only show when main window is active (#392396)
    if (window() && !window()->isActiveWindow()) {
        return;
    }

    QRect grooveRect;
    if (m_showMiniMap) {
        // If mini-map is shown, the height of the map might not be the whole height
        grooveRect = m_mapGroveRect;
    } else {
        QStyleOptionSlider opt;
        opt.initFrom(this);
        opt.subControls = QStyle::SC_None;
        opt.activeSubControls = QStyle::SC_None;
        opt.orientation = orientation();
        opt.minimum = minimum();
        opt.maximum = maximum();
        opt.sliderPosition = sliderPosition();
        opt.sliderValue = value();
        opt.singleStep = singleStep();
        opt.pageStep = pageStep();

        grooveRect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, this);
    }

    if (m_view->config()->scrollPastEnd()) {
        // Adjust the grove size to accommodate the added pageStep at the bottom
        int adjust = pageStep() * grooveRect.height() / (maximum() + pageStep() - minimum());
        grooveRect.adjust(0, 0, 0, -adjust);
    }

    const QPoint cursorPos = mapFromGlobal(QCursor::pos());
    if (grooveRect.contains(cursorPos)) {
        if (!m_textPreview) {
            m_textPreview = new KateTextPreview(m_view, this);
            m_textPreview->setAttribute(Qt::WA_ShowWithoutActivating);
            m_textPreview->setFrameStyle(QFrame::StyledPanel);

            // event filter to catch application WindowDeactivate event, to hide the preview window
            qApp->installEventFilter(this);
        }

        const qreal posInPercent = static_cast<double>(cursorPos.y() - grooveRect.top()) / grooveRect.height();
        const qreal startLine = posInPercent * m_view->textFolding().visibleLines();

        m_textPreview->resize(m_view->width() / 2, m_view->height() / 5);
        const int xGlobal = mapToGlobal(QPoint(0, 0)).x();
        const int yGlobal = qMin(mapToGlobal(QPoint(0, height())).y() - m_textPreview->height(),
                                 qMax(mapToGlobal(QPoint(0, 0)).y(), mapToGlobal(cursorPos).y() - m_textPreview->height() / 2));
        m_textPreview->move(xGlobal - m_textPreview->width(), yGlobal);
        m_textPreview->setLine(startLine);
        m_textPreview->setCenterView(true);
        m_textPreview->setScaleFactor(0.75);
        m_textPreview->raise();
        m_textPreview->show();
    } else {
        hideTextPreview();
    }
}

void KateScrollBar::hideTextPreview()
{
    if (m_delayTextPreviewTimer.isActive()) {
        m_delayTextPreviewTimer.stop();
    }

    qApp->removeEventFilter(this);
    delete m_textPreview;
}

// This function is optimized for bing called in sequence.
void KateScrollBar::getCharColorRanges(const QList<Kate::TextLine::Attribute> &attributes,
                                       const QList<Kate::TextRange *> &decorations,
                                       const QString &text,
                                       QList<KateScrollBar::ColumnRangeWithColor> &ranges,
                                       QVarLengthArray<std::pair<QRgb, QPen>, 20> &penCache)
{
    ranges.clear();

    auto getPen = [&](const QBrush &color) -> int {
        uint rgb = color.color().rgb();
        auto it = std::find_if(penCache.begin(), penCache.end(), [rgb](const std::pair<QRgb, QPen> &rgbToPen) {
            return rgb == rgbToPen.first;
        });
        if (it != penCache.end()) {
            return it - penCache.begin();
        }
        penCache.push_back({rgb, QPen(color, 1)});
        return (int)penCache.size() - 1;
    };

    constexpr QChar space = QLatin1Char(' ');
    constexpr QChar tab = QLatin1Char('\t');

    for (int i = 0; i < text.size() && i < s_lineWidth; ++i) {
        if (text[i] == space || text[i] == tab) {
            continue;
        }

        bool styleFound = false;
        for (auto range : decorations) {
            if (range->containsColumn(i)) {
                QBrush color = range->attribute()->foreground();
                styleFound = true;
                int startCol = range->start().column();
                int endCol = range->end().column();
                ranges << ColumnRangeWithColor{.penIndex=getPen(color), .startColumn=startCol, .endColumn=endCol};
                i = endCol;
                break;
            }
        }

        if (styleFound) {
            continue;
        }

        // If there's no decoration set for the current character (this will mostly be the case for
        // plain Kate), query the styles, that is, the default kate syntax highlighting.
        // go to the block containing x
        qsizetype attributeIndex = 0;
        while ((attributeIndex < attributes.size()) && ((attributes[attributeIndex].offset + attributes[attributeIndex].length) < i)) {
            ++attributeIndex;
        }
        if (attributeIndex < attributes.size()) {
            const auto attr = attributes[attributeIndex];
            if ((i < attr.offset + attr.length)) {
                QBrush color = m_view->renderer()->attribute(attr.attributeValue)->foreground();
                int startCol = attr.offset;
                int endCol = attr.offset + attr.length;
                ranges << ColumnRangeWithColor{.penIndex=getPen(color), .startColumn=startCol, .endColumn=endCol};
                i = endCol;
            }
        }
    }
}

void KateScrollBar::updatePixmap()
{
    // QElapsedTimer time;
    // time.start();

    if (!m_showMiniMap) {
        // make sure no time is wasted if the option is disabled
        return;
    }

    if (!isVisible()) {
        // don't update now if the document is not visible; do it when
        // the document is shown again instead
        m_needsUpdateOnShow = true;
        return;
    }

    // For performance reason, only every n-th line will be drawn if the widget is
    // sufficiently small compared to the amount of lines in the document.
    int docLineCount = m_view->textFolding().visibleLines();
    int pixmapLineCount = docLineCount;
    if (m_view->config()->scrollPastEnd()) {
        pixmapLineCount += pageStep();
    }
    int pixmapLinesUnscaled = pixmapLineCount;
    if (m_grooveHeight < 5) {
        m_grooveHeight = 5;
    }
    int charIncrement = 1;
    int lineIncrement = 1;
    if ((m_grooveHeight > 10) && (pixmapLineCount >= m_grooveHeight * 2)) {
        charIncrement = pixmapLineCount / m_grooveHeight;
        while (charIncrement > s_linePixelIncLimit) {
            lineIncrement++;
            pixmapLineCount = pixmapLinesUnscaled / lineIncrement;
            charIncrement = pixmapLineCount / m_grooveHeight;
        }
        pixmapLineCount /= charIncrement;
    }

    int pixmapLineWidth = s_pixelMargin + s_lineWidth / charIncrement;

    // qCDebug(LOG_KTE) << "l" << lineIncrement << "c" << charIncrement << "d";
    // qCDebug(LOG_KTE) << "pixmap" << pixmapLineCount << pixmapLineWidth << "docLines" << m_view->textFolding().visibleLines() << "height" << m_grooveHeight;

    const QBrush backgroundColor = m_view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->background();
    const QBrush defaultTextColor = m_view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->foreground();
    const QBrush selectionBgColor = m_view->rendererConfig()->selectionColor();

    QColor modifiedLineColor = m_view->rendererConfig()->modifiedLineColor();
    QColor savedLineColor = m_view->rendererConfig()->savedLineColor();
    // move the modified line color away from the background color
    modifiedLineColor.setHsv(modifiedLineColor.hue(), 255, 255 - backgroundColor.color().value() / 3);
    savedLineColor.setHsv(savedLineColor.hue(), 100, 255 - backgroundColor.color().value() / 3);

    const QBrush modifiedLineBrush = modifiedLineColor;
    const QBrush savedLineBrush = savedLineColor;

    // increase dimensions by ratio
    m_pixmap = QPixmap(pixmapLineWidth * m_view->devicePixelRatioF(), pixmapLineCount * m_view->devicePixelRatioF());
    m_pixmap.fill(QColor("transparent"));

    // The text currently selected in the document, to be drawn later.
    const KTextEditor::Range selection = m_view->selectionRange();
    const bool hasSelection = !selection.isEmpty();
    // reusable buffer for color->range
    QList<KateScrollBar::ColumnRangeWithColor> colorRangesForLine;
    const QPen defaultTextPen = QPen(defaultTextColor, 1);
    // resusable buffer for line ranges;
    QList<Kate::TextRange *> decorations;

    QPainter painter;
    if (painter.begin(&m_pixmap)) {
        // init pen once, afterwards, only change it if color changes to avoid a lot of allocation for setPen
        painter.setPen(QPen(selectionBgColor, 1));

        // Do not force updates of the highlighting if the document is very large
        const bool simpleMode = m_doc->lines() > 7500;

        int pixelY = 0;
        int drawnLines = 0;

        // pen cache to avoid a lot of allocations from pen creation
        QVarLengthArray<std::pair<QRgb, QPen>, 20> penCache;

        // Iterate over all visible lines, drawing them.
        for (int virtualLine = 0; virtualLine < docLineCount; virtualLine += lineIncrement) {
            int realLineNumber = m_view->textFolding().visibleLineToLine(virtualLine);
            const Kate::TextLine kateline = m_doc->plainKateTextLine(realLineNumber);
            const QString lineText = kateline.text();

            if (!simpleMode) {
                m_doc->buffer().ensureHighlighted(realLineNumber);
            }

            // get normal highlighting stuff
            const auto &attributes = kateline.attributesList();

            // get moving ranges with attribs (semantic highlighting and co.)
            m_view->doc()->buffer().rangesForLine(realLineNumber, m_view, true, decorations);

            // Draw selection if it is on an empty line

            int pixelX = s_pixelMargin; // use this to control the offset of the text from the left

            if (hasSelection) {
                if (selection.contains(KTextEditor::Cursor(realLineNumber, 0)) && lineText.size() == 0) {
                    if (selectionBgColor != painter.pen().brush()) {
                        painter.setPen(QPen(selectionBgColor, 1));
                    }
                    painter.drawLine(s_pixelMargin, pixelY, s_pixelMargin + s_lineWidth - 1, pixelY);
                }
                // Iterate over the line to draw the background
                int selStartX = -1;
                int selEndX = -1;
                for (int x = 0; (x < lineText.size() && x < s_lineWidth); x += charIncrement) {
                    if (pixelX >= s_lineWidth + s_pixelMargin) {
                        break;
                    }
                    // Query the selection and draw it behind the character
                    if (selection.contains(KTextEditor::Cursor(realLineNumber, x))) {
                        if (selStartX == -1) {
                            selStartX = pixelX;
                        }
                        selEndX = pixelX;
                        if (lineText.size() - 1 == x) {
                            selEndX = s_lineWidth + s_pixelMargin - 1;
                        }
                    }

                    if (lineText[x] == QLatin1Char('\t')) {
                        pixelX += qMax(4 / charIncrement, 1); // FIXME: tab width...
                    } else {
                        pixelX++;
                    }
                }

                if (selStartX != -1) {
                    if (selectionBgColor != painter.pen().brush()) {
                        painter.setPen(QPen(selectionBgColor, 1));
                    }
                    painter.drawLine(selStartX, pixelY, selEndX, pixelY);
                }
            }

            // Iterate over all the characters in the current line
            getCharColorRanges(attributes, decorations, lineText, colorRangesForLine, penCache);
            pixelX = s_pixelMargin;
            for (int x = 0; (x < lineText.size() && x < s_lineWidth); x += charIncrement) {
                if (pixelX >= s_lineWidth + s_pixelMargin) {
                    break;
                }

                // draw the pixels
                if (lineText[x] == QLatin1Char(' ')) {
                    pixelX++;
                } else if (lineText[x] == QLatin1Char('\t')) {
                    pixelX += qMax(4 / charIncrement, 1); // FIXME: tab width...
                } else {
                    const QPen *pen = nullptr;
                    int rangeEnd = x + 1;
                    for (const auto &cr : colorRangesForLine) {
                        if (cr.startColumn <= x && x <= cr.endColumn) {
                            rangeEnd = cr.endColumn;
                            if (cr.penIndex != -1) {
                                pen = &penCache[cr.penIndex].second;
                            }
                        }
                    }

                    if (!pen) {
                        pen = &defaultTextPen;
                    }
                    // get the column range and color in which this 'x' lies
                    painter.setPen(*pen);

                    // Actually draw the pixels with the color queried from the renderer.
                    QVarLengthArray<QPoint, 100> points;
                    for (; x < rangeEnd; x += charIncrement) {
                        if (pixelX >= s_lineWidth + s_pixelMargin) {
                            break;
                        }
                        points.append({pixelX++, pixelY});
                    }
                    painter.drawPoints(points.data(), points.size());
                }
            }
            drawnLines++;
            if (((drawnLines) % charIncrement) == 0) {
                pixelY++;
            }
        }
        // qCDebug(LOG_KTE) << drawnLines;
        // Draw line modification marker map.
        // Disable this if the document is really huge,
        // since it requires querying every line.
        if (m_doc->lines() < 50000) {
            for (int lineno = 0; lineno < docLineCount; lineno++) {
                int realLineNo = m_view->textFolding().visibleLineToLine(lineno);
                const auto line = m_doc->plainKateTextLine(realLineNo);
                const QBrush &col = line.markedAsModified() ? modifiedLineBrush : savedLineBrush;
                if (line.markedAsModified() || line.markedAsSavedOnDisk()) {
                    int pos = (lineno * pixmapLineCount) / pixmapLinesUnscaled;
                    painter.fillRect(2, pos, 3, 1, col);
                }
            }
        }

        // end painting
        painter.end();
    }

    // set right ratio
    m_pixmap.setDevicePixelRatio(m_view->devicePixelRatioF());

    // qCDebug(LOG_KTE) << time.elapsed();
    // Redraw the scrollbar widget with the updated pixmap.
    update();
}

void KateScrollBar::miniMapPaintEvent(QPaintEvent *e)
{
    QScrollBar::paintEvent(e);

    QPainter painter(this);

    QStyleOptionSlider opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation();
    opt.minimum = minimum();
    opt.maximum = maximum();
    opt.sliderPosition = sliderPosition();
    opt.sliderValue = value();
    opt.singleStep = singleStep();
    opt.pageStep = pageStep();

    QRect grooveRect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, this);
    m_stdGroveRect = grooveRect;
    if (style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSubLine, this).height() == 0) {
        int alignMargin = style()->pixelMetric(QStyle::PM_FocusFrameVMargin, &opt, this);
        grooveRect.moveTop(alignMargin);
        grooveRect.setHeight(grooveRect.height() - alignMargin);
    }
    if (style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarAddLine, this).height() == 0) {
        int alignMargin = style()->pixelMetric(QStyle::PM_FocusFrameVMargin, &opt, this);
        grooveRect.setHeight(grooveRect.height() - alignMargin);
    }
    m_grooveHeight = grooveRect.height();

    const int docXMargin = 1;
    // style()->drawControl(QStyle::CE_ScrollBarAddLine, &opt, &painter, this);
    // style()->drawControl(QStyle::CE_ScrollBarSubLine, &opt, &painter, this);

    // calculate the document size and position
    const int docHeight = qMin(grooveRect.height(), int(m_pixmap.height() / m_pixmap.devicePixelRatio() * 2)) - 2 * docXMargin;
    const int yoffset = 1; // top-aligned in stead of center-aligned (grooveRect.height() - docHeight) / 2;
    const QRect docRect(QPoint(grooveRect.left() + docXMargin, yoffset + grooveRect.top()), QSize(grooveRect.width() - docXMargin, docHeight));
    m_mapGroveRect = docRect;

    // calculate the visible area
    int max = qMax(maximum() + 1, 1);
    int visibleStart = value() * docHeight / (max + pageStep()) + docRect.top() + 0.5;
    int visibleEnd = (value() + pageStep()) * docHeight / (max + pageStep()) + docRect.top();
    QRect visibleRect = docRect;
    visibleRect.moveTop(visibleStart);
    visibleRect.setHeight(visibleEnd - visibleStart);

    // calculate colors
    const QColor backgroundColor = m_view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->background().color();
    const QColor foregroundColor = m_view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->foreground().color();
    const QColor highlightColor = palette().highlight().color();

    const int backgroundLightness = backgroundColor.lightness();
    const int foregroundLightness = foregroundColor.lightness();
    const int lighnessDiff = (foregroundLightness - backgroundLightness);

    // get a color suited for the color theme
    QColor darkShieldColor = palette().color(QPalette::Mid);
    int hue;
    int sat;
    int light;
    darkShieldColor.getHsl(&hue, &sat, &light);
    // apply suitable lightness
    darkShieldColor.setHsl(hue, sat, backgroundLightness + lighnessDiff * 0.35);
    // gradient for nicer results
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0, darkShieldColor);
    gradient.setColorAt(0.3, darkShieldColor.lighter(115));
    gradient.setColorAt(1, darkShieldColor);

    QColor lightShieldColor;
    lightShieldColor.setHsl(hue, sat, backgroundLightness + lighnessDiff * 0.15);

    QColor outlineColor;
    outlineColor.setHsl(hue, sat, backgroundLightness + lighnessDiff * 0.5);

    // draw the grove background in case the document is small
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawRect(grooveRect);

    // adjust the rectangles
    QRect sliderRect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, this);
    sliderRect.setX(docXMargin);
    sliderRect.setWidth(width() - docXMargin * 2);

    if ((docHeight + 2 * docXMargin >= grooveRect.height()) && (sliderRect.height() > visibleRect.height() + 2)) {
        visibleRect.adjust(2, 0, -3, 0);
    } else {
        visibleRect.adjust(1, 0, -1, 2);
        sliderRect.setTop(visibleRect.top() - 1);
        sliderRect.setBottom(visibleRect.bottom() + 1);
    }

    // Smooth transform only when squeezing
    if (grooveRect.height() < m_pixmap.height() / m_pixmap.devicePixelRatio()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
    }

    // draw the modified lines margin
    QRect pixmapMarginRect(QPoint(0, 0), QSize(s_pixelMargin, m_pixmap.height() / m_pixmap.devicePixelRatio()));
    QRect docPixmapMarginRect(QPoint(0, docRect.top()), QSize(s_pixelMargin, docRect.height()));
    painter.drawPixmap(docPixmapMarginRect, m_pixmap, pixmapMarginRect);

    // calculate the stretch and draw the stretched lines (scrollbar marks)
    QRect pixmapRect(QPoint(s_pixelMargin, 0),
                     QSize(m_pixmap.width() / m_pixmap.devicePixelRatio() - s_pixelMargin, m_pixmap.height() / m_pixmap.devicePixelRatio()));
    QRect docPixmapRect(QPoint(s_pixelMargin, docRect.top()), QSize(docRect.width() - s_pixelMargin, docRect.height()));
    painter.drawPixmap(docPixmapRect, m_pixmap, pixmapRect);

    // delimit the end of the document
    const int y = docPixmapRect.height() + grooveRect.y();
    if (y + 2 < grooveRect.y() + grooveRect.height()) {
        QColor fg(foregroundColor);
        fg.setAlpha(30);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(fg, 1));
        painter.drawLine(grooveRect.x() + 1, y + 2, width() - 1, y + 2);
    }

    // fade the invisible sections
    const QRect top(grooveRect.x(),
                    grooveRect.y(),
                    grooveRect.width(),
                    visibleRect.y() - grooveRect.y() // Pen width
    );
    const QRect bottom(grooveRect.x(),
                       grooveRect.y() + visibleRect.y() + visibleRect.height() - grooveRect.y(), // Pen width
                       grooveRect.width(),
                       grooveRect.height() - (visibleRect.y() - grooveRect.y()) - visibleRect.height());

    QColor faded(backgroundColor);
    faded.setAlpha(110);
    painter.fillRect(top, faded);
    painter.fillRect(bottom, faded);

    // add a thin line to limit the scrollbar
    QColor c(foregroundColor);
    c.setAlpha(10);
    painter.setPen(QPen(c, 1));
    painter.drawLine(0, 0, 0, height());

    if (m_showMarks) {
        QHashIterator<int, QColor> it = m_lines;
        QPen penBg;
        penBg.setWidth(4);
        lightShieldColor.setAlpha(180);
        penBg.setColor(lightShieldColor);
        painter.setPen(penBg);
        while (it.hasNext()) {
            it.next();
            int y = (it.key() - grooveRect.top()) * docHeight / grooveRect.height() + docRect.top();
            ;
            painter.drawLine(6, y, width() - 6, y);
        }

        it = m_lines;
        QPen pen;
        pen.setWidth(2);
        while (it.hasNext()) {
            it.next();
            pen.setColor(it.value());
            painter.setPen(pen);
            int y = (it.key() - grooveRect.top()) * docHeight / grooveRect.height() + docRect.top();
            painter.drawLine(6, y, width() - 6, y);
        }
    }

    // slider outline
    QColor sliderColor(highlightColor);
    sliderColor.setAlpha(50);
    painter.fillRect(sliderRect, sliderColor);
    painter.setPen(QPen(highlightColor, 0));
    m_sliderRect = sliderRect;
    // rounded rect looks ugly for some reason, so we draw 4 lines.
    painter.drawLine(sliderRect.left(), sliderRect.top() + 1, sliderRect.left(), sliderRect.bottom() - 1);
    painter.drawLine(sliderRect.right(), sliderRect.top() + 1, sliderRect.right(), sliderRect.bottom() - 1);
    painter.drawLine(sliderRect.left() + 1, sliderRect.top(), sliderRect.right() - 1, sliderRect.top());
    painter.drawLine(sliderRect.left() + 1, sliderRect.bottom(), sliderRect.right() - 1, sliderRect.bottom());
}

void KateScrollBar::normalPaintEvent(QPaintEvent *e)
{
    QScrollBar::paintEvent(e);

    if (!m_showMarks) {
        return;
    }

    QPainter painter(this);

    QStyleOptionSlider opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation();
    opt.minimum = minimum();
    opt.maximum = maximum();
    opt.sliderPosition = sliderPosition();
    opt.sliderValue = value();
    opt.singleStep = singleStep();
    opt.pageStep = pageStep();

    QRect rect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, this);
    int sideMargin = width() - rect.width();
    if (sideMargin < 4) {
        sideMargin = 4;
    }
    sideMargin /= 2;

    QHashIterator<int, QColor> it = m_lines;
    while (it.hasNext()) {
        it.next();
        painter.setPen(it.value());
        if (it.key() < rect.top() || it.key() > rect.bottom()) {
            painter.drawLine(0, it.key(), width(), it.key());
        } else {
            painter.drawLine(0, it.key(), sideMargin, it.key());
            painter.drawLine(width() - sideMargin, it.key(), width(), it.key());
        }
    }
}

void KateScrollBar::resizeEvent(QResizeEvent *e)
{
    QScrollBar::resizeEvent(e);
    m_updateTimer.start();
    m_lines.clear();
    update();
}

void KateScrollBar::sliderChange(SliderChange change)
{
    // call parents implementation
    QScrollBar::sliderChange(change);

    if (change == QAbstractSlider::SliderValueChange) {
        redrawMarks();
    } else if (change == QAbstractSlider::SliderRangeChange) {
        marksChanged();
    }

    if (m_leftMouseDown || m_middleMouseDown) {
        const int fromLine = m_viewInternal->toRealCursor(m_viewInternal->startPos()).line() + 1;
        const int lastLine = m_viewInternal->toRealCursor(m_viewInternal->endPos()).line() + 1;
        QToolTip::showText(m_toolTipPos, i18nc("from line - to line", "<center>%1<br/>&#x2014;<br/>%2</center>", fromLine, lastLine), this);
    }
}

void KateScrollBar::marksChanged()
{
    m_lines.clear();
    update();
}

void KateScrollBar::redrawMarks()
{
    if (!m_showMarks) {
        return;
    }
    update();
}

void KateScrollBar::recomputeMarksPositions()
{
    // get the style options to compute the scrollbar pixels
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect grooveRect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, this);

    // cache top margin and groove height
    const int top = grooveRect.top();
    const int h = grooveRect.height() - 1;

    // make sure we have a sane height
    if (h <= 0) {
        return;
    }

    // get total visible (=without folded) lines in the document
    int visibleLines = m_view->textFolding().visibleLines() - 1;
    if (m_view->config()->scrollPastEnd()) {
        visibleLines += m_viewInternal->linesDisplayed() - 1;
        visibleLines -= m_view->config()->autoCenterLines();
    }

    // now repopulate the scrollbar lines list
    m_lines.clear();
    const QHash<int, KTextEditor::Mark *> &marks = m_doc->marks();
    for (QHash<int, KTextEditor::Mark *>::const_iterator i = marks.constBegin(); i != marks.constEnd(); ++i) {
        KTextEditor::Mark *mark = i.value();
        const int line = m_view->textFolding().lineToVisibleLine(mark->line);
        const double ratio = static_cast<double>(line) / visibleLines;
        m_lines.insert(top + (int)(h * ratio), KateRendererConfig::global()->lineMarkerColor((KTextEditor::Document::MarkTypes)mark->type));
    }
}

void KateScrollBar::sliderMaybeMoved(int value)
{
    if (m_middleMouseDown) {
        // we only need to emit this signal once, as for the following slider
        // movements the signal sliderMoved() is already emitted.
        // Thus, set m_middleMouseDown to false right away.
        m_middleMouseDown = false;
        Q_EMIT sliderMMBMoved(value);
    }
}
// END

// BEGIN KateCmdLineEditFlagCompletion
/**
 * This class provide completion of flags. It shows a short description of
 * each flag, and flags are appended.
 */
class KateCmdLineEditFlagCompletion : public KCompletion
{
public:
    KateCmdLineEditFlagCompletion()
    {
        ;
    }

    QString makeCompletion(const QString & /*s*/) override
    {
        return QString();
    }
};
// END KateCmdLineEditFlagCompletion

// BEGIN KateCmdLineEdit
KateCommandLineBar::KateCommandLineBar(KTextEditor::ViewPrivate *view, QWidget *parent)
    : KateViewBarWidget(true, parent)
{
    QHBoxLayout *topLayout = new QHBoxLayout(centralWidget());
    topLayout->setContentsMargins(0, 0, 0, 0);
    m_lineEdit = new KateCmdLineEdit(this, view);
    connect(m_lineEdit, &KateCmdLineEdit::hideRequested, this, &KateCommandLineBar::hideMe);
    topLayout->addWidget(m_lineEdit);

    QToolButton *helpButton = new QToolButton(this);
    helpButton->setAutoRaise(true);
    helpButton->setIcon(QIcon::fromTheme(QStringLiteral("help-contextual")));
    topLayout->addWidget(helpButton);
    connect(helpButton, &QToolButton::clicked, this, &KateCommandLineBar::showHelpPage);

    setFocusProxy(m_lineEdit);
}

void KateCommandLineBar::showHelpPage()
{
    KHelpClient::invokeHelp(QStringLiteral("advanced-editing-tools-commandline"), QStringLiteral("katepart"));
}

KateCommandLineBar::~KateCommandLineBar() = default;

// inserts the given string in the command line edit and (if selected = true) selects it so the user
// can type over it if they want to
void KateCommandLineBar::setText(const QString &text, bool selected)
{
    m_lineEdit->setText(text);
    if (selected) {
        m_lineEdit->selectAll();
    }
}

void KateCommandLineBar::execute(const QString &text)
{
    m_lineEdit->slotReturnPressed(text);
}

KateCmdLineEdit::KateCmdLineEdit(KateCommandLineBar *bar, KTextEditor::ViewPrivate *view)
    : KLineEdit()
    , m_view(view)
    , m_bar(bar)
    , m_msgMode(false)
    , m_histpos(0)
    , m_cmdend(0)
    , m_command(nullptr)
{
    connect(this, &KateCmdLineEdit::returnKeyPressed, this, &KateCmdLineEdit::slotReturnPressed);

    setCompletionObject(KateCmd::self()->commandCompletionObject());
    setAutoDeleteCompletionObject(false);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, &QTimer::timeout, this, &KateCmdLineEdit::hideLineEdit);

    // make sure the timer is stopped when the user switches views. if not, focus will be given to the
    // wrong view when KateViewBar::hideCurrentBarWidget() is called after 4 seconds. (the timer is
    // used for showing things like "Success" for four seconds after the user has used the kate
    // command line)
    connect(m_view, &KTextEditor::ViewPrivate::focusOut, m_hideTimer, &QTimer::stop);
}

void KateCmdLineEdit::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
}

QString KateCmdLineEdit::helptext(const QPoint &) const
{
    const QString beg = QStringLiteral("<qt background=\"white\"><div><table width=\"100%\"><tr><td bgcolor=\"brown\"><font color=\"white\"><b>Help: <big>");
    const QString mid = QStringLiteral("</big></b></font></td></tr><tr><td>");
    const QString end = QStringLiteral("</td></tr></table></div><qt>");

    const QString t = text();
    static const QRegularExpression re(QStringLiteral("\\s*help\\s+(.*)"));
    auto match = re.match(t);
    if (match.hasMatch()) {
        QString s;
        // get help for command
        const QString name = match.captured(1);
        if (name == QLatin1String("list")) {
            return beg + i18n("Available Commands") + mid + KateCmd::self()->commandList().join(QLatin1Char(' '))
                + i18n("<p>For help on individual commands, do <code>'help &lt;command&gt;'</code></p>") + end;
        } else if (!name.isEmpty()) {
            KTextEditor::Command *cmd = KateCmd::self()->queryCommand(name);
            if (cmd) {
                if (cmd->help(m_view, name, s)) {
                    return beg + name + mid + s + end;
                } else {
                    return beg + name + mid + i18n("No help for '%1'", name) + end;
                }
            } else {
                return beg + mid + i18n("No such command <b>%1</b>", name) + end;
            }
        }
    }

    return beg + mid
        + i18n("<p>This is the Katepart <b>command line</b>.<br />"
               "Syntax: <code><b>command [ arguments ]</b></code><br />"
               "For a list of available commands, enter <code><b>help list</b></code><br />"
               "For help for individual commands, enter <code><b>help &lt;command&gt;</b></code></p>")
        + end;
}

bool KateCmdLineEdit::event(QEvent *e)
{
    if (e->type() == QEvent::QueryWhatsThis) {
        setWhatsThis(helptext(QPoint()));
        e->accept();
        return true;
    }
    return KLineEdit::event(e);
}

/**
 * Parse the text as a command.
 *
 * The following is a simple PEG grammar for the syntax of the command.
 * (A PEG grammar is like a BNF grammar, except that "/" stands for
 * ordered choice: only the first matching rule is used. In other words,
 * the parsing is short-circuited in the manner of the "or" operator in
 * programming languages, and so the grammar is unambiguous.)
 *
 * Text <- Range? Command
 *       / Position
 * Range <- Position ("," Position)?
 *        / "%"
 * Position <- Base Offset?
 * Base <- Line
 *       / LastLine
 *       / ThisLine
 *       / Mark
 * Offset <- [+-] Base
 * Line <- [0-9]+
 * LastLine <- "$"
 * ThisLine <- "."
 * Mark <- "'" [a-z]
 */

void KateCmdLineEdit::slotReturnPressed(const QString &text)
{
    static const QRegularExpression focusChangingCommands(QStringLiteral("^(?:buffer|b|new|vnew|bp|bprev|bn|bnext|bf|bfirst|bl|blast|edit|e)$"));

    if (text.isEmpty()) {
        return;
    }
    // silently ignore leading space characters
    uint n = 0;
    const uint textlen = text.length();
    while ((n < textlen) && (text[n].isSpace())) {
        n++;
    }

    if (n >= textlen) {
        return;
    }

    QString cmd = text.mid(n);

    // Parse any leading range expression, and strip it (and maybe do some other transforms on the command).
    QString leadingRangeExpression;
    KTextEditor::Range range = CommandRangeExpressionParser::parseRangeExpression(cmd, m_view, leadingRangeExpression, cmd);

    // Built in help: if the command starts with "help", [try to] show some help
    if (cmd.startsWith(QLatin1String("help"))) {
        QWhatsThis::showText(mapToGlobal(QPoint(0, 0)), helptext(QPoint()));
        clear();
        KateCmd::self()->appendHistory(cmd);
        m_histpos = KateCmd::self()->historyLength();
        m_oldText.clear();
        return;
    }

    if (cmd.length() > 0) {
        KTextEditor::Command *p = KateCmd::self()->queryCommand(cmd);

        m_oldText = leadingRangeExpression + cmd;
        m_msgMode = true;

        // if the command changes the focus itself, the bar should be hidden before execution.
        if (focusChangingCommands.matchView(QStringView(cmd).left(cmd.indexOf(QLatin1Char(' ')))).hasMatch()) {
            Q_EMIT hideRequested();
        }

        if (!p) {
            setText(i18n("No such command: \"%1\"", cmd));
        } else if (range.isValid() && !p->supportsRange(cmd)) {
            // Raise message, when the command does not support ranges.
            setText(i18n("Error: No range allowed for command \"%1\".", cmd));
        } else {
            QString msg;
            if (p->exec(m_view, cmd, msg, range)) {
                // append command along with range (will be empty if none given) to history
                KateCmd::self()->appendHistory(leadingRangeExpression + cmd);
                m_histpos = KateCmd::self()->historyLength();
                m_oldText.clear();

                if (msg.length() > 0) {
                    setText(i18n("Success: ") + msg);
                } else if (isVisible()) {
                    // always hide on success without message
                    Q_EMIT hideRequested();
                }
            } else {
                if (msg.length() > 0) {
                    if (msg.contains(QLatin1Char('\n'))) {
                        // multiline error, use widget with more space
                        QWhatsThis::showText(mapToGlobal(QPoint(0, 0)), msg);
                    } else {
                        setText(msg);
                    }
                } else {
                    setText(i18n("Command \"%1\" failed.", cmd));
                }
            }
        }
    }

    // clean up
    if (completionObject() != KateCmd::self()->commandCompletionObject()) {
        KCompletion *c = completionObject();
        setCompletionObject(KateCmd::self()->commandCompletionObject());
        delete c;
    }
    m_command = nullptr;
    m_cmdend = 0;

    if (!focusChangingCommands.matchView(QStringView(cmd).left(cmd.indexOf(QLatin1Char(' ')))).hasMatch()) {
        m_view->setFocus();
    }

    if (isVisible()) {
        m_hideTimer->start(4000);
    }
}

void KateCmdLineEdit::hideLineEdit() // unless i have focus ;)
{
    if (!hasFocus()) {
        Q_EMIT hideRequested();
    }
}

void KateCmdLineEdit::focusInEvent(QFocusEvent *ev)
{
    if (m_msgMode) {
        m_msgMode = false;
        setText(m_oldText);
        selectAll();
    }

    KLineEdit::focusInEvent(ev);
}

void KateCmdLineEdit::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Escape || (ev->key() == Qt::Key_BracketLeft && ev->modifiers() == Qt::ControlModifier)) {
        m_view->setFocus();
        hideLineEdit();
        clear();
    } else if (ev->key() == Qt::Key_Up) {
        fromHistory(true);
    } else if (ev->key() == Qt::Key_Down) {
        fromHistory(false);
    }

    uint cursorpos = cursorPosition();
    KLineEdit::keyPressEvent(ev);

    // during typing, let us see if we have a valid command
    if (!m_cmdend || cursorpos <= m_cmdend) {
        QChar c;
        if (!ev->text().isEmpty()) {
            c = ev->text().at(0);
        }

        if (!m_cmdend && !c.isNull()) { // we have no command, so lets see if we got one
            if (!c.isLetterOrNumber() && c != QLatin1Char('-') && c != QLatin1Char('_')) {
                m_command = KateCmd::self()->queryCommand(text().trimmed());
                if (m_command) {
                    // qCDebug(LOG_KTE)<<"keypress in commandline: We have a command! "<<m_command<<". text is '"<<text()<<"'";
                    // if the typed character is ":",
                    // we try if the command has flag completions
                    m_cmdend = cursorpos;
                    // qCDebug(LOG_KTE)<<"keypress in commandline: Set m_cmdend to "<<m_cmdend;
                } else {
                    m_cmdend = 0;
                }
            }
        } else { // since cursor is inside the command name, we reconsider it
            // qCDebug(LOG_KTE) << "keypress in commandline: \\W -- text is " << text();
            m_command = KateCmd::self()->queryCommand(text().trimmed());
            if (m_command) {
                // qCDebug(LOG_KTE)<<"keypress in commandline: We have a command! "<<m_command;
                QString t = text();
                m_cmdend = 0;
                bool b = false;
                for (; (int)m_cmdend < t.length(); m_cmdend++) {
                    if (t[m_cmdend].isLetter()) {
                        b = true;
                    }
                    if (b && (!t[m_cmdend].isLetterOrNumber() && t[m_cmdend] != QLatin1Char('-') && t[m_cmdend] != QLatin1Char('_'))) {
                        break;
                    }
                }

                if (c == QLatin1Char(':') && cursorpos == m_cmdend) {
                    // check if this command wants to complete flags
                    // qCDebug(LOG_KTE)<<"keypress in commandline: Checking if flag completion is desired!";
                }
            } else {
                // clean up if needed
                if (completionObject() != KateCmd::self()->commandCompletionObject()) {
                    KCompletion *c = completionObject();
                    setCompletionObject(KateCmd::self()->commandCompletionObject());
                    delete c;
                }

                m_cmdend = 0;
            }
        }

        // if we got a command, check if it wants to do something.
        if (m_command) {
            KCompletion *cmpl = m_command->completionObject(m_view, text().left(m_cmdend).trimmed());
            if (cmpl) {
                // We need to prepend the current command name + flag string
                // when completion is done
                // qCDebug(LOG_KTE)<<"keypress in commandline: Setting completion object!";

                setCompletionObject(cmpl);
            }
        }
    } else if (m_command && !ev->text().isEmpty()) {
        // check if we should call the commands processText()
        if (m_command->wantsToProcessText(text().left(m_cmdend).trimmed())) {
            m_command->processText(m_view, text());
        }
    }
}

void KateCmdLineEdit::fromHistory(bool up)
{
    if (!KateCmd::self()->historyLength()) {
        return;
    }

    QString s;

    if (up) {
        if (m_histpos > 0) {
            m_histpos--;
            s = KateCmd::self()->fromHistory(m_histpos);
        }
    } else {
        if (m_histpos < (KateCmd::self()->historyLength() - 1)) {
            m_histpos++;
            s = KateCmd::self()->fromHistory(m_histpos);
        } else {
            m_histpos = KateCmd::self()->historyLength();
            setText(m_oldText);
        }
    }
    if (!s.isEmpty()) {
        // Select the argument part of the command, so that it is easy to overwrite
        setText(s);
        static const QRegularExpression reCmd(QStringLiteral("^[\\w\\-]+(?:[^a-zA-Z0-9_-]|:\\w+)(.*)"), QRegularExpression::UseUnicodePropertiesOption);
        const auto match = reCmd.match(text());
        if (match.hasMatch()) {
            setSelection(text().length() - match.capturedLength(1), match.capturedLength(1));
        }
    }
}
// END KateCmdLineEdit

// BEGIN KateIconBorder
using namespace KTextEditor;

KateIconBorder::KateIconBorder(KateViewInternal *internalView, QWidget *parent)
    : QWidget(parent)
    , m_view(internalView->m_view)
    , m_doc(internalView->doc())
    , m_viewInternal(internalView)
    , m_iconBorderOn(false)
    , m_lineNumbersOn(false)
    , m_relLineNumbersOn(false)
    , m_updateRelLineNumbers(false)
    , m_foldingMarkersOn(false)
    , m_dynWrapIndicatorsOn(false)
    , m_annotationBorderOn(false)
    , m_updatePositionToArea(true)
    , m_annotationItemDelegate(new KateAnnotationItemDelegate(this))
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_StaticContents);

    // See: https://doc.qt.io/qt-5/qwidget.html#update. As this widget does not
    // have a background, there's no need for Qt to erase the widget's area
    // before repainting. Enabling this prevents flickering when the widget is
    // repainted.
    setAttribute(Qt::WA_OpaquePaintEvent);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    setMouseTracking(true);
    m_doc->setMarkDescription(Document::markType01, i18n("Bookmark"));
    m_doc->setMarkIcon(Document::markType01, QIcon::fromTheme(QStringLiteral("bookmarks")));

    connect(m_annotationItemDelegate, &AbstractAnnotationItemDelegate::sizeHintChanged, this, &KateIconBorder::updateAnnotationBorderWidth);

    updateFont();

    m_antiFlickerTimer.setSingleShot(true);
    m_antiFlickerTimer.setInterval(300);
    connect(&m_antiFlickerTimer, &QTimer::timeout, this, &KateIconBorder::highlightFolding);

    // user interaction (scrolling) hides e.g. preview
    connect(m_view, &KTextEditor::ViewPrivate::displayRangeChanged, this, &KateIconBorder::displayRangeChanged);
}

KateIconBorder::~KateIconBorder()
{
    delete m_foldingPreview;
    delete m_foldingRange;
}

void KateIconBorder::setIconBorderOn(bool enable)
{
    if (enable == m_iconBorderOn) {
        return;
    }

    m_iconBorderOn = enable;

    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

void KateIconBorder::setAnnotationBorderOn(bool enable)
{
    if (enable == m_annotationBorderOn) {
        return;
    }

    m_annotationBorderOn = enable;

    // make sure the tooltip is hidden
    if (!m_annotationBorderOn && !m_hoveredAnnotationGroupIdentifier.isEmpty()) {
        m_hoveredAnnotationGroupIdentifier.clear();
        hideAnnotationTooltip();
    }

    Q_EMIT m_view->annotationBorderVisibilityChanged(m_view, enable);

    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

void KateIconBorder::removeAnnotationHovering()
{
    // remove hovering if it's still there
    if (m_annotationBorderOn && !m_hoveredAnnotationGroupIdentifier.isEmpty()) {
        m_hoveredAnnotationGroupIdentifier.clear();
        QTimer::singleShot(0, this, SLOT(update()));
    }
}

void KateIconBorder::setLineNumbersOn(bool enable)
{
    if (enable == m_lineNumbersOn) {
        return;
    }

    m_lineNumbersOn = enable;
    m_dynWrapIndicatorsOn = (m_dynWrapIndicators == 1) ? enable : m_dynWrapIndicators;

    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

void KateIconBorder::setRelLineNumbersOn(bool enable)
{
    if (enable == m_relLineNumbersOn) {
        return;
    }

    m_relLineNumbersOn = enable;
    /*
     * We don't have to touch the m_dynWrapIndicatorsOn because
     * we already got it right from the m_lineNumbersOn
     */
    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

void KateIconBorder::updateForCursorLineChange()
{
    if (m_relLineNumbersOn) {
        m_updateRelLineNumbers = true;
    }

    // always do normal update, e.g. for different current line color!
    update();
}

void KateIconBorder::setDynWrapIndicators(int state)
{
    if (state == m_dynWrapIndicators) {
        return;
    }

    m_dynWrapIndicators = state;
    m_dynWrapIndicatorsOn = (state == 1) ? m_lineNumbersOn : state;

    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

void KateIconBorder::setFoldingMarkersOn(bool enable)
{
    if (enable == m_foldingMarkersOn) {
        return;
    }

    m_foldingMarkersOn = enable;

    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

QSize KateIconBorder::sizeHint() const
{
    int w = 1; // Must be any value != 0 or we will never painted!

    const int i = m_positionToArea.size();
    if (i > 0) {
        w = m_positionToArea.at(i - 1).first;
    }

    return QSize(w, 0);
}

// This function (re)calculates the maximum width of any of the digit characters (0 -> 9)
// for graceful handling of variable-width fonts as the linenumber font.
void KateIconBorder::updateFont()
{
    // Loop to determine the widest numeric character in the current font.
    const QFontMetricsF &fm = m_view->renderer()->currentFontMetrics();
    m_maxCharWidth = 0.0;
    for (char c = '0'; c <= '9'; ++c) {
        const qreal charWidth = ceil(fm.horizontalAdvance(QLatin1Char(c)));
        m_maxCharWidth = qMax(m_maxCharWidth, charWidth);
    }

    // NOTE/TODO(or not) Take size of m_dynWrapIndicatorChar into account.
    // It's a multi-char and it's reported size is, even with a mono-space font,
    // bigger than each digit, e.g. 10 vs 12. Currently it seems to work even with
    // "Line Numbers Off" but all these width calculating looks slightly hacky

    // the icon pane scales with the font...
    m_iconAreaWidth = fm.height();

    // Only for now, later may that become an own value
    m_foldingAreaWidth = m_iconAreaWidth;

    calcAnnotationBorderWidth();

    m_updatePositionToArea = true;

    QMetaObject::invokeMethod(
        this,
        [this] {
            update();
        },
        Qt::QueuedConnection);
}

int KateIconBorder::lineNumberWidth() const
{
    int width = 0;
    // Avoid unneeded expensive calculations ;-)
    if (m_lineNumbersOn) {
        // width = (number of digits + 1) * char width
        const int digits = (int)ceil(log10((double)(m_view->doc()->lines() + 1)));
        width = (int)ceil((digits + 1) * m_maxCharWidth);
    }

    if ((width < 1) && m_dynWrapIndicatorsOn && m_view->config()->dynWordWrap()) {
        // FIXME Why 2x? because of above (number of digits + 1)
        // -> looks to me like a hint for bad calculation elsewhere
        width = 2 * m_maxCharWidth;
    }

    return width;
}

void KateIconBorder::dragEnterEvent(QDragEnterEvent *event)
{
    m_view->m_viewInternal->dragEnterEvent(event);
}

void KateIconBorder::dragMoveEvent(QDragMoveEvent *event)
{
    // FIXME Just calling m_view->m_viewInternal->dragMoveEvent(e) don't work
    // as intended, we need to set the cursor at column 1
    // Is there a way to change the pos of the event?
    QPoint pos(0, event->position().y());
    // Code copy of KateViewInternal::dragMoveEvent
    m_view->m_viewInternal->placeCursor(pos, true, false);
    m_view->m_viewInternal->fixDropEvent(event);
}

void KateIconBorder::dropEvent(QDropEvent *event)
{
    m_view->m_viewInternal->dropEvent(event);
}

void KateIconBorder::paintEvent(QPaintEvent *e)
{
    paintBorder(e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height());
}

static void paintTriangle(QPainter &painter, QColor c, int xOffset, int yOffset, int width, int height, bool open)
{
    painter.setRenderHint(QPainter::Antialiasing);

    qreal size = qMin(width, height);

    if (open) {
        // Paint unfolded icon less pushy
        if (KColorUtils::luma(c) < 0.25) {
            c = KColorUtils::darken(c);
        } else {
            c = KColorUtils::shade(c, 0.1);
        }

    } else {
        // Paint folded icon in contrast to popup highlighting
        if (KColorUtils::luma(c) > 0.25) {
            c = KColorUtils::darken(c);
        } else {
            c = KColorUtils::shade(c, 0.1);
        }
    }

    QPen pen;
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setColor(c);
    pen.setWidthF(1.5);
    painter.setPen(pen);
    painter.setBrush(c);

    // let some border, if possible
    size *= 0.6;

    qreal halfSize = size / 2;
    qreal halfSizeP = halfSize * 0.6;
    QPointF middle(xOffset + (qreal)width / 2, yOffset + (qreal)height / 2);

    if (open) {
        QPointF points[3] = {middle + QPointF(-halfSize, -halfSizeP), middle + QPointF(halfSize, -halfSizeP), middle + QPointF(0, halfSizeP)};
        painter.drawConvexPolygon(points, 3);
    } else {
        QPointF points[3] = {middle + QPointF(-halfSizeP, -halfSize), middle + QPointF(-halfSizeP, halfSize), middle + QPointF(halfSizeP, 0)};
        painter.drawConvexPolygon(points, 3);
    }

    painter.setRenderHint(QPainter::Antialiasing, false);
}

/**
 * Helper class for an identifier which can be an empty or non-empty string or invalid.
 * Avoids complicated explicit statements in code dealing with the identifier
 * received as QVariant from a model.
 */
class KateAnnotationGroupIdentifier
{
public:
    KateAnnotationGroupIdentifier(const QVariant &identifier)
        : m_isValid(identifier.isValid() && identifier.canConvert<QString>())
        , m_id(m_isValid ? identifier.toString() : QString())
    {
    }
    KateAnnotationGroupIdentifier() = default;
    KateAnnotationGroupIdentifier(const KateAnnotationGroupIdentifier &rhs) = default;

    KateAnnotationGroupIdentifier &operator=(const KateAnnotationGroupIdentifier &rhs)
    {
        m_isValid = rhs.m_isValid;
        m_id = rhs.m_id;
        return *this;
    }
    KateAnnotationGroupIdentifier &operator=(const QVariant &identifier)
    {
        m_isValid = (identifier.isValid() && identifier.canConvert<QString>());
        if (m_isValid) {
            m_id = identifier.toString();
        } else {
            m_id.clear();
        }
        return *this;
    }

    bool operator==(const KateAnnotationGroupIdentifier &rhs) const
    {
        return (m_isValid == rhs.m_isValid) && (!m_isValid || (m_id == rhs.m_id));
    }
    bool operator!=(const KateAnnotationGroupIdentifier &rhs) const
    {
        return (m_isValid != rhs.m_isValid) || (m_isValid && (m_id != rhs.m_id));
    }

    void clear()
    {
        m_isValid = false;
        m_id.clear();
    }
    bool isValid() const
    {
        return m_isValid;
    }
    const QString &id() const
    {
        return m_id;
    }

private:
    bool m_isValid = false;
    QString m_id;
};

/**
 * Helper class for iterative calculation of data regarding the position
 * of a line with regard to annotation item grouping.
 */
class KateAnnotationGroupPositionState
{
public:
    /**
     * @param startz first rendered displayed line
     * @param isUsed flag whether the KateAnnotationGroupPositionState object will
     *               be used or is just created due to being on the stack
     */
    KateAnnotationGroupPositionState(KateViewInternal *viewInternal,
                                     const KTextEditor::AnnotationModel *model,
                                     const QString &hoveredAnnotationGroupIdentifier,
                                     uint startz,
                                     bool isUsed);
    /**
     * @param styleOption option to fill with data for the given line
     * @param z rendered displayed line
     * @param realLine real line which is rendered here (passed to avoid another look-up)
     */
    void nextLine(KTextEditor::StyleOptionAnnotationItem &styleOption, uint z, int realLine);

private:
    KateViewInternal *m_viewInternal;
    const KTextEditor::AnnotationModel *const m_model;
    const QString m_hoveredAnnotationGroupIdentifier;

    int m_visibleWrappedLineInAnnotationGroup = -1;
    KateAnnotationGroupIdentifier m_lastAnnotationGroupIdentifier;
    KateAnnotationGroupIdentifier m_nextAnnotationGroupIdentifier;
    bool m_isSameAnnotationGroupsSinceLast = false;
};

KateAnnotationGroupPositionState::KateAnnotationGroupPositionState(KateViewInternal *viewInternal,
                                                                   const KTextEditor::AnnotationModel *model,
                                                                   const QString &hoveredAnnotationGroupIdentifier,
                                                                   uint startz,
                                                                   bool isUsed)
    : m_viewInternal(viewInternal)
    , m_model(model)
    , m_hoveredAnnotationGroupIdentifier(hoveredAnnotationGroupIdentifier)
{
    if (!isUsed) {
        return;
    }

    if (!m_model || (static_cast<int>(startz) >= m_viewInternal->cache()->viewCacheLineCount())) {
        return;
    }

    const auto realLineAtStart = m_viewInternal->cache()->viewLine(startz).line();
    m_nextAnnotationGroupIdentifier = m_model->data(realLineAtStart, (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole);
    if (m_nextAnnotationGroupIdentifier.isValid()) {
        // estimate state of annotation group before first rendered line
        if (startz == 0) {
            if (realLineAtStart > 0) {
                // TODO: here we would want to scan until the next line that would be displayed,
                // to see if there are any group changes until then
                // for now simply taking neighbour line into account, not a grave bug on the first displayed line
                m_lastAnnotationGroupIdentifier = m_model->data(realLineAtStart - 1, (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole);
                m_isSameAnnotationGroupsSinceLast = (m_lastAnnotationGroupIdentifier == m_nextAnnotationGroupIdentifier);
            }
        } else {
            const auto realLineBeforeStart = m_viewInternal->cache()->viewLine(startz - 1).line();
            m_lastAnnotationGroupIdentifier = m_model->data(realLineBeforeStart, (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole);
            if (m_lastAnnotationGroupIdentifier.isValid()) {
                if (m_lastAnnotationGroupIdentifier.id() == m_nextAnnotationGroupIdentifier.id()) {
                    m_isSameAnnotationGroupsSinceLast = true;
                    // estimate m_visibleWrappedLineInAnnotationGroup from lines before startz
                    for (uint z = startz; z > 0; --z) {
                        const auto realLine = m_viewInternal->cache()->viewLine(z - 1).line();
                        const KateAnnotationGroupIdentifier identifier =
                            m_model->data(realLine, (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole);
                        if (identifier != m_lastAnnotationGroupIdentifier) {
                            break;
                        }
                        ++m_visibleWrappedLineInAnnotationGroup;
                    }
                }
            }
        }
    }
}

void KateAnnotationGroupPositionState::nextLine(KTextEditor::StyleOptionAnnotationItem &styleOption, uint z, int realLine)
{
    styleOption.wrappedLine = m_viewInternal->cache()->viewLine(z).viewLine();
    styleOption.wrappedLineCount = m_viewInternal->cache()->viewLineCount(realLine);

    // Estimate position in group
    const KateAnnotationGroupIdentifier annotationGroupIdentifier = m_nextAnnotationGroupIdentifier;
    bool isSameAnnotationGroupsSinceThis = false;
    // Calculate next line's group identifier
    // shortcut: assuming wrapped lines are always displayed together, test is simple
    if (styleOption.wrappedLine + 1 < styleOption.wrappedLineCount) {
        m_nextAnnotationGroupIdentifier = annotationGroupIdentifier;
        isSameAnnotationGroupsSinceThis = true;
    } else {
        if (static_cast<int>(z + 1) < m_viewInternal->cache()->viewCacheLineCount()) {
            const int realLineAfter = m_viewInternal->cache()->viewLine(z + 1).line();
            // search for any realLine with a different group id, also the non-displayed
            int rl = realLine + 1;
            for (; rl <= realLineAfter; ++rl) {
                m_nextAnnotationGroupIdentifier = m_model->data(rl, (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole);
                if (!m_nextAnnotationGroupIdentifier.isValid() || (m_nextAnnotationGroupIdentifier.id() != annotationGroupIdentifier.id())) {
                    break;
                }
            }
            isSameAnnotationGroupsSinceThis = (rl > realLineAfter);
            if (rl < realLineAfter) {
                m_nextAnnotationGroupIdentifier = m_model->data(realLineAfter, (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole);
            }
        } else {
            // TODO: check next line after display end
            m_nextAnnotationGroupIdentifier.clear();
            isSameAnnotationGroupsSinceThis = false;
        }
    }

    if (annotationGroupIdentifier.isValid()) {
        if (m_hoveredAnnotationGroupIdentifier == annotationGroupIdentifier.id()) {
            styleOption.state |= QStyle::State_MouseOver;
        } else {
            styleOption.state &= ~QStyle::State_MouseOver;
        }

        if (m_isSameAnnotationGroupsSinceLast) {
            ++m_visibleWrappedLineInAnnotationGroup;
        } else {
            m_visibleWrappedLineInAnnotationGroup = 0;
        }

        styleOption.annotationItemGroupingPosition = StyleOptionAnnotationItem::InGroup;
        if (!m_isSameAnnotationGroupsSinceLast) {
            styleOption.annotationItemGroupingPosition |= StyleOptionAnnotationItem::GroupBegin;
        }
        if (!isSameAnnotationGroupsSinceThis) {
            styleOption.annotationItemGroupingPosition |= StyleOptionAnnotationItem::GroupEnd;
        }
    } else {
        m_visibleWrappedLineInAnnotationGroup = 0;
    }
    styleOption.visibleWrappedLineInGroup = m_visibleWrappedLineInAnnotationGroup;

    m_lastAnnotationGroupIdentifier = m_nextAnnotationGroupIdentifier;
    m_isSameAnnotationGroupsSinceLast = isSameAnnotationGroupsSinceThis;
}

void KateIconBorder::paintBorder(int /*x*/, int y, int /*width*/, int height)
{
    const uint h = m_view->renderer()->lineHeight();
    const uint startz = (y / h);
    const uint endz = qMin(startz + 1 + (height / h), static_cast<uint>(m_viewInternal->cache()->viewCacheLineCount()));
    const uint currentLine = m_view->cursorPosition().line();

    // center the folding boxes
    int m_px = (h - 11) / 2;
    if (m_px < 0) {
        m_px = 0;
    }

    // Ensure we miss no change of the count of line number digits
    const int newNeededWidth = lineNumberWidth();

    if (m_updatePositionToArea || (newNeededWidth != m_lineNumberAreaWidth)) {
        m_lineNumberAreaWidth = newNeededWidth;
        m_updatePositionToArea = true;
        m_positionToArea.clear();
    }

    QPainter p(this);
    p.setRenderHints(QPainter::TextAntialiasing);
    p.setFont(m_view->renderer()->currentFont()); // for line numbers

    KTextEditor::AnnotationModel *model = m_view->annotationModel() ? m_view->annotationModel() : m_doc->annotationModel();
    KateAnnotationGroupPositionState annotationGroupPositionState(m_viewInternal, model, m_hoveredAnnotationGroupIdentifier, startz, m_annotationBorderOn);

    // Fetch often used data only once, improve readability
    const int w = width();
    const QColor iconBarColor = m_view->rendererConfig()->iconBarColor(); // Effective our background
    const QColor lineNumberColor = m_view->rendererConfig()->lineNumberColor();
    const QColor backgroundColor = m_view->rendererConfig()->backgroundColor(); // Of the edit area
    const QColor currentLineHighlight = m_view->rendererConfig()->highlightedLineColor(); // Of the edit area

    // Paint the border in chunks line by line
    for (uint z = startz; z < endz; z++) {
        // Painting coordinates, lineHeight * lineNumber
        const uint y = h * z;

        // Paint the border in chunks left->right, remember used width
        uint lnX = 0;

        // get line for this coordinates if possible
        const KateTextLayout lineLayout = m_viewInternal->cache()->viewLine(z);
        const int realLine = lineLayout.line();

        // Paint background over full width
        p.fillRect(lnX, y, w, h, iconBarColor);

        // overpaint with current line highlighting over full width
        const bool isCurrentLine = (realLine == static_cast<int>(currentLine)) && lineLayout.includesCursor(m_view->cursorPosition());
        if (isCurrentLine) {
            p.fillRect(lnX, y, w, h, currentLineHighlight);
        }

        // for real lines we need to do more stuff ;=)
        if (realLine >= 0) {
            // icon pane
            if (m_iconBorderOn) {
                const uint mrk(m_doc->mark(realLine)); // call only once
                if (mrk && lineLayout.startCol() == 0) {
                    for (uint bit = 0; bit < 32; bit++) {
                        Document::MarkTypes markType = (Document::MarkTypes)(1U << bit);
                        if (mrk & markType) {
                            const QIcon markIcon = m_doc->markIcon(markType);

                            if (!markIcon.isNull() && h > 0 && m_iconAreaWidth > 0) {
                                const int s = qMin(m_iconAreaWidth, static_cast<int>(h)) - 2;

                                // center the mark pixmap
                                const int x_px = qMax(m_iconAreaWidth - s, 0) / 2;
                                const int y_px = qMax(static_cast<int>(h) - s, 0) / 2;

                                markIcon.paint(&p, lnX + x_px, y + y_px, s, s);
                            }
                        }
                    }
                }

                lnX += m_iconAreaWidth;
                if (m_updatePositionToArea) {
                    m_positionToArea.push_back(AreaPosition(lnX, IconBorder));
                }
            }

            // annotation information
            if (m_annotationBorderOn) {
                // Draw a border line between annotations and the line numbers
                p.setPen(lineNumberColor);
                p.setBrush(lineNumberColor);

                const qreal borderX = lnX + m_annotationAreaWidth + 0.5;
                p.drawLine(QPointF(borderX, y + 0.5), QPointF(borderX, y + h - 0.5));

                if (model) {
                    KTextEditor::StyleOptionAnnotationItem styleOption;
                    initStyleOption(&styleOption);
                    styleOption.rect.setRect(lnX, y, m_annotationAreaWidth, h);
                    annotationGroupPositionState.nextLine(styleOption, z, realLine);

                    m_annotationItemDelegate->paint(&p, styleOption, model, realLine);
                }

                lnX += m_annotationAreaWidth + m_separatorWidth;
                if (m_updatePositionToArea) {
                    m_positionToArea.push_back(AreaPosition(lnX, AnnotationBorder));
                }
            }

            // line number
            if (m_lineNumbersOn || m_dynWrapIndicatorsOn) {
                QColor usedLineNumberColor;
                const int distanceToCurrent = abs(realLine - static_cast<int>(currentLine));
                if (distanceToCurrent == 0) {
                    usedLineNumberColor = m_view->rendererConfig()->currentLineNumberColor();
                } else {
                    usedLineNumberColor = lineNumberColor;
                }
                p.setPen(usedLineNumberColor);
                p.setBrush(usedLineNumberColor);

                if (lineLayout.startCol() == 0) {
                    if (m_relLineNumbersOn) {
                        if (distanceToCurrent == 0) {
                            p.drawText(lnX + m_maxCharWidth / 2,
                                       y,
                                       m_lineNumberAreaWidth - m_maxCharWidth,
                                       h,
                                       Qt::TextDontClip | Qt::AlignLeft | Qt::AlignVCenter,
                                       QString::number(realLine + 1));
                        } else {
                            p.drawText(lnX + m_maxCharWidth / 2,
                                       y,
                                       m_lineNumberAreaWidth - m_maxCharWidth,
                                       h,
                                       Qt::TextDontClip | Qt::AlignRight | Qt::AlignVCenter,
                                       QString::number(distanceToCurrent));
                        }
                        if (m_updateRelLineNumbers) {
                            m_updateRelLineNumbers = false;
                            update();
                        }
                    } else if (m_lineNumbersOn) {
                        p.drawText(lnX + m_maxCharWidth / 2,
                                   y,
                                   m_lineNumberAreaWidth - m_maxCharWidth,
                                   h,
                                   Qt::TextDontClip | Qt::AlignRight | Qt::AlignVCenter,
                                   QString::number(realLine + 1));
                    }
                } else if (m_dynWrapIndicatorsOn) {
                    p.drawText(lnX + m_maxCharWidth / 2,
                               y,
                               m_lineNumberAreaWidth - m_maxCharWidth,
                               h,
                               Qt::TextDontClip | Qt::AlignRight | Qt::AlignVCenter,
                               m_dynWrapIndicatorChar);
                }

                lnX += m_lineNumberAreaWidth + m_separatorWidth;
                if (m_updatePositionToArea) {
                    m_positionToArea.push_back(AreaPosition(lnX, LineNumbers));
                }
            }

            // modified line system
            if (m_view->config()->lineModification() && !m_doc->url().isEmpty()) {
                const auto tl = m_doc->plainKateTextLine(realLine);
                if (tl.markedAsModified()) {
                    p.fillRect(lnX, y, m_modAreaWidth, h, m_view->rendererConfig()->modifiedLineColor());
                } else if (tl.markedAsSavedOnDisk()) {
                    p.fillRect(lnX, y, m_modAreaWidth, h, m_view->rendererConfig()->savedLineColor());
                }

                lnX += m_modAreaWidth; // No m_separatorWidth
                if (m_updatePositionToArea) {
                    m_positionToArea.push_back(AreaPosition(lnX, None));
                }
            }

            // folding markers
            if (m_foldingMarkersOn) {
                const QColor foldingColor(m_view->rendererConfig()->foldingColor());
                // possible additional folding highlighting
                if (m_foldingRange && m_foldingRange->overlapsLine(realLine)) {
                    p.fillRect(lnX, y, m_foldingAreaWidth, h, foldingColor);
                }

                if (lineLayout.startCol() == 0) {
                    QList<QPair<qint64, Kate::TextFolding::FoldingRangeFlags>> startingRanges = m_view->textFolding().foldingRangesStartingOnLine(realLine);
                    bool anyFolded = false;
                    for (int i = 0; i < startingRanges.size(); ++i) {
                        if (startingRanges[i].second & Kate::TextFolding::Folded) {
                            anyFolded = true;
                        }
                    }
                    if (!m_view->config()->showFoldingOnHoverOnly() || m_mouseOver) {
                        if (!startingRanges.isEmpty() || m_doc->buffer().isFoldingStartingOnLine(realLine).first) {
                            if (anyFolded) {
                                paintTriangle(p, foldingColor, lnX, y, m_foldingAreaWidth, h, false);
                            } else {
                                // Don't try to use currentLineNumberColor, the folded icon gets also not highligted
                                paintTriangle(p, lineNumberColor, lnX, y, m_foldingAreaWidth, h, true);
                            }
                        }
                    }
                }

                lnX += m_foldingAreaWidth;
                if (m_updatePositionToArea) {
                    m_positionToArea.push_back(AreaPosition(lnX, FoldingMarkers));
                }
            }
        }

        // Overpaint again the end to simulate some margin to the edit area,
        // so that the text not looks like stuck to the border
        // we do this AFTER all other painting to ensure this leaves no artifacts
        // we kill 2 separator widths as we will below paint a line over this
        // otherwise this has some minimal overlap and looks ugly e.g. for scaled rendering
        p.fillRect(w - 2 * m_separatorWidth, y, w, h, backgroundColor);

        // overpaint again with selection or current line highlighting if necessary
        if (realLine >= 0 && m_view->selection() && !m_view->blockSelection() && m_view->selectionRange().start() < lineLayout.start()
            && m_view->selectionRange().end() >= lineLayout.start()) {
            // selection overpaint to signal the end of the previous line is included in the selection
            p.fillRect(w - 2 * m_separatorWidth, y, w, h, m_view->rendererConfig()->selectionColor());
        } else if (isCurrentLine) {
            // normal current line overpaint
            p.fillRect(w - 2 * m_separatorWidth, y, w, h, currentLineHighlight);
        }

        // add separator line if needed
        // we do this AFTER all other painting to ensure this leaves no artifacts
        p.setPen(m_view->rendererConfig()->separatorColor());
        p.setBrush(m_view->rendererConfig()->separatorColor());
        p.drawLine(w - 2 * m_separatorWidth, y, w - 2 * m_separatorWidth, y + h);

        // we might need to trigger geometry updates
        if ((realLine >= 0) && m_updatePositionToArea) {
            m_updatePositionToArea = false;
            // Don't forget our "text-stuck-to-border" protector + border line
            lnX += 2 * m_separatorWidth;
            m_positionToArea.push_back(AreaPosition(lnX, None));

            // Now that we know our needed space, ensure we are painted properly
            // we still keep painting to not have strange flicker
            QTimer::singleShot(0, this, &KateIconBorder::delayedUpdateOfSizeWithRepaint);
        }
    }
}

KateIconBorder::BorderArea KateIconBorder::positionToArea(const QPoint &p) const
{
    auto it = std::find_if(m_positionToArea.cbegin(), m_positionToArea.cend(), [p](const AreaPosition &ap) {
        return p.x() <= ap.first;
    });
    if (it != m_positionToArea.cend()) {
        return it->second;
    }
    return None;
}

void KateIconBorder::mousePressEvent(QMouseEvent *e)
{
    const KateTextLayout &t = m_viewInternal->yToKateTextLayout(e->position().y());
    if (t.isValid()) {
        m_lastClickedLine = t.line();
        const auto area = positionToArea(e->pos());
        // IconBorder and AnnotationBorder have their own behavior; don't forward to view
        if (area != IconBorder && area != AnnotationBorder) {
            const auto pos = QPoint(0, e->position().y());
            if (area == LineNumbers && e->button() == Qt::LeftButton && !(e->modifiers() & Qt::ShiftModifier)) {
                // setup view so the following mousePressEvent will select the line
                m_viewInternal->beginSelectLine(pos);
            }
            QMouseEvent forward(QEvent::MouseButtonPress, pos, m_viewInternal->mapToGlobal(pos), e->button(), e->buttons(), e->modifiers());
            m_viewInternal->mousePressEvent(&forward);
        }
        return e->accept();
    }

    QWidget::mousePressEvent(e);
}

void KateIconBorder::highlightFoldingDelayed(int line)
{
    if ((line == m_currentLine) || (line >= m_doc->buffer().lines())) {
        return;
    }

    m_currentLine = line;

    if (m_foldingRange) {
        // We are for a while in the folding area, no need for delay
        highlightFolding();

    } else if (!m_antiFlickerTimer.isActive()) {
        m_antiFlickerTimer.start();
    }
}

void KateIconBorder::highlightFolding()
{
    // compute to which folding range we belong
    // FIXME: optimize + perhaps have some better threshold or use timers!
    KTextEditor::Range newRange = KTextEditor::Range::invalid();
    for (int line = m_currentLine; line >= qMax(0, m_currentLine - 1024); --line) {
        // try if we have folding range from that line, should be fast per call
        KTextEditor::Range foldingRange = m_doc->buffer().computeFoldingRangeForStartLine(line);
        if (!foldingRange.isValid()) {
            continue;
        }

        // does the range reach us?
        if (foldingRange.overlapsLine(m_currentLine)) {
            newRange = foldingRange;
            break;
        }
    }

    if (newRange.isValid() && m_foldingRange && *m_foldingRange == newRange) {
        // new range equals the old one, nothing to do.
        return;
    }

    // the ranges differ, delete the old, if it exists
    delete m_foldingRange;
    m_foldingRange = nullptr;
    // New range, new preview!
    delete m_foldingPreview;

    bool showPreview = false;

    if (newRange.isValid()) {
        // When next line is not visible we have a folded range, only then we want a preview!
        showPreview = !m_view->textFolding().isLineVisible(newRange.start().line() + 1);

        // qCDebug(LOG_KTE) << "new folding hl-range:" << newRange;
        m_foldingRange = m_doc->newMovingRange(newRange, KTextEditor::MovingRange::ExpandRight);
        KTextEditor::Attribute::Ptr attr(new KTextEditor::Attribute());

        // create highlighting color
        // we avoid alpha as overpainting leads to ugly lines (https://bugreports.qt.io/browse/QTBUG-66036)
        attr->setBackground(QBrush(m_view->rendererConfig()->foldingColor()));

        m_foldingRange->setView(m_view);
        // use z depth defined in moving ranges interface
        m_foldingRange->setZDepth(-100.0);
        m_foldingRange->setAttribute(attr);
    }

    // show text preview, if a folded region starts here...
    // ...but only when main window is active (#392396)
    const bool isWindowActive = !window() || window()->isActiveWindow();
    if (showPreview && m_view->config()->foldingPreview() && isWindowActive) {
        m_foldingPreview = new KateTextPreview(m_view, this);
        m_foldingPreview->setAttribute(Qt::WA_ShowWithoutActivating);
        m_foldingPreview->setFrameStyle(QFrame::StyledPanel);

        // Calc how many lines can be displayed in the popup
        const int lineHeight = m_view->renderer()->lineHeight();
        const int foldingStartLine = m_foldingRange->start().line();
        // FIXME Is there really no easier way to find lineInDisplay?
        const QPoint pos = m_viewInternal->mapFrom(m_view, m_view->cursorToCoordinate(KTextEditor::Cursor(foldingStartLine, 0)));
        const int lineInDisplay = pos.y() / lineHeight;
        // Allow slightly overpainting of the view bottom to proper cover all lines
        const int extra = (m_viewInternal->height() % lineHeight) > (lineHeight * 0.6) ? 1 : 0;
        const int lineCount = qMin(m_foldingRange->numberOfLines() + 1, m_viewInternal->linesDisplayed() - lineInDisplay + extra);

        m_foldingPreview->resize(m_viewInternal->width(), lineCount * lineHeight + 2 * m_foldingPreview->frameWidth());
        const int xGlobal = mapToGlobal(QPoint(width(), 0)).x();
        const int yGlobal = m_view->mapToGlobal(m_view->cursorToCoordinate(KTextEditor::Cursor(foldingStartLine, 0))).y();
        m_foldingPreview->move(QPoint(xGlobal, yGlobal) - m_foldingPreview->contentsRect().topLeft());
        m_foldingPreview->setLine(foldingStartLine);
        m_foldingPreview->setCenterView(false);
        m_foldingPreview->setShowFoldedLines(true);
        m_foldingPreview->raise();
        m_foldingPreview->show();
    }
}

void KateIconBorder::hideFolding()
{
    if (m_antiFlickerTimer.isActive()) {
        m_antiFlickerTimer.stop();
    }

    m_currentLine = -1;
    delete m_foldingRange;
    m_foldingRange = nullptr;

    delete m_foldingPreview;
}

void KateIconBorder::enterEvent(QEnterEvent *event)
{
    m_mouseOver = true;
    if (m_view->config()->showFoldingOnHoverOnly())
        repaint();
    QWidget::enterEvent(event);
}

void KateIconBorder::leaveEvent(QEvent *event)
{
    m_mouseOver = false;
    hideFolding();
    removeAnnotationHovering();
    if (m_view->config()->showFoldingOnHoverOnly())
        repaint();

    QWidget::leaveEvent(event);
}

void KateIconBorder::mouseMoveEvent(QMouseEvent *e)
{
    const KateTextLayout &t = m_viewInternal->yToKateTextLayout(e->position().y());
    if (!t.isValid()) {
        // Cleanup everything which may be shown
        removeAnnotationHovering();
        hideFolding();

    } else {
        const BorderArea area = positionToArea(e->pos());
        if (area == FoldingMarkers) {
            highlightFoldingDelayed(t.line());
        } else {
            hideFolding();
        }
        if (area == AnnotationBorder) {
            KTextEditor::AnnotationModel *model = m_view->annotationModel() ? m_view->annotationModel() : m_doc->annotationModel();
            if (model) {
                m_hoveredAnnotationGroupIdentifier = model->data(t.line(), (Qt::ItemDataRole)KTextEditor::AnnotationModel::GroupIdentifierRole).toString();
                const QPoint viewRelativePos = m_view->mapFromGlobal(e->globalPosition()).toPoint();
                QHelpEvent helpEvent(QEvent::ToolTip, viewRelativePos, e->globalPosition().toPoint());
                KTextEditor::StyleOptionAnnotationItem styleOption;
                initStyleOption(&styleOption);
                styleOption.rect = annotationLineRectInView(t.line());
                setStyleOptionLineData(&styleOption, e->position().y(), t.line(), model, m_hoveredAnnotationGroupIdentifier);
                m_annotationItemDelegate->helpEvent(&helpEvent, m_view, styleOption, model, t.line());

                QTimer::singleShot(0, this, SLOT(update()));
            }
        } else {
            if (area == IconBorder) {
                m_doc->requestMarkTooltip(t.line(), e->globalPosition().toPoint());
            }

            m_hoveredAnnotationGroupIdentifier.clear();
            QTimer::singleShot(0, this, SLOT(update()));
        }
        if (area != IconBorder) {
            QPoint p = m_viewInternal->mapFromGlobal(e->globalPosition().toPoint());
            QMouseEvent forward(QEvent::MouseMove, p, m_viewInternal->mapToGlobal(p), e->button(), e->buttons(), e->modifiers());
            m_viewInternal->mouseMoveEvent(&forward);
        }
    }

    QWidget::mouseMoveEvent(e);
}

void KateIconBorder::mouseReleaseEvent(QMouseEvent *e)
{
    const int cursorOnLine = m_viewInternal->yToKateTextLayout(e->position().y()).line();
    if (cursorOnLine == m_lastClickedLine && cursorOnLine >= 0 && cursorOnLine <= m_doc->lastLine()) {
        const BorderArea area = positionToArea(e->pos());
        if (area == IconBorder) {
            if (e->button() == Qt::LeftButton) {
                if (!m_doc->handleMarkClick(cursorOnLine)) {
                    KateViewConfig *config = m_view->config();
                    const uint editBits = m_doc->editableMarks();
                    // is the default or the only editable mark
                    bool ctrlPressed = QGuiApplication::keyboardModifiers() == Qt::KeyboardModifier::ControlModifier;
                    if (qPopulationCount(editBits) == 1 || ctrlPressed) {
                        const uint singleMark = (qPopulationCount(editBits) > 1) ? (editBits & config->defaultMarkType()) : editBits;
                        if (m_doc->mark(cursorOnLine) & singleMark) {
                            m_doc->removeMark(cursorOnLine, singleMark);
                        } else {
                            m_doc->addMark(cursorOnLine, singleMark);
                        }
                    } else if (config->allowMarkMenu()) {
                        showMarkMenu(cursorOnLine, QCursor::pos());
                    }
                }
            } else if (e->button() == Qt::RightButton) {
                showMarkMenu(cursorOnLine, QCursor::pos());
            }
        }

        if (area == FoldingMarkers) {
            // Prefer the highlighted range over the exact clicked line
            const int lineToToggle = m_foldingRange ? m_foldingRange->toRange().start().line() : cursorOnLine;
            if (e->button() == Qt::LeftButton) {
                m_view->toggleFoldingOfLine(lineToToggle);
            } else if (e->button() == Qt::RightButton) {
                m_view->toggleFoldingsInRange(lineToToggle);
            }

            delete m_foldingPreview;
        }

        if (area == AnnotationBorder) {
            const bool singleClick = style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, nullptr, this);
            if (e->button() == Qt::LeftButton && singleClick) {
                Q_EMIT m_view->annotationActivated(m_view, cursorOnLine);
            }
        }
    }

    const QPoint pos(0, e->position().y());
    QMouseEvent forward(QEvent::MouseButtonRelease, pos, m_viewInternal->mapToGlobal(pos), e->button(), e->buttons(), e->modifiers());
    m_viewInternal->mouseReleaseEvent(&forward);
}

void KateIconBorder::mouseDoubleClickEvent(QMouseEvent *e)
{
    int cursorOnLine = m_viewInternal->yToKateTextLayout(e->position().y()).line();

    if (cursorOnLine == m_lastClickedLine && cursorOnLine <= m_doc->lastLine()) {
        const BorderArea area = positionToArea(e->pos());
        const bool singleClick = style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, nullptr, this);
        if (area == AnnotationBorder && !singleClick) {
            Q_EMIT m_view->annotationActivated(m_view, cursorOnLine);
        }
    }
    const QPoint pos(0, e->position().y());
    QMouseEvent forward(QEvent::MouseButtonDblClick, pos, m_viewInternal->mapToGlobal(pos), e->button(), e->buttons(), e->modifiers());
    m_viewInternal->mouseDoubleClickEvent(&forward);
}

void KateIconBorder::contextMenuEvent(QContextMenuEvent *e)
{
    const BorderArea area = positionToArea(e->pos());
    const int cursorOnLine = m_viewInternal->yToKateTextLayout(e->y()).line();

    if (area == AnnotationBorder) {
        showAnnotationMenu(cursorOnLine, e->globalPos());
        return;
    }

    QMenu menu(this);

    KActionCollection *ac = m_view->actionCollection();

    // NOTE Assumes cursor position was updated before the menu opens
    if (QAction *bookmarkToggle = ac->action(QStringLiteral("bookmarks_toggle"))) {
        menu.addAction(bookmarkToggle);
    }
    if (QAction *bookmarkClear = ac->action(QStringLiteral("bookmarks_clear"))) {
        menu.addAction(bookmarkClear);
    }

    menu.addSeparator();

    if (auto a = ac->action(QStringLiteral("edit_copy_file_location"))) {
        menu.addAction(a);
    }

    menu.addSeparator();

    if (QAction *toggleDynWrap = ac->action(QStringLiteral("view_dynamic_word_wrap"))) {
        menu.addAction(toggleDynWrap);
    }
    menu.addSeparator();
    if (QAction *toggleIconBar = ac->action(QStringLiteral("view_border"))) {
        menu.addAction(toggleIconBar);
    }
    if (QAction *toggleLineNumbers = ac->action(QStringLiteral("view_line_numbers"))) {
        menu.addAction(toggleLineNumbers);
    }
    if (QAction *toggleFoldingMarkers = ac->action(QStringLiteral("view_folding_markers"))) {
        menu.addAction(toggleFoldingMarkers);
    }

    menu.exec(e->globalPos());
}

void KateIconBorder::wheelEvent(QWheelEvent *e)
{
    QCoreApplication::sendEvent(m_viewInternal, e);
}

void KateIconBorder::showMarkMenu(uint line, const QPoint &pos)
{
    if (m_doc->handleMarkContextMenu(line, pos)) {
        return;
    }

    if (!m_view->config()->allowMarkMenu()) {
        return;
    }

    QMenu markMenu;
    QMenu selectDefaultMark;
    auto selectDefaultMarkActionGroup = new QActionGroup(&selectDefaultMark);

    std::vector<int> vec(33);
    int i = 1;

    for (uint bit = 0; bit < 32; bit++) {
        KTextEditor::Document::MarkTypes markType = (KTextEditor::Document::MarkTypes)(1U << bit);
        if (!(m_doc->editableMarks() & markType)) {
            continue;
        }

        QAction *mA;
        QAction *dMA;
        const QIcon icon = m_doc->markIcon(markType);
        if (!m_doc->markDescription(markType).isEmpty()) {
            mA = markMenu.addAction(icon, m_doc->markDescription(markType));
            dMA = selectDefaultMark.addAction(icon, m_doc->markDescription(markType));
        } else {
            mA = markMenu.addAction(icon, i18n("Mark Type %1", bit + 1));
            dMA = selectDefaultMark.addAction(icon, i18n("Mark Type %1", bit + 1));
        }
        selectDefaultMarkActionGroup->addAction(dMA);
        mA->setData(i);
        mA->setCheckable(true);
        dMA->setData(i + 100);
        dMA->setCheckable(true);
        if (m_doc->mark(line) & markType) {
            mA->setChecked(true);
        }

        if (markType & KateViewConfig::global()->defaultMarkType()) {
            dMA->setChecked(true);
        }

        vec[i++] = markType;
    }

    if (markMenu.actions().count() == 0) {
        return;
    }

    if (markMenu.actions().count() > 1) {
        markMenu.addAction(i18n("Set Default Mark Type"))->setMenu(&selectDefaultMark);
    }

    QAction *rA = markMenu.exec(pos);
    if (!rA) {
        return;
    }
    int result = rA->data().toInt();
    if (result > 100) {
        KateViewConfig::global()->setValue(KateViewConfig::DefaultMarkType, vec[result - 100]);
    } else {
        KTextEditor::Document::MarkTypes markType = (KTextEditor::Document::MarkTypes)vec[result];
        if (m_doc->mark(line) & markType) {
            m_doc->removeMark(line, markType);
        } else {
            m_doc->addMark(line, markType);
        }
    }
}

KTextEditor::AbstractAnnotationItemDelegate *KateIconBorder::annotationItemDelegate() const
{
    return m_annotationItemDelegate;
}

void KateIconBorder::setAnnotationItemDelegate(KTextEditor::AbstractAnnotationItemDelegate *delegate)
{
    if (delegate == m_annotationItemDelegate) {
        return;
    }

    // reset to default, but already on that?
    if (!delegate && m_isDefaultAnnotationItemDelegate) {
        // nothing to do
        return;
    }

    // make sure the tooltip is hidden
    if (m_annotationBorderOn && !m_hoveredAnnotationGroupIdentifier.isEmpty()) {
        m_hoveredAnnotationGroupIdentifier.clear();
        hideAnnotationTooltip();
    }

    disconnect(m_annotationItemDelegate, &AbstractAnnotationItemDelegate::sizeHintChanged, this, &KateIconBorder::updateAnnotationBorderWidth);
    if (!m_isDefaultAnnotationItemDelegate) {
        disconnect(m_annotationItemDelegate, &QObject::destroyed, this, &KateIconBorder::handleDestroyedAnnotationItemDelegate);
    }

    if (!delegate) {
        // reset to a default delegate
        m_annotationItemDelegate = new KateAnnotationItemDelegate(this);
        m_isDefaultAnnotationItemDelegate = true;
    } else {
        // drop any default delegate
        if (m_isDefaultAnnotationItemDelegate) {
            delete m_annotationItemDelegate;
            m_isDefaultAnnotationItemDelegate = false;
        }

        m_annotationItemDelegate = delegate;
        // catch delegate being destroyed
        connect(delegate, &QObject::destroyed, this, &KateIconBorder::handleDestroyedAnnotationItemDelegate);
    }

    connect(m_annotationItemDelegate, &AbstractAnnotationItemDelegate::sizeHintChanged, this, &KateIconBorder::updateAnnotationBorderWidth);

    if (m_annotationBorderOn) {
        QTimer::singleShot(0, this, &KateIconBorder::delayedUpdateOfSizeWithRepaint);
    }
}

void KateIconBorder::handleDestroyedAnnotationItemDelegate()
{
    setAnnotationItemDelegate(nullptr);
}

void KateIconBorder::delayedUpdateOfSizeWithRepaint()
{
    // ensure we update size + repaint at once to avoid flicker, see bug 435361
    setUpdatesEnabled(false);
    updateGeometry();
    repaint();
    setUpdatesEnabled(true);
}

void KateIconBorder::initStyleOption(KTextEditor::StyleOptionAnnotationItem *styleOption) const
{
    styleOption->initFrom(this);
    styleOption->view = m_view;
    styleOption->decorationSize = QSize(m_iconAreaWidth, m_iconAreaWidth);
    styleOption->contentFontMetrics = m_view->renderer()->currentFontMetrics();
}

void KateIconBorder::setStyleOptionLineData(KTextEditor::StyleOptionAnnotationItem *styleOption,
                                            int y,
                                            int realLine,
                                            const KTextEditor::AnnotationModel *model,
                                            const QString &annotationGroupIdentifier) const
{
    // calculate rendered displayed line
    const uint h = m_view->renderer()->lineHeight();
    const uint z = (y / h);

    KateAnnotationGroupPositionState annotationGroupPositionState(m_viewInternal, model, annotationGroupIdentifier, z, true);
    annotationGroupPositionState.nextLine(*styleOption, z, realLine);
}

QRect KateIconBorder::annotationLineRectInView(int line) const
{
    int x = 0;
    if (m_iconBorderOn) {
        x += m_iconAreaWidth + 2;
    }
    const int y = m_view->m_viewInternal->lineToY(line);

    return QRect(x, y, m_annotationAreaWidth, m_view->renderer()->lineHeight());
}

void KateIconBorder::updateAnnotationLine(int line)
{
    // TODO: why has the default value been 8, where is that magic number from?
    int width = 8;
    KTextEditor::AnnotationModel *model = m_view->annotationModel() ? m_view->annotationModel() : m_doc->annotationModel();

    if (model) {
        KTextEditor::StyleOptionAnnotationItem styleOption;
        initStyleOption(&styleOption);
        width = m_annotationItemDelegate->sizeHint(styleOption, model, line).width();
    }

    if (width > m_annotationAreaWidth) {
        m_annotationAreaWidth = width;
        m_updatePositionToArea = true;

        QTimer::singleShot(0, this, SLOT(update()));
    }
}

void KateIconBorder::showAnnotationMenu(int line, const QPoint &pos)
{
    QMenu menu;
    QAction a(i18n("Disable Annotation Bar"), &menu);
    a.setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    menu.addAction(&a);
    Q_EMIT m_view->annotationContextMenuAboutToShow(m_view, &menu, line);
    if (menu.exec(pos) == &a) {
        m_view->setAnnotationBorderVisible(false);
    }
}

void KateIconBorder::hideAnnotationTooltip()
{
    m_annotationItemDelegate->hideTooltip(m_view);
}

void KateIconBorder::updateAnnotationBorderWidth()
{
    calcAnnotationBorderWidth();

    m_updatePositionToArea = true;

    QTimer::singleShot(0, this, SLOT(update()));
}

void KateIconBorder::calcAnnotationBorderWidth()
{
    // TODO: another magic number, not matching the one in updateAnnotationLine()
    m_annotationAreaWidth = 6;
    KTextEditor::AnnotationModel *model = m_view->annotationModel() ? m_view->annotationModel() : m_doc->annotationModel();

    if (model) {
        KTextEditor::StyleOptionAnnotationItem styleOption;
        initStyleOption(&styleOption);

        const int lineCount = m_view->doc()->lines();
        if (lineCount > 0) {
            const int checkedLineCount = m_hasUniformAnnotationItemSizes ? 1 : lineCount;
            for (int i = 0; i < checkedLineCount; ++i) {
                const int curwidth = m_annotationItemDelegate->sizeHint(styleOption, model, i).width();
                if (curwidth > m_annotationAreaWidth) {
                    m_annotationAreaWidth = curwidth;
                }
            }
        }
    }
}

void KateIconBorder::annotationModelChanged(KTextEditor::AnnotationModel *oldmodel, KTextEditor::AnnotationModel *newmodel)
{
    if (oldmodel) {
        oldmodel->disconnect(this);
    }
    if (newmodel) {
        connect(newmodel, &KTextEditor::AnnotationModel::reset, this, &KateIconBorder::updateAnnotationBorderWidth);
        connect(newmodel, &KTextEditor::AnnotationModel::lineChanged, this, &KateIconBorder::updateAnnotationLine);
    }
    updateAnnotationBorderWidth();
}

void KateIconBorder::displayRangeChanged()
{
    hideFolding();
    removeAnnotationHovering();
}

// END KateIconBorder

// BEGIN KateViewEncodingAction
// According to https://www.iana.org/assignments/ianacharset-mib/ianacharset-mib
// the default/unknown mib value is 2.
#define MIB_DEFAULT 2

bool lessThanAction(KSelectAction *a, KSelectAction *b)
{
    return a->text() < b->text();
}

void KateViewEncodingAction::init()
{
    QList<KSelectAction *> actions;

    setToolBarMode(MenuMode);

    int i;
    const auto encodingsByScript = KCharsets::charsets()->encodingsByScript();
    actions.reserve(encodingsByScript.size());
    for (const QStringList &encodingsForScript : encodingsByScript) {
        KSelectAction *tmp = new KSelectAction(encodingsForScript.at(0), this);

        for (i = 1; i < encodingsForScript.size(); ++i) {
            tmp->addAction(encodingsForScript.at(i));
        }
        connect(tmp, &KSelectAction::actionTriggered, this, [this](QAction *action) {
            subActionTriggered(action);
        });
        // tmp->setCheckable(true);
        actions << tmp;
    }
    std::sort(actions.begin(), actions.end(), lessThanAction);
    for (KSelectAction *action : std::as_const(actions)) {
        addAction(action);
    }
}

void KateViewEncodingAction::subActionTriggered(QAction *action)
{
    if (currentSubAction == action) {
        return;
    }
    currentSubAction = action;
    Q_EMIT textTriggered(action->text());
}

KateViewEncodingAction::KateViewEncodingAction(KTextEditor::DocumentPrivate *_doc,
                                               KTextEditor::ViewPrivate *_view,
                                               const QString &text,
                                               QObject *parent,
                                               bool saveAsMode)
    : KSelectAction(text, parent)
    , doc(_doc)
    , view(_view)
    , m_saveAsMode(saveAsMode)
{
    init();

    connect(menu(), &QMenu::aboutToShow, this, &KateViewEncodingAction::slotAboutToShow);
    connect(this, &KSelectAction::textTriggered, this, &KateViewEncodingAction::setEncoding);
}

void KateViewEncodingAction::slotAboutToShow()
{
    setCurrentCodec(doc->config()->encoding());
}

void KateViewEncodingAction::setEncoding(const QString &e)
{
    // in save as mode => trigger saveAs
    if (m_saveAsMode) {
        doc->documentSaveAsWithEncoding(e);
        return;
    }

    // else switch encoding
    doc->userSetEncodingForNextReload();
    doc->setEncoding(e);
    view->reloadFile();
}

bool KateViewEncodingAction::setCurrentCodec(const QString &codec)
{
    disconnect(this, &KSelectAction::textTriggered, this, &KateViewEncodingAction::setEncoding);

    int i;
    int j;
    for (i = 0; i < actions().size(); ++i) {
        if (actions().at(i)->menu()) {
            for (j = 0; j < actions().at(i)->menu()->actions().size(); ++j) {
                if (!j && !actions().at(i)->menu()->actions().at(j)->data().isNull()) {
                    continue;
                }
                if (actions().at(i)->menu()->actions().at(j)->isSeparator()) {
                    continue;
                }

                if (codec == actions().at(i)->menu()->actions().at(j)->text()) {
                    currentSubAction = actions().at(i)->menu()->actions().at(j);
                    currentSubAction->setChecked(true);
                } else {
                    actions().at(i)->menu()->actions().at(j)->setChecked(false);
                }
            }
        }
    }

    connect(this, &KSelectAction::textTriggered, this, &KateViewEncodingAction::setEncoding);
    return true;
}

// END KateViewEncodingAction

// BEGIN KateViewBar related classes

KateViewBarWidget::KateViewBarWidget(bool addCloseButton, QWidget *parent)
    : QWidget(parent)
    , m_viewBar(nullptr)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    // NOTE: Here be cosmetics.
    layout->setContentsMargins(0, 0, 0, 0);

    // widget to be used as parent for the real content
    m_centralWidget = new QWidget(this);
    layout->addWidget(m_centralWidget);
    setFocusProxy(m_centralWidget);

    // hide button
    if (addCloseButton) {
        m_closeButton = new QToolButton(this);
        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
        connect(m_closeButton, &QToolButton::clicked, this, &KateViewBarWidget::hideMe);
        layout->addWidget(m_closeButton);
        layout->setAlignment(m_closeButton, Qt::AlignCenter | Qt::AlignVCenter);
    }
}

KateViewBar::KateViewBar(bool external, QWidget *parent, KTextEditor::ViewPrivate *view)
    : QWidget(parent)
    , m_external(external)
    , m_view(view)
    , m_permanentBarWidget(nullptr)

{
    m_layout = new QVBoxLayout(this);
    m_stack = new QStackedWidget(this);
    m_layout->addWidget(m_stack);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_stack->hide();
    hide();
}

void KateViewBar::addBarWidget(KateViewBarWidget *newBarWidget)
{
    // just ignore additional adds for already existing widgets
    if (hasBarWidget(newBarWidget)) {
        return;
    }

    // add new widget, invisible...
    newBarWidget->hide();
    m_stack->addWidget(newBarWidget);
    newBarWidget->setAssociatedViewBar(this);
    connect(newBarWidget, &KateViewBarWidget::hideMe, this, &KateViewBar::hideCurrentBarWidget);
}

void KateViewBar::removeBarWidget(KateViewBarWidget *barWidget)
{
    // remove only if there
    if (!hasBarWidget(barWidget)) {
        return;
    }

    m_stack->removeWidget(barWidget);
    barWidget->setAssociatedViewBar(nullptr);
    barWidget->hide();
    disconnect(barWidget, nullptr, this, nullptr);
}

void KateViewBar::addPermanentBarWidget(KateViewBarWidget *barWidget)
{
    Q_ASSERT(barWidget);
    Q_ASSERT(!m_permanentBarWidget);

    m_layout->addWidget(barWidget);
    m_permanentBarWidget = barWidget;
    m_permanentBarWidget->show();

    setViewBarVisible(true);
}

void KateViewBar::removePermanentBarWidget(KateViewBarWidget *barWidget)
{
    Q_ASSERT(m_permanentBarWidget == barWidget);
    Q_UNUSED(barWidget);

    m_permanentBarWidget->hide();
    m_layout->removeWidget(m_permanentBarWidget);
    m_permanentBarWidget = nullptr;

    if (!barWidgetVisible()) {
        setViewBarVisible(false);
    }
}

void KateViewBar::showBarWidget(KateViewBarWidget *barWidget)
{
    Q_ASSERT(barWidget != nullptr);

    if (barWidget != qobject_cast<KateViewBarWidget *>(m_stack->currentWidget())) {
        hideCurrentBarWidget();
    }

    // raise correct widget
    m_stack->addWidget(barWidget);
    m_stack->setCurrentWidget(barWidget);
    barWidget->show();
    barWidget->setFocus(Qt::ShortcutFocusReason);
    m_stack->show();
    setViewBarVisible(true);
}

bool KateViewBar::hasBarWidget(KateViewBarWidget *barWidget) const
{
    return m_stack->indexOf(barWidget) != -1;
}

void KateViewBar::hideCurrentBarWidget()
{
    KateViewBarWidget *current = qobject_cast<KateViewBarWidget *>(m_stack->currentWidget());
    if (current) {
        m_stack->removeWidget(current);
        current->closed();
    }

    // hide the bar
    m_stack->hide();
    if (!m_permanentBarWidget) {
        setViewBarVisible(false);
    }

    m_view->setFocus();
}

void KateViewBar::setViewBarVisible(bool visible)
{
    if (m_external) {
        if (visible) {
            m_view->mainWindow()->showViewBar(m_view);
        } else {
            m_view->mainWindow()->hideViewBar(m_view);
        }
    } else {
        setVisible(visible);
    }
}

bool KateViewBar::barWidgetVisible() const
{
    KateViewBarWidget *current = qobject_cast<KateViewBarWidget *>(m_stack->currentWidget());
    return current && current->isVisible();
}

void KateViewBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideCurrentBarWidget();
        return;
    }
    QWidget::keyPressEvent(event);
}

void KateViewBar::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    //   if (!event->spontaneous())
    //     m_view->setFocus();
}

// END KateViewBar related classes

// BEGIN SCHEMA ACTION -- the 'View->Color theme' menu action
void KateViewSchemaAction::init()
{
    m_group = nullptr;
    m_view = nullptr;
    last = 0;

    connect(menu(), &QMenu::aboutToShow, this, &KateViewSchemaAction::slotAboutToShow);
}

void KateViewSchemaAction::updateMenu(KTextEditor::ViewPrivate *view)
{
    m_view = view;
}

void KateViewSchemaAction::slotAboutToShow()
{
    KTextEditor::ViewPrivate *view = m_view;

    const auto themes = KateHlManager::self()->sortedThemes();

    if (!m_group) {
        m_group = new QActionGroup(menu());
        m_group->setExclusive(true);
    }

    for (int z = 0; z < themes.count(); z++) {
        QString hlName = themes[z].translatedName();

        if (!names.contains(hlName)) {
            names << hlName;
            QAction *a = menu()->addAction(hlName, this, &KateViewSchemaAction::setSchema);
            a->setData(themes[z].name());
            a->setCheckable(true);
            a->setActionGroup(m_group);
        }
    }

    if (!view) {
        return;
    }

    QString id = view->rendererConfig()->schema();
    const auto menuActions = menu()->actions();
    for (QAction *a : menuActions) {
        a->setChecked(a->data().toString() == id);
    }
}

void KateViewSchemaAction::setSchema()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (!action) {
        return;
    }
    QString mode = action->data().toString();

    KTextEditor::ViewPrivate *view = m_view;

    if (view) {
        view->rendererConfig()->setSchema(mode);
    }
}
// END SCHEMA ACTION

#include "moc_kateviewhelpers.cpp"
