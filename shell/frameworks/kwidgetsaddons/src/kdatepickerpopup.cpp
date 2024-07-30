/*
  SPDX-FileCopyrightText: 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdatepickerpopup.h"
#include "kdatepicker.h"
#include "kdaterangecontrol_p.h"

#include <QDateTime>
#include <QLocale>
#include <QWidgetAction>

class KDatePickerAction : public QWidgetAction
{
public:
    KDatePickerAction(KDatePicker *widget, QObject *parent)
        : QWidgetAction(parent)
        , mDatePicker(widget)
        , mOriginalParent(widget->parentWidget())
    {
    }

protected:
    QWidget *createWidget(QWidget *parent) override
    {
        mDatePicker->setParent(parent);
        return mDatePicker;
    }

    void deleteWidget(QWidget *widget) override
    {
        if (widget != mDatePicker) {
            return;
        }

        mDatePicker->setParent(mOriginalParent);
    }

private:
    KDatePicker *const mDatePicker;
    QWidget *const mOriginalParent;
};

class KDatePickerPopupPrivate : public KDateRangeControlPrivate
{
public:
    explicit KDatePickerPopupPrivate(KDatePickerPopup *qq)
        : q(qq)
    {
    }

    void addMenuAction(const QString &text, const QDate &date);
    void buildMenu();
    void menuActionTriggered(QAction *action);
    void slotDateChanged(QDate);

    KDatePickerPopup *const q;
    KDatePicker *mDatePicker = nullptr;
    KDatePickerPopup::Modes mModes;
    QMap<QDate, QString> m_dateMap;
};

void KDatePickerPopupPrivate::addMenuAction(const QString &text, const QDate &date)
{
    // skip out-of-range dates
    // an invalid date is ok though, that's for the "No Date" action
    if (date.isValid() && !isInDateRange(date)) {
        return;
    }

    QAction *action = new QAction(q);
    action->setText(text);
    action->setData(date);
    QObject::connect(action, &QAction::triggered, q, [this, action]() {
        Q_EMIT q->dateChanged(action->data().toDate());
    });
    q->addAction(action);
}

void KDatePickerPopupPrivate::buildMenu()
{
    q->clear();

    if (mModes & KDatePickerPopup::DatePicker) {
        q->addAction(new KDatePickerAction(mDatePicker, q));

        if ((mModes & KDatePickerPopup::NoDate) || (mModes & KDatePickerPopup::Words)) {
            q->addSeparator();
        }
    }

    if (mModes & KDatePickerPopup::Words) {
        if (m_dateMap.isEmpty()) {
            const QDate today = QDate::currentDate();
            addMenuAction(KDatePickerPopup::tr("Next Year", "@option next year"), today.addYears(1));
            addMenuAction(KDatePickerPopup::tr("Next Month", "@option next month"), today.addMonths(1));
            addMenuAction(KDatePickerPopup::tr("Next Week", "@option next week"), today.addDays(7));
            addMenuAction(KDatePickerPopup::tr("Tomorrow", "@option tomorrow"), today.addDays(1));
            addMenuAction(KDatePickerPopup::tr("Today", "@option today"), today);
            addMenuAction(KDatePickerPopup::tr("Yesterday", "@option yesterday"), today.addDays(-1));
            addMenuAction(KDatePickerPopup::tr("Last Week", "@option last week"), today.addDays(-7));
            addMenuAction(KDatePickerPopup::tr("Last Month", "@option last month"), today.addMonths(-1));
            addMenuAction(KDatePickerPopup::tr("Last Year", "@option last year"), today.addYears(-1));
        } else {
            for (auto it = m_dateMap.constBegin(); it != m_dateMap.constEnd(); ++it) {
                if (it.value().isEmpty()) {
                    addMenuAction(QLocale().toString(it.key()), it.key());
                } else if (it.value().toLower() == QLatin1String("separator")) {
                    q->addSeparator();
                } else {
                    addMenuAction(it.value(), it.key());
                }
            }
        }

        if (mModes & KDatePickerPopup::NoDate) {
            q->addSeparator();
        }
    }

    if (mModes & KDatePickerPopup::NoDate) {
        addMenuAction(KDatePickerPopup::tr("No Date", "@option do not specify a date"), QDate());
    }
}

void KDatePickerPopupPrivate::slotDateChanged(QDate date)
{
    Q_EMIT q->dateChanged(date);
    q->hide();
}

KDatePickerPopup::KDatePickerPopup(Modes modes, QDate date, QWidget *parent)
    : QMenu(parent)
    , d(new KDatePickerPopupPrivate(this))
{
    d->mModes = modes;

    d->mDatePicker = new KDatePicker(this);
    d->mDatePicker->setCloseButton(false);

    connect(d->mDatePicker, &KDatePicker::dateEntered, this, [this](QDate date) {
        d->slotDateChanged(date);
    });
    connect(d->mDatePicker, &KDatePicker::dateSelected, this, [this](QDate date) {
        d->slotDateChanged(date);
    });

    d->mDatePicker->setDate(date);

    connect(this, &QMenu::aboutToShow, this, [this]() {
        d->buildMenu();
    });
}

KDatePickerPopup::~KDatePickerPopup() = default;

KDatePicker *KDatePickerPopup::datePicker() const
{
    return d->mDatePicker;
}

void KDatePickerPopup::setDate(QDate date)
{
    d->mDatePicker->setDate(date);
}

KDatePickerPopup::Modes KDatePickerPopup::modes() const
{
    return d->mModes;
}

void KDatePickerPopup::setModes(KDatePickerPopup::Modes modes)
{
    d->mModes = modes;
}

void KDatePickerPopup::setDateRange(const QDate &minDate, const QDate &maxDate)
{
    d->setDateRange(minDate, maxDate);
}

QMap<QDate, QString> KDatePickerPopup::dateMap() const
{
    return d->m_dateMap;
}

void KDatePickerPopup::setDateMap(const QMap<QDate, QString> &dateMap)
{
    d->m_dateMap = dateMap;
}

#include "moc_kdatepickerpopup.cpp"
