/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D. Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdatetable_p.h"

#include <QAction>
#include <QActionEvent>
#include <QApplication>
#include <QDate>
#include <QFontDatabase>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

#include <cmath>

class KDateTable::KDateTablePrivate
{
public:
    KDateTablePrivate(KDateTable *qq)
        : q(qq)
    {
        m_popupMenuEnabled = false;
        m_useCustomColors = false;
        m_hoveredPos = -1;
        setDate(QDate::currentDate());
    }

    ~KDateTablePrivate()
    {
    }

    void setDate(const QDate &date);
    void nextMonth();
    void previousMonth();
    void beginningOfMonth();
    void endOfMonth();
    void beginningOfWeek();
    void endOfWeek();

    KDateTable *q;

    /**
     * The currently selected date.
     */
    QDate m_date;

    /**
     * The weekday number of the first day in the month [1..daysInWeek()].
     */
    int m_weekDayFirstOfMonth;

    /**
     * The number of days in the current month.
     */
    int m_numDaysThisMonth;

    /**
     * Save the size of the largest used cell content.
     */
    QRectF m_maxCell;

    /**
     * How many week rows we are to draw.
     */
    int m_numWeekRows;

    /**
     * How many day columns we are to draw, i.e. days in a week.
     */
    int m_numDayColumns;

    /**
     * The font size of the displayed text.
     */
    int fontsize;

    bool m_popupMenuEnabled;
    bool m_useCustomColors;

    struct DatePaintingMode {
        QColor fgColor;
        QColor bgColor;
        BackgroundMode bgMode;
    };
    QHash<int, DatePaintingMode> m_customPaintingModes;

    int m_hoveredPos;
};

KDateTable::KDateTable(const QDate &date, QWidget *parent)
    : QWidget(parent)
    , d(new KDateTablePrivate(this))
{
    initWidget(date);
}

KDateTable::KDateTable(QWidget *parent)
    : QWidget(parent)
    , d(std::make_unique<KDateTablePrivate>(this))
{
    initWidget(QDate::currentDate());
}

KDateTable::~KDateTable()
{
}

void KDateTable::initWidget(const QDate &date)
{
    d->m_numWeekRows = 7;

    setFontSize(10);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    initAccels();
    setAttribute(Qt::WA_Hover, true);

    setDate(date);
}

void KDateTable::initAccels()
{
    QAction *next = new QAction(this);
    next->setObjectName(QStringLiteral("next"));
    next->setShortcuts(QKeySequence::keyBindings(QKeySequence::Forward));
    next->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(next, &QAction::triggered, this, [this]() {
        d->nextMonth();
    });

    QAction *prior = new QAction(this);
    prior->setObjectName(QStringLiteral("prior"));
    prior->setShortcuts(QKeySequence::keyBindings(QKeySequence::Back));
    prior->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(prior, &QAction::triggered, this, [this]() {
        d->previousMonth();
    });

    QAction *beginMonth = new QAction(this);
    beginMonth->setObjectName(QStringLiteral("beginMonth"));
    beginMonth->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToStartOfDocument));
    beginMonth->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(beginMonth, &QAction::triggered, this, [this]() {
        d->beginningOfMonth();
    });

    QAction *endMonth = new QAction(this);
    endMonth->setObjectName(QStringLiteral("endMonth"));
    endMonth->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToEndOfDocument));
    endMonth->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(endMonth, &QAction::triggered, this, [this]() {
        d->endOfMonth();
    });

    QAction *beginWeek = new QAction(this);
    beginWeek->setObjectName(QStringLiteral("beginWeek"));
    beginWeek->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToStartOfLine));
    beginWeek->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(beginWeek, &QAction::triggered, this, [this]() {
        d->beginningOfWeek();
    });

    QAction *endWeek = new QAction(this);
    endWeek->setObjectName(QStringLiteral("endWeek"));
    endWeek->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToEndOfLine));
    endWeek->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(endWeek, &QAction::triggered, this, [this]() {
        d->endOfWeek();
    });
}

int KDateTable::posFromDate(const QDate &date)
{
    int initialPosition = date.day();
    int offset = (d->m_weekDayFirstOfMonth - locale().firstDayOfWeek() + d->m_numDayColumns) % d->m_numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if (offset < 1) {
        offset += d->m_numDayColumns;
    }

    return initialPosition + offset;
}

QDate KDateTable::dateFromPos(int position)
{
    int offset = (d->m_weekDayFirstOfMonth - locale().firstDayOfWeek() + d->m_numDayColumns) % d->m_numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if (offset < 1) {
        offset += d->m_numDayColumns;
    }

    return QDate(d->m_date.year(), d->m_date.month(), 1).addDays(position - offset);
}

void KDateTable::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    const QRect &rectToUpdate = e->rect();
    double cellWidth = width() / (double)d->m_numDayColumns;
    double cellHeight = height() / (double)d->m_numWeekRows;
    int leftCol = (int)std::floor(rectToUpdate.left() / cellWidth);
    int topRow = (int)std::floor(rectToUpdate.top() / cellHeight);
    int rightCol = (int)std::ceil(rectToUpdate.right() / cellWidth);
    int bottomRow = (int)std::ceil(rectToUpdate.bottom() / cellHeight);
    bottomRow = qMin(bottomRow, d->m_numWeekRows - 1);
    rightCol = qMin(rightCol, d->m_numDayColumns - 1);
    if (layoutDirection() == Qt::RightToLeft) {
        p.translate((d->m_numDayColumns - leftCol - 1) * cellWidth, topRow * cellHeight);
    } else {
        p.translate(leftCol * cellWidth, topRow * cellHeight);
    }
    for (int i = leftCol; i <= rightCol; ++i) {
        for (int j = topRow; j <= bottomRow; ++j) {
            paintCell(&p, j, i);
            p.translate(0, cellHeight);
        }
        if (layoutDirection() == Qt::RightToLeft) {
            p.translate(-cellWidth, 0);
        } else {
            p.translate(cellWidth, 0);
        }
        p.translate(0, -cellHeight * (bottomRow - topRow + 1));
    }
}

void KDateTable::paintCell(QPainter *painter, int row, int col)
{
    double w = (width() / (double)d->m_numDayColumns) - 1;
    double h = (height() / (double)d->m_numWeekRows) - 1;
    QRectF cell = QRectF(0, 0, w, h);
    QString cellText;
    QColor cellBackgroundColor;
    QColor cellTextColor;
    QFont cellFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    bool workingDay = false;
    int cellWeekDay;
    int pos;

    // Calculate the position of the cell in the grid
    pos = d->m_numDayColumns * (row - 1) + col;

    // Calculate what day of the week the cell is
    if (col + locale().firstDayOfWeek() <= d->m_numDayColumns) {
        cellWeekDay = col + locale().firstDayOfWeek();
    } else {
        cellWeekDay = col + locale().firstDayOfWeek() - d->m_numDayColumns;
    }

    // FIXME This is wrong if the widget is not using the global!
    // See if cell day is normally a working day
    if (locale().weekdays().first() <= locale().weekdays().last()) {
        if (cellWeekDay >= locale().weekdays().first() && cellWeekDay <= locale().weekdays().last()) {
            workingDay = true;
        }
    } else {
        if (cellWeekDay >= locale().weekdays().first() //
            || cellWeekDay <= locale().weekdays().last()) {
            workingDay = true;
        }
    }

    if (row == 0) {
        // We are drawing a header cell

        // If not a normal working day, then use "do not work today" color
        if (workingDay) {
            cellTextColor = palette().color(QPalette::WindowText);
        } else {
            cellTextColor = Qt::darkRed;
        }
        cellBackgroundColor = palette().color(QPalette::Window);

        // Set the text to the short day name and bold it
        cellFont.setBold(true);
        cellText = locale().dayName(cellWeekDay, QLocale::ShortFormat);

    } else {
        // We are drawing a day cell

        // Calculate the date the cell represents
        QDate cellDate = dateFromPos(pos);

        bool validDay = cellDate.isValid();

        // Draw the day number in the cell, if the date is not valid then we don't want to show it
        if (validDay) {
            cellText = locale().toString(cellDate.day());
        } else {
            cellText = QString();
        }

        if (!validDay || cellDate.month() != d->m_date.month()) {
            // we are either
            // ° painting an invalid day
            // ° painting a day of the previous month or
            // ° painting a day of the following month or
            cellBackgroundColor = palette().color(backgroundRole());
            cellTextColor = palette().color(QPalette::Disabled, QPalette::Text);
        } else {
            // Paint a day of the current month

            // Background Colour priorities will be (high-to-low):
            // * Selected Day Background Colour
            // * Customized Day Background Colour
            // * Normal Day Background Colour

            // Background Shape priorities will be (high-to-low):
            // * Customized Day Shape
            // * Normal Day Shape

            // Text Colour priorities will be (high-to-low):
            // * Customized Day Colour
            // * Day of Pray Colour (Red letter)
            // * Selected Day Colour
            // * Normal Day Colour

            // Determine various characteristics of the cell date
            bool selectedDay = (cellDate == date());
            bool currentDay = (cellDate == QDate::currentDate());
            bool dayOfPray = (cellDate.dayOfWeek() == Qt::Sunday);
            // TODO: Uncomment if QLocale ever gets the feature...
            // bool dayOfPray = ( cellDate.dayOfWeek() == locale().dayOfPray() );
            bool customDay = (d->m_useCustomColors && d->m_customPaintingModes.contains(cellDate.toJulianDay()));

            // Default values for a normal cell
            cellBackgroundColor = palette().color(backgroundRole());
            cellTextColor = palette().color(foregroundRole());

            // If we are drawing the current date, then draw it bold and active
            if (currentDay) {
                cellFont.setBold(true);
                cellTextColor = palette().color(QPalette::LinkVisited);
            }

            // if we are drawing the day cell currently selected in the table
            if (selectedDay) {
                // set the background to highlighted
                cellBackgroundColor = palette().color(QPalette::Highlight);
                cellTextColor = palette().color(QPalette::HighlightedText);
            }

            // If custom colors or shape are required for this date
            if (customDay) {
                KDateTablePrivate::DatePaintingMode mode = d->m_customPaintingModes[cellDate.toJulianDay()];
                if (mode.bgMode != NoBgMode) {
                    if (!selectedDay) {
                        cellBackgroundColor = mode.bgColor;
                    }
                }
                cellTextColor = mode.fgColor;
            }

            // If the cell day is the day of religious observance, then always color text red unless Custom overrides
            if (!customDay && dayOfPray) {
                cellTextColor = Qt::darkRed;
            }
        }
    }

    // Draw the background
    if (row == 0) {
        painter->setPen(cellBackgroundColor);
        painter->setBrush(cellBackgroundColor);
        painter->drawRect(cell);
    } else if (cellBackgroundColor != palette().color(backgroundRole()) || pos == d->m_hoveredPos) {
        QStyleOptionViewItem opt;
        opt.initFrom(this);
        opt.rect = cell.toRect();
        if (cellBackgroundColor != palette().color(backgroundRole())) {
            opt.palette.setBrush(QPalette::Highlight, cellBackgroundColor);
            opt.state |= QStyle::State_Selected;
        }
        if (pos == d->m_hoveredPos && opt.state & QStyle::State_Enabled) {
            opt.state |= QStyle::State_MouseOver;
        } else {
            opt.state &= ~QStyle::State_MouseOver;
        }
        opt.showDecorationSelected = true;
        opt.viewItemPosition = QStyleOptionViewItem::OnlyOne;
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, this);
    }

    // Draw the text
    painter->setPen(cellTextColor);
    painter->setFont(cellFont);
    painter->drawText(cell, Qt::AlignCenter, cellText, &cell);

    // Draw the base line
    if (row == 0) {
        painter->setPen(palette().color(foregroundRole()));
        painter->drawLine(QPointF(0, h), QPointF(w, h));
    }

    // If the day cell we just drew is bigger than the current max cell sizes,
    // then adjust the max to the current cell
    if (cell.width() > d->m_maxCell.width()) {
        d->m_maxCell.setWidth(cell.width());
    }
    if (cell.height() > d->m_maxCell.height()) {
        d->m_maxCell.setHeight(cell.height());
    }
}

void KDateTable::KDateTablePrivate::nextMonth()
{
    // setDate does validity checking for us
    q->setDate(m_date.addMonths(1));
}

void KDateTable::KDateTablePrivate::previousMonth()
{
    // setDate does validity checking for us
    q->setDate(m_date.addMonths(-1));
}

void KDateTable::KDateTablePrivate::beginningOfMonth()
{
    // setDate does validity checking for us
    q->setDate(QDate(m_date.year(), m_date.month(), 1));
}

void KDateTable::KDateTablePrivate::endOfMonth()
{
    // setDate does validity checking for us
    q->setDate(QDate(m_date.year(), m_date.month() + 1, 0));
}

// JPL Do these make the assumption that first day of week is weekday 1? As it may not be.
void KDateTable::KDateTablePrivate::beginningOfWeek()
{
    // setDate does validity checking for us
    q->setDate(m_date.addDays(1 - m_date.dayOfWeek()));
}

// JPL Do these make the assumption that first day of week is weekday 1? As it may not be.
void KDateTable::KDateTablePrivate::endOfWeek()
{
    // setDate does validity checking for us
    q->setDate(m_date.addDays(7 - m_date.dayOfWeek()));
}

void KDateTable::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        // setDate does validity checking for us
        setDate(d->m_date.addDays(-d->m_numDayColumns));
        break;
    case Qt::Key_Down:
        // setDate does validity checking for us
        setDate(d->m_date.addDays(d->m_numDayColumns));
        break;
    case Qt::Key_Left:
        // setDate does validity checking for us
        setDate(d->m_date.addDays(-1));
        break;
    case Qt::Key_Right:
        // setDate does validity checking for us
        setDate(d->m_date.addDays(1));
        break;
    case Qt::Key_Minus:
        // setDate does validity checking for us
        setDate(d->m_date.addDays(-1));
        break;
    case Qt::Key_Plus:
        // setDate does validity checking for us
        setDate(d->m_date.addDays(1));
        break;
    case Qt::Key_N:
        // setDate does validity checking for us
        setDate(QDate::currentDate());
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        Q_EMIT tableClicked();
        break;
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Shift:
        // Don't beep for modifiers
        break;
    default:
        if (!e->modifiers()) { // hm
            QApplication::beep();
        }
    }
}

void KDateTable::setFontSize(int size)
{
    QFontMetricsF metrics(fontMetrics());
    QRectF rect;
    // ----- store rectangles:
    d->fontsize = size;
    // ----- find largest day name:
    d->m_maxCell.setWidth(0);
    d->m_maxCell.setHeight(0);
    for (int weekday = 1; weekday <= 7; ++weekday) {
        rect = metrics.boundingRect(locale().dayName(weekday, QLocale::ShortFormat));
        d->m_maxCell.setWidth(qMax(d->m_maxCell.width(), rect.width()));
        d->m_maxCell.setHeight(qMax(d->m_maxCell.height(), rect.height()));
    }
    // ----- compare with a real wide number and add some space:
    rect = metrics.boundingRect(QStringLiteral("88"));
    d->m_maxCell.setWidth(qMax(d->m_maxCell.width() + 2, rect.width()));
    d->m_maxCell.setHeight(qMax(d->m_maxCell.height() + 4, rect.height()));
}

void KDateTable::wheelEvent(QWheelEvent *e)
{
    setDate(d->m_date.addMonths(-(int)(e->angleDelta().y() / 120)));
    e->accept();
}

bool KDateTable::event(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::HoverMove: {
        QHoverEvent *e = static_cast<QHoverEvent *>(ev);
        const int row = e->position().y() * d->m_numWeekRows / height();
        int col;
        if (layoutDirection() == Qt::RightToLeft) {
            col = d->m_numDayColumns - (e->position().x() * d->m_numDayColumns / width()) - 1;
        } else {
            col = e->position().x() * d->m_numDayColumns / width();
        }

        const int pos = row < 1 ? -1 : (d->m_numDayColumns * (row - 1)) + col;

        if (pos != d->m_hoveredPos) {
            d->m_hoveredPos = pos;
            update();
        }
        break;
    }
    case QEvent::HoverLeave:
        if (d->m_hoveredPos != -1) {
            d->m_hoveredPos = -1;
            update();
        }
        break;
    default:
        break;
    }
    return QWidget::event(ev);
}

void KDateTable::mousePressEvent(QMouseEvent *e)
{
    if (e->type() != QEvent::MouseButtonPress) { // the KDatePicker only reacts on mouse press events:
        return;
    }

    if (!isEnabled()) {
        QApplication::beep();
        return;
    }

    int row;
    int col;
    int pos;

    QPoint mouseCoord = e->pos();
    row = mouseCoord.y() * d->m_numWeekRows / height();
    if (layoutDirection() == Qt::RightToLeft) {
        col = d->m_numDayColumns - (mouseCoord.x() * d->m_numDayColumns / width()) - 1;
    } else {
        col = mouseCoord.x() * d->m_numDayColumns / width();
    }

    if (row < 1 || col < 0) { // the user clicked on the frame of the table
        return;
    }

    // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
    // the row with the days of the week in the calculation.

    // new position and date
    pos = (d->m_numDayColumns * (row - 1)) + col;
    QDate clickedDate = dateFromPos(pos);

    // set the new date. If it is in the previous or next month, the month will
    // automatically be changed, no need to do that manually...
    // validity checking done inside setDate
    setDate(clickedDate);

    // This could be optimized to only call update over the regions
    // of old and new cell, but 99% of times there is also a call to
    // setDate that already calls update() so no need to optimize that
    // much here
    update();

    Q_EMIT tableClicked();

    if (e->button() == Qt::RightButton && d->m_popupMenuEnabled) {
        QMenu *menu = new QMenu();
        menu->addSection(locale().toString(d->m_date));
        Q_EMIT aboutToShowContextMenu(menu, clickedDate);
        menu->popup(e->globalPosition().toPoint());
    }
}

void KDateTable::KDateTablePrivate::setDate(const QDate &date)
{
    m_date = date;
    m_weekDayFirstOfMonth = QDate(date.year(), date.month(), 1).dayOfWeek();
    m_numDaysThisMonth = m_date.daysInMonth();
    m_numDayColumns = 7;
}

bool KDateTable::setDate(const QDate &toDate)
{
    if (!toDate.isValid()) {
        return false;
    }

    if (toDate == date()) {
        return true;
    }

    d->setDate(toDate);
    Q_EMIT dateChanged(date());
    update();

    return true;
}

const QDate &KDateTable::date() const
{
    return d->m_date;
}

void KDateTable::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
}

void KDateTable::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
}

QSize KDateTable::sizeHint() const
{
    if (d->m_maxCell.height() > 0 && d->m_maxCell.width() > 0) {
        return QSize(qRound(d->m_maxCell.width() * d->m_numDayColumns), (qRound(d->m_maxCell.height() + 2) * d->m_numWeekRows));
    } else {
        // qCDebug(KWidgetsAddonsLog) << "KDateTable::sizeHint: obscure failure - " << endl;
        return QSize(-1, -1);
    }
}

void KDateTable::setPopupMenuEnabled(bool enable)
{
    d->m_popupMenuEnabled = enable;
}

bool KDateTable::popupMenuEnabled() const
{
    return d->m_popupMenuEnabled;
}

void KDateTable::setCustomDatePainting(const QDate &date, const QColor &fgColor, BackgroundMode bgMode, const QColor &bgColor)
{
    if (!fgColor.isValid()) {
        unsetCustomDatePainting(date);
        return;
    }

    KDateTablePrivate::DatePaintingMode mode;
    mode.bgMode = bgMode;
    mode.fgColor = fgColor;
    mode.bgColor = bgColor;

    d->m_customPaintingModes.insert(date.toJulianDay(), mode);
    d->m_useCustomColors = true;
    update();
}

void KDateTable::unsetCustomDatePainting(const QDate &date)
{
    d->m_customPaintingModes.remove(date.toJulianDay());
    if (d->m_customPaintingModes.isEmpty()) {
        d->m_useCustomColors = false;
    }
    update();
}

#include "moc_kdatetable_p.cpp"
