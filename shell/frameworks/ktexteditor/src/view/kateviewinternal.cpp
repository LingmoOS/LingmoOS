/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2003 Anakim Border <aborder@sources.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>

    Based on KWriteView:
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "kateviewinternal.h"

#include "kateabstractinputmode.h"
#include "kateabstractinputmodefactory.h"
#include "katebuffer.h"
#include "katecompletionwidget.h"
#include "kateconfig.h"
#include "kateglobal.h"
#include "katehighlight.h"
#include "katelayoutcache.h"
#include "katemessagewidget.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "katetextanimation.h"
#include "katetextpreview.h"
#include "kateview.h"
#include "kateviewaccessible.h"
#include "kateviewhelpers.h"
#include "spellcheck/spellingmenu.h"

#include <KCursor>
#include <ktexteditor/documentcursor.h>
#include <ktexteditor/inlinenoteprovider.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/texthintinterface.h>

#include <QAccessible>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QLayout>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QScopeGuard>
#include <QScroller>
#include <QStyle>
#include <QToolTip>

static const bool debugPainting = false;

class ZoomEventFilter
{
public:
    ZoomEventFilter() = default;

    bool detectZoomingEvent(QWheelEvent *e, Qt::KeyboardModifiers modifier = Qt::ControlModifier)
    {
        Qt::KeyboardModifiers modState = e->modifiers();
        if (modState == modifier) {
            if (m_lastWheelEvent.isValid()) {
                const qint64 deltaT = m_lastWheelEvent.elapsed();
                // Pressing the specified modifier key within 200ms of the previous "unmodified"
                // wheelevent is not allowed to toggle on text zooming
                if (m_lastWheelEventUnmodified && deltaT < 200) {
                    m_ignoreZoom = true;
                } else if (deltaT > 1000) {
                    // the protection is kept active for 1s after the last wheel event
                    // TODO: this value should be tuned, preferably by someone using
                    // Ctrl+Wheel zooming frequently.
                    m_ignoreZoom = false;
                }
            } else {
                // we can't say anything and have to assume there's nothing
                // accidental to the modifier being pressed.
                m_ignoreZoom = false;
            }
            m_lastWheelEventUnmodified = false;
            if (m_ignoreZoom) {
                // unset the modifier so the view scrollbars can handle the scroll
                // event and produce normal, not accelerated scrolling
                modState &= ~modifier;
                e->setModifiers(modState);
            }
        } else {
            // state is reset after any wheel event without the zoom modifier
            m_lastWheelEventUnmodified = true;
            m_ignoreZoom = false;
        }
        m_lastWheelEvent.start();

        // inform the caller whether this event is allowed to trigger text zooming.
        return !m_ignoreZoom && modState == modifier;
    }

protected:
    QElapsedTimer m_lastWheelEvent;
    bool m_ignoreZoom = false;
    bool m_lastWheelEventUnmodified = false;
};

KateViewInternal::KateViewInternal(KTextEditor::ViewPrivate *view)
    : QWidget(view)
    , editSessionNumber(0)
    , editIsRunning(false)
    , m_view(view)
    , m_cursor(doc()->buffer(), KTextEditor::Cursor(0, 0), Kate::TextCursor::MoveOnInsert)
    , m_mouse()
    , m_possibleTripleClick(false)
    , m_bm(doc()->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::DoNotExpand))
    , m_bmStart(doc()->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::DoNotExpand))
    , m_bmEnd(doc()->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::DoNotExpand))
    , m_bmLastFlashPos(doc()->newMovingCursor(KTextEditor::Cursor::invalid()))

    // folding marker
    , m_fmStart(doc()->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::DoNotExpand))
    , m_fmEnd(doc()->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::DoNotExpand))

    , m_dummy(nullptr)

    // stay on cursor will avoid that the view scroll around on press return at beginning
    , m_startPos(doc()->buffer(), KTextEditor::Cursor(0, 0), Kate::TextCursor::StayOnInsert)

    , m_visibleLineCount(0)
    , m_madeVisible(false)
    , m_shiftKeyPressed(false)
    , m_autoCenterLines(0)
    , m_minLinesVisible(0)
    , m_selChangedByUser(false)
    , m_selectAnchor(-1, -1)
    , m_selectionMode(Default)
    , m_layoutCache(new KateLayoutCache(renderer(), this))
    , m_preserveX(false)
    , m_preservedX(0)
    , m_cachedMaxStartPos(-1, -1)
    , m_dragScrollTimer(this)
    , m_scrollTimer(this)
    , m_cursorTimer(this)
    , m_textHintTimer(this)
    , m_textHintDelay(500)
    , m_textHintPos(-1, -1)
    , m_imPreeditRange(nullptr)
{
    // setup input modes
    Q_ASSERT(m_inputModes.size() == KTextEditor::EditorPrivate::self()->inputModeFactories().size());
    m_inputModes[KTextEditor::View::NormalInputMode].reset(
        KTextEditor::EditorPrivate::self()->inputModeFactories()[KTextEditor::View::NormalInputMode]->createInputMode(this));
    m_inputModes[KTextEditor::View::ViInputMode].reset(
        KTextEditor::EditorPrivate::self()->inputModeFactories()[KTextEditor::View::ViInputMode]->createInputMode(this));
    m_currentInputMode = m_inputModes[KTextEditor::View::NormalInputMode].get();

    setMinimumSize(0, 0);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_InputMethodEnabled);

    // invalidate m_selectionCached.start(), or keyb selection is screwed initially
    m_selectionCached = KTextEditor::Range::invalid();

    // bracket markers are only for this view and should not be printed
    m_bm->setView(m_view);
    m_bmStart->setView(m_view);
    m_bmEnd->setView(m_view);
    m_bm->setAttributeOnlyForViews(true);
    m_bmStart->setAttributeOnlyForViews(true);
    m_bmEnd->setAttributeOnlyForViews(true);

    // use z depth defined in moving ranges interface
    m_bm->setZDepth(-1000.0);
    m_bmStart->setZDepth(-1000.0);
    m_bmEnd->setZDepth(-1000.0);

    // update mark attributes
    updateBracketMarkAttributes();

    //
    // scrollbar for lines
    //
    m_lineScroll = new KateScrollBar(Qt::Vertical, this);
    m_lineScroll->show();
    m_lineScroll->setTracking(true);
    m_lineScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // Hijack the line scroller's controls, so we can scroll nicely for word-wrap
    connect(m_lineScroll, &KateScrollBar::actionTriggered, this, &KateViewInternal::scrollAction);

    auto viewScrollLinesSlot = qOverload<int>(&KateViewInternal::scrollLines);
    connect(m_lineScroll, &KateScrollBar::sliderMoved, this, viewScrollLinesSlot);
    connect(m_lineScroll, &KateScrollBar::sliderMMBMoved, this, viewScrollLinesSlot);
    connect(m_lineScroll, &KateScrollBar::valueChanged, this, viewScrollLinesSlot);

    //
    // scrollbar for columns
    //
    m_columnScroll = new QScrollBar(Qt::Horizontal, m_view);
    m_scroller = QScroller::scroller(this);
    QScrollerProperties prop;
    prop.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.3);
    prop.setScrollMetric(QScrollerProperties::MaximumVelocity, 1);
    prop.setScrollMetric(QScrollerProperties::AcceleratingFlickMaximumTime,
                         0.2); // Workaround for QTBUG-88249 (non-flick gestures recognized as accelerating flick)
    prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0);
    m_scroller->setScrollerProperties(prop);
#ifndef Q_OS_MACOS
    // On macOS the trackpad also emits touch events which are sometimes picked up by the flick
    // gesture recogniser registered by QScroller; this results in some odd scrolling behaviour
    // as described in bug #442060.  Therefore it's better to not let the QScroller register
    // it on that platform.
    m_scroller->grabGesture(this);
#endif

    if (m_view->dynWordWrap()) {
        m_columnScroll->hide();
    } else {
        m_columnScroll->show();
    }

    m_columnScroll->setTracking(true);
    m_startX = 0;

    connect(m_columnScroll, &QScrollBar::valueChanged, this, &KateViewInternal::scrollColumns);

    // bottom corner box
    m_dummy = new QWidget(m_view);
    m_dummy->setFixedSize(m_lineScroll->width(), m_columnScroll->sizeHint().height());
    m_dummy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (m_view->dynWordWrap()) {
        m_dummy->hide();
    } else {
        m_dummy->show();
    }

    cache()->setWrap(m_view->dynWordWrap());

    //
    // iconborder ;)
    //
    m_leftBorder = new KateIconBorder(this, m_view);
    m_leftBorder->show();

    // update view if folding ranges change
    connect(&m_view->textFolding(), &Kate::TextFolding::foldingRangesChanged, this, &KateViewInternal::slotRegionVisibilityChanged);

    m_displayCursor.setPosition(0, 0);

    setAcceptDrops(true);

    m_zoomEventFilter.reset(new ZoomEventFilter());
    // event filter
    installEventFilter(this);

    // set initial cursor
    m_mouseCursor = Qt::IBeamCursor;
    setCursor(m_mouseCursor);

    // call mouseMoveEvent also if no mouse button is pressed
    setMouseTracking(true);

    m_dragInfo.state = diNone;

    // timers
    connect(&m_dragScrollTimer, &QTimer::timeout, this, &KateViewInternal::doDragScroll);

    connect(&m_scrollTimer, &QTimer::timeout, this, &KateViewInternal::scrollTimeout);

    connect(&m_cursorTimer, &QTimer::timeout, this, &KateViewInternal::cursorTimeout);

    connect(&m_textHintTimer, &QTimer::timeout, this, &KateViewInternal::textHintTimeout);

    // selection changed to set anchor
    connect(m_view, &KTextEditor::ViewPrivate::selectionChanged, this, &KateViewInternal::viewSelectionChanged);

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::installFactory(accessibleInterfaceFactory);
#endif
    connect(doc(), &KTextEditor::DocumentPrivate::textInsertedRange, this, &KateViewInternal::documentTextInserted);
    connect(doc(), &KTextEditor::DocumentPrivate::textRemoved, this, &KateViewInternal::documentTextRemoved);

    // update is called in KTextEditor::ViewPrivate, after construction and layout is over
    // but before any other kateviewinternal call
}

KateViewInternal::~KateViewInternal()
{
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::removeFactory(accessibleInterfaceFactory);
#endif

    // delete text animation object here, otherwise it updates the view in its destructor
    delete m_textAnimation;

    // delete border here to not trigger crash later due to access to stuff in painting
    delete m_leftBorder;
    m_leftBorder = nullptr;
}

void KateViewInternal::dynWrapChanged()
{
    m_dummy->setFixedSize(m_lineScroll->width(), m_columnScroll->sizeHint().height());
    if (view()->dynWordWrap()) {
        m_columnScroll->hide();
        m_dummy->hide();

    } else {
        // column scrollbar + bottom corner box
        m_columnScroll->show();
        m_dummy->show();
    }

    cache()->setWrap(view()->dynWordWrap());
    updateView();

    if (view()->dynWordWrap()) {
        scrollColumns(0);
    }

    update();
}

KTextEditor::Cursor KateViewInternal::endPos() const
{
    // Hrm, no lines laid out at all??
    if (!cache()->viewCacheLineCount()) {
        return KTextEditor::Cursor();
    }

    for (int i = qMin(linesDisplayed() - 1, cache()->viewCacheLineCount() - 1); i >= 0; i--) {
        const KateTextLayout &thisLine = cache()->viewLine(i);

        if (thisLine.line() == -1) {
            continue;
        }

        if (thisLine.virtualLine() >= view()->textFolding().visibleLines()) {
            // Cache is too out of date
            return KTextEditor::Cursor(view()->textFolding().visibleLines() - 1,
                                       doc()->lineLength(view()->textFolding().visibleLineToLine(view()->textFolding().visibleLines() - 1)));
        }

        return KTextEditor::Cursor(thisLine.virtualLine(), thisLine.wrap() ? thisLine.endCol() - 1 : thisLine.endCol());
    }

    // can happen, if view is still invisible
    return KTextEditor::Cursor();
}

int KateViewInternal::endLine() const
{
    return endPos().line();
}

KateTextLayout KateViewInternal::yToKateTextLayout(int y) const
{
    if (y < 0 || y > size().height()) {
        return KateTextLayout::invalid();
    }

    int range = y / renderer()->lineHeight();

    // lineRanges is always bigger than 0, after the initial updateView call
    if (range >= 0 && range < cache()->viewCacheLineCount()) {
        return cache()->viewLine(range);
    }

    return KateTextLayout::invalid();
}

int KateViewInternal::lineToY(int viewLine) const
{
    return (viewLine - startLine()) * renderer()->lineHeight();
}

void KateViewInternal::slotIncFontSizes(qreal step)
{
    renderer()->increaseFontSizes(step);
}

void KateViewInternal::slotDecFontSizes(qreal step)
{
    renderer()->decreaseFontSizes(step);
}

void KateViewInternal::slotResetFontSizes()
{
    renderer()->resetFontSizes();
}

/**
 * Line is the real line number to scroll to.
 */
void KateViewInternal::scrollLines(int line)
{
    KTextEditor::Cursor newPos(line, 0);
    scrollPos(newPos);
}

// This can scroll less than one true line
void KateViewInternal::scrollViewLines(int offset)
{
    KTextEditor::Cursor c = viewLineOffset(startPos(), offset);
    scrollPos(c);

    bool blocked = m_lineScroll->blockSignals(true);
    m_lineScroll->setValue(startLine());
    m_lineScroll->blockSignals(blocked);
}

void KateViewInternal::scrollAction(int action)
{
    switch (action) {
    case QAbstractSlider::SliderSingleStepAdd:
        scrollNextLine();
        break;

    case QAbstractSlider::SliderSingleStepSub:
        scrollPrevLine();
        break;

    case QAbstractSlider::SliderPageStepAdd:
        scrollNextPage();
        break;

    case QAbstractSlider::SliderPageStepSub:
        scrollPrevPage();
        break;

    case QAbstractSlider::SliderToMinimum:
        top_home();
        break;

    case QAbstractSlider::SliderToMaximum:
        bottom_end();
        break;
    }
}

void KateViewInternal::scrollNextPage()
{
    scrollViewLines(qMax(linesDisplayed() - 1, 0));
}

void KateViewInternal::scrollPrevPage()
{
    scrollViewLines(-qMax(linesDisplayed() - 1, 0));
}

void KateViewInternal::scrollPrevLine()
{
    scrollViewLines(-1);
}

void KateViewInternal::scrollNextLine()
{
    scrollViewLines(1);
}

KTextEditor::Cursor KateViewInternal::maxStartPos(bool changed)
{
    cache()->setAcceptDirtyLayouts(true);

    if (m_cachedMaxStartPos.line() == -1 || changed) {
        KTextEditor::Cursor end(view()->textFolding().visibleLines() - 1,
                                doc()->lineLength(view()->textFolding().visibleLineToLine(view()->textFolding().visibleLines() - 1)));

        if (view()->config()->scrollPastEnd()) {
            m_cachedMaxStartPos = viewLineOffset(end, -m_minLinesVisible);
        } else {
            m_cachedMaxStartPos = viewLineOffset(end, -(linesDisplayed() - 1));
        }
    }

    cache()->setAcceptDirtyLayouts(false);

    return m_cachedMaxStartPos;
}

// c is a virtual cursor
void KateViewInternal::scrollPos(KTextEditor::Cursor &c, bool force, bool calledExternally, bool emitSignals)
{
    if (!force && ((!view()->dynWordWrap() && c.line() == startLine()) || c == startPos())) {
        return;
    }

    if (c.line() < 0) {
        c.setLine(0);
    }

    KTextEditor::Cursor limit = maxStartPos();
    if (c > limit) {
        c = limit;

        // Re-check we're not just scrolling to the same place
        if (!force && ((!view()->dynWordWrap() && c.line() == startLine()) || c == startPos())) {
            return;
        }
    }

    int viewLinesScrolled = 0;

    // only calculate if this is really used and useful, could be wrong here, please recheck
    // for larger scrolls this makes 2-4 seconds difference on my xeon with dyn. word wrap on
    // try to get it really working ;)
    bool viewLinesScrolledUsable = !force && (c.line() >= startLine() - linesDisplayed() - 1) && (c.line() <= endLine() + linesDisplayed() + 1);

    if (viewLinesScrolledUsable) {
        viewLinesScrolled = cache()->displayViewLine(c);
    }

    m_startPos.setPosition(c);

    // set false here but reversed if we return to makeVisible
    m_madeVisible = false;

    if (viewLinesScrolledUsable) {
        int lines = linesDisplayed();
        if (view()->textFolding().visibleLines() < lines) {
            KTextEditor::Cursor end(view()->textFolding().visibleLines() - 1,
                                    doc()->lineLength(view()->textFolding().visibleLineToLine(view()->textFolding().visibleLines() - 1)));
            lines = qMin(linesDisplayed(), cache()->displayViewLine(end) + 1);
        }

        Q_ASSERT(lines >= 0);

        if (!calledExternally && qAbs(viewLinesScrolled) < lines &&
            // NOTE: on some machines we must update if the floating widget is visible
            //       otherwise strange painting bugs may occur during scrolling...
            !((view()->m_messageWidgets[KTextEditor::Message::TopInView] && view()->m_messageWidgets[KTextEditor::Message::TopInView]->isVisible())
              || (view()->m_messageWidgets[KTextEditor::Message::CenterInView] && view()->m_messageWidgets[KTextEditor::Message::CenterInView]->isVisible())
              || (view()->m_messageWidgets[KTextEditor::Message::BottomInView] && view()->m_messageWidgets[KTextEditor::Message::BottomInView]->isVisible()))) {
            updateView(false, viewLinesScrolled);

            int scrollHeight = -(viewLinesScrolled * (int)renderer()->lineHeight());

            // scroll excluding child widgets (floating notifications)
            scroll(0, scrollHeight, rect());
            m_leftBorder->scroll(0, scrollHeight);

            if (emitSignals) {
                Q_EMIT view()->verticalScrollPositionChanged(m_view, c);
                Q_EMIT view()->displayRangeChanged(m_view);
            }
            return;
        }
    }

    updateView();
    update();
    m_leftBorder->update();
    if (emitSignals) {
        Q_EMIT view()->verticalScrollPositionChanged(m_view, c);
        Q_EMIT view()->displayRangeChanged(m_view);
    }
}

void KateViewInternal::scrollColumns(int x)
{
    if (x < 0) {
        x = 0;
    }

    if (x > m_columnScroll->maximum()) {
        x = m_columnScroll->maximum();
    }

    if (x == startX()) {
        return;
    }

    int dx = startX() - x;
    m_startX = x;

    if (qAbs(dx) < width()) {
        // scroll excluding child widgets (floating notifications)
        scroll(dx, 0, rect());
    } else {
        update();
    }

    Q_EMIT view()->horizontalScrollPositionChanged(m_view);
    Q_EMIT view()->displayRangeChanged(m_view);

    bool blocked = m_columnScroll->blockSignals(true);
    m_columnScroll->setValue(startX());
    m_columnScroll->blockSignals(blocked);
}

// If changed is true, the lines that have been set dirty have been updated.
void KateViewInternal::updateView(bool changed, int viewLinesScrolled)
{
    if (!isVisible() && !viewLinesScrolled && !changed) {
        return; // When this view is not visible, don't do anything
    }

    view()->doc()->delayAutoReload(); // Don't reload while user scrolls around
    bool blocked = m_lineScroll->blockSignals(true);

    int wrapWidth = width();
    if (view()->config()->dynWrapAtStaticMarker() && view()->config()->dynWordWrap()) {
        // We need to transform char count to a pixel width, stolen from PrintPainter::updateCache()
        QString s;
        s.fill(QLatin1Char('5'), view()->doc()->config()->wordWrapAt());
        wrapWidth = qMin(width(), static_cast<int>(renderer()->currentFontMetrics().boundingRect(s).width()));
    }

    if (wrapWidth != cache()->viewWidth()) {
        cache()->setViewWidth(wrapWidth);
        changed = true;
    }

    /* It was observed that height() could be negative here --
       when the main Kate view has 0 as size (during creation),
       and there frame around KateViewInternal.  In which
       case we'd set the view cache to 0 (or less!) lines, and
       start allocating huge chunks of data, later. */
    int newSize = (qMax(0, height()) / renderer()->lineHeight()) + 1;
    cache()->updateViewCache(startPos(), newSize, viewLinesScrolled);
    m_visibleLineCount = newSize;

    KTextEditor::Cursor maxStart = maxStartPos(changed);
    int maxLineScrollRange = maxStart.line();
    if (view()->dynWordWrap() && maxStart.column() != 0) {
        maxLineScrollRange++;
    }
    m_lineScroll->setRange(0, maxLineScrollRange);

    m_lineScroll->setValue(startLine());
    m_lineScroll->setSingleStep(1);
    m_lineScroll->setPageStep(qMax(0, height()) / renderer()->lineHeight());
    m_lineScroll->blockSignals(blocked);

    KateViewConfig::ScrollbarMode show_scrollbars = static_cast<KateViewConfig::ScrollbarMode>(view()->config()->showScrollbars());

    bool visible = ((show_scrollbars == KateViewConfig::AlwaysOn) || ((show_scrollbars == KateViewConfig::ShowWhenNeeded) && (maxLineScrollRange != 0)));
    bool visible_dummy = visible;

    m_lineScroll->setVisible(visible);

    if (!view()->dynWordWrap()) {
        int max = maxLen(startLine()) - width();
        if (max < 0) {
            max = 0;
        }

        // if we lose the ability to scroll horizontally, move view to the far-left
        if (max == 0) {
            scrollColumns(0);
        }

        blocked = m_columnScroll->blockSignals(true);

        // disable scrollbar
        m_columnScroll->setDisabled(max == 0);

        visible = ((show_scrollbars == KateViewConfig::AlwaysOn) || ((show_scrollbars == KateViewConfig::ShowWhenNeeded) && (max != 0)));
        visible_dummy &= visible;
        m_columnScroll->setVisible(visible);

        m_columnScroll->setRange(0, max + (renderer()->spaceWidth() / 2)); // Add some space for the caret at EOL

        m_columnScroll->setValue(startX());

        // Approximate linescroll
        m_columnScroll->setSingleStep(renderer()->currentFontMetrics().horizontalAdvance(QLatin1Char('a')));
        m_columnScroll->setPageStep(width());

        m_columnScroll->blockSignals(blocked);
    } else {
        visible_dummy = false;
    }

    m_dummy->setVisible(visible_dummy);

    if (changed) {
        updateDirty();
    }
}

/**
 * this function ensures a certain location is visible on the screen.
 * if endCol is -1, ignore making the columns visible.
 */
void KateViewInternal::makeVisible(const KTextEditor::Cursor c, int endCol, bool force, bool center, bool calledExternally)
{
    // qCDebug(LOG_KTE) << "MakeVisible start " << startPos() << " end " << endPos() << " -> request: " << c;// , new start [" << scroll.line << "," <<
    // scroll.col << "] lines " << (linesDisplayed() - 1) << " height " << height(); if the line is in a folded region, unfold all the way up if (
    // doc()->foldingTree()->findNodeForLine( c.line )->visible )
    //  qCDebug(LOG_KTE)<<"line ("<<c.line<<") should be visible";

    const int lnDisp = linesDisplayed();
    const int viewLine = cache()->displayViewLine(c, true);
    const bool curBelowScreen = (viewLine == -2);

    if (force) {
        KTextEditor::Cursor scroll = c;
        scrollPos(scroll, force, calledExternally);
    } else if (center && (c < startPos() || c > endPos())) {
        KTextEditor::Cursor scroll = viewLineOffset(c, -int(lnDisp) / 2);
        scrollPos(scroll, false, calledExternally);
    } else if ((viewLine >= (lnDisp - m_minLinesVisible)) || (curBelowScreen)) {
        KTextEditor::Cursor scroll = viewLineOffset(c, -(lnDisp - m_minLinesVisible - 1));
        scrollPos(scroll, false, calledExternally);
    } else if (c < viewLineOffset(startPos(), m_minLinesVisible)) {
        KTextEditor::Cursor scroll = viewLineOffset(c, -m_minLinesVisible);
        scrollPos(scroll, false, calledExternally);
    } else {
        // Check to see that we're not showing blank lines
        KTextEditor::Cursor max = maxStartPos();
        if (startPos() > max) {
            scrollPos(max, max.column(), calledExternally);
        }
    }

    if (!view()->dynWordWrap() && (endCol != -1 || view()->wrapCursor())) {
        KTextEditor::Cursor rc = toRealCursor(c);
        int sX = renderer()->cursorToX(cache()->textLayout(rc), rc, !view()->wrapCursor());

        int sXborder = sX - 8;
        if (sXborder < 0) {
            sXborder = 0;
        }

        if (sX < startX()) {
            scrollColumns(sXborder);
        } else if (sX > startX() + width()) {
            scrollColumns(sX - width() + 8);
        }
    }

    m_madeVisible = !force;

#ifndef QT_NO_ACCESSIBILITY
    // FIXME -- is this needed?
//    QAccessible::updateAccessibility(this, KateCursorAccessible::ChildId, QAccessible::Focus);
#endif
}

void KateViewInternal::slotRegionVisibilityChanged()
{
    qCDebug(LOG_KTE);

    // ensure the layout cache is ok for the updateCursor calls below
    // without the updateView() the view will jump to the bottom on hiding blocks after
    // change cfb0af25bdfac0d8f86b42db0b34a6bc9f9a361e
    cache()->clear();
    updateView();

    m_cachedMaxStartPos.setLine(-1);
    KTextEditor::Cursor max = maxStartPos();
    if (startPos() > max) {
        scrollPos(max, false, false, false /* don't emit signals! */);
    }

    // if text was folded: make sure the cursor is on a visible line
    qint64 foldedRangeId = -1;
    if (!view()->textFolding().isLineVisible(m_cursor.line(), &foldedRangeId)) {
        KTextEditor::Range foldingRange = view()->textFolding().foldingRange(foldedRangeId);
        Q_ASSERT(foldingRange.start().isValid());

        // set cursor to start of folding region
        updateCursor(foldingRange.start(), true);
    } else {
        // force an update of the cursor, since otherwise the m_displayCursor
        // line may be below the total amount of visible lines.
        updateCursor(m_cursor, true);
    }

    updateView();
    update();
    m_leftBorder->update();

    // emit signals here, scrollPos has this disabled, to ensure we do this after all stuff is updated!
    Q_EMIT view()->verticalScrollPositionChanged(m_view, max);
    Q_EMIT view()->displayRangeChanged(m_view);
}

void KateViewInternal::slotRegionBeginEndAddedRemoved(unsigned int)
{
    qCDebug(LOG_KTE);
    // FIXME: performance problem
    m_leftBorder->update();
}

void KateViewInternal::showEvent(QShowEvent *e)
{
    updateView();

    QWidget::showEvent(e);
}

KTextEditor::Attribute::Ptr KateViewInternal::attributeAt(const KTextEditor::Cursor position) const
{
    KTextEditor::Attribute::Ptr attrib(new KTextEditor::Attribute());
    Kate::TextLine kateLine = doc()->kateTextLine(position.line());
    *attrib = *m_view->renderer()->attribute(kateLine.attribute(position.column()));
    return attrib;
}

int KateViewInternal::linesDisplayed() const
{
    int h = height();

    // catch zero heights, even if should not happen
    int fh = qMax(1, renderer()->lineHeight());

    // default to 1, there is always one line around....
    // too many places calc with linesDisplayed() - 1
    return qMax(1, (h - (h % fh)) / fh);
}

QPoint KateViewInternal::cursorToCoordinate(const KTextEditor::Cursor cursor, bool realCursor, bool includeBorder) const
{
    if (cursor.line() >= doc()->lines()) {
        return QPoint(-1, -1);
    }

    int viewLine = cache()->displayViewLine(realCursor ? toVirtualCursor(cursor) : cursor, true);

    if (viewLine < 0 || viewLine >= cache()->viewCacheLineCount()) {
        return QPoint(-1, -1);
    }

    const int y = (int)viewLine * renderer()->lineHeight();

    KateTextLayout layout = cache()->viewLine(viewLine);

    const auto textLength = doc()->lineLength(cursor.line());
    if (cursor.column() > textLength) {
        return QPoint(-1, -1);
    }

    int x = 0;

    // only set x value if we have a valid layout (bug #171027)
    if (layout.isValid()) {
        if (!layout.isRightToLeft() || (layout.isRightToLeft() && view()->dynWordWrap())) {
            x = (int)layout.lineLayout().cursorToX(cursor.column());
        } else /* rtl + dynWordWrap == false */ {
            // if text is rtl and dynamic wrap is false, the x offsets are in the opposite
            // direction i.e., [0] == biggest offset, [1] = next
            x = (int)layout.lineLayout().cursorToX(textLength - cursor.column());
        }
    }
    //  else
    //    qCDebug(LOG_KTE) << "Invalid Layout";

    if (includeBorder) {
        x += m_leftBorder->width();
    }

    x -= startX();

    return QPoint(x, y);
}

QPoint KateViewInternal::cursorCoordinates(bool includeBorder) const
{
    return cursorToCoordinate(m_displayCursor, false, includeBorder);
}

KTextEditor::Cursor KateViewInternal::findMatchingBracket()
{
    KTextEditor::Cursor c;

    if (!m_bm->toRange().isValid()) {
        return KTextEditor::Cursor::invalid();
    }

    Q_ASSERT(m_bmEnd->toRange().isValid());
    Q_ASSERT(m_bmStart->toRange().isValid());

    // For e.g. the text "{|}" (where | is the cursor), m_bmStart is equal to [ (0, 0)  ->  (0, 1) ]
    // and the closing bracket is in (0, 1). Thus, we check m_bmEnd first.
    if (m_bmEnd->toRange().contains(m_cursor) || m_bmEnd->end() == m_cursor.toCursor()) {
        c = m_bmStart->start();
    } else if (m_bmStart->toRange().contains(m_cursor) || m_bmStart->end() == m_cursor.toCursor()) {
        c = m_bmEnd->end();
        // We need to adjust the cursor position in case of override mode, BUG-402594
        if (doc()->config()->ovr()) {
            c.setColumn(c.column() - 1);
        }
    } else {
        // should never happen: a range exists, but the cursor position is
        // neither at the start nor at the end...
        return KTextEditor::Cursor::invalid();
    }

    return c;
}

class CalculatingCursor
{
public:
    // These constructors constrain their arguments to valid positions
    // before only the third one did, but that leads to crashes
    // see bug 227449
    CalculatingCursor(KateViewInternal *vi)
        : m_vi(vi)
    {
        makeValid();
    }

    CalculatingCursor(KateViewInternal *vi, const KTextEditor::Cursor c)
        : m_cursor(c)
        , m_vi(vi)
    {
        makeValid();
    }

    CalculatingCursor(KateViewInternal *vi, int line, int col)
        : m_cursor(line, col)
        , m_vi(vi)
    {
        makeValid();
    }

    virtual ~CalculatingCursor()
    {
    }

    int line() const
    {
        return m_cursor.line();
    }

    int column() const
    {
        return m_cursor.column();
    }

    operator KTextEditor::Cursor() const
    {
        return m_cursor;
    }

    virtual CalculatingCursor &operator+=(int n) = 0;

    virtual CalculatingCursor &operator-=(int n) = 0;

    CalculatingCursor &operator++()
    {
        return operator+=(1);
    }

    CalculatingCursor &operator--()
    {
        return operator-=(1);
    }

    void makeValid()
    {
        m_cursor.setLine(qBound(0, line(), int(doc()->lines() - 1)));
        if (view()->wrapCursor()) {
            m_cursor.setColumn(qBound(0, column(), doc()->lineLength(line())));
        } else {
            m_cursor.setColumn(qMax(0, column()));
        }
        Q_ASSERT(valid());
    }

    void toEdge(KateViewInternal::Bias bias)
    {
        if (bias == KateViewInternal::left) {
            m_cursor.setColumn(0);
        } else if (bias == KateViewInternal::right) {
            m_cursor.setColumn(doc()->lineLength(line()));
        }
    }

    bool atEdge() const
    {
        return atEdge(KateViewInternal::left) || atEdge(KateViewInternal::right);
    }

    bool atEdge(KateViewInternal::Bias bias) const
    {
        switch (bias) {
        case KateViewInternal::left:
            return column() == 0;
        case KateViewInternal::none:
            return atEdge();
        case KateViewInternal::right:
            return column() >= doc()->lineLength(line());
        default:
            Q_ASSERT(false);
            return false;
        }
    }

protected:
    bool valid() const
    {
        return line() >= 0 && line() < doc()->lines() && column() >= 0 && (!view()->wrapCursor() || column() <= doc()->lineLength(line()));
    }
    KTextEditor::ViewPrivate *view()
    {
        return m_vi->m_view;
    }
    const KTextEditor::ViewPrivate *view() const
    {
        return m_vi->m_view;
    }
    KTextEditor::DocumentPrivate *doc()
    {
        return view()->doc();
    }
    const KTextEditor::DocumentPrivate *doc() const
    {
        return view()->doc();
    }
    KTextEditor::Cursor m_cursor;
    KateViewInternal *m_vi;
};

class BoundedCursor final : public CalculatingCursor
{
public:
    BoundedCursor(KateViewInternal *vi)
        : CalculatingCursor(vi)
    {
    }
    BoundedCursor(KateViewInternal *vi, const KTextEditor::Cursor c)
        : CalculatingCursor(vi, c)
    {
    }
    BoundedCursor(KateViewInternal *vi, int line, int col)
        : CalculatingCursor(vi, line, col)
    {
    }
    CalculatingCursor &operator+=(int n) override
    {
        KateLineLayout *thisLine = m_vi->cache()->line(line());
        if (!thisLine || !thisLine->isValid()) {
            qCWarning(LOG_KTE) << "Did not retrieve valid layout for line " << line();
            return *this;
        }

        const bool wrapCursor = view()->wrapCursor();
        int maxColumn = -1;
        if (n >= 0) {
            for (int i = 0; i < n; i++) {
                if (column() >= thisLine->length()) {
                    if (wrapCursor) {
                        break;

                    } else if (view()->dynWordWrap()) {
                        // Don't go past the edge of the screen in dynamic wrapping mode
                        if (maxColumn == -1) {
                            maxColumn = thisLine->length() + ((m_vi->width() - thisLine->widthOfLastLine()) / m_vi->renderer()->spaceWidth()) - 1;
                        }

                        if (column() >= maxColumn) {
                            m_cursor.setColumn(maxColumn);
                            break;
                        }

                        m_cursor.setColumn(column() + 1);

                    } else {
                        m_cursor.setColumn(column() + 1);
                    }

                } else {
                    m_cursor.setColumn(thisLine->layout()->nextCursorPosition(column()));
                }
            }
        } else {
            for (int i = 0; i > n; i--) {
                if (column() >= thisLine->length()) {
                    m_cursor.setColumn(column() - 1);
                } else if (column() == 0) {
                    break;
                } else {
                    m_cursor.setColumn(thisLine->layout()->previousCursorPosition(column()));
                }
            }
        }

        Q_ASSERT(valid());
        return *this;
    }
    CalculatingCursor &operator-=(int n) override
    {
        return operator+=(-n);
    }
};

class WrappingCursor final : public CalculatingCursor
{
public:
    WrappingCursor(KateViewInternal *vi)
        : CalculatingCursor(vi)
    {
    }
    WrappingCursor(KateViewInternal *vi, const KTextEditor::Cursor c)
        : CalculatingCursor(vi, c)
    {
    }
    WrappingCursor(KateViewInternal *vi, int line, int col)
        : CalculatingCursor(vi, line, col)
    {
    }

    CalculatingCursor &operator+=(int n) override
    {
        KateLineLayout *thisLine = m_vi->cache()->line(line());
        if (!thisLine || !thisLine->isValid()) {
            qCWarning(LOG_KTE) << "Did not retrieve a valid layout for line " << line();
            return *this;
        }

        if (n >= 0) {
            for (int i = 0; i < n; i++) {
                if (column() >= thisLine->length()) {
                    // Have come to the end of a line
                    if (line() >= doc()->lines() - 1)
                    // Have come to the end of the document
                    {
                        break;
                    }

                    // Advance to the beginning of the next line
                    m_cursor.setColumn(0);
                    m_cursor.setLine(line() + 1);

                    // Retrieve the next text range
                    thisLine = m_vi->cache()->line(line());
                    if (!thisLine || !thisLine->isValid()) {
                        qCWarning(LOG_KTE) << "Did not retrieve a valid layout for line " << line();
                        return *this;
                    }

                    continue;
                }

                m_cursor.setColumn(thisLine->layout()->nextCursorPosition(column()));
            }

        } else {
            for (int i = 0; i > n; i--) {
                if (column() == 0) {
                    // Have come to the start of the document
                    if (line() == 0) {
                        break;
                    }

                    // Start going back to the end of the last line
                    m_cursor.setLine(line() - 1);

                    // Retrieve the next text range
                    thisLine = m_vi->cache()->line(line());
                    if (!thisLine || !thisLine->isValid()) {
                        qCWarning(LOG_KTE) << "Did not retrieve a valid layout for line " << line();
                        return *this;
                    }

                    // Finish going back to the end of the last line
                    m_cursor.setColumn(thisLine->length());

                    continue;
                }

                if (column() > thisLine->length()) {
                    m_cursor.setColumn(column() - 1);
                } else {
                    m_cursor.setColumn(thisLine->layout()->previousCursorPosition(column()));
                }
            }
        }

        Q_ASSERT(valid());
        return *this;
    }
    CalculatingCursor &operator-=(int n) override
    {
        return operator+=(-n);
    }
};

/**
 * @brief The CamelCursor class
 *
 * This class allows for "camel humps" when moving the cursor
 * using Ctrl + Left / Right. Similarly, this will also get triggered
 * when you press Ctrl+Shift+Left/Right for selection and Ctrl+Del
 * Ctrl + backspace for deletion.
 *
 * It is absoloutely essential that if you move through a word in 'n'
 * jumps, you should be able to move back with exactly same 'n' movements
 * which you made when moving forward. Example:
 *
 * Word: KateViewInternal
 *
 * Moving cursor towards right while holding control will result in cursor
 * landing in the following places (assuming you start from column 0)
 *
 *     |   |       |
 * KateViewInternal
 *
 * Moving the cursor back to get to the starting position should also
 * take exactly 3 movements:
 *
 * |   |   |
 * KateViewInternal
 *
 * In addition to simple camel case, this class also handles snake_case
 * capitalized snake, and mixtures of Camel + snake/underscore, for example
 * m_someMember. If a word has underscores in it, for example:
 *
 * snake_case_word
 *
 * the leading underscore is considered part of the word and thus a cursor
 * movement will land right after the underscore. Moving the cursor to end
 * for the above word should be like this:
 *
 * startpos: 0
 *       |    |   |
 * snake_case_word
 *
 * When jumping back to startpos, exact same "spots" need to be hit on your way
 * back.
 *
 * If a word has multiple leading underscores: snake___case, the underscores will
 * be considered part of the word and thus a jump wil land us "after" the last underscore.
 *
 * There are some other variations in this, for example Capitalized words, or identifiers
 * with numbers in betweens. For such cases, All capitalized words are skipped in one go
 * till we are on some "non-capitalized" word. In this context, a number is a non-capitalized
 * word so will break the jump. Examples:
 *
 *   | |
 * W1RD
 *
 * but for WORD_CAPITAL, following will happen:
 *
 *      |      |
 * WORD_CAPITAL
 *
 * The first case here is tricky to handle for reverse movement. I haven't noticed any
 * cases which the current implementation is unable to handle but there might be some.
 *
 * With languages like PHP where you have $ as part of the identifier, the cursor jump
 * will break "after" dollar. Consider: $phpVar, Following will happen:
 *
 *  |  |  |
 * $phpVar
 *
 * And of course, the reverse will be exact opposite.
 *
 * Similar to PHP, with CSS Colors, jump will break on '#' charachter
 *
 * Please see the test cases testWordMovementSingleRow() for more examples/data.
 *
 * It is absoloutely essential to know that this class *only* gets triggered for
 * cursor movement if we are in a word.
 *
 * Note to bugfixer: If some bug occurs, before changing anything please add a test
 * case for the bug and make sure everything passes before and after. The test case
 * for this can be found autotests/src/camelcursortest.cpp.
 *
 * @author Waqar Ahmed <waqar.17a@gmail.com>
 */
class CamelCursor final : public CalculatingCursor
{
public:
    CamelCursor(KateViewInternal *vi, const KTextEditor::Cursor c)
        : CalculatingCursor(vi, c)
    {
    }

    CalculatingCursor &operator+=(int n) override
    {
        KateLineLayout *thisLine = m_vi->cache()->line(line());
        if (!thisLine || !thisLine->isValid()) {
            qCWarning(LOG_KTE) << "Did not retrieve valid layout for line " << line();
            return *this;
        }

        auto isSurrogate = [](QChar c) {
            return c.isLowSurrogate() || c.isHighSurrogate();
        };

        if (n >= 0) {
            auto skipCaps = [](QStringView text, int &col) {
                int count = 0;
                while (col < text.size() && text.at(col).isUpper()) {
                    ++count;
                    ++col;
                }
                // if this is a letter, then it means we are in the
                // middle of a word, step back one position so that
                // we are at the last Cap letter
                // Otherwise, it's an all cap word
                if (count > 1 && col < text.size() && text.at(col).isLetterOrNumber()) {
                    --col;
                }
            };

            int jump = -1;
            int col = column();
            const QString &text = thisLine->textLine().text();

            if (col < text.size() && text.at(col).isUpper()) {
                skipCaps(text, col);
            }

            for (int i = col; i < thisLine->length(); ++i) {
                const auto c = text.at(i);
                if (isSurrogate(c)) {
                    col++;
                    continue;
                } else if (c.isUpper() || !c.isLetterOrNumber()) {
                    break;
                }
                ++col;
            }

            // eat any '_' that are after the word BEFORE any space happens
            if (col < text.size() && text.at(col) == QLatin1Char('_')) {
                while (col < text.size() && text.at(col) == QLatin1Char('_')) {
                    ++col;
                }
            }

            // Underscores eaten, so now eat any spaces till next word
            if (col < text.size() && text.at(col).isSpace()) {
                while (col < text.size() && text.at(col).isSpace()) {
                    ++col;
                }
            }

            jump = col < 0 || (column() == col) ? (column() + 1) : col;
            m_cursor.setColumn(jump);
        } else {
            int jump = -1;

            auto skipCapsRev = [](QStringView text, int &col) {
                int count = 0;
                while (col > 0 && text.at(col).isUpper()) {
                    ++count;
                    --col;
                }

                // if more than one cap found, and current
                // column is not upper, we want to move ahead
                // to the upper
                if (count >= 1 && col >= 0 && !text.at(col).isUpper()) {
                    ++col;
                }
            };

            const QString &text = thisLine->textLine().text();
            int col = std::min<int>(column(), text.size() - 1);
            // decrement because we might be standing at a camel hump
            // already and don't want to return the same position
            col = col - 1;

            // if we are at the end of line
            if (column() == text.size()) {
                // there is a letter before
                if (text.at(col + 1).isLetter()) {
                    // and a space before that
                    if (col >= 0 && text.at(col).isSpace()) {
                        // we have a one letter word,
                        // so move forward to end of word
                        ++col;
                    }
                }
            }

            // skip any spaces
            if (col > 0 && text.at(col).isSpace()) {
                while (text.at(col).isSpace() && col > 0) {
                    --col;
                }
            }

            // Skip Underscores
            if (col > 0 && text.at(col) == QLatin1Char('_')) {
                while (col > 0 && text.at(col) == QLatin1Char('_')) {
                    --col;
                }
            }

            if (col > 0 && text.at(col).isUpper()) {
                skipCapsRev(text, col);
            }

            for (int i = col; i > 0; --i) {
                const auto c = text.at(i);
                if (isSurrogate(c)) {
                    --col;
                    continue;
                } else if (c.isUpper() || !c.isLetterOrNumber()) {
                    break;
                }
                --col;
            }

            if (col >= 0 && !text.at(col).isLetterOrNumber() && !isSurrogate(text.at(col))) {
                ++col;
            }

            if (col < 0) {
                jump = 0;
            } else if (col == column() && column() > 0) {
                jump = column() - 1;
            } else {
                jump = col;
            }

            m_cursor.setColumn(jump);
        }

        Q_ASSERT(valid());
        return *this;
    }

    CalculatingCursor &operator-=(int n) override
    {
        return operator+=(-n);
    }
};

void KateViewInternal::moveChar(KateViewInternal::Bias bias, bool sel)
{
    KTextEditor::Cursor c;
    if (view()->wrapCursor()) {
        c = WrappingCursor(this, m_cursor) += bias;
    } else {
        c = BoundedCursor(this, m_cursor) += bias;
    }

    const auto &sc = view()->m_secondaryCursors;
    QVarLengthArray<CursorPair, 16> multiCursors;
    const int lastLine = doc()->lastLine();
    bool shouldEnsureUniqueCursors = false;
    for (const auto &c : sc) {
        auto oldPos = c.cursor();
        if (view()->wrapCursor()) {
            c.pos->setPosition(WrappingCursor(this, oldPos) += bias);
        } else {
            c.pos->setPosition(BoundedCursor(this, oldPos) += bias);
        }
        const auto newPos = c.pos->toCursor();
        multiCursors.push_back({.oldPos=oldPos, .newPos=newPos});
        // We only need to do this if cursors were in first or last line
        if (!shouldEnsureUniqueCursors) {
            shouldEnsureUniqueCursors = newPos.line() == 0 || newPos.line() == lastLine;
        }
    }

    updateSelection(c, sel);
    updateCursor(c);
    updateSecondaryCursors(multiCursors, sel);
    if (shouldEnsureUniqueCursors) {
        view()->ensureUniqueCursors();
    }
}

void KateViewInternal::cursorPrevChar(bool sel)
{
    if (!view()->wrapCursor() && m_cursor.column() == 0) {
        return;
    }

    moveChar(KateViewInternal::left, sel);
}

void KateViewInternal::cursorNextChar(bool sel)
{
    moveChar(KateViewInternal::right, sel);
}

void KateViewInternal::wordPrev(bool sel)
{
    auto characterAtPreviousColumn = [this](KTextEditor::Cursor cursor) -> QChar {
        return doc()->characterAt({cursor.line(), cursor.column() - 1});
    };

    auto wordPrevious = [this, &characterAtPreviousColumn](KTextEditor::Cursor cursor) -> KTextEditor::Cursor {
        WrappingCursor c(this, cursor);

        // First we skip backwards all space.
        // Then we look up into which category the current position falls:
        // 1. a "word" character
        // 2. a "non-word" character (except space)
        // 3. the beginning of the line
        // and skip all preceding characters that fall into this class.
        // The code assumes that space is never part of the word character class.

        KateHighlighting *h = doc()->highlight();

        while (!c.atEdge(left) && (c.column() > doc()->lineLength(c.line()) || characterAtPreviousColumn(c).isSpace())) {
            --c;
        }

        if (c.atEdge(left)) {
            --c;
        } else if (h->isInWord(characterAtPreviousColumn(c))) {
            if (doc()->config()->camelCursor()) {
                CamelCursor cc(this, cursor);
                --cc;
                return cc;
            } else {
                while (!c.atEdge(left) && h->isInWord(characterAtPreviousColumn(c))) {
                    --c;
                }
            }
        } else {
            while (!c.atEdge(left)
                   && !h->isInWord(characterAtPreviousColumn(c))
                   // in order to stay symmetric to wordLeft()
                   // we must not skip space preceding a non-word sequence
                   && !characterAtPreviousColumn(c).isSpace()) {
                --c;
            }
        }

        return c;
    };

    const auto &secondaryCursors = view()->m_secondaryCursors;
    QVarLengthArray<CursorPair, 16> cursorsToUpdate;
    for (const auto &cursor : secondaryCursors) {
        auto oldPos = cursor.cursor();
        auto newCursorPos = wordPrevious(cursor.cursor());
        cursor.pos->setPosition(newCursorPos);
        cursorsToUpdate.push_back({.oldPos=oldPos, .newPos=newCursorPos});
    }

    // update primary cursor
    const auto c = wordPrevious(m_cursor);
    updateSelection(c, sel);
    updateCursor(c);

    if (!sel) {
        view()->ensureUniqueCursors();
    }
    updateSecondaryCursors(cursorsToUpdate, sel);
}

void KateViewInternal::wordNext(bool sel)
{
    auto nextWord = [this](KTextEditor::Cursor cursor) -> KTextEditor::Cursor {
        WrappingCursor c(this, cursor);

        // We look up into which category the current position falls:
        // 1. a "word" character
        // 2. a "non-word" character (except space)
        // 3. the end of the line
        // and skip all following characters that fall into this class.
        // If the skipped characters are followed by space, we skip that too.
        // The code assumes that space is never part of the word character class.

        KateHighlighting *h = doc()->highlight();
        if (c.atEdge(right)) {
            ++c;
        } else if (h->isInWord(doc()->characterAt(c))) {
            if (doc()->config()->camelCursor()) {
                CamelCursor cc(this, cursor);
                ++cc;
                return cc;
            } else {
                while (!c.atEdge(right) && h->isInWord(doc()->characterAt(c))) {
                    ++c;
                }
            }
        } else {
            while (!c.atEdge(right)
                   && !h->isInWord(doc()->characterAt(c))
                   // we must not skip space, because if that space is followed
                   // by more non-word characters, we would skip them, too
                   && !doc()->characterAt(c).isSpace()) {
                ++c;
            }
        }

        while (!c.atEdge(right) && doc()->characterAt(c).isSpace()) {
            ++c;
        }

        return c;
    };

    const auto &secondaryCursors = view()->m_secondaryCursors;
    QVarLengthArray<CursorPair, 16> cursorsToUpdate;
    for (const auto &cursor : secondaryCursors) {
        auto oldPos = cursor.cursor();
        auto newCursorPos = nextWord(cursor.cursor());
        cursor.pos->setPosition(newCursorPos);
        cursorsToUpdate.push_back({.oldPos=oldPos, .newPos=newCursorPos});
    }

    // update primary cursor
    const auto c = nextWord(m_cursor);
    updateSelection(c, sel);
    updateCursor(c);

    // Remove cursors which have same position
    if (!sel) {
        view()->ensureUniqueCursors();
    }
    updateSecondaryCursors(cursorsToUpdate, sel);
}

void KateViewInternal::moveEdge(KateViewInternal::Bias bias, bool sel)
{
    BoundedCursor c(this, m_cursor);
    c.toEdge(bias);
    updateSelection(c, sel);
    updateCursor(c);
}

KTextEditor::Cursor KateViewInternal::moveCursorToLineStart(KTextEditor::Cursor cursor)
{
    if (view()->dynWordWrap() && currentLayout(cursor).startCol()) {
        // Allow us to go to the real start if we're already at the start of the view line
        if (cursor.column() != currentLayout(cursor).startCol()) {
            KTextEditor::Cursor c = currentLayout(cursor).start();
            return c;
        }
    }

    if (!doc()->config()->smartHome()) {
        BoundedCursor c(this, cursor);
        c.toEdge(left);
        return c;
    }

    if (cursor.line() < 0 || cursor.line() >= doc()->lines()) {
        return KTextEditor::Cursor::invalid();
    }

    Kate::TextLine l = doc()->kateTextLine(cursor.line());

    KTextEditor::Cursor c = cursor;
    int lc = l.firstChar();

    if (lc < 0 || c.column() == lc) {
        c.setColumn(0);
    } else {
        c.setColumn(lc);
    }
    return c;
}

void KateViewInternal::home(bool sel)
{
    // Multicursor
    view()->ensureUniqueCursors(/*matchLine*/ true);
    const auto &secondaryCursors = view()->m_secondaryCursors;
    QVarLengthArray<CursorPair, 16> cursorsToUpdate;
    for (const auto &c : secondaryCursors) {
        auto oldPos = c.cursor();
        // These will end up in same place so just remove
        auto newPos = moveCursorToLineStart(oldPos);
        c.pos->setPosition(newPos);
        cursorsToUpdate.push_back({.oldPos=oldPos, .newPos=newPos});
    }

    // Primary cursor
    auto newPos = moveCursorToLineStart(m_cursor);
    if (newPos.isValid()) {
        updateSelection(newPos, sel);
        updateCursor(newPos, true);
    }
    updateSecondaryCursors(cursorsToUpdate, sel);
}

KTextEditor::Cursor KateViewInternal::moveCursorToLineEnd(KTextEditor::Cursor cursor)
{
    KateTextLayout layout = currentLayout(cursor);

    if (view()->dynWordWrap() && layout.wrap()) {
        // Allow us to go to the real end if we're already at the end of the view line
        if (cursor.column() < layout.endCol() - 1) {
            KTextEditor::Cursor c(cursor.line(), layout.endCol() - 1);
            return c;
        }
    }

    if (!doc()->config()->smartHome()) {
        BoundedCursor c(this, cursor);
        c.toEdge(right);
        return c;
    }

    if (cursor.line() < 0 || cursor.line() >= doc()->lines()) {
        return KTextEditor::Cursor::invalid();
    }

    Kate::TextLine l = doc()->kateTextLine(cursor.line());

    // "Smart End", as requested in bugs #78258 and #106970
    if (cursor.column() == doc()->lineLength(cursor.line())) {
        KTextEditor::Cursor c = cursor;
        c.setColumn(l.lastChar() + 1);
        return c;
    } else {
        BoundedCursor c(this, cursor);
        c.toEdge(right);
        return c;
    }
}

void KateViewInternal::end(bool sel)
{
    // Multicursor
    view()->ensureUniqueCursors(/*matchLine*/ true);

    QVarLengthArray<CursorPair, 16> cursorsToUpdate;
    const auto &secondaryCursors = view()->m_secondaryCursors;
    for (const auto &c : secondaryCursors) {
        auto oldPos = c.cursor();
        // These will end up in same place so just remove
        auto newPos = moveCursorToLineEnd(oldPos);
        c.pos->setPosition(newPos);
        cursorsToUpdate.push_back({.oldPos=oldPos, .newPos=newPos});
    }

    auto newPos = moveCursorToLineEnd(m_cursor);
    if (newPos.isValid()) {
        updateSelection(newPos, sel);
        updateCursor(newPos);
    }

    updateSecondaryCursors(cursorsToUpdate, sel);
    paintCursor();
}

KateTextLayout KateViewInternal::currentLayout(KTextEditor::Cursor c) const
{
    return cache()->textLayout(c);
}

KateTextLayout KateViewInternal::previousLayout(KTextEditor::Cursor c) const
{
    int currentViewLine = cache()->viewLine(c);

    if (currentViewLine) {
        return cache()->textLayout(c.line(), currentViewLine - 1);
    } else {
        return cache()->textLayout(view()->textFolding().visibleLineToLine(toVirtualCursor(c).line() - 1), -1);
    }
}

KateTextLayout KateViewInternal::nextLayout(KTextEditor::Cursor c) const
{
    int currentViewLine = cache()->viewLine(c) + 1;

    const KateLineLayout *thisLine = cache()->line(c.line());
    if (thisLine && currentViewLine >= thisLine->viewLineCount()) {
        currentViewLine = 0;
        return cache()->textLayout(view()->textFolding().visibleLineToLine(toVirtualCursor(c).line() + 1), currentViewLine);
    } else {
        return cache()->textLayout(c.line(), currentViewLine);
    }
}

/*
 * This returns the cursor which is offset by (offset) view lines.
 * This is the main function which is called by code not specifically dealing with word-wrap.
 * The opposite conversion (cursor to offset) can be done with cache()->displayViewLine().
 *
 * The cursors involved are virtual cursors (ie. equivalent to m_displayCursor)
 */

KTextEditor::Cursor KateViewInternal::viewLineOffset(const KTextEditor::Cursor virtualCursor, int offset, bool keepX)
{
    if (!view()->dynWordWrap()) {
        KTextEditor::Cursor ret(qMin((int)view()->textFolding().visibleLines() - 1, virtualCursor.line() + offset), 0);

        if (ret.line() < 0) {
            ret.setLine(0);
        }

        if (keepX) {
            int realLine = view()->textFolding().visibleLineToLine(ret.line());
            KateTextLayout t = cache()->textLayout(realLine, 0);
            Q_ASSERT(t.isValid());

            ret.setColumn(renderer()->xToCursor(t, m_preservedX, !view()->wrapCursor()).column());
        }

        return ret;
    }

    KTextEditor::Cursor realCursor = virtualCursor;
    realCursor.setLine(view()->textFolding().visibleLineToLine(view()->textFolding().lineToVisibleLine(virtualCursor.line())));

    int cursorViewLine = cache()->viewLine(realCursor);

    int currentOffset = 0;
    int virtualLine = 0;

    bool forwards = (offset > 0) ? true : false;

    if (forwards) {
        currentOffset = cache()->lastViewLine(realCursor.line()) - cursorViewLine;
        if (offset <= currentOffset) {
            // the answer is on the same line
            KateTextLayout thisLine = cache()->textLayout(realCursor.line(), cursorViewLine + offset);
            Q_ASSERT(thisLine.virtualLine() == (int)view()->textFolding().lineToVisibleLine(virtualCursor.line()));
            return KTextEditor::Cursor(virtualCursor.line(), thisLine.startCol());
        }

        virtualLine = virtualCursor.line() + 1;

    } else {
        offset = -offset;
        currentOffset = cursorViewLine;
        if (offset <= currentOffset) {
            // the answer is on the same line
            KateTextLayout thisLine = cache()->textLayout(realCursor.line(), cursorViewLine - offset);
            Q_ASSERT(thisLine.virtualLine() == (int)view()->textFolding().lineToVisibleLine(virtualCursor.line()));
            return KTextEditor::Cursor(virtualCursor.line(), thisLine.startCol());
        }

        virtualLine = virtualCursor.line() - 1;
    }

    currentOffset++;

    while (virtualLine >= 0 && virtualLine < (int)view()->textFolding().visibleLines()) {
        int realLine = view()->textFolding().visibleLineToLine(virtualLine);
        KateLineLayout *thisLine = cache()->line(realLine, virtualLine);
        if (!thisLine) {
            break;
        }

        for (int i = 0; i < thisLine->viewLineCount(); ++i) {
            if (offset == currentOffset) {
                KateTextLayout thisViewLine = thisLine->viewLine(i);

                if (!forwards) {
                    // We actually want it the other way around
                    int requiredViewLine = cache()->lastViewLine(realLine) - thisViewLine.viewLine();
                    if (requiredViewLine != thisViewLine.viewLine()) {
                        thisViewLine = thisLine->viewLine(requiredViewLine);
                    }
                }

                KTextEditor::Cursor ret(virtualLine, thisViewLine.startCol());

                // keep column position
                if (keepX) {
                    realCursor = renderer()->xToCursor(thisViewLine, m_preservedX, !view()->wrapCursor());
                    ret.setColumn(realCursor.column());
                }

                return ret;
            }

            currentOffset++;
        }

        if (forwards) {
            virtualLine++;
        } else {
            virtualLine--;
        }
    }

    // Looks like we were asked for something a bit exotic.
    // Return the max/min valid position.
    if (forwards) {
        return KTextEditor::Cursor(view()->textFolding().visibleLines() - 1,
                                   doc()->lineLength(view()->textFolding().visibleLineToLine(view()->textFolding().visibleLines() - 1)));
    } else {
        return KTextEditor::Cursor(0, 0);
    }
}

int KateViewInternal::lineMaxCursorX(const KateTextLayout &range)
{
    if (!view()->wrapCursor() && !range.wrap()) {
        return INT_MAX;
    }

    int maxX = range.endX();

    if (maxX && range.wrap()) {
        QChar lastCharInLine = doc()->kateTextLine(range.line()).at(range.endCol() - 1);
        maxX -= renderer()->currentFontMetrics().horizontalAdvance(lastCharInLine);
    }

    return maxX;
}

int KateViewInternal::lineMaxCol(const KateTextLayout &range)
{
    int maxCol = range.endCol();

    if (maxCol && range.wrap()) {
        maxCol--;
    }

    return maxCol;
}

void KateViewInternal::cursorUp(bool sel)
{
    if (!sel && view()->completionWidget()->isCompletionActive()) {
        view()->completionWidget()->cursorUp();
        return;
    }

    m_preserveX = true;

    // Handle Multi cursors
    // usually this will have only one element, rarely
    int i = 0;
    for (const auto &c : view()->m_secondaryCursors) {
        auto cursor = c.pos->toCursor();
        auto vCursor = toVirtualCursor(cursor);

        // our cursor is in the first line already
        if (vCursor.line() == 0 && (!view()->dynWordWrap() || cache()->viewLine(cursor) == 0)) {
            auto newPos = moveCursorToLineStart(cursor);
            c.pos->setPosition(newPos);
            auto newVcursor = toVirtualCursor(newPos);
            if (sel) {
                updateSecondarySelection(i, cursor, newPos);
            } else {
                view()->clearSecondarySelections();
            }
            tagLines(newVcursor.line(), vCursor.line());
            i++;
            continue;
        }

        auto lineLayout = currentLayout(cursor);
        Q_ASSERT(lineLayout.line() == cursor.line());
        Q_ASSERT(lineLayout.startCol() <= cursor.column());
        Q_ASSERT(!lineLayout.wrap() || cursor.column() < lineLayout.endCol());

        KateTextLayout pRange = previousLayout(cursor);

        KTextEditor::Cursor newPos = renderer()->xToCursor(pRange, m_preservedX, !view()->wrapCursor());
        c.pos->setPosition(newPos);

        auto newVcursor = toVirtualCursor(newPos);
        if (sel) {
            updateSecondarySelection(i, cursor, newPos);
        } else {
            view()->clearSecondarySelections();
        }
        tagLines(newVcursor.line(), vCursor.line());
        i++;
    }

    auto mergeOnFuncEnd = qScopeGuard([this, sel] {
        if (sel) {
            mergeSelections();
        } else {
            view()->ensureUniqueCursors();
        }
    });

    // Normal single cursor

    // assert that the display cursor is in visible lines
    Q_ASSERT(m_displayCursor.line() < view()->textFolding().visibleLines());

    // move cursor to start of line, if we are at first line!
    if (m_displayCursor.line() == 0 && (!view()->dynWordWrap() || cache()->viewLine(m_cursor) == 0)) {
        auto newPos = moveCursorToLineStart(m_cursor);
        if (newPos.isValid()) {
            updateSelection(newPos, sel);
            updateCursor(newPos, true);
        }
        return;
    }

    KateTextLayout thisLine = currentLayout(m_cursor);
    // This is not the first line because that is already simplified out above
    KateTextLayout pRange = previousLayout(m_cursor);

    // Ensure we're in the right spot
    Q_ASSERT(m_cursor.line() == thisLine.line());
    Q_ASSERT(m_cursor.column() >= thisLine.startCol());
    Q_ASSERT(!thisLine.wrap() || m_cursor.column() < thisLine.endCol());

    KTextEditor::Cursor c = renderer()->xToCursor(pRange, m_preservedX, !view()->wrapCursor());

    updateSelection(c, sel);
    updateCursor(c);
}

void KateViewInternal::cursorDown(bool sel)
{
    if (!sel && view()->completionWidget()->isCompletionActive()) {
        view()->completionWidget()->cursorDown();
        return;
    }

    m_preserveX = true;

    // Handle multiple cursors
    int i = 0;
    for (const auto &c : view()->m_secondaryCursors) {
        auto cursor = c.cursor();
        auto vCursor = toVirtualCursor(cursor);

        // at end?
        if ((vCursor.line() >= view()->textFolding().visibleLines() - 1)
            && (!view()->dynWordWrap() || cache()->viewLine(cursor) == cache()->lastViewLine(cursor.line()))) {
            KTextEditor::Cursor newPos = moveCursorToLineEnd(cursor);
            c.pos->setPosition(newPos);
            if (sel) {
                updateSecondarySelection(i, cursor, newPos);
            } else {
                view()->clearSecondarySelections();
            }
            auto vNewPos = toVirtualCursor(newPos);
            tagLines(vCursor.line(), vNewPos.line());
            i++;
            continue;
        }

        KateTextLayout thisLine = currentLayout(cursor);
        // This is not the last line because that is already simplified out above
        KateTextLayout nRange = nextLayout(cursor);

        // Ensure we're in the right spot
        Q_ASSERT((cursor.line() == thisLine.line()) && (cursor.column() >= thisLine.startCol()) && (!thisLine.wrap() || cursor.column() < thisLine.endCol()));
        KTextEditor::Cursor newPos = renderer()->xToCursor(nRange, m_preservedX, !view()->wrapCursor());

        c.pos->setPosition(newPos);
        if (sel) {
            updateSecondarySelection(i, cursor, newPos);
        } else {
            view()->clearSecondarySelections();
        }
        auto vNewPos = toVirtualCursor(newPos);
        tagLines(vCursor.line(), vNewPos.line());
        i++;
    }
    auto mergeOnFuncEnd = qScopeGuard([this, sel] {
        if (sel) {
            mergeSelections();
        } else {
            view()->ensureUniqueCursors();
        }
    });

    // Handle normal single cursor

    // move cursor to end of line, if we are at last line!
    if ((m_displayCursor.line() >= view()->textFolding().visibleLines() - 1)
        && (!view()->dynWordWrap() || cache()->viewLine(m_cursor) == cache()->lastViewLine(m_cursor.line()))) {
        auto newPos = moveCursorToLineEnd(m_cursor);
        if (newPos.isValid()) {
            updateSelection(newPos, sel);
            updateCursor(newPos);
        }
        return;
    }

    KateTextLayout thisLine = currentLayout(m_cursor);
    // This is not the last line because that is already simplified out above
    KateTextLayout nRange = nextLayout(m_cursor);

    // Ensure we're in the right spot
    Q_ASSERT((m_cursor.line() == thisLine.line()) && (m_cursor.column() >= thisLine.startCol()) && (!thisLine.wrap() || m_cursor.column() < thisLine.endCol()));

    KTextEditor::Cursor c = renderer()->xToCursor(nRange, m_preservedX, !view()->wrapCursor());

    updateSelection(c, sel);
    updateCursor(c);
}

void KateViewInternal::cursorToMatchingBracket(bool sel)
{
    KTextEditor::Cursor c = findMatchingBracket();

    if (c.isValid()) {
        updateSelection(c, sel);
        updateCursor(c);
    }
}

void KateViewInternal::topOfView(bool sel)
{
    view()->clearSecondaryCursors();
    KTextEditor::Cursor c = viewLineOffset(startPos(), m_minLinesVisible);
    updateSelection(toRealCursor(c), sel);
    updateCursor(toRealCursor(c));
}

void KateViewInternal::bottomOfView(bool sel)
{
    view()->clearSecondaryCursors();
    KTextEditor::Cursor c = viewLineOffset(endPos(), -m_minLinesVisible);
    updateSelection(toRealCursor(c), sel);
    updateCursor(toRealCursor(c));
}

// lines is the offset to scroll by
void KateViewInternal::scrollLines(int lines, bool sel)
{
    KTextEditor::Cursor c = viewLineOffset(m_displayCursor, lines, true);

    // Fix the virtual cursor -> real cursor
    c.setLine(view()->textFolding().visibleLineToLine(c.line()));

    updateSelection(c, sel);
    updateCursor(c);
}

// This is a bit misleading... it's asking for the view to be scrolled, not the cursor
void KateViewInternal::scrollUp()
{
    KTextEditor::Cursor newPos = viewLineOffset(startPos(), -1);
    scrollPos(newPos);
}

void KateViewInternal::scrollDown()
{
    KTextEditor::Cursor newPos = viewLineOffset(startPos(), 1);
    scrollPos(newPos);
}

void KateViewInternal::setAutoCenterLines(int viewLines, bool updateView)
{
    m_autoCenterLines = viewLines;
    m_minLinesVisible = qMin(int((linesDisplayed() - 1) / 2), m_autoCenterLines);
    if (updateView) {
        KateViewInternal::updateView();
    }
}

void KateViewInternal::pageUp(bool sel, bool half)
{
    if (view()->isCompletionActive()) {
        view()->completionWidget()->pageUp();
        return;
    }
    view()->clearSecondaryCursors();

    // jump back to where the cursor is, otherwise it is super
    // slow to call cache()->displayViewLine
    if (!view()->visibleRange().contains(m_displayCursor)) {
        scrollLines(m_displayCursor.line());
    }

    // remember the view line and x pos
    int viewLine = cache()->displayViewLine(m_displayCursor);
    bool atTop = startPos().atStartOfDocument();

    // Adjust for an auto-centering cursor
    int lineadj = m_minLinesVisible;

    int linesToScroll;
    if (!half) {
        linesToScroll = -qMax((linesDisplayed() - 1) - lineadj, 0);
    } else {
        linesToScroll = -qMax((linesDisplayed() / 2 - 1) - lineadj, 0);
    }

    m_preserveX = true;

    if (!doc()->pageUpDownMovesCursor() && !atTop) {
        KTextEditor::Cursor newStartPos = viewLineOffset(startPos(), linesToScroll - 1);
        scrollPos(newStartPos);

        // put the cursor back approximately where it was
        KTextEditor::Cursor newPos = toRealCursor(viewLineOffset(newStartPos, viewLine, true));

        KateTextLayout newLine = cache()->textLayout(newPos);

        newPos = renderer()->xToCursor(newLine, m_preservedX, !view()->wrapCursor());

        m_preserveX = true;
        updateSelection(newPos, sel);
        updateCursor(newPos);

    } else {
        scrollLines(linesToScroll, sel);
    }
}

void KateViewInternal::pageDown(bool sel, bool half)
{
    if (view()->isCompletionActive()) {
        view()->completionWidget()->pageDown();
        return;
    }

    view()->clearSecondaryCursors();

    // jump back to where the cursor is, otherwise it is super
    // slow to call cache()->displayViewLine
    if (!view()->visibleRange().contains(m_displayCursor)) {
        scrollLines(m_displayCursor.line());
    }

    // remember the view line
    int viewLine = cache()->displayViewLine(m_displayCursor);
    bool atEnd = startPos() >= m_cachedMaxStartPos;

    // Adjust for an auto-centering cursor
    int lineadj = m_minLinesVisible;

    int linesToScroll;
    if (!half) {
        linesToScroll = qMax((linesDisplayed() - 1) - lineadj, 0);
    } else {
        linesToScroll = qMax((linesDisplayed() / 2 - 1) - lineadj, 0);
    }

    m_preserveX = true;

    if (!doc()->pageUpDownMovesCursor() && !atEnd) {
        KTextEditor::Cursor newStartPos = viewLineOffset(startPos(), linesToScroll + 1);
        scrollPos(newStartPos);

        // put the cursor back approximately where it was
        KTextEditor::Cursor newPos = toRealCursor(viewLineOffset(newStartPos, viewLine, true));

        KateTextLayout newLine = cache()->textLayout(newPos);

        newPos = renderer()->xToCursor(newLine, m_preservedX, !view()->wrapCursor());

        m_preserveX = true;
        updateSelection(newPos, sel);
        updateCursor(newPos);

    } else {
        scrollLines(linesToScroll, sel);
    }
}

int KateViewInternal::maxLen(int startLine)
{
    Q_ASSERT(!view()->dynWordWrap());

    int displayLines = (view()->height() / renderer()->lineHeight()) + 1;

    int maxLen = 0;

    for (int z = 0; z < displayLines; z++) {
        int virtualLine = startLine + z;

        if (virtualLine < 0 || virtualLine >= (int)view()->textFolding().visibleLines()) {
            break;
        }

        const KateLineLayout *line = cache()->line(view()->textFolding().visibleLineToLine(virtualLine));
        if (!line) {
            continue;
        }

        maxLen = qMax(maxLen, line->width());
    }

    return maxLen;
}

bool KateViewInternal::columnScrollingPossible()
{
    return !view()->dynWordWrap() && m_columnScroll->isEnabled() && (m_columnScroll->maximum() > 0);
}

bool KateViewInternal::lineScrollingPossible()
{
    return m_lineScroll->minimum() != m_lineScroll->maximum();
}

void KateViewInternal::top(bool sel)
{
    KTextEditor::Cursor newCursor(0, 0);

    newCursor = renderer()->xToCursor(cache()->textLayout(newCursor), m_preservedX, !view()->wrapCursor());

    view()->clearSecondaryCursors();
    updateSelection(newCursor, sel);
    updateCursor(newCursor);
}

void KateViewInternal::bottom(bool sel)
{
    KTextEditor::Cursor newCursor(doc()->lastLine(), 0);

    newCursor = renderer()->xToCursor(cache()->textLayout(newCursor), m_preservedX, !view()->wrapCursor());

    view()->clearSecondaryCursors();
    updateSelection(newCursor, sel);
    updateCursor(newCursor);
}

void KateViewInternal::top_home(bool sel)
{
    if (view()->isCompletionActive()) {
        view()->completionWidget()->top();
        return;
    }

    view()->clearSecondaryCursors();
    KTextEditor::Cursor c(0, 0);
    updateSelection(c, sel);
    updateCursor(c);
}

void KateViewInternal::bottom_end(bool sel)
{
    if (view()->isCompletionActive()) {
        view()->completionWidget()->bottom();
        return;
    }

    view()->clearSecondaryCursors();
    KTextEditor::Cursor c(doc()->lastLine(), doc()->lineLength(doc()->lastLine()));
    updateSelection(c, sel);
    updateCursor(c);
}

void KateViewInternal::updateSecondarySelection(int cursorIdx, KTextEditor::Cursor old, KTextEditor::Cursor newPos) const
{
    if (m_selectionMode != SelectionMode::Default) {
        view()->clearSecondaryCursors();
    }

    auto &secondaryCursors = view()->m_secondaryCursors;
    if (secondaryCursors.empty()) {
        qWarning() << "Invalid updateSecondarySelection with no secondaryCursors";
        return;
    }
    Q_ASSERT(secondaryCursors.size() > (size_t)cursorIdx);

    auto &cursor = secondaryCursors[cursorIdx];
    if (cursor.cursor() != newPos) {
        qWarning() << "Unexpected different cursor at cursorIdx" << cursorIdx << "found" << cursor.cursor() << "looking for: " << newPos;
        return;
    }

    if (cursor.range) {
        Q_ASSERT(cursor.anchor.isValid());
        cursor.range->setRange(cursor.anchor, newPos);
    } else {
        cursor.range.reset(view()->newSecondarySelectionRange({old, newPos}));
        cursor.anchor = old;
    }
}

void KateViewInternal::updateSelection(const KTextEditor::Cursor _newCursor, bool keepSel)
{
    KTextEditor::Cursor newCursor = _newCursor;
    if (keepSel) {
        if (!view()->selection()
            || (m_selectAnchor.line() == -1)
            // don't kill the selection if we have a persistent selection and
            // the cursor is inside or at the boundaries of the selected area
            || (view()->config()->persistentSelection()
                && !(view()->selectionRange().contains(m_cursor) || view()->selectionRange().boundaryAtCursor(m_cursor)))) {
            m_selectAnchor = m_cursor;
            setSelection(KTextEditor::Range(m_cursor, newCursor));
        } else {
            bool doSelect = true;
            switch (m_selectionMode) {
            case Word: {
                // Restore selStartCached if needed. It gets nuked by
                // viewSelectionChanged if we drag the selection into non-existence,
                // which can legitimately happen if a shift+DC selection is unable to
                // set a "proper" (i.e. non-empty) cached selection, e.g. because the
                // start was on something that isn't a word. Word select mode relies
                // on the cached selection being set properly, even if it is empty
                // (i.e. selStartCached == selEndCached).
                if (!m_selectionCached.isValid()) {
                    m_selectionCached.setStart(m_selectionCached.end());
                }

                int c;
                if (newCursor > m_selectionCached.start()) {
                    m_selectAnchor = m_selectionCached.start();

                    Kate::TextLine l = doc()->kateTextLine(newCursor.line());

                    c = newCursor.column();
                    if (c > 0 && doc()->highlight()->isInWord(l.at(c - 1))) {
                        for (; c < l.length(); c++) {
                            if (!doc()->highlight()->isInWord(l.at(c))) {
                                break;
                            }
                        }
                    }

                    newCursor.setColumn(c);
                } else if (newCursor < m_selectionCached.start()) {
                    m_selectAnchor = m_selectionCached.end();

                    Kate::TextLine l = doc()->kateTextLine(newCursor.line());

                    c = newCursor.column();
                    if (c > 0 && c < doc()->lineLength(newCursor.line()) && doc()->highlight()->isInWord(l.at(c))
                        && doc()->highlight()->isInWord(l.at(c - 1))) {
                        for (c -= 2; c >= 0; c--) {
                            if (!doc()->highlight()->isInWord(l.at(c))) {
                                break;
                            }
                        }
                        newCursor.setColumn(c + 1);
                    }
                } else {
                    doSelect = false;
                }

            } break;
            case Line:
                if (!m_selectionCached.isValid()) {
                    m_selectionCached = KTextEditor::Range(endLine(), 0, endLine(), 0);
                }
                if (newCursor.line() > m_selectionCached.start().line()) {
                    if (newCursor.line() + 1 >= doc()->lines()) {
                        newCursor.setColumn(doc()->line(newCursor.line()).length());
                    } else {
                        newCursor.setPosition(newCursor.line() + 1, 0);
                    }
                    // Grow to include the entire line
                    m_selectAnchor = m_selectionCached.start();
                    m_selectAnchor.setColumn(0);
                } else if (newCursor.line() < m_selectionCached.start().line()) {
                    newCursor.setColumn(0);
                    // Grow to include entire line
                    m_selectAnchor = m_selectionCached.end();
                    if (m_selectAnchor.column() > 0) {
                        if (m_selectAnchor.line() + 1 >= doc()->lines()) {
                            m_selectAnchor.setColumn(doc()->line(newCursor.line()).length());
                        } else {
                            m_selectAnchor.setPosition(m_selectAnchor.line() + 1, 0);
                        }
                    }
                } else { // same line, ignore
                    doSelect = false;
                }
                break;
            case Mouse: {
                if (!m_selectionCached.isValid()) {
                    break;
                }

                if (newCursor > m_selectionCached.end()) {
                    m_selectAnchor = m_selectionCached.start();
                } else if (newCursor < m_selectionCached.start()) {
                    m_selectAnchor = m_selectionCached.end();
                } else {
                    doSelect = false;
                }
            } break;
            default: /* nothing special to do */;
            }

            if (doSelect) {
                setSelection(KTextEditor::Range(m_selectAnchor, newCursor));
            } else if (m_selectionCached.isValid()) { // we have a cached selection, so we restore that
                setSelection(m_selectionCached);
            }
        }

        m_selChangedByUser = true;
    } else if (!view()->config()->persistentSelection()) {
        view()->clearSelection();

        m_selectionCached = KTextEditor::Range::invalid();
        m_selectAnchor = KTextEditor::Cursor::invalid();
    }

#ifndef QT_NO_ACCESSIBILITY
//    FIXME KF5
//    QAccessibleTextSelectionEvent ev(this, /* selection start, selection end*/);
//    QAccessible::updateAccessibility(&ev);
#endif
}

void KateViewInternal::setSelection(KTextEditor::Range range)
{
    disconnect(m_view, &KTextEditor::ViewPrivate::selectionChanged, this, &KateViewInternal::viewSelectionChanged);
    view()->setSelection(range);
    connect(m_view, &KTextEditor::ViewPrivate::selectionChanged, this, &KateViewInternal::viewSelectionChanged);
}

void KateViewInternal::moveCursorToSelectionEdge(bool scroll)
{
    if (!view()->selection()) {
        return;
    }

    int tmp = m_minLinesVisible;
    m_minLinesVisible = 0;

    if (view()->selectionRange().start() < m_selectAnchor) {
        updateCursor(view()->selectionRange().start(), false, false, false, scroll);
    } else {
        updateCursor(view()->selectionRange().end(), false, false, false, scroll);
    }
    if (!scroll) {
        m_madeVisible = false;
    }

    m_minLinesVisible = tmp;
}

KTextEditor::Range KateViewInternal::findMatchingFoldingMarker(const KTextEditor::Cursor currentCursorPos,
                                                               const KSyntaxHighlighting::FoldingRegion foldingRegion,
                                                               const int maxLines)
{
    const int direction = (foldingRegion.type() == KSyntaxHighlighting::FoldingRegion::Begin) ? 1 : -1;
    int foldCounter = 0;
    int lineCounter = 0;
    const auto foldMarkers = m_view->doc()->buffer().computeFoldings(currentCursorPos.line());

    // searching a end folding marker? go left to right
    // otherwise, go right to left
    long i = direction == 1 ? 0 : (long)foldMarkers.size() - 1;

    // For the first line, we start considering the first folding after the cursor
    for (; i >= 0 && i < (long)foldMarkers.size(); i += direction) {
        if ((foldMarkers[i].offset - currentCursorPos.column()) * direction > 0 && foldMarkers[i].foldingRegion.id() == foldingRegion.id()) {
            if (foldMarkers[i].foldingRegion.type() == foldingRegion.type()) {
                foldCounter += 1;
            } else if (foldCounter > 0) {
                foldCounter -= 1;
            } else if (foldCounter == 0) {
                return KTextEditor::Range(currentCursorPos.line(),
                                          getStartOffset(direction, foldMarkers[i].offset, foldMarkers[i].length),
                                          currentCursorPos.line(),
                                          getEndOffset(direction, foldMarkers[i].offset, foldMarkers[i].length));
            }
        }
    }

    // for the other lines
    int currentLine = currentCursorPos.line() + direction;
    for (; currentLine >= 0 && currentLine < m_view->doc()->lines() && lineCounter < maxLines; currentLine += direction) {
        // update line attributes
        const auto foldMarkers = m_view->doc()->buffer().computeFoldings(currentLine);
        i = direction == 1 ? 0 : (long)foldMarkers.size() - 1;

        // iterate through the markers
        for (; i >= 0 && i < (long)foldMarkers.size(); i += direction) {
            if (foldMarkers[i].foldingRegion.id() == foldingRegion.id()) {
                if (foldMarkers[i].foldingRegion.type() == foldingRegion.type()) {
                    foldCounter += 1;
                } else if (foldCounter != 0) {
                    foldCounter -= 1;
                } else if (foldCounter == 0) {
                    return KTextEditor::Range(currentLine,
                                              getStartOffset(direction, foldMarkers[i].offset, foldMarkers[i].length),
                                              currentLine,
                                              getEndOffset(direction, foldMarkers[i].offset, foldMarkers[i].length));
                }
            }
        }
        lineCounter += 1;
    }

    // got out of loop, no matching folding found
    // returns a invalid folding range
    return KTextEditor::Range::invalid();
}

void KateViewInternal::updateFoldingMarkersHighlighting()
{
    const auto foldings = m_view->doc()->buffer().computeFoldings(m_cursor.line());
    for (unsigned long i = 0; i < foldings.size(); i++) {
        // 1 -> left to right, the current folding is start type
        // -1 -> right to left, the current folding is end type
        int direction = (foldings[i].foldingRegion.type() == KSyntaxHighlighting::FoldingRegion::Begin) ? 1 : -1;

        int startOffset = getStartOffset(-direction, foldings[i].offset, foldings[i].length);
        int endOffset = getEndOffset(-direction, foldings[i].offset, foldings[i].length);

        if (m_cursor.column() >= startOffset && m_cursor.column() <= endOffset) {
            const auto foldingMarkerMatch = findMatchingFoldingMarker(KTextEditor::Cursor(m_cursor.line(), m_cursor.column()), foldings[i].foldingRegion, 2000);

            if (!foldingMarkerMatch.isValid()) {
                break;
            }

            // set fmStart to Opening Folding Marker and fmEnd to Ending Folding Marker
            if (direction == 1) {
                m_fmStart->setRange(KTextEditor::Range(m_cursor.line(), startOffset, m_cursor.line(), endOffset));
                m_fmEnd->setRange(foldingMarkerMatch);
            } else {
                m_fmStart->setRange(foldingMarkerMatch);
                m_fmEnd->setRange(KTextEditor::Range(m_cursor.line(), startOffset, m_cursor.line(), endOffset));
            }

            KTextEditor::Attribute::Ptr fill = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
            fill->setBackground(view()->rendererConfig()->highlightedBracketColor());

            m_fmStart->setAttribute(fill);
            m_fmEnd->setAttribute(fill);
            return;
        }
    }
    m_fmStart->setRange(KTextEditor::Range::invalid());
    m_fmEnd->setRange(KTextEditor::Range::invalid());
}

void KateViewInternal::updateSecondaryCursors(const QVarLengthArray<CursorPair, 16> &cursors, bool sel)
{
    if (sel) {
        for (int i = 0; i < cursors.size(); ++i) {
            updateSecondarySelection(i, cursors[i].oldPos, cursors[i].newPos);
        }
        if (!cursors.isEmpty()) {
            mergeSelections();
        }
    } else {
        view()->clearSecondarySelections();
    }

    QVarLengthArray<int> linesToUpdate;
    for (auto cpair : cursors) {
        linesToUpdate.push_back(cpair.oldPos.line());
        linesToUpdate.push_back(cpair.newPos.line());
    }
    // Remove duplicate stuff
    std::sort(linesToUpdate.begin(), linesToUpdate.end());
    auto it = std::unique(linesToUpdate.begin(), linesToUpdate.end());

    // Collapse ranges to avoid extra work
    using Range = std::pair<int, int>; // start, length
    QVarLengthArray<Range> ranges;
    int prev = 0;
    for (auto i = linesToUpdate.begin(); i != it; ++i) {
        int curLine = *i;
        if (!ranges.isEmpty() && prev + 1 == curLine) {
            ranges.back().second++;
        } else {
            ranges.push_back({curLine, 0});
        }
        prev = curLine;
    }

    for (auto range : ranges) {
        int startLine = range.first;
        int endLine = range.first + range.second;
        tagLines(startLine, endLine, /*realLines=*/true);
    }
    updateDirty();
}

void KateViewInternal::mergeSelections()
{
    using SecondaryCursor = KTextEditor::ViewPrivate::SecondaryCursor;
    using Range = KTextEditor::Range;
    auto doMerge = [](Range newRange, SecondaryCursor &a, SecondaryCursor &b) {
        a.range->setRange(newRange);

        b.pos.reset();
        b.range.reset();
    };

    auto &cursors = view()->m_secondaryCursors;
    for (auto it = cursors.begin(); it != cursors.end(); ++it) {
        if (!it->range)
            continue;
        if (it + 1 == cursors.end()) {
            break;
        }

        auto n = std::next(it);
        if (/*!it->range || */ !n->range) {
            continue;
        }

        auto curRange = it->range->toRange();
        auto nextRange = n->range->toRange();
        if (!curRange.overlaps(nextRange)) {
            continue;
        }

        bool isLefSel = it->cursor() < it->anchor;

        // if the ranges overlap we expand the next range
        // to include the current one. This allows us to
        // check all ranges for overlap in one go
        auto curPos = it->cursor();
        nextRange.expandToRange(curRange);
        if (isLefSel) {
            // in left selection our next cursor
            // is ahead somewhere, we want to keep
            // the smallest position
            n->pos->setPosition(curPos);
            n->anchor = qMax(n->anchor, it->anchor);
        } else {
            n->anchor = qMin(n->anchor, it->anchor);
        }
        doMerge(nextRange, *n, *it);
    }

    if (view()->selection()) {
        auto primarySel = view()->m_selection.toRange();
        auto primCursor = cursorPosition();
        for (auto it = cursors.begin(); it != cursors.end(); ++it) {
            // If range is valid, we merge selection into primary
            // Otherwise if cursor is inside primary selection, it
            // is removed
            auto curRange = it->range ? it->range->toRange() : Range::invalid();
            if (curRange.isValid() && primarySel.overlaps(curRange)) {
                primarySel.expandToRange(curRange);
                bool isLeft = it->cursor() < it->anchor;

                if (isLeft) {
                    if (it->cursor() < primCursor) {
                        updateCursor(it->cursor());
                    }
                    m_selectAnchor = qMax(m_selectAnchor, it->anchor);
                } else {
                    if (it->cursor() > primCursor) {
                        updateCursor(it->cursor());
                    }
                    m_selectAnchor = qMin(m_selectAnchor, it->anchor);
                }

                setSelection(primarySel);
                it->pos.reset();
                it->range.reset();
            } else if (it->pos) {
                // This only needs to be done for primary selection
                // because remember, mouse selection is always with
                // primary cursor
                auto pos = it->cursor();
                if (!primarySel.boundaryAtCursor(pos) && primarySel.contains(pos)) {
                    it->pos.reset();
                }
            }
        }
    }

    cursors.erase(std::remove_if(cursors.begin(),
                                 cursors.end(),
                                 [](const SecondaryCursor &c) {
                                     return !c.pos.get();
                                 }),
                  cursors.end());
}

void KateViewInternal::updateCursor(const KTextEditor::Cursor newCursor, bool force, bool center, bool calledExternally, bool scroll)
{
    if (!force && (m_cursor.toCursor() == newCursor)) {
        m_displayCursor = toVirtualCursor(newCursor);
        if (scroll && !m_madeVisible && m_view == doc()->activeView()) {
            // unfold if required
            view()->textFolding().ensureLineIsVisible(newCursor.line());

            makeVisible(m_displayCursor, m_displayCursor.column(), false, center, calledExternally);
        }

        return;
    }

    if (m_cursor.line() != newCursor.line()) {
        m_leftBorder->updateForCursorLineChange();
    }

    // unfold if required
    view()->textFolding().ensureLineIsVisible(newCursor.line());

    KTextEditor::Cursor oldDisplayCursor = m_displayCursor;

    m_displayCursor = toVirtualCursor(newCursor);
    m_cursor.setPosition(newCursor);

    if (m_view == doc()->activeView() && scroll) {
        makeVisible(m_displayCursor, m_displayCursor.column(), false, center, calledExternally);
    }

    updateBracketMarks();

    updateFoldingMarkersHighlighting();

    // avoid double work, tagLine => tagLines => not that cheap, much more costly than to compare 2 ints
    tagLine(oldDisplayCursor);
    if (oldDisplayCursor.line() != m_displayCursor.line()) {
        tagLine(m_displayCursor);
    }

    updateMicroFocus();

    if (m_cursorTimer.isActive()) {
        if (QApplication::cursorFlashTime() > 0) {
            m_cursorTimer.start(QApplication::cursorFlashTime() / 2);
        }
        renderer()->setDrawCaret(true);
    }

    // Remember the maximum X position if requested
    if (m_preserveX) {
        m_preserveX = false;
    } else {
        m_preservedX = renderer()->cursorToX(cache()->textLayout(m_cursor), m_cursor, !view()->wrapCursor());
    }

    // qCDebug(LOG_KTE) << "m_preservedX: " << m_preservedX << " (was "<< oldmaxx << "), m_cursorX: " << m_cursorX;
    // qCDebug(LOG_KTE) << "Cursor now located at real " << cursor.line << "," << cursor.col << ", virtual " << m_displayCursor.line << ", " <<
    // m_displayCursor.col << "; Top is " << startLine() << ", " << startPos().col;

    cursorMoved();

    updateDirty(); // paintText(0, 0, width(), height(), true);

    Q_EMIT view()->cursorPositionChanged(m_view, m_cursor);
}

void KateViewInternal::updateBracketMarkAttributes()
{
    KTextEditor::Attribute::Ptr bracketFill = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    bracketFill->setBackground(view()->rendererConfig()->highlightedBracketColor());
    bracketFill->setBackgroundFillWhitespace(false);
    if (QFontInfo(renderer()->currentFont()).fixedPitch()) {
        // make font bold only for fixed fonts, otherwise text jumps around
        bracketFill->setFontBold();
    }

    m_bmStart->setAttribute(bracketFill);
    m_bmEnd->setAttribute(bracketFill);

    if (view()->rendererConfig()->showWholeBracketExpression()) {
        KTextEditor::Attribute::Ptr expressionFill = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
        expressionFill->setBackground(view()->rendererConfig()->highlightedBracketColor());
        expressionFill->setBackgroundFillWhitespace(false);

        m_bm->setAttribute(expressionFill);
    } else {
        m_bm->setAttribute(KTextEditor::Attribute::Ptr(new KTextEditor::Attribute()));
    }
}

void KateViewInternal::updateBracketMarks()
{
    // add some limit to this, this is really endless on big files without limit
    const int maxLines = 5000;
    const KTextEditor::Range newRange = doc()->findMatchingBracket(m_cursor, maxLines);

    // new range valid, then set ranges to it
    if (newRange.isValid()) {
        if (m_bm->toRange() == newRange) {
            // hide preview as it now (probably) blocks the top of the view
            hideBracketMatchPreview();
            return;
        }

        // modify full range
        m_bm->setRange(newRange);

        // modify start and end ranges
        m_bmStart->setRange(KTextEditor::Range(m_bm->start(), KTextEditor::Cursor(m_bm->start().line(), m_bm->start().column() + 1)));
        m_bmEnd->setRange(KTextEditor::Range(m_bm->end(), KTextEditor::Cursor(m_bm->end().line(), m_bm->end().column() + 1)));

        // show preview of the matching bracket's line
        if (m_view->config()->value(KateViewConfig::ShowBracketMatchPreview).toBool()) {
            showBracketMatchPreview();
        }

        // flash matching bracket
        if (!m_view->rendererConfig()->animateBracketMatching()) {
            return;
        }

        const KTextEditor::Cursor flashPos = (m_cursor == m_bmStart->start() || m_cursor == m_bmStart->end()) ? m_bmEnd->start() : m_bm->start();
        if (flashPos != m_bmLastFlashPos->toCursor()) {
            m_bmLastFlashPos->setPosition(flashPos);

            KTextEditor::Attribute::Ptr attribute = attributeAt(flashPos);
            attribute->setBackground(view()->rendererConfig()->highlightedBracketColor());
            if (m_bmStart->attribute()->fontBold()) {
                attribute->setFontBold(true);
            }

            flashChar(flashPos, attribute);
        }
        return;
    }

    // new range was invalid
    m_bm->setRange(KTextEditor::Range::invalid());
    m_bmStart->setRange(KTextEditor::Range::invalid());
    m_bmEnd->setRange(KTextEditor::Range::invalid());
    m_bmLastFlashPos->setPosition(KTextEditor::Cursor::invalid());
    hideBracketMatchPreview();
}

bool KateViewInternal::tagLine(const KTextEditor::Cursor virtualCursor)
{
    // we had here some special case handling for one line, it was just randomly wrong for dyn. word wrapped stuff => use the generic function
    return tagLines(virtualCursor, virtualCursor, false);
}

bool KateViewInternal::tagLines(int start, int end, bool realLines)
{
    return tagLines(KTextEditor::Cursor(start, 0), KTextEditor::Cursor(end, -1), realLines);
}

bool KateViewInternal::tagLines(KTextEditor::Cursor start, KTextEditor::Cursor end, bool realCursors)
{
    if (realCursors) {
        cache()->relayoutLines(start.line(), end.line());

        // qCDebug(LOG_KTE)<<"realLines is true";
        start = toVirtualCursor(start);
        end = toVirtualCursor(end);

    } else {
        cache()->relayoutLines(toRealCursor(start).line(), toRealCursor(end).line());
    }

    if (end.line() < startLine()) {
        // qCDebug(LOG_KTE)<<"end<startLine";
        return false;
    }
    // Used to be > endLine(), but cache may not be valid when checking, so use a
    // less optimal but still adequate approximation (potential overestimation but minimal performance difference)
    if (start.line() > startLine() + cache()->viewCacheLineCount()) {
        // qCDebug(LOG_KTE)<<"start> endLine"<<start<<" "<<(endLine());
        return false;
    }

    cache()->updateViewCache(startPos());

    // qCDebug(LOG_KTE) << "tagLines( [" << start << "], [" << end << "] )";

    bool ret = false;

    for (int z = 0; z < cache()->viewCacheLineCount(); z++) {
        KateTextLayout &line = cache()->viewLine(z);
        if ((line.virtualLine() > start.line() || (line.virtualLine() == start.line() && line.endCol() >= start.column() && start.column() != -1))
            && (line.virtualLine() < end.line() || (line.virtualLine() == end.line() && (line.startCol() <= end.column() || end.column() == -1)))) {
            ret = true;
            break;
            // qCDebug(LOG_KTE) << "Tagged line " << line.line();
        }
    }

    // full update of border it we have indentation based hl and show the markers always
    if (!m_view->config()->showFoldingOnHoverOnly() && doc()->highlight() && doc()->highlight()->foldingIndentationSensitive()) {
        // not the default setting, can be optimized if ever some issue
        m_leftBorder->update();
    } else if (!view()->dynWordWrap()) {
        int y = lineToY(start.line());
        // FIXME is this enough for when multiple lines are deleted
        int h = (end.line() - start.line() + 2) * renderer()->lineHeight();
        if (end.line() >= view()->textFolding().visibleLines() - 1) {
            h = height();
        }

        m_leftBorder->update(0, y, m_leftBorder->width(), h);
    } else {
        // FIXME Do we get enough good info in editRemoveText to optimize this more?
        // bool justTagged = false;
        for (int z = 0; z < cache()->viewCacheLineCount(); z++) {
            KateTextLayout &line = cache()->viewLine(z);
            if (!line.isValid()
                || ((line.virtualLine() > start.line() || (line.virtualLine() == start.line() && line.endCol() >= start.column() && start.column() != -1))
                    && (line.virtualLine() < end.line() || (line.virtualLine() == end.line() && (line.startCol() <= end.column() || end.column() == -1))))) {
                // justTagged = true;
                m_leftBorder->update(0, z * renderer()->lineHeight(), m_leftBorder->width(), m_leftBorder->height());
                break;
            }
            /*else if (justTagged)
            {
              justTagged = false;
              leftBorder->update (0, z * doc()->viewFont.fontHeight, leftBorder->width(), doc()->viewFont.fontHeight);
              break;
            }*/
        }
    }

    return ret;
}

bool KateViewInternal::tagRange(KTextEditor::Range range, bool realCursors)
{
    return tagLines(range.start(), range.end(), realCursors);
}

void KateViewInternal::tagAll()
{
    // clear the cache...
    cache()->clear();

    m_leftBorder->updateFont();
    m_leftBorder->update();
}

void KateViewInternal::paintCursor()
{
    QVarLengthArray<int, 64> updatedLines;
    if (tagLine(m_displayCursor)) {
        updatedLines.push_back(m_displayCursor.line());
    }

    const int s = view()->firstDisplayedLine();
    const int e = view()->lastDisplayedLine();
    for (const auto &c : view()->m_secondaryCursors) {
        auto p = c.cursor();
        if (p.line() >= s - 1 && p.line() <= e + 1 && !updatedLines.contains(p.line())) {
            updatedLines.push_back(p.line());
            tagLines(p, p, true);
        }
    }

    if (!updatedLines.isEmpty()) {
        updateDirty(); // paintText (0,0,width(), height(), true);
    }
}

KTextEditor::Cursor KateViewInternal::cursorForPoint(QPoint p)
{
    KateTextLayout thisLine = yToKateTextLayout(p.y());
    KTextEditor::Cursor c;

    if (!thisLine.isValid()) { // probably user clicked below the last line -> use the last line
        thisLine = cache()->textLayout(doc()->lines() - 1, -1);
    }

    c = renderer()->xToCursor(thisLine, startX() + p.x(), !view()->wrapCursor());

    if (c.line() < 0 || c.line() >= doc()->lines()) {
        return KTextEditor::Cursor::invalid();
    }

    // loop over all notes and check if the point is inside it
    const auto inlineNotes = view()->inlineNotes(c.line());
    p = mapToGlobal(p);
    for (const auto &note : inlineNotes) {
        auto noteCursor = note.m_position;
        // we are not interested in notes that are past the end or at 0
        if (note.m_position.column() >= doc()->lineLength(c.line()) || note.m_position.column() == 0) {
            continue;
        }
        // an inline note is "part" of a char i.e., char width is increased so that
        // an inline note can be painted beside it. So we need to find the actual
        // char width
        const auto caretWidth = renderer()->caretStyle() == KTextEditor::caretStyles::Line ? 2. : 0.;
        const auto width = KTextEditor::InlineNote(note).width() + caretWidth;
        const auto charWidth = renderer()->currentFontMetrics().horizontalAdvance(doc()->characterAt(noteCursor));
        const auto halfCharWidth = (charWidth / 2);
        // we leave the first half of the char width. If the user has clicked in the first half, let it go the
        // previous column.
        const auto totalWidth = width + halfCharWidth;
        auto start = mapToGlobal(cursorToCoordinate(noteCursor, true, false));
        start = start - QPoint(totalWidth, 0);
        QRect r(start, QSize{(int)halfCharWidth, renderer()->lineHeight()});
        if (r.contains(p)) {
            c = noteCursor;
            break;
        }
    }

    return c;
}

// Point in content coordinates
void KateViewInternal::placeCursor(const QPoint &p, bool keepSelection, bool updateSelection)
{
    KTextEditor::Cursor c = cursorForPoint(p);
    if (!c.isValid()) {
        return;
    }

    if (updateSelection) {
        KateViewInternal::updateSelection(c, keepSelection);
    }

    int tmp = m_minLinesVisible;
    m_minLinesVisible = 0;
    updateCursor(c);
    m_minLinesVisible = tmp;

    if (updateSelection && keepSelection) {
        moveCursorToSelectionEdge();
    }
}

// Point in content coordinates
bool KateViewInternal::isTargetSelected(const QPoint &p)
{
    const KateTextLayout &thisLine = yToKateTextLayout(p.y());
    if (!thisLine.isValid()) {
        return false;
    }

    return view()->cursorSelected(renderer()->xToCursor(thisLine, startX() + p.x(), !view()->wrapCursor()));
}

// BEGIN EVENT HANDLING STUFF

bool KateViewInternal::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved: {
        QChildEvent *c = static_cast<QChildEvent *>(e);
        if (c->added()) {
            c->child()->installEventFilter(this);

        } else if (c->removed()) {
            c->child()->removeEventFilter(this);
        }
    } break;

    case QEvent::ShortcutOverride: {
        QKeyEvent *k = static_cast<QKeyEvent *>(e);

        if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
            if (!view()->m_secondaryCursors.empty()) {
                view()->clearSecondaryCursors();
                k->accept();
                return true;
            }

            if (view()->isCompletionActive()) {
                view()->abortCompletion();
                k->accept();
                // qCDebug(LOG_KTE) << obj << "shortcut override" << k->key() << "aborting completion";
                return true;
            } else if (view()->bottomViewBar()->barWidgetVisible()) {
                view()->bottomViewBar()->hideCurrentBarWidget();
                k->accept();
                // qCDebug(LOG_KTE) << obj << "shortcut override" << k->key() << "closing view bar";
                return true;
            } else if (!view()->config()->persistentSelection() && view()->selection()) {
                m_currentInputMode->clearSelection();
                k->accept();
                // qCDebug(LOG_KTE) << obj << "shortcut override" << k->key() << "clearing selection";
                return true;
            }
        }

        if (k->modifiers() == Qt::AltModifier && view()->isCompletionActive()) {
            if (view()->completionWidget()->handleShortcutOverride(k)) {
                k->accept();
                return true;
            }
        }

        if (m_currentInputMode->stealKey(k)) {
            k->accept();
            return true;
        }

        // CompletionReplayer.replay only gets called when a Ctrl-Space gets to InsertViMode::handleKeyPress
        // Workaround for BUG: 334032 (https://bugs.kde.org/show_bug.cgi?id=334032)
        if (k->key() == Qt::Key_Space && k->modifiers() == Qt::ControlModifier) {
            keyPressEvent(k);
            if (k->isAccepted()) {
                return true;
            }
        }
    } break;

    case QEvent::KeyPress: {
        QKeyEvent *k = static_cast<QKeyEvent *>(e);

        // Override all other single key shortcuts which do not use a modifier other than Shift
        if (obj == this && (!k->modifiers() || k->modifiers() == Qt::ShiftModifier)) {
            keyPressEvent(k);
            if (k->isAccepted()) {
                // qCDebug(LOG_KTE) << obj << "shortcut override" << k->key() << "using keystroke";
                return true;
            }
        }

        // qCDebug(LOG_KTE) << obj << "shortcut override" << k->key() << "ignoring";
    } break;

    case QEvent::DragMove: {
        QPoint currentPoint = ((QDragMoveEvent *)e)->position().toPoint();

        QRect doNotScrollRegion(s_scrollMargin, s_scrollMargin, width() - s_scrollMargin * 2, height() - s_scrollMargin * 2);

        if (!doNotScrollRegion.contains(currentPoint)) {
            startDragScroll();
            // Keep sending move events
            ((QDragMoveEvent *)e)->accept(QRect(0, 0, 0, 0));
        }

        dragMoveEvent((QDragMoveEvent *)e);
    } break;

    case QEvent::DragLeave:
        // happens only when pressing ESC while dragging
        stopDragScroll();
        break;

    case QEvent::WindowDeactivate:
        hideBracketMatchPreview();
        break;

    case QEvent::ScrollPrepare: {
        QScrollPrepareEvent *s = static_cast<QScrollPrepareEvent *>(e);
        scrollPrepareEvent(s);
    }
        return true;

    case QEvent::Scroll: {
        QScrollEvent *s = static_cast<QScrollEvent *>(e);
        scrollEvent(s);
    }
        return true;

    default:
        break;
    }

    return QWidget::eventFilter(obj, e);
}

void KateViewInternal::keyPressEvent(QKeyEvent *e)
{
    m_shiftKeyPressed = e->modifiers() & Qt::ShiftModifier;
    if (e->key() == Qt::Key_Left && e->modifiers() == Qt::AltModifier) {
        view()->emitNavigateLeft();
        e->setAccepted(true);
        return;
    }
    if (e->key() == Qt::Key_Right && e->modifiers() == Qt::AltModifier) {
        view()->emitNavigateRight();
        e->setAccepted(true);
        return;
    }
    if (e->key() == Qt::Key_Up && e->modifiers() == Qt::AltModifier) {
        view()->emitNavigateUp();
        e->setAccepted(true);
        return;
    }
    if (e->key() == Qt::Key_Down && e->modifiers() == Qt::AltModifier) {
        view()->emitNavigateDown();
        e->setAccepted(true);
        return;
    }
    if (e->key() == Qt::Key_Return && e->modifiers() == Qt::AltModifier) {
        view()->emitNavigateAccept();
        e->setAccepted(true);
        return;
    }
    if (e->key() == Qt::Key_Backspace && e->modifiers() == Qt::AltModifier) {
        view()->emitNavigateBack();
        e->setAccepted(true);
        return;
    }

    if (e->key() == Qt::Key_Alt && view()->completionWidget()->isCompletionActive()) {
        view()->completionWidget()->toggleDocumentation();
    }

    // Note: AND'ing with <Shift> is a quick hack to fix Key_Enter
    const int key = e->key() | (e->modifiers() & Qt::ShiftModifier);

    if (m_currentInputMode->keyPress(e)) {
        return;
    }

    if (!doc()->isReadWrite()) {
        e->ignore();
        return;
    }

    if ((key == Qt::Key_Return) || (key == Qt::Key_Enter) || key == (Qt::SHIFT | Qt::Key_Return).toCombined()
        || key == (Qt::SHIFT | Qt::Key_Enter).toCombined()) {
        view()->keyReturn();
        e->accept();
        return;
    }

    if (key == Qt::Key_Backspace || key == (Qt::SHIFT | Qt::Key_Backspace).toCombined()) {
        // view()->backspace();
        e->accept();

        return;
    }

    if (key == Qt::Key_Tab || key == (Qt::SHIFT | Qt::Key_Backtab).toCombined() || key == Qt::Key_Backtab) {
        if (key == Qt::Key_Tab) {
            uint tabHandling = doc()->config()->tabHandling();
            // convert tabSmart into tabInsertsTab or tabIndents:
            if (tabHandling == KateDocumentConfig::tabSmart) {
                // multiple lines selected
                if (view()->selection() && !view()->selectionRange().onSingleLine()) {
                    tabHandling = KateDocumentConfig::tabIndents;
                }

                // otherwise: take look at cursor position
                else {
                    // if the cursor is at or before the first non-space character
                    // or on an empty line,
                    // Tab indents, otherwise it inserts a tab character.
                    Kate::TextLine line = doc()->kateTextLine(m_cursor.line());
                    int first = line.firstChar();
                    if (first < 0 || m_cursor.column() <= first) {
                        tabHandling = KateDocumentConfig::tabIndents;
                    } else {
                        tabHandling = KateDocumentConfig::tabInsertsTab;
                    }
                }
            }

            // either we just insert a tab or we convert that into an indent action
            if (tabHandling == KateDocumentConfig::tabInsertsTab) {
                doc()->typeChars(m_view, QStringLiteral("\t"));
            } else {
                doc()->editStart();
                for (const auto &c : std::as_const(m_view->m_secondaryCursors)) {
                    auto cursor = c.cursor();
                    doc()->indent(KTextEditor::Range(cursor.line(), 0, cursor.line(), 0), 1);
                }

                doc()->indent(view()->selection() ? view()->selectionRange() : KTextEditor::Range(m_cursor.line(), 0, m_cursor.line(), 0), 1);
                doc()->editEnd();
            }

            e->accept();

            return;
        } else if (doc()->config()->tabHandling() != KateDocumentConfig::tabInsertsTab) {
            // key == Qt::SHIFT+Qt::Key_Backtab || key == Qt::Key_Backtab
            doc()->indent(view()->selection() ? view()->selectionRange() : KTextEditor::Range(m_cursor.line(), 0, m_cursor.line(), 0), -1);
            e->accept();

            return;
        }
    }

    if (isAcceptableInput(e)) {
        doc()->typeChars(m_view, e->text());
        e->accept();
        return;
    }

    e->ignore();
}

void KateViewInternal::keyReleaseEvent(QKeyEvent *e)
{
    if (m_shiftKeyPressed && (e->modifiers() & Qt::ShiftModifier) == 0) {
        m_shiftKeyPressed = false;

        if (m_selChangedByUser) {
            if (view()->selection()) {
                QApplication::clipboard()->setText(view()->selectionText(), QClipboard::Selection);
            }

            m_selChangedByUser = false;
        }
    }

    e->ignore();
    return;
}

bool KateViewInternal::isAcceptableInput(const QKeyEvent *e)
{
    // reimplemented from QInputControl::isAcceptableInput()

    const QString text = e->text();
    if (text.isEmpty()) {
        return false;
    }

    const QChar c = text.at(0);

    // Formatting characters such as ZWNJ, ZWJ, RLM, etc. This needs to go before the
    // next test, since CTRL+SHIFT is sometimes used to input it on Windows.
    // see bug 389796 (typing formatting characters such as ZWNJ)
    // and bug 396764 (typing soft-hyphens)
    if (c.category() == QChar::Other_Format) {
        return true;
    }

    // QTBUG-35734: ignore Ctrl/Ctrl+Shift; accept only AltGr (Alt+Ctrl) on German keyboards
    if ((e->modifiers() == Qt::ControlModifier) || (e->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier))) {
        return false;
    }

    if (c.isPrint() || (c.category() == QChar::Other_PrivateUse)) {
        return true;
    };

    if (c.isHighSurrogate() && text.size() > 1 && text.at(1).isLowSurrogate()) {
        return true;
    }

    return false;
}

void KateViewInternal::contextMenuEvent(QContextMenuEvent *e)
{
    // calculate where to show the context menu

    QPoint p = e->pos();

    if (e->reason() == QContextMenuEvent::Keyboard) {
        makeVisible(m_displayCursor, 0);
        p = cursorCoordinates(false);
        p.rx() -= startX();
    } else if (!view()->selection() || view()->config()->persistentSelection()) {
        placeCursor(e->pos());
    }

    // show it
    QMenu *cm = view()->contextMenu();
    if (cm) {
        view()->spellingMenu()->prepareToBeShown(cm);
        cm->popup(mapToGlobal(p));
        e->accept();
    }
}

void KateViewInternal::mousePressEvent(QMouseEvent *e)
{
    if (sendMouseEventToInputContext(e)) {
        return;
    }

    // was an inline note clicked?
    const auto noteData = inlineNoteAt(e->globalPosition().toPoint());
    const KTextEditor::InlineNote note(noteData);
    if (note.position().isValid()) {
        note.provider()->inlineNoteActivated(noteData, e->button(), e->globalPosition().toPoint());
        return;
    }

    commitPreedit();

    // no -- continue with normal handling
    switch (e->button()) {
    case Qt::LeftButton:

        m_selChangedByUser = false;

        if (!view()->isMulticursorNotAllowed() && e->modifiers() == view()->config()->multiCursorModifiers()) {
            auto pos = cursorForPoint(e->pos());
            if (pos.isValid()) {
                view()->addSecondaryCursor(pos);
                e->accept();
                return;
            }
        } else {
            view()->clearSecondaryCursors();
        }

        if (m_possibleTripleClick) {
            m_possibleTripleClick = false;

            m_selectionMode = Line;

            if (e->modifiers() & Qt::ShiftModifier) {
                updateSelection(m_cursor, true);
            } else {
                view()->selectLine(m_cursor);
                if (view()->selection()) {
                    m_selectAnchor = view()->selectionRange().start();
                }
            }

            if (view()->selection()) {
                QApplication::clipboard()->setText(view()->selectionText(), QClipboard::Selection);
            }

            // Keep the line at the select anchor selected during further
            // mouse selection
            if (m_selectAnchor.line() > view()->selectionRange().start().line()) {
                // Preserve the last selected line
                if (m_selectAnchor == view()->selectionRange().end() && m_selectAnchor.column() == 0) {
                    m_selectionCached.setStart(KTextEditor::Cursor(m_selectAnchor.line() - 1, 0));
                } else {
                    m_selectionCached.setStart(KTextEditor::Cursor(m_selectAnchor.line(), 0));
                }
                m_selectionCached.setEnd(view()->selectionRange().end());
            } else {
                // Preserve the first selected line
                m_selectionCached.setStart(view()->selectionRange().start());
                if (view()->selectionRange().end().line() > view()->selectionRange().start().line()) {
                    m_selectionCached.setEnd(KTextEditor::Cursor(view()->selectionRange().start().line() + 1, 0));
                } else {
                    m_selectionCached.setEnd(view()->selectionRange().end());
                }
            }

            moveCursorToSelectionEdge();

            m_scrollX = 0;
            m_scrollY = 0;
            m_scrollTimer.start(50);

            e->accept();
            return;
        } else if (m_selectionMode == Default) {
            m_selectionMode = Mouse;
        }

        // request the software keyboard, if any
        if (e->button() == Qt::LeftButton && qApp->autoSipEnabled()) {
            QStyle::RequestSoftwareInputPanel behavior = QStyle::RequestSoftwareInputPanel(style()->styleHint(QStyle::SH_RequestSoftwareInputPanel));
            if (hasFocus() || behavior == QStyle::RSIP_OnMouseClick) {
                QEvent event(QEvent::RequestSoftwareInputPanel);
                QApplication::sendEvent(this, &event);
            }
        }

        if (e->modifiers() & Qt::ShiftModifier) {
            if (!m_selectAnchor.isValid()) {
                m_selectAnchor = m_cursor;
            }
        } else {
            m_selectionCached = KTextEditor::Range::invalid();
        }

        if (view()->config()->textDragAndDrop() && !(e->modifiers() & Qt::ShiftModifier) && isTargetSelected(e->pos())) {
            m_dragInfo.state = diPending;
            m_dragInfo.start = e->pos();
        } else {
            m_dragInfo.state = diNone;

            if (e->modifiers() & Qt::ShiftModifier) {
                placeCursor(e->pos(), true, false);
                if (m_selectionCached.start().isValid()) {
                    if (m_cursor.toCursor() < m_selectionCached.start()) {
                        m_selectAnchor = m_selectionCached.end();
                    } else {
                        m_selectAnchor = m_selectionCached.start();
                    }
                }

                // update selection and do potential clipboard update, see bug 443642
                setSelection(KTextEditor::Range(m_selectAnchor, m_cursor));
                if (view()->selection()) {
                    QApplication::clipboard()->setText(view()->selectionText(), QClipboard::Selection);
                }
            } else {
                placeCursor(e->pos());
            }

            m_scrollX = 0;
            m_scrollY = 0;

            m_scrollTimer.start(50);
        }

        e->accept();
        break;

    case Qt::RightButton:
        if (e->pos().x() == 0) {
            // Special handling for folding by right click
            placeCursor(e->pos());
            e->accept();
        }
        break;

    default:
        e->ignore();
        break;
    }
}

void KateViewInternal::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (sendMouseEventToInputContext(e)) {
        return;
    }
    if (e->button() == Qt::LeftButton) {
        m_selectionMode = Word;

        if (e->modifiers() & Qt::ShiftModifier) {
            // Now select the word under the select anchor
            int cs;
            int ce;
            Kate::TextLine l = doc()->kateTextLine(m_selectAnchor.line());

            ce = m_selectAnchor.column();
            if (ce > 0 && doc()->highlight()->isInWord(l.at(ce))) {
                for (; ce < l.length(); ce++) {
                    if (!doc()->highlight()->isInWord(l.at(ce))) {
                        break;
                    }
                }
            }

            cs = m_selectAnchor.column() - 1;
            if (cs < doc()->lineLength(m_selectAnchor.line()) && doc()->highlight()->isInWord(l.at(cs))) {
                for (cs--; cs >= 0; cs--) {
                    if (!doc()->highlight()->isInWord(l.at(cs))) {
                        break;
                    }
                }
            }

            // ...and keep it selected
            if (cs + 1 < ce) {
                m_selectionCached.setStart(KTextEditor::Cursor(m_selectAnchor.line(), cs + 1));
                m_selectionCached.setEnd(KTextEditor::Cursor(m_selectAnchor.line(), ce));
            } else {
                m_selectionCached.setStart(m_selectAnchor);
                m_selectionCached.setEnd(m_selectAnchor);
            }
            // Now word select to the mouse cursor
            placeCursor(e->pos(), true);
        } else {
            // first clear the selection, otherwise we run into bug #106402
            // ...and set the cursor position, for the same reason (otherwise there
            // are *other* idiosyncrasies we can't fix without reintroducing said
            // bug)
            // Parameters: don't redraw, and don't emit selectionChanged signal yet
            view()->clearSelection(false, false);
            placeCursor(e->pos());
            view()->selectWord(m_cursor);
            cursorToMatchingBracket(true);

            if (view()->selection()) {
                m_selectAnchor = view()->selectionRange().start();
                m_selectionCached = view()->selectionRange();
            } else {
                m_selectAnchor = m_cursor;
                m_selectionCached = KTextEditor::Range(m_cursor, m_cursor);
            }
        }

        // Move cursor to end (or beginning) of selected word
#ifndef Q_OS_MACOS
        if (view()->selection()) {
            QApplication::clipboard()->setText(view()->selectionText(), QClipboard::Selection);
        }
#endif

        moveCursorToSelectionEdge();
        m_possibleTripleClick = true;
        QTimer::singleShot(QApplication::doubleClickInterval(), this, SLOT(tripleClickTimeout()));

        m_scrollX = 0;
        m_scrollY = 0;

        m_scrollTimer.start(50);

        e->accept();
    } else {
        e->ignore();
    }
}

void KateViewInternal::tripleClickTimeout()
{
    m_possibleTripleClick = false;
}

void KateViewInternal::beginSelectLine(const QPoint &pos)
{
    placeCursor(pos);
    m_possibleTripleClick = true; // set so subsequent mousePressEvent will select line
}

void KateViewInternal::mouseReleaseEvent(QMouseEvent *e)
{
    if (sendMouseEventToInputContext(e)) {
        return;
    }

    switch (e->button()) {
    case Qt::LeftButton:
        m_selectionMode = Default;
        //       m_selectionCached.start().setLine( -1 );

        if (m_selChangedByUser) {
            if (view()->selection()) {
                QApplication::clipboard()->setText(view()->selectionText(), QClipboard::Selection);
            }
            moveCursorToSelectionEdge();

            m_selChangedByUser = false;
        }

        if (m_dragInfo.state == diPending) {
            placeCursor(e->pos(), e->modifiers() & Qt::ShiftModifier);
        } else if (m_dragInfo.state == diNone) {
            m_scrollTimer.stop();
        }

        m_dragInfo.state = diNone;

        // merge any overlapping selections/cursors
        if (view()->selection() && !view()->m_secondaryCursors.empty()) {
            mergeSelections();
        }

        e->accept();
        break;

    case Qt::MiddleButton:
        if (!view()->config()->mousePasteAtCursorPosition()) {
            placeCursor(e->pos());
        }

        if (doc()->isReadWrite()) {
            QString clipboard = QApplication::clipboard()->text(QClipboard::Selection);
            view()->paste(&clipboard);
        }

        e->accept();
        break;

    default:
        e->ignore();
        break;
    }
}

void KateViewInternal::leaveEvent(QEvent *)
{
    m_textHintTimer.stop();

    // fix bug 194452, scrolling keeps going if you scroll via mouse drag and press and other mouse
    // button outside the view area
    if (m_dragInfo.state == diNone) {
        m_scrollTimer.stop();
    }

    hideBracketMatchPreview();
}

KTextEditor::Cursor KateViewInternal::coordinatesToCursor(const QPoint &_coord, bool includeBorder) const
{
    QPoint coord(_coord);

    KTextEditor::Cursor ret = KTextEditor::Cursor::invalid();

    if (includeBorder) {
        coord.rx() -= m_leftBorder->width();
    }
    coord.rx() += startX();

    const KateTextLayout &thisLine = yToKateTextLayout(coord.y());
    if (thisLine.isValid()) {
        ret = renderer()->xToCursor(thisLine, coord.x(), !view()->wrapCursor());
    }

    if (ret.column() > view()->document()->lineLength(ret.line())) {
        // The cursor is beyond the end of the line; in that case the renderer
        // gives the index of the character behind the last one.
        return KTextEditor::Cursor::invalid();
    }

    return ret;
}

void KateViewInternal::mouseMoveEvent(QMouseEvent *e)
{
    if (m_scroller->state() != QScroller::Inactive) {
        // Touchscreen is handled by scrollEvent()
        return;
    }
    KTextEditor::Cursor newPosition = coordinatesToCursor(e->pos(), false);
    if (newPosition != m_mouse) {
        m_mouse = newPosition;
        mouseMoved();
    }

    if (e->buttons() == Qt::NoButton) {
        auto noteData = inlineNoteAt(e->globalPosition().toPoint());
        auto focusChanged = false;
        if (noteData.m_position.isValid()) {
            if (!m_activeInlineNote.m_position.isValid()) {
                // no active note -- focus in
                tagLine(noteData.m_position);
                focusChanged = true;
                noteData.m_underMouse = true;
                noteData.m_provider->inlineNoteFocusInEvent(KTextEditor::InlineNote(noteData), e->globalPosition().toPoint());
                m_activeInlineNote = noteData;
            } else {
                noteData.m_provider->inlineNoteMouseMoveEvent(KTextEditor::InlineNote(noteData), e->globalPosition().toPoint());
            }
        } else if (m_activeInlineNote.m_position.isValid()) {
            tagLine(m_activeInlineNote.m_position);
            focusChanged = true;
            m_activeInlineNote.m_underMouse = false;
            m_activeInlineNote.m_provider->inlineNoteFocusOutEvent(KTextEditor::InlineNote(m_activeInlineNote));
            m_activeInlineNote = {};
        }
        if (focusChanged) {
            // the note might change its appearance in reaction to the focus event
            updateDirty();
        }
    }

    if (e->buttons() & Qt::LeftButton) {
        if (m_dragInfo.state == diPending) {
            // we had a mouse down, but haven't confirmed a drag yet
            // if the mouse has moved sufficiently, we will confirm
            QPoint p(e->pos() - m_dragInfo.start);

            // we've left the drag square, we can start a real drag operation now
            if (p.manhattanLength() > QApplication::startDragDistance()) {
                doDrag();
            }

            return;
        } else if (m_dragInfo.state == diDragging) {
            // Don't do anything after a canceled drag until the user lets go of
            // the mouse button!
            return;
        }

        m_mouseX = e->position().x();
        m_mouseY = e->position().y();

        m_scrollX = 0;
        m_scrollY = 0;
        int d = renderer()->lineHeight();

        if (m_mouseX < 0) {
            m_scrollX = -d;
        }

        if (m_mouseX > width()) {
            m_scrollX = d;
        }

        if (m_mouseY < 0) {
            m_mouseY = 0;
            m_scrollY = -d;
        }

        if (m_mouseY > height()) {
            m_mouseY = height();
            m_scrollY = d;
        }

        if (!m_scrollY) {
            // We are on top of line number area and dragging
            // Since we never get negative y, ensure to scroll up
            // a bit otherwise we get stuck on the topview line
            if (!m_mouseY) {
                m_scrollY -= d;
            }
            placeCursor(QPoint(m_mouseX, m_mouseY), true);
        }
    } else {
        if (view()->config()->textDragAndDrop() && isTargetSelected(e->pos())) {
            // mouse is over selected text. indicate that the text is draggable by setting
            // the arrow cursor as other Qt text editing widgets do
            if (m_mouseCursor != Qt::ArrowCursor) {
                m_mouseCursor = Qt::ArrowCursor;
                setCursor(m_mouseCursor);
            }
        } else {
            // normal text cursor
            if (m_mouseCursor != Qt::IBeamCursor) {
                m_mouseCursor = Qt::IBeamCursor;
                setCursor(m_mouseCursor);
            }
        }
        // We need to check whether the mouse position is actually within the widget,
        // because other widgets like the icon border forward their events to this,
        // and we will create invalid text hint requests if we don't check
        if (textHintsEnabled() && geometry().contains(parentWidget()->mapFromGlobal(e->globalPosition()).toPoint())) {
            if (QToolTip::isVisible()) {
                QToolTip::hideText();
            }
            m_textHintTimer.start(m_textHintDelay);
            m_textHintPos = e->pos();
        }
    }
}

void KateViewInternal::updateDirty()
{
    const int h = renderer()->lineHeight();

    int currentRectStart = -1;
    int currentRectEnd = -1;

    QRegion updateRegion;

    {
        for (int i = 0; i < cache()->viewCacheLineCount(); ++i) {
            if (cache()->viewLine(i).isDirty()) {
                if (currentRectStart == -1) {
                    currentRectStart = h * i;
                    currentRectEnd = h;
                } else {
                    currentRectEnd += h;
                }

            } else if (currentRectStart != -1) {
                updateRegion += QRect(0, currentRectStart, width(), currentRectEnd);
                currentRectStart = -1;
                currentRectEnd = -1;
            }
        }
    }

    if (currentRectStart != -1) {
        updateRegion += QRect(0, currentRectStart, width(), currentRectEnd);
    }

    if (!updateRegion.isEmpty()) {
        if (debugPainting) {
            qCDebug(LOG_KTE) << "Update dirty region " << updateRegion;
        }
        update(updateRegion);
    }
}

void KateViewInternal::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
    if (view()->isCompletionActive()) {
        view()->completionWidget()->abortCompletion();
    }
}

void KateViewInternal::paintEvent(QPaintEvent *e)
{
    if (debugPainting) {
        qCDebug(LOG_KTE) << "GOT PAINT EVENT: Region" << e->region();
    }

    const QRect &unionRect = e->rect();

    int xStart = startX() + unionRect.x();
    int xEnd = xStart + unionRect.width();
    uint h = renderer()->lineHeight();
    uint startz = (unionRect.y() / h);
    uint endz = startz + 1 + (unionRect.height() / h);
    uint lineRangesSize = cache()->viewCacheLineCount();
    const KTextEditor::Cursor pos = m_cursor;

    QPainter paint(this);

    // THIS IS ULTRA EVIL AND ADDS STRANGE RENDERING ARTIFACTS WITH SCALING!!!!
    // SEE BUG https://bugreports.qt.io/browse/QTBUG-66036
    // paint.setRenderHints(QPainter::TextAntialiasing);

    paint.save();

    renderer()->setCaretStyle(m_currentInputMode->caretStyle());
    renderer()->setShowTabs(doc()->config()->showTabs());
    renderer()->setShowSpaces(doc()->config()->showSpaces());
    renderer()->updateMarkerSize();

    // paint line by line
    // this includes parts that span areas without real lines
    // translate to first line to paint
    paint.translate(unionRect.x(), startz * h);
    for (uint z = startz; z <= endz; z++) {
        // paint regions without lines mapped to
        if ((z >= lineRangesSize) || (cache()->viewLine(z).line() == -1)) {
            if (!(z >= lineRangesSize)) {
                cache()->viewLine(z).setDirty(false);
            }
            paint.fillRect(0, 0, unionRect.width(), h, m_view->rendererConfig()->backgroundColor());
        }

        // paint text lines
        else {
            // If viewLine() returns non-zero, then a document line was split
            // in several visual lines, and we're trying to paint visual line
            // that is not the first.  In that case, this line was already
            // painted previously, since KateRenderer::paintTextLine paints
            // all visual lines.
            //
            // Except if we're at the start of the region that needs to
            // be painted -- when no previous calls to paintTextLine were made.
            KateTextLayout &thisLine = cache()->viewLine(z);
            if (!thisLine.viewLine() || z == startz) {
                // paint our line
                // set clipping region to only paint the relevant parts
                paint.save();
                const int thisLineTop = h * thisLine.viewLine();
                paint.translate(QPoint(0, -thisLineTop));

                // compute rect for line, fill the stuff
                // important: as we allow some ARGB colors for other stuff, it is REALLY important to fill the full range once!
                const QRectF lineRect(0, 0, unionRect.width(), h * thisLine.kateLineLayout()->viewLineCount());
                paint.fillRect(lineRect, m_view->rendererConfig()->backgroundColor());

                // THIS IS ULTRA EVIL AND ADDS STRANGE RENDERING ARTIFACTS WITH SCALING!!!!
                // SEE BUG https://bugreports.qt.io/browse/QTBUG-66036
                // => using a QRectF solves the cut of 1 pixel, the same call with QRect does create artifacts!
                paint.setClipRect(lineRect);

                // QTextLayout::draw does not take into account QPainter's viewport, so it will try to render
                // lines outside visible bounds. This causes a significant slowdown when a very long line
                // dynamically broken into multiple lines. To avoid this, an explicit text clip rect is set.
                const QRect textClipRect{xStart, thisLineTop, xEnd - xStart, height()};

                renderer()->paintTextLine(paint, thisLine.kateLineLayout(), xStart, xEnd, textClipRect.toRectF(), &pos);
                paint.restore();

                // line painted, reset and state + mark line as non-dirty
                thisLine.setDirty(false);
            }
        }

        // translate to next line
        paint.translate(0, h);
    }

    paint.restore();

    if (m_textAnimation) {
        m_textAnimation->draw(paint);
    }
}

void KateViewInternal::resizeEvent(QResizeEvent *e)
{
    bool expandedHorizontally = width() > e->oldSize().width();
    bool expandedVertically = height() > e->oldSize().height();
    bool heightChanged = height() != e->oldSize().height();

    m_dummy->setFixedSize(m_lineScroll->width(), m_columnScroll->sizeHint().height());
    m_madeVisible = false;

    // resize the bracket match preview
    if (m_bmPreview) {
        showBracketMatchPreview();
    }

    if (heightChanged) {
        setAutoCenterLines(m_autoCenterLines, false);
        m_cachedMaxStartPos.setPosition(-1, -1);
    }

    if (view()->dynWordWrap()) {
        bool dirtied = false;

        for (int i = 0; i < cache()->viewCacheLineCount(); i++) {
            // find the first dirty line
            // the word wrap updateView algorithm is forced to check all lines after a dirty one
            KateTextLayout viewLine = cache()->viewLine(i);

            if (viewLine.wrap() || viewLine.isRightToLeft() || viewLine.width() > width()) {
                dirtied = true;
                viewLine.setDirty();
                break;
            }
        }

        if (dirtied || heightChanged) {
            updateView(true);
            m_leftBorder->update();
        }
    } else {
        updateView();

        if (expandedHorizontally && startX() > 0) {
            scrollColumns(startX() - (width() - e->oldSize().width()));
        }
    }

    if (width() < e->oldSize().width() && !view()->wrapCursor()) {
        // May have to restrain cursor to new smaller width...
        if (m_cursor.column() > doc()->lineLength(m_cursor.line())) {
            KateTextLayout thisLine = currentLayout(m_cursor);

            KTextEditor::Cursor newCursor(m_cursor.line(),
                                          thisLine.endCol() + ((width() - thisLine.xOffset() - (thisLine.width() - startX())) / renderer()->spaceWidth()) - 1);
            if (newCursor.column() < m_cursor.column()) {
                updateCursor(newCursor);
            }
        }
    }

    if (expandedVertically) {
        KTextEditor::Cursor max = maxStartPos();
        if (startPos() > max) {
            scrollPos(max);
            return; // already fired displayRangeChanged
        }
    }
    Q_EMIT view()->displayRangeChanged(m_view);
}

void KateViewInternal::moveEvent(QMoveEvent *e)
{
    // move the bracket match preview to the new location
    if (e->pos() != e->oldPos() && m_bmPreview) {
        showBracketMatchPreview();
    }

    QWidget::moveEvent(e);
}

void KateViewInternal::scrollTimeout()
{
    if (m_scrollX || m_scrollY) {
        const int scrollTo = startPos().line() + (m_scrollY / (int)renderer()->lineHeight());
        placeCursor(QPoint(m_mouseX, m_mouseY), true);
        scrollLines(scrollTo);
    }
}

void KateViewInternal::cursorTimeout()
{
    if (!debugPainting && m_currentInputMode->blinkCaret()) {
        renderer()->setDrawCaret(!renderer()->drawCaret());
        paintCursor();
    }
}

void KateViewInternal::textHintTimeout()
{
    m_textHintTimer.stop();

    KTextEditor::Cursor c = coordinatesToCursor(m_textHintPos, false);
    if (!c.isValid()) {
        return;
    }

    QStringList textHints;
    for (KTextEditor::TextHintProvider *const p : m_textHintProviders) {
        if (!p) {
            continue;
        }

        const QString hint = p->textHint(m_view, c);
        if (!hint.isEmpty()) {
            textHints.append(hint);
        }
    }

    if (!textHints.isEmpty()) {
        qCDebug(LOG_KTE) << "Hint text: " << textHints;
        QString hint;
        for (const QString &str : std::as_const(textHints)) {
            hint += QStringLiteral("<p>%1</p>").arg(str);
        }
        QPoint pos(startX() + m_textHintPos.x(), m_textHintPos.y());
        QToolTip::showText(mapToGlobal(pos), hint);
    }
}

void KateViewInternal::focusInEvent(QFocusEvent *)
{
    if (QApplication::cursorFlashTime() > 0) {
        m_cursorTimer.start(QApplication::cursorFlashTime() / 2);
    }

    paintCursor();

    doc()->setActiveView(m_view);

    // this will handle focus stuff in kateview
    view()->slotGotFocus();
}

void KateViewInternal::focusOutEvent(QFocusEvent *)
{
    // if (view()->isCompletionActive())
    // view()->abortCompletion();

    m_cursorTimer.stop();
    view()->renderer()->setDrawCaret(true);
    paintCursor();

    m_textHintTimer.stop();

    view()->slotLostFocus();

    hideBracketMatchPreview();
}

void KateViewInternal::doDrag()
{
    m_dragInfo.state = diDragging;
    m_dragInfo.dragObject = new QDrag(this);
    std::unique_ptr<QMimeData> mimeData(new QMimeData());
    mimeData->setText(view()->selectionText());

    const auto startCur = view()->selectionRange().start();
    const auto endCur = view()->selectionRange().end();
    if (!startCur.isValid() || !endCur.isValid()) {
        return;
    }

    int startLine = startCur.line();
    int endLine = endCur.line();

    /**
     * Get real first and last visible line nos.
     * This is important as startLine() / endLine() are virtual and we can't use
     * them here
     */
    const int firstVisibleLine = view()->firstDisplayedLineInternal(KTextEditor::View::RealLine);
    const int lastVisibleLine = view()->lastDisplayedLineInternal(KTextEditor::View::RealLine);

    // get visible selected lines
    for (int l = startLine; l <= endLine; ++l) {
        if (l >= firstVisibleLine) {
            break;
        }
        ++startLine;
    }
    for (int l = endLine; l >= startLine; --l) {
        if (l <= lastVisibleLine) {
            break;
        }
        --endLine;
    }

    // calculate the height / width / scale
    int w = 0;
    int h = 0;
    const QFontMetricsF &fm = renderer()->currentFontMetrics();
    for (int l = startLine; l <= endLine; ++l) {
        w = std::max((int)fm.horizontalAdvance(doc()->line(l)), w);
        h += renderer()->lineHeight();
    }
    qreal scale = h > m_view->height() / 2 ? 0.75 : 1.0;

    // Calculate start x pos on start line
    int sX = 0;
    if (startLine == startCur.line()) {
        sX = renderer()->cursorToX(cache()->textLayout(startCur), startCur, !view()->wrapCursor());
    }

    // Calculate end x pos on end line
    int eX = 0;
    if (endLine == endCur.line()) {
        eX = renderer()->cursorToX(cache()->textLayout(endCur), endCur, !view()->wrapCursor());
    }

    // These are the occurunce highlights, the ones you get when you select a word
    // We clear them here so that they don't come up in the dragged pixmap
    // After we are done creating the pixmap, we restore them
    if (view()->selection()) {
        view()->clearHighlights();
    }

    // Create a pixmap this selection
    const qreal dpr = devicePixelRatioF();
    QPixmap pixmap(w * dpr, h * dpr);
    if (!pixmap.isNull()) {
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);
        renderer()->paintSelection(&pixmap, startLine, sX, endLine, eX, scale);

        if (view()->selection()) {
            // Tell the view to restore the highlights
            Q_EMIT view()->selectionChanged(view());
        }
    }

    // Calculate position where pixmap will appear when user
    // starts dragging
    const int x = 0;
    /**
     * lineToVisibleLine() = real line => virtual line
     * This is necessary here because if there is a folding in the current
     * view lines, the y pos can be incorrect. So, we make sure to convert
     * it to virtual line before calculating y
     */
    const int y = lineToY(view()->m_textFolding.lineToVisibleLine(startLine));
    const QPoint pos = mapFromGlobal(QCursor::pos()) - QPoint(x, y);

    m_dragInfo.dragObject->setPixmap(pixmap);
    m_dragInfo.dragObject->setHotSpot(pos);
    m_dragInfo.dragObject->setMimeData(mimeData.release());
    m_dragInfo.dragObject->exec(Qt::MoveAction | Qt::CopyAction);
}

void KateViewInternal::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
    }
    event->setAccepted((event->mimeData()->hasText() && doc()->isReadWrite()) || event->mimeData()->hasUrls());
}

void KateViewInternal::fixDropEvent(QDropEvent *event)
{
    if (event->source() != this) {
        event->setDropAction(Qt::CopyAction);
    } else {
        Qt::DropAction action = Qt::MoveAction;
#ifdef Q_WS_MAC
        if (event->modifiers() & Qt::AltModifier) {
            action = Qt::CopyAction;
        }
#else
        if (event->modifiers() & Qt::ControlModifier) {
            action = Qt::CopyAction;
        }
#endif
        event->setDropAction(action);
    }
}

void KateViewInternal::dragMoveEvent(QDragMoveEvent *event)
{
    // track the cursor to the current drop location
    placeCursor(event->position().toPoint(), true, false);

    // important: accept action to switch between copy and move mode
    // without this, the text will always be copied.
    fixDropEvent(event);
}

void KateViewInternal::dropEvent(QDropEvent *event)
{
    // if we have urls, pass this event off to the hosting application
    if (event->mimeData()->hasUrls()) {
        Q_EMIT dropEventPass(event);
        return;
    }

    if (event->mimeData()->hasText() && doc()->isReadWrite()) {
        const QString text = event->mimeData()->text();
        const bool blockMode = view()->blockSelection();

        fixDropEvent(event);

        // Remember where to paste/move...
        KTextEditor::Cursor targetCursor(m_cursor);
        // Use powerful MovingCursor to track our changes we may do
        std::unique_ptr<KTextEditor::MovingCursor> targetCursor2(doc()->newMovingCursor(m_cursor));

        // As always need the BlockMode some special treatment
        const KTextEditor::Range selRange(view()->selectionRange());
        const KTextEditor::Cursor blockAdjust(selRange.numberOfLines(), selRange.columnWidth());

        // Restore the cursor position before editStart(), so that it is correctly stored for the undo action
        if (event->dropAction() != Qt::CopyAction) {
            editSetCursor(selRange.end());
        } else {
            view()->clearSelection();
        }

        // use one transaction
        doc()->editStart();

        if (event->dropAction() != Qt::CopyAction) {
            view()->removeSelectedText();
            if (targetCursor2->toCursor() != targetCursor) {
                // Hm, multi line selection moved down, we need to adjust our dumb cursor
                targetCursor = targetCursor2->toCursor();
            }
            doc()->insertText(targetCursor2->toCursor(), text, blockMode);

        } else {
            doc()->insertText(targetCursor, text, blockMode);
        }

        if (blockMode) {
            setSelection(KTextEditor::Range(targetCursor, targetCursor + blockAdjust));
            editSetCursor(targetCursor + blockAdjust);
        } else {
            setSelection(KTextEditor::Range(targetCursor, targetCursor2->toCursor()));
            editSetCursor(targetCursor2->toCursor()); // Just to satisfy autotest
        }

        doc()->editEnd();

        event->acceptProposedAction();
        updateView();
    }

    // finally finish drag and drop mode
    m_dragInfo.state = diNone;
    // important, because the eventFilter`s DragLeave does not occur
    stopDragScroll();
}
// END EVENT HANDLING STUFF

void KateViewInternal::clear()
{
    m_startPos.setPosition(0, 0);
    m_displayCursor = KTextEditor::Cursor(0, 0);
    m_cursor.setPosition(0, 0);
    view()->clearSecondaryCursors();
    cache()->clear();
    updateView(true);
    m_lineScroll->updatePixmap();
}

void KateViewInternal::wheelEvent(QWheelEvent *e)
{
    // check if this event should change the font size (Ctrl pressed, angle reported and not accidentally so)
    // Note: if detectZoomingEvent() doesn't unset the ControlModifier we'll get accelerated scrolling.
    if (m_zoomEventFilter->detectZoomingEvent(e)) {
        if (e->angleDelta().y() > 0) {
            slotIncFontSizes(qreal(e->angleDelta().y()) / (qreal)QWheelEvent::DefaultDeltasPerStep);
        } else if (e->angleDelta().y() < 0) {
            slotDecFontSizes(qreal(-e->angleDelta().y()) / (qreal)QWheelEvent::DefaultDeltasPerStep);
        }

        // accept always and be done for zooming
        e->accept();
        return;
    }

    // handle vertical scrolling via the scrollbar
    if (e->angleDelta().y() != 0) {
        // compute distance
        auto sign = m_lineScroll->invertedControls() ? -1 : 1;
        auto offset = sign * qreal(e->angleDelta().y()) / 120.0;
        if (e->modifiers() & Qt::ShiftModifier) {
            const auto pageStep = m_lineScroll->pageStep();
            offset = qBound(-pageStep, int(offset * pageStep), pageStep);
        } else {
            offset *= QApplication::wheelScrollLines();
        }

        // handle accumulation
        m_accumulatedScroll += offset - int(offset);
        const auto extraAccumulated = int(m_accumulatedScroll);
        m_accumulatedScroll -= extraAccumulated;

        // do scroll
        scrollViewLines(int(offset) + extraAccumulated);
        e->accept();
    }

    // handle horizontal scrolling via the scrollbar
    if (e->angleDelta().x() != 0) {
        // if we have dyn word wrap, we should ignore the scroll events
        if (view()->dynWordWrap()) {
            e->accept();
            return;
        }

        // if we scroll up/down we do not want to trigger unintended sideways scrolls
        if (qAbs(e->angleDelta().y()) > qAbs(e->angleDelta().x())) {
            e->accept();
            return;
        }

        if (QApplication::sendEvent(m_columnScroll, e)) {
            e->accept();
        }
    }

    // hide bracket match preview so that it won't linger while scrolling'
    hideBracketMatchPreview();
}

void KateViewInternal::scrollPrepareEvent(QScrollPrepareEvent *event)
{
    int lineHeight = renderer()->lineHeight();
    event->setViewportSize(QSizeF(0.0, 0.0));
    event->setContentPosRange(QRectF(0.0, 0.0, 0.0, m_lineScroll->maximum() * lineHeight));
    event->setContentPos(QPointF(0.0, m_lineScroll->value() * lineHeight));
    event->accept();
}

void KateViewInternal::scrollEvent(QScrollEvent *event)
{
    // FIXME Add horizontal scrolling, overscroll, scroll between lines, and word wrap awareness
    KTextEditor::Cursor newPos((int)event->contentPos().y() / renderer()->lineHeight(), 0);
    scrollPos(newPos);
    event->accept();
}

void KateViewInternal::startDragScroll()
{
    if (!m_dragScrollTimer.isActive()) {
        m_dragScrollTimer.start(s_scrollTime);
    }
}

void KateViewInternal::stopDragScroll()
{
    m_dragScrollTimer.stop();
    updateView();
}

void KateViewInternal::doDragScroll()
{
    QPoint p = this->mapFromGlobal(QCursor::pos());

    int dx = 0;
    int dy = 0;
    if (p.y() < s_scrollMargin) {
        dy = p.y() - s_scrollMargin;
    } else if (p.y() > height() - s_scrollMargin) {
        dy = s_scrollMargin - (height() - p.y());
    }

    if (p.x() < s_scrollMargin) {
        dx = p.x() - s_scrollMargin;
    } else if (p.x() > width() - s_scrollMargin) {
        dx = s_scrollMargin - (width() - p.x());
    }

    dy /= 4;

    if (dy) {
        scrollLines(startLine() + dy);
    }

    if (columnScrollingPossible() && dx) {
        scrollColumns(qMin(startX() + dx, m_columnScroll->maximum()));
    }

    if (!dy && !dx) {
        stopDragScroll();
    }
}

void KateViewInternal::registerTextHintProvider(KTextEditor::TextHintProvider *provider)
{
    if (std::find(m_textHintProviders.cbegin(), m_textHintProviders.cend(), provider) == m_textHintProviders.cend()) {
        m_textHintProviders.push_back(provider);
    }

    // we have a client, so start timeout
    m_textHintTimer.start(m_textHintDelay);
}

void KateViewInternal::unregisterTextHintProvider(KTextEditor::TextHintProvider *provider)
{
    const auto it = std::find(m_textHintProviders.cbegin(), m_textHintProviders.cend(), provider);
    if (it != m_textHintProviders.cend()) {
        m_textHintProviders.erase(it);
    }

    if (m_textHintProviders.empty()) {
        m_textHintTimer.stop();
    }
}

void KateViewInternal::setTextHintDelay(int delay)
{
    if (delay <= 0) {
        m_textHintDelay = 200; // ms
    } else {
        m_textHintDelay = delay; // ms
    }
}

int KateViewInternal::textHintDelay() const
{
    return m_textHintDelay;
}

bool KateViewInternal::textHintsEnabled()
{
    return !m_textHintProviders.empty();
}

// BEGIN EDIT STUFF
void KateViewInternal::editStart()
{
    editSessionNumber++;

    if (editSessionNumber > 1) {
        return;
    }

    editIsRunning = true;
    editOldCursor = m_cursor;
    editOldSelection = view()->selectionRange();
}

void KateViewInternal::editEnd(int editTagLineStart, int editTagLineEnd, bool tagFrom)
{
    if (editSessionNumber == 0) {
        return;
    }

    editSessionNumber--;

    if (editSessionNumber > 0) {
        return;
    }

    // fix start position, might have moved from column 0
    // try to clever calculate the right start column for the tricky dyn word wrap case
    int col = 0;
    if (view()->dynWordWrap()) {
        if (KateLineLayout *layout = cache()->line(startLine())) {
            int index = layout->viewLineForColumn(startPos().column());
            if (index >= 0 && index < layout->viewLineCount()) {
                col = layout->viewLine(index).startCol();
            }
        }
    }
    m_startPos.setPosition(startLine(), col);

    if (tagFrom && (editTagLineStart <= int(view()->textFolding().visibleLineToLine(startLine())))) {
        tagAll();
    } else {
        tagLines(editTagLineStart, tagFrom ? qMax(doc()->lastLine() + 1, editTagLineEnd) : editTagLineEnd, true);
    }

    if (editOldCursor == m_cursor.toCursor()) {
        updateBracketMarks();
    }

    updateView(true);

    if (editOldCursor != m_cursor.toCursor() || m_view == doc()->activeView()) {
        // Only scroll the view to the cursor if the insertion happens at the cursor.
        // This might not be the case for e.g. collaborative editing, when a remote user
        // inserts text at a position not at the caret.
        if (m_cursor.line() >= editTagLineStart && m_cursor.line() <= editTagLineEnd) {
            m_madeVisible = false;
            updateCursor(m_cursor, true);
        }
    }

    // selection changed?
    // fixes bug 316226
    if (editOldSelection != view()->selectionRange()
        || (editOldSelection.isValid() && !editOldSelection.isEmpty()
            && !(editTagLineStart > editOldSelection.end().line() && editTagLineEnd < editOldSelection.start().line()))) {
        Q_EMIT view()->selectionChanged(m_view);
    }

    editIsRunning = false;
}

void KateViewInternal::editSetCursor(const KTextEditor::Cursor _cursor)
{
    if (m_cursor.toCursor() != _cursor) {
        m_cursor.setPosition(_cursor);
    }
}
// END

void KateViewInternal::viewSelectionChanged()
{
    if (!view()->selection()) {
        m_selectAnchor = KTextEditor::Cursor::invalid();
    } else {
        const auto r = view()->selectionRange();
        m_selectAnchor = r.start() == m_cursor ? r.end() : r.start();
    }
    // Do NOT nuke the entire range! The reason is that a shift+DC selection
    // might (correctly) set the range to be empty (i.e. start() == end()), and
    // subsequent dragging might shrink the selection into non-existence. When
    // this happens, we use the cached end to restore the cached start so that
    // updateSelection is not confused. See also comments in updateSelection.
    m_selectionCached.setStart(KTextEditor::Cursor::invalid());
    updateMicroFocus();
}

KateLayoutCache *KateViewInternal::cache() const
{
    return m_layoutCache;
}

KTextEditor::Cursor KateViewInternal::toRealCursor(const KTextEditor::Cursor virtualCursor) const
{
    return KTextEditor::Cursor(view()->textFolding().visibleLineToLine(virtualCursor.line()), virtualCursor.column());
}

KTextEditor::Cursor KateViewInternal::toVirtualCursor(const KTextEditor::Cursor realCursor) const
{
    // only convert valid lines, folding doesn't like invalid input!
    // don't validate whole cursor, column might be -1
    if (realCursor.line() < 0) {
        return KTextEditor::Cursor::invalid();
    }

    return KTextEditor::Cursor(view()->textFolding().lineToVisibleLine(realCursor.line()), realCursor.column());
}

KateRenderer *KateViewInternal::renderer() const
{
    return view()->renderer();
}

void KateViewInternal::mouseMoved()
{
    view()->notifyMousePositionChanged(m_mouse);
    view()->updateRangesIn(KTextEditor::Attribute::ActivateMouseIn);
}

void KateViewInternal::cursorMoved()
{
    view()->updateRangesIn(KTextEditor::Attribute::ActivateCaretIn);

#ifndef QT_NO_ACCESSIBILITY
    if (view()->m_accessibilityEnabled && QAccessible::isActive()) {
        QAccessibleTextCursorEvent ev(this, static_cast<KateViewAccessible *>(QAccessible::queryAccessibleInterface(this))->positionFromCursor(this, m_cursor));
        QAccessible::updateAccessibility(&ev);
    }
#endif
}

KTextEditor::DocumentPrivate *KateViewInternal::doc()
{
    return m_view->doc();
}

KTextEditor::DocumentPrivate *KateViewInternal::doc() const
{
    return m_view->doc();
}

bool KateViewInternal::rangeAffectsView(KTextEditor::Range range, bool realCursors) const
{
    int startLine = KateViewInternal::startLine();
    int endLine = startLine + (int)m_visibleLineCount;

    if (realCursors) {
        startLine = (int)view()->textFolding().visibleLineToLine(startLine);
        endLine = (int)view()->textFolding().visibleLineToLine(endLine);
    }

    return (range.end().line() >= startLine) || (range.start().line() <= endLine);
}

// BEGIN IM INPUT STUFF
QVariant KateViewInternal::inputMethodQuery(Qt::InputMethodQuery query) const
{
    switch (query) {
    case Qt::ImCursorRectangle: {
        // Cursor placement code is changed for Asian input method that
        // shows candidate window. This behavior is same as Qt/E 2.3.7
        // which supports Asian input methods. Asian input methods need
        // start point of IM selection text to place candidate window as
        // adjacent to the selection text.
        //
        // in Qt5, cursor rectangle is used as QRectF internally, and it
        // will be checked by QRectF::isValid(), which will mark rectangle
        // with width == 0 or height == 0 as invalid.
        auto lineHeight = renderer()->lineHeight();
        return QRect(cursorToCoordinate(m_cursor, true, false), QSize(1, lineHeight ? lineHeight : 1));
    }

    case Qt::ImFont:
        return renderer()->currentFont();

    case Qt::ImCursorPosition:
        return m_cursor.column();

    case Qt::ImAnchorPosition:
        // If selectAnchor is at the same line, return the real anchor position
        // Otherwise return the same position of cursor
        if (view()->selection() && m_selectAnchor.line() == m_cursor.line()) {
            return m_selectAnchor.column();
        } else {
            return m_cursor.column();
        }

    case Qt::ImSurroundingText:
        return doc()->kateTextLine(m_cursor.line()).text();

    case Qt::ImCurrentSelection:
        if (view()->selection()) {
            return view()->selectionText();
        } else {
            return QString();
        }
    default:
        /* values: ImMaximumTextLength */
        break;
    }

    return QWidget::inputMethodQuery(query);
}

void KateViewInternal::inputMethodEvent(QInputMethodEvent *e)
{
    if (doc()->readOnly()) {
        e->ignore();
        return;
    }

    // qCDebug(LOG_KTE) << "Event: cursor" << m_cursor << "commit" << e->commitString() << "preedit" << e->preeditString() << "replacement start" <<
    // e->replacementStart() << "length" << e->replacementLength();

    if (!m_imPreeditRange) {
        m_imPreeditRange.reset(
            doc()->newMovingRange(KTextEditor::Range(m_cursor, m_cursor), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight));
    }

    if (!m_imPreeditRange->toRange().isEmpty()) {
        doc()->inputMethodStart();
        doc()->removeText(*m_imPreeditRange);
        doc()->inputMethodEnd();
    }

    if (!e->commitString().isEmpty() || e->replacementLength() || !e->preeditString().isEmpty()) {
        view()->removeSelectedText();
    }

    if (!e->commitString().isEmpty() || e->replacementLength()) {
        KTextEditor::Range preeditRange = *m_imPreeditRange;

        KTextEditor::Cursor start(m_imPreeditRange->start().line(), m_imPreeditRange->start().column() + e->replacementStart());
        KTextEditor::Cursor removeEnd = start + KTextEditor::Cursor(0, e->replacementLength());

        doc()->editStart();
        if (start != removeEnd) {
            doc()->removeText(KTextEditor::Range(start, removeEnd));
        }

        // if the input method event is text that should be inserted, call KTextEditor::DocumentPrivate::typeChars()
        // with the text. that method will handle the input and take care of overwrite mode, etc.
        // we call this via keyPressEvent, as that handles our input methods, see bug 357062
        QKeyEvent ke(QEvent::KeyPress, 0, Qt::NoModifier, e->commitString());
        keyPressEvent(&ke);

        doc()->editEnd();

        // Revert to the same range as above
        m_imPreeditRange->setRange(preeditRange);
    }

    if (!e->preeditString().isEmpty()) {
        doc()->inputMethodStart();
        doc()->insertText(m_imPreeditRange->start(), e->preeditString());
        doc()->inputMethodEnd();
        // The preedit range gets automatically repositioned
    }

    // Finished this input method context?
    if (m_imPreeditRange && e->preeditString().isEmpty()) {
        // delete the range and reset the pointer
        m_imPreeditRange.reset();
        m_imPreeditRangeChildren.clear();

        if (QApplication::cursorFlashTime() > 0) {
            renderer()->setDrawCaret(false);
        }
        renderer()->setCaretOverrideColor(QColor());

        e->accept();
        return;
    }

    KTextEditor::Cursor newCursor = m_cursor;
    bool hideCursor = false;
    QColor caretColor;

    if (m_imPreeditRange) {
        m_imPreeditRangeChildren.clear();

        int decorationColumn = 0;
        const auto &attributes = e->attributes();
        for (auto &a : attributes) {
            if (a.type == QInputMethodEvent::Cursor) {
                const int cursor = qMin(a.start, e->preeditString().length());
                newCursor = m_imPreeditRange->start() + KTextEditor::Cursor(0, cursor);
                hideCursor = !a.length;
                QColor c = qvariant_cast<QColor>(a.value);
                if (c.isValid()) {
                    caretColor = c;
                }

            } else if (a.type == QInputMethodEvent::TextFormat) {
                QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
                const int start = qMin(a.start, e->preeditString().length());
                const int end = qMin(a.start + a.length, e->preeditString().length());
                if (f.isValid() && decorationColumn <= start && start != end) {
                    const KTextEditor::MovingCursor &preEditRangeStart = m_imPreeditRange->start();
                    const int startLine = preEditRangeStart.line();
                    const int startCol = preEditRangeStart.column();
                    KTextEditor::Range fr(startLine, startCol + start, startLine, startCol + end);
                    std::unique_ptr<KTextEditor::MovingRange> formatRange(doc()->newMovingRange(fr));
                    KTextEditor::Attribute::Ptr attribute(new KTextEditor::Attribute());
                    attribute->merge(f);
                    formatRange->setAttribute(attribute);
                    decorationColumn = end;
                    m_imPreeditRangeChildren.push_back(std::move(formatRange));
                }
            }
        }
    }

    renderer()->setDrawCaret(hideCursor);
    renderer()->setCaretOverrideColor(caretColor);

    if (newCursor != m_cursor.toCursor()) {
        updateCursor(newCursor);
    }

    e->accept();
}

// END IM INPUT STUFF

void KateViewInternal::flashChar(const KTextEditor::Cursor pos, KTextEditor::Attribute::Ptr attribute)
{
    Q_ASSERT(pos.isValid());
    Q_ASSERT(attribute.constData());

    // if line is folded away, do nothing
    if (!view()->textFolding().isLineVisible(pos.line())) {
        return;
    }

    KTextEditor::Range range(pos, KTextEditor::Cursor(pos.line(), pos.column() + 1));
    if (m_textAnimation) {
        m_textAnimation->deleteLater();
    }
    m_textAnimation = new KateTextAnimation(range, std::move(attribute), this);
}

void KateViewInternal::showBracketMatchPreview()
{
    // only show when main window is active
    if (window() && !window()->isActiveWindow()) {
        return;
    }

    const KTextEditor::Cursor openBracketCursor = m_bmStart->start();
    // make sure that the matching bracket is an opening bracket that is not visible on the current view, and that the preview won't be blocking the cursor
    if (m_cursor == openBracketCursor || toVirtualCursor(openBracketCursor).line() >= startLine() || m_cursor.line() - startLine() < 2) {
        hideBracketMatchPreview();
        return;
    }

    if (!m_bmPreview) {
        m_bmPreview.reset(new KateTextPreview(m_view, this));
        m_bmPreview->setAttribute(Qt::WA_ShowWithoutActivating);
        m_bmPreview->setFrameStyle(QFrame::Box);
    }

    const int previewLine = openBracketCursor.line();
    KateRenderer *const renderer_ = renderer();
    KateLineLayout *lineLayout(new KateLineLayout(*renderer_));
    lineLayout->setLine(previewLine, -1);

    // If the opening bracket is on its own line, start preview at the line above it instead (where the context is likely to be)
    const int col = lineLayout->textLine().firstChar();
    if (previewLine > 0 && (col == -1 || col == openBracketCursor.column())) {
        lineLayout->setLine(previewLine - 1, lineLayout->virtualLine() - 1);
    }

    renderer_->layoutLine(lineLayout, -1 /* no wrap */, false /* no layout cache */);
    const int lineWidth =
        qBound(m_view->width() / 5, int(lineLayout->width() + renderer_->spaceWidth() * 2), m_view->width() - m_leftBorder->width() - m_lineScroll->width());
    m_bmPreview->resize(lineWidth, renderer_->lineHeight() * 2);
    const QPoint topLeft = mapToGlobal(QPoint(0, 0));
    m_bmPreview->move(topLeft.x(), topLeft.y());
    m_bmPreview->setLine(lineLayout->virtualLine());
    m_bmPreview->setCenterView(false);
    m_bmPreview->raise();
    m_bmPreview->show();
}

void KateViewInternal::hideBracketMatchPreview()
{
    m_bmPreview.reset();
}

void KateViewInternal::documentTextInserted(KTextEditor::Document *, KTextEditor::Range range)
{
#ifndef QT_NO_ACCESSIBILITY
    if (view()->m_accessibilityEnabled && QAccessible::isActive()) {
        auto doc = view()->doc();
        QAccessibleTextInsertEvent ev(this, doc->cursorToOffset(range.start()), doc->text(range));
        QAccessible::updateAccessibility(&ev);
    }
#endif
}

void KateViewInternal::documentTextRemoved(KTextEditor::Document * /*document*/, KTextEditor::Range range, const QString &oldText)
{
#ifndef QT_NO_ACCESSIBILITY
    if (view()->m_accessibilityEnabled && QAccessible::isActive()) {
        auto doc = view()->doc();
        QAccessibleTextRemoveEvent ev(this, doc->cursorToOffset(range.start()), oldText);
        QAccessible::updateAccessibility(&ev);
    }
#endif
}

QRect KateViewInternal::inlineNoteRect(const KateInlineNoteData &noteData) const
{
    KTextEditor::InlineNote note(noteData);
    // compute note width and position
    const auto noteWidth = note.width();
    auto noteCursor = note.position();

    // The cursor might be outside of the text. In that case, clamp it to the text and
    // later on add the missing x offset.
    const auto lineLength = view()->document()->lineLength(noteCursor.line());
    int extraOffset = -noteWidth;
    if (noteCursor.column() == lineLength) {
        extraOffset = 0;
    } else if (noteCursor.column() > lineLength) {
        extraOffset = (noteCursor.column() - lineLength) * renderer()->spaceWidth();
        noteCursor.setColumn(lineLength);
    }
    auto noteStartPos = mapToGlobal(cursorToCoordinate(noteCursor, true, false));
    bool rtl = false;
    if (view()->dynWordWrap()) {
        const KateLineLayout *lineLayout = cache()->line(noteCursor.line());
        rtl = lineLayout && lineLayout->layout()->textOption().textDirection() == Qt::RightToLeft;
    }
    if (rtl) {
        noteStartPos.rx() -= note.width();
        extraOffset = -extraOffset;
    }

    // compute the note's rect
    auto globalNoteRect = QRect(noteStartPos + QPoint{extraOffset, 0}, QSize(noteWidth, renderer()->lineHeight()));

    return globalNoteRect;
}

KateInlineNoteData KateViewInternal::inlineNoteAt(const QPoint &globalPos) const
{
    // compute the associated cursor to get the right line
    const int line = coordinatesToCursor(mapFromGlobal(globalPos)).line();
    const auto inlineNotes = view()->inlineNotes(line);
    // loop over all notes and check if the point is inside it
    for (const auto &note : inlineNotes) {
        auto globalNoteRect = inlineNoteRect(note);
        if (globalNoteRect.contains(globalPos)) {
            return note;
        }
    }
    // none found -- return an invalid note
    return {};
}

bool KateViewInternal::sendMouseEventToInputContext(QMouseEvent *e)
{
    if (!m_imPreeditRange) {
        return false;
    }

    KTextEditor::Cursor c = cursorForPoint(e->pos());
    if (!m_imPreeditRange->contains(c) && c != m_imPreeditRange->end()) {
        return false;
    }

    auto cursorPos = (c - m_imPreeditRange->start());

    if (cursorPos.column() >= 0) {
        if (e->type() == QEvent::MouseButtonRelease)
            QGuiApplication::inputMethod()->invokeAction(QInputMethod::Click, cursorPos.column());
        e->setAccepted(true);
        return true;
    }
    return false;
}

void KateViewInternal::commitPreedit()
{
    if (!m_imPreeditRange) {
        return;
    }

    QGuiApplication::inputMethod()->commit();
}

#include "moc_kateviewinternal.cpp"
