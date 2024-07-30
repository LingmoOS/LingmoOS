/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D. Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm (mirko@kde.org)
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdatepicker.h"
#include "kdatepicker_p.h"

#include "common_helpers_p.h"
#include "kdatetable_p.h"
#include <kpopupframe.h>

#include <QApplication>
#include <QComboBox>
#include <QFont>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLayout>
#include <QMenu>
#include <QStyle>
#include <QToolButton>

#include "loggingcategory.h"
#include "moc_kdatepicker.cpp"
#include "moc_kdatepicker_p.cpp"

class DatePickerValidator : public QValidator
{
    Q_OBJECT
public:
    explicit DatePickerValidator(KDatePicker *parent)
        : QValidator(parent)
        , picker(parent)
    {
    }

    State validate(QString &text, int &) const override
    {
        return toDate(text).isValid() ? QValidator::Acceptable : QValidator::Intermediate;
    }

    QDate toDate(const QString &text) const
    {
        QLocale locale = picker->locale();
        const QList<QString> formats{
            locale.dateFormat(QLocale::LongFormat),
            locale.dateFormat(QLocale::ShortFormat),
            locale.dateFormat(QLocale::NarrowFormat),
            dateFormatWith4DigitYear(locale, QLocale::ShortFormat),
            QStringLiteral("yyyy-MM-dd"),
        };

        QDate date;
        for (const auto &format : formats) {
            date = locale.toDate(text, format);
            if (date.isValid()) {
                break;
            }
        }
        if (!date.isValid()) {
            qCDebug(KWidgetsAddonsLog) << "Could not parse text as date:" << text;
        }
        return date;
    }

private:
    KDatePicker *const picker;
};

// Week numbers are defined by ISO 8601
// See http://www.merlyn.demon.co.uk/weekinfo.htm for details

KDatePickerPrivateYearSelector::KDatePickerPrivateYearSelector(const QDate &currentDate, QWidget *parent)
    : QLineEdit(parent)
    , val(new QIntValidator(this))
    , result(0)
    , oldDate{currentDate}
{
    setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    setFrame(false);

    // TODO: Find a way to get that from QLocale
    // val->setRange( calendar->year( calendar->earliestValidDate() ),
    //               calendar->year( calendar->latestValidDate() ) );
    setValidator(val);

    connect(this, &QLineEdit::returnPressed, this, &KDatePickerPrivateYearSelector::yearEnteredSlot);
}

void KDatePickerPrivateYearSelector::yearEnteredSlot()
{
    bool ok;
    int newYear;

    // check if entered value is a number
    newYear = text().toInt(&ok);
    if (!ok) {
        QApplication::beep();
        return;
    }

    // check if new year will lead to a valid date
    if (QDate(newYear, oldDate.month(), oldDate.day()).isValid()) {
        result = newYear;
        Q_EMIT closeMe(1);
    } else {
        QApplication::beep();
    }
}

int KDatePickerPrivateYearSelector::year()
{
    return result;
}

void KDatePickerPrivateYearSelector::setYear(const QDate &year)
{
    setText(locale().toString(year, QStringLiteral("yyyy")).rightJustified(4, QLatin1Char('0')));
}

class KDatePickerPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KDatePicker)

public:
    explicit KDatePickerPrivate(KDatePicker *qq)
        : q(qq)
    {
    }

    void fillWeeksCombo();
    QDate validDateInYearMonth(int year, int month);

    /// the date table
    KDatePicker *q;

    QToolButton *closeButton = nullptr;
    QComboBox *selectWeek = nullptr;
    QToolButton *todayButton = nullptr;
    QBoxLayout *navigationLayout = nullptr;

    /// the year forward button
    QToolButton *yearForward = nullptr;
    /// the year backward button
    QToolButton *yearBackward = nullptr;
    /// the month forward button
    QToolButton *monthForward = nullptr;
    /// the month backward button
    QToolButton *monthBackward = nullptr;
    /// the button for selecting the month directly
    QToolButton *selectMonth = nullptr;
    /// the button for selecting the year directly
    QToolButton *selectYear = nullptr;
    /// the line edit to enter the date directly
    QLineEdit *line = nullptr;
    /// the validator for the line edit:
    DatePickerValidator *val = nullptr;
    /// the date table
    KDateTable *table = nullptr;
    /// the widest month string in pixels:
    QSize maxMonthRect;

    /// the font size for the widget
    int fontsize = -1;
};

void KDatePickerPrivate::fillWeeksCombo()
{
    // every year can have a different number of weeks
    // it could be that we had 53,1..52 and now 1..53 which is the same number but different
    // so always fill with new values
    // We show all week numbers for all weeks between first day of year to last day of year
    // This of course can be a list like 53,1,2..52

    const QDate thisDate = q->date();
    const int thisYear = thisDate.year();
    QDate day(thisDate.year(), 1, 1);
    const QDate lastDayOfYear = QDate(thisDate.year() + 1, 1, 1).addDays(-1);

    selectWeek->clear();

    // Starting from the first day in the year, loop through the year a week at a time
    // adding an entry to the week combo for each week in the year

    for (; day.isValid() && day <= lastDayOfYear; day = day.addDays(7)) {
        // Get the ISO week number for the current day and what year that week is in
        // e.g. 1st day of this year may fall in week 53 of previous year
        int weekYear = thisYear;
        const int week = day.weekNumber(&weekYear);
        QString weekString = tr("Week %1").arg(q->locale().toString(week));

        // show that this is a week from a different year
        if (weekYear != thisYear) {
            weekString += QLatin1Char('*');
        }

        // when the week is selected, go to the same weekday as the one
        // that is currently selected in the date table
        QDate targetDate = day.addDays(thisDate.dayOfWeek() - day.dayOfWeek());
        selectWeek->addItem(weekString, targetDate);

        // make sure that the week of the lastDayOfYear is always inserted: in Chinese calendar
        // system, this is not always the case
        if (day < lastDayOfYear //
            && day.daysTo(lastDayOfYear) < 7 //
            && lastDayOfYear.weekNumber() != day.weekNumber()) {
            day = lastDayOfYear.addDays(-7);
        }
    }
}

QDate KDatePickerPrivate::validDateInYearMonth(int year, int month)
{
    QDate newDate;

    // Try to create a valid date in this year and month
    // First try the first of the month, then try last of month
    if (QDate(year, month, 1).isValid()) {
        newDate = QDate(year, month, 1);
    } else if (QDate(year, month + 1, 1).isValid()) {
        newDate = QDate(year, month + 1, 1).addDays(-1);
    } else {
        newDate = QDate::fromJulianDay(0);
    }

    return newDate;
}

KDatePicker::KDatePicker(QWidget *parent)
    : QFrame(parent)
    , d(new KDatePickerPrivate(this))
{
    initWidget(QDate::currentDate());
}

KDatePicker::KDatePicker(const QDate &date_, QWidget *parent)
    : QFrame(parent)
    , d(new KDatePickerPrivate(this))
{
    initWidget(date_);
}

void KDatePicker::initWidget(const QDate &date_)
{
    const int horizontalSpacing = style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);

    QBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(0);
    topLayout->setContentsMargins(0, 0, 0, 0);

    d->navigationLayout = new QHBoxLayout();
    d->navigationLayout->setSpacing(0);
    d->navigationLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addLayout(d->navigationLayout);
    d->navigationLayout->addStretch();
    d->yearBackward = new QToolButton(this);
    d->yearBackward->setAutoRaise(true);
    d->navigationLayout->addWidget(d->yearBackward);
    d->monthBackward = new QToolButton(this);
    d->monthBackward->setAutoRaise(true);
    d->navigationLayout->addWidget(d->monthBackward);
    d->navigationLayout->addSpacing(horizontalSpacing);

    d->selectMonth = new QToolButton(this);
    d->selectMonth->setAutoRaise(true);
    d->navigationLayout->addWidget(d->selectMonth);
    d->selectYear = new QToolButton(this);
    d->selectYear->setCheckable(true);
    d->selectYear->setAutoRaise(true);
    d->navigationLayout->addWidget(d->selectYear);
    d->navigationLayout->addSpacing(horizontalSpacing);

    d->monthForward = new QToolButton(this);
    d->monthForward->setAutoRaise(true);
    d->navigationLayout->addWidget(d->monthForward);
    d->yearForward = new QToolButton(this);
    d->yearForward->setAutoRaise(true);
    d->navigationLayout->addWidget(d->yearForward);
    d->navigationLayout->addStretch();

    d->line = new QLineEdit(this);
    d->val = new DatePickerValidator(this);
    d->table = new KDateTable(this);
    setFocusProxy(d->table);

    d->fontsize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    if (d->fontsize == -1) {
        d->fontsize = QFontInfo(QFontDatabase::systemFont(QFontDatabase::GeneralFont)).pointSize();
    }

    d->fontsize++; // Make a little bigger

    d->selectWeek = new QComboBox(this); // read only week selection
    d->selectWeek->setFocusPolicy(Qt::NoFocus);
    d->todayButton = new QToolButton(this);
    d->todayButton->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-today")));

    d->yearForward->setToolTip(tr("Next year", "@info:tooltip"));
    d->yearBackward->setToolTip(tr("Previous year", "@info:tooltip"));
    d->monthForward->setToolTip(tr("Next month", "@info:tooltip"));
    d->monthBackward->setToolTip(tr("Previous month", "@info:tooltip"));
    d->selectWeek->setToolTip(tr("Select a week", "@info:tooltip"));
    d->selectMonth->setToolTip(tr("Select a month", "@info:tooltip"));
    d->selectYear->setToolTip(tr("Select a year", "@info:tooltip"));
    d->todayButton->setToolTip(tr("Select the current day", "@info:tooltip"));

    // -----
    setFontSize(d->fontsize);
    d->line->setValidator(d->val);
    d->line->installEventFilter(this);
    if (QApplication::isRightToLeft()) {
        d->yearForward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left-double")));
        d->yearBackward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right-double")));
        d->monthForward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left")));
        d->monthBackward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
    } else {
        d->yearForward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right-double")));
        d->yearBackward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left-double")));
        d->monthForward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
        d->monthBackward->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left")));
    }

    connect(d->table, &KDateTable::dateChanged, this, &KDatePicker::dateChangedSlot);
    connect(d->table, &KDateTable::tableClicked, this, &KDatePicker::tableClickedSlot);
    connect(d->monthForward, &QAbstractButton::clicked, this, &KDatePicker::monthForwardClicked);
    connect(d->monthBackward, &QAbstractButton::clicked, this, &KDatePicker::monthBackwardClicked);
    connect(d->yearForward, &QAbstractButton::clicked, this, &KDatePicker::yearForwardClicked);
    connect(d->yearBackward, &QAbstractButton::clicked, this, &KDatePicker::yearBackwardClicked);
    connect(d->selectWeek, &QComboBox::activated, this, &KDatePicker::weekSelected);
    connect(d->todayButton, &QAbstractButton::clicked, this, &KDatePicker::todayButtonClicked);
    connect(d->selectMonth, &QAbstractButton::clicked, this, &KDatePicker::selectMonthClicked);
    connect(d->selectYear, &QAbstractButton::toggled, this, &KDatePicker::selectYearClicked);
    connect(d->line, &QLineEdit::returnPressed, this, &KDatePicker::lineEnterPressed);

    topLayout->addWidget(d->table);

    QBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);
    topLayout->addLayout(bottomLayout);

    bottomLayout->addWidget(d->todayButton);
    bottomLayout->addWidget(d->line);
    bottomLayout->addWidget(d->selectWeek);

    d->table->setDate(date_);
    dateChangedSlot(date_); // needed because table emits changed only when newDate != oldDate
}

KDatePicker::~KDatePicker() = default;

bool KDatePicker::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *k = (QKeyEvent *)e;

        if ((k->key() == Qt::Key_PageUp) //
            || (k->key() == Qt::Key_PageDown) //
            || (k->key() == Qt::Key_Up) //
            || (k->key() == Qt::Key_Down)) {
            QApplication::sendEvent(d->table, e);
            d->table->setFocus();
            return true; // eat event
        }
    }
    return QFrame::eventFilter(o, e);
}

void KDatePicker::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void KDatePicker::dateChangedSlot(const QDate &date_)
{
    d->line->setText(locale().toString(date_, dateFormatWith4DigitYear(locale(), QLocale::ShortFormat)));
    d->selectMonth->setText(locale().standaloneMonthName(date_.month(), QLocale::LongFormat));
    d->fillWeeksCombo();

    // calculate the item num in the week combo box; normalize selected day so as if 1.1. is the first day of the week
    QDate firstDay(date_.year(), 1, 1);
    // If we cannot successfully create the 1st of the year, this can only mean that
    // the 1st is before the earliest valid date in the current calendar system, so use
    // the earliestValidDate as the first day.
    // In particular covers the case of Gregorian where 1/1/-4713 is not a valid QDate
    d->selectWeek->setCurrentIndex((date_.dayOfYear() + firstDay.dayOfWeek() - 2) / 7);
    d->selectYear->setText(locale().toString(date_, QStringLiteral("yyyy")).rightJustified(4, QLatin1Char('0')));

    Q_EMIT dateChanged(date_);
}

void KDatePicker::tableClickedSlot()
{
    Q_EMIT dateSelected(date());
    Q_EMIT tableClicked();
}

// TODO KF6: remove the const & from the returned QDate
const QDate &KDatePicker::date() const
{
    return d->table->date();
}

bool KDatePicker::setDate(const QDate &date_)
{
    // the table setDate does validity checking for us
    // this also emits dateChanged() which then calls our dateChangedSlot()
    return d->table->setDate(date_);
}

void KDatePicker::monthForwardClicked()
{
    if (!setDate(date().addMonths(1))) {
        QApplication::beep();
    }
    d->table->setFocus();
}

void KDatePicker::monthBackwardClicked()
{
    if (!setDate(date().addMonths(-1))) {
        QApplication::beep();
    }
    d->table->setFocus();
}

void KDatePicker::yearForwardClicked()
{
    if (!setDate(d->table->date().addYears(1))) {
        QApplication::beep();
    }
    d->table->setFocus();
}

void KDatePicker::yearBackwardClicked()
{
    if (!setDate(d->table->date().addYears(-1))) {
        QApplication::beep();
    }
    d->table->setFocus();
}

void KDatePicker::weekSelected(int index)
{
    QDate targetDay = d->selectWeek->itemData(index).toDateTime().date();

    if (!setDate(targetDay)) {
        QApplication::beep();
    }
    d->table->setFocus();
}

void KDatePicker::selectMonthClicked()
{
    QDate thisDate(date());
    d->table->setFocus();

    QMenu popup(d->selectMonth);
    // Populate the pick list with all the month names, this may change by year
    // JPL do we need to do something here for months that fall outside valid range?
    const int monthsInYear = QDate(thisDate.year() + 1, 1, 1).addDays(-1).month();
    for (int m = 1; m <= monthsInYear; m++) {
        popup.addAction(locale().standaloneMonthName(m))->setData(m);
    }

    QAction *item = popup.actions()[thisDate.month() - 1];
    // if this happens the above should already given an assertion
    if (item) {
        popup.setActiveAction(item);
    }

    // cancelled
    if ((item = popup.exec(d->selectMonth->mapToGlobal(QPoint(0, 0)), item)) == nullptr) {
        return;
    }

    // We need to create a valid date in the month selected so we can find out how many days are
    // in the month.
    QDate newDate(thisDate.year(), item->data().toInt(), 1);

    // If we have succeeded in creating a date in the new month, then try to create the new date,
    // checking we don't set a day after the last day of the month
    newDate.setDate(newDate.year(), newDate.month(), qMin(thisDate.day(), newDate.daysInMonth()));

    // Set the date, if it's invalid in any way then alert user and don't update
    if (!setDate(newDate)) {
        QApplication::beep();
    }
}

void KDatePicker::selectYearClicked()
{
    if (!d->selectYear->isChecked()) {
        return;
    }

    QDate thisDate(date());

    KPopupFrame *popup = new KPopupFrame(this);
    KDatePickerPrivateYearSelector *picker = new KDatePickerPrivateYearSelector(date(), popup);
    picker->resize(picker->sizeHint());
    picker->setYear(thisDate);
    picker->selectAll();
    popup->setMainWidget(picker);
    connect(picker, &KDatePickerPrivateYearSelector::closeMe, popup, &KPopupFrame::close);
    picker->setFocus();

    if (popup->exec(d->selectYear->mapToGlobal(QPoint(0, d->selectMonth->height())))) {
        // We need to create a valid date in the year/month selected so we can find out how many
        // days are in the month.
        QDate newDate(picker->year(), thisDate.month(), 1);

        // If we have succeeded in creating a date in the new month, then try to create the new
        // date, checking we don't set a day after the last day of the month
        newDate = QDate(newDate.year(), newDate.month(), qMin(thisDate.day(), newDate.daysInMonth()));

        // Set the date, if it's invalid in any way then alert user and don't update
        if (!setDate(newDate)) {
            QApplication::beep();
        }
    }
    delete popup;

    d->selectYear->setChecked(false);
}

void KDatePicker::uncheckYearSelector()
{
    d->selectYear->setChecked(false);
    d->selectYear->update();
}

void KDatePicker::changeEvent(QEvent *event)
{
    if (event && event->type() == QEvent::EnabledChange) {
        if (isEnabled()) {
            d->table->setFocus();
        }
    }
}

KDateTable *KDatePicker::dateTable() const
{
    return d->table;
}

void KDatePicker::lineEnterPressed()
{
    const QDate newDate = d->val->toDate(d->line->text());

    if (newDate.isValid()) {
        Q_EMIT dateEntered(newDate);
        setDate(newDate);
        d->table->setFocus();
    } else {
        QApplication::beep();
    }
}

void KDatePicker::todayButtonClicked()
{
    setDate(QDate::currentDate());
    d->table->setFocus();
}

QSize KDatePicker::sizeHint() const
{
    return QWidget::sizeHint();
}

void KDatePicker::setFontSize(int s)
{
    QWidget *const buttons[] = {
        d->selectMonth,
        d->selectYear,
    };
    QFont font;
    QRect r;
    // -----
    d->fontsize = s;
    for (QWidget *button : buttons) {
        font = button->font();
        font.setPointSize(s);
        button->setFont(font);
    }
    d->table->setFontSize(s);

    QFontMetrics metrics(d->selectMonth->fontMetrics());
    QString longestMonth;

    for (int i = 1;; ++i) {
        QString str = locale().standaloneMonthName(i, QLocale::LongFormat);
        if (str.isNull()) {
            break;
        }
        r = metrics.boundingRect(str);

        if (r.width() > d->maxMonthRect.width()) {
            d->maxMonthRect.setWidth(r.width());
            longestMonth = str;
        }
        if (r.height() > d->maxMonthRect.height()) {
            d->maxMonthRect.setHeight(r.height());
        }
    }

    QStyleOptionToolButton opt;
    opt.initFrom(d->selectMonth);
    opt.text = longestMonth;

    // stolen from QToolButton
    QSize textSize = metrics.size(Qt::TextShowMnemonic, longestMonth);
    textSize.setWidth(textSize.width() + metrics.horizontalAdvance(QLatin1Char(' ')) * 2);
    int w = textSize.width();
    int h = textSize.height();
    opt.rect.setHeight(h); // PM_MenuButtonIndicator depends on the height

    QSize metricBound = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(w, h), d->selectMonth);

    d->selectMonth->setMinimumSize(metricBound);
}

int KDatePicker::fontSize() const
{
    return d->fontsize;
}

void KDatePicker::setCloseButton(bool enable)
{
    if (enable == (d->closeButton != nullptr)) {
        return;
    }

    if (enable) {
        d->closeButton = new QToolButton(this);
        d->closeButton->setAutoRaise(true);
        const int horizontalSpacing = style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
        d->navigationLayout->addSpacing(horizontalSpacing);
        d->navigationLayout->addWidget(d->closeButton);
        d->closeButton->setToolTip(tr("Close", "@action:button"));
        d->closeButton->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        connect(d->closeButton, &QAbstractButton::clicked, topLevelWidget(), &QWidget::close);
    } else {
        delete d->closeButton;
        d->closeButton = nullptr;
    }

    updateGeometry();
}

bool KDatePicker::hasCloseButton() const
{
    return (d->closeButton);
}

#include "kdatepicker.moc"
