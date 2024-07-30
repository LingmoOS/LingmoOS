/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdatecombobox.h"

#include "common_helpers_p.h"
#include "kdatepickerpopup.h"
#include "kdaterangecontrol_p.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDate>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QScreen>
#include <QWidgetAction>

#include "kdatepicker.h"
#include "kmessagebox.h"

class KDateComboBoxPrivate : public KDateRangeControlPrivate
{
public:
    KDateComboBoxPrivate(KDateComboBox *qq);

    // TODO: Find a way to get that from QLocale
#if 0
    QDate defaultMinDate();
    QDate defaultMaxDate();
#endif

    QString dateFormat(QLocale::FormatType format);
    QString formatDate(const QDate &date);

    void initDateWidget();
    void updateDateWidget();
    void setDateRange(const QDate &minDate, const QDate &maxDate, const QString &minWarnMsg, const QString &maxWarnMsg);
    using KDateRangeControlPrivate::setDateRange;

    void editDate(const QString &text);
    void enterDate(const QDate &date);
    void parseDate();
    void warnDate();

    KDateComboBox *const q;
    KDatePickerPopup *m_dateMenu;

    QDate m_date;
    KDateComboBox::Options m_options;
    QString m_minWarnMsg;
    QString m_maxWarnMsg;
    bool m_warningShown;
    bool m_edited; // and dateChanged not yet emitted
    QLocale::FormatType m_displayFormat;
};

KDateComboBoxPrivate::KDateComboBoxPrivate(KDateComboBox *qq)
    : q(qq)
    , m_dateMenu(new KDatePickerPopup(KDatePickerPopup::DatePicker | KDatePickerPopup::Words, QDate::currentDate(), qq))
    , m_warningShown(false)
    , m_edited(false)
    , m_displayFormat(QLocale::ShortFormat)
{
    m_options = KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords;
    m_date = QDate::currentDate();
    // m_minDate = defaultMinDate();
    // m_maxDate = defaultMaxDate();
}

#if 0
QDate KDateComboBoxPrivate::defaultMinDate()
{
    return m_date.calendar()->earliestValidDate();
}

QDate KDateComboBoxPrivate::defaultMaxDate()
{
    return m_date.calendar()->latestValidDate();
}
#endif

QString KDateComboBoxPrivate::dateFormat(QLocale::FormatType format)
{
    return dateFormatWith4DigitYear(q->locale(), format);
}

QString KDateComboBoxPrivate::formatDate(const QDate &date)
{
    return q->locale().toString(date, dateFormat(m_displayFormat));
}

void KDateComboBoxPrivate::initDateWidget()
{
    q->blockSignals(true);
    q->clear();

    // If EditTime then set the line edit
    q->lineEdit()->setReadOnly((m_options & KDateComboBox::EditDate) != KDateComboBox::EditDate);

    // If SelectDate then make list items visible
    if ((m_options & KDateComboBox::SelectDate) == KDateComboBox::SelectDate //
        || (m_options & KDateComboBox::DatePicker) == KDateComboBox::DatePicker //
        || (m_options & KDateComboBox::DatePicker) == KDateComboBox::DateKeywords) {
        q->setMaxVisibleItems(1);
    } else {
        q->setMaxVisibleItems(0);
    }

    q->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    q->addItem(formatDate(m_date));
    q->setCurrentIndex(0);
    q->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    q->blockSignals(false);

    KDatePickerPopup::Modes modes;
    if (m_options & KDateComboBox::DatePicker) {
        modes |= KDatePickerPopup::DatePicker;
    }
    if (m_options & KDateComboBox::DateKeywords) {
        modes |= KDatePickerPopup::Words;
    }
    m_dateMenu->setModes(modes);
}

void KDateComboBoxPrivate::updateDateWidget()
{
    q->blockSignals(true);
    m_dateMenu->setDate(m_date);
    int pos = q->lineEdit()->cursorPosition();
    q->setItemText(0, formatDate(m_date));
    q->lineEdit()->setText(formatDate(m_date));
    q->lineEdit()->setCursorPosition(pos);
    q->blockSignals(false);
}

void KDateComboBoxPrivate::setDateRange(const QDate &minDate, const QDate &maxDate, const QString &minWarnMsg, const QString &maxWarnMsg)
{
    if (!setDateRange(minDate, maxDate)) {
        return;
    }

    m_dateMenu->setDateRange(minDate, maxDate);
    m_minWarnMsg = minWarnMsg;
    m_maxWarnMsg = maxWarnMsg;
}

void KDateComboBoxPrivate::editDate(const QString &text)
{
    m_warningShown = false;
    m_date = q->locale().toDate(text, dateFormat(m_displayFormat));
    m_edited = true;
    Q_EMIT q->dateEdited(m_date);
}

void KDateComboBoxPrivate::parseDate()
{
    m_date = q->locale().toDate(q->lineEdit()->text(), dateFormat(m_displayFormat));
}

void KDateComboBoxPrivate::enterDate(const QDate &date)
{
    q->setDate(date);
    // Re-add the combo box item in order to retain the correct widget width
    q->blockSignals(true);
    q->clear();
    q->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    q->addItem(formatDate(m_date));
    q->setCurrentIndex(0);
    q->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    q->blockSignals(false);

    m_dateMenu->hide();
    warnDate();
    Q_EMIT q->dateEntered(m_date);
}

void KDateComboBoxPrivate::warnDate()
{
    if (!m_warningShown && !q->isValid() && (m_options & KDateComboBox::WarnOnInvalid) == KDateComboBox::WarnOnInvalid) {
        QString warnMsg;
        if (!m_date.isValid()) {
            warnMsg = KDateComboBox::tr("The date you entered is invalid", "@info");
        } else if (m_minDate.isValid() && m_date < m_minDate) {
            if (m_minWarnMsg.isEmpty()) {
                warnMsg = KDateComboBox::tr("Date cannot be earlier than %1", "@info").arg(formatDate(m_minDate));
            } else {
                warnMsg = m_minWarnMsg;
                warnMsg.replace(QLatin1String("%1"), formatDate(m_minDate));
            }
        } else if (m_maxDate.isValid() && m_date > m_maxDate) {
            if (m_maxWarnMsg.isEmpty()) {
                warnMsg = KDateComboBox::tr("Date cannot be later than %1", "@info").arg(formatDate(m_maxDate));
            } else {
                warnMsg = m_maxWarnMsg;
                warnMsg.replace(QLatin1String("%1"), formatDate(m_maxDate));
            }
        }
        m_warningShown = true;
        KMessageBox::error(q, warnMsg);
    }
}

KDateComboBox::KDateComboBox(QWidget *parent)
    : QComboBox(parent)
    , d(new KDateComboBoxPrivate(this))
{
    setEditable(true);
    setMaxVisibleItems(1);
    setInsertPolicy(QComboBox::NoInsert);
    d->initDateWidget();
    d->updateDateWidget();

    connect(d->m_dateMenu, &KDatePickerPopup::dateChanged, this, [this](QDate date) {
        if (d->isInDateRange(date)) {
            d->enterDate(date);
        }
    });

    connect(this, &QComboBox::editTextChanged, this, [this](const QString &text) {
        d->editDate(text);
    });

    connect(lineEdit(), &QLineEdit::returnPressed, this, [this]() {
        if (d->m_edited) {
            d->enterDate(date());
            Q_EMIT dateChanged(date());
        }
    });
}

KDateComboBox::~KDateComboBox() = default;

QDate KDateComboBox::date() const
{
    d->parseDate();
    return d->m_date;
}

void KDateComboBox::setDate(const QDate &date)
{
    if (date == d->m_date) {
        return;
    }

    d->m_edited = false;
    assignDate(date);
    d->updateDateWidget();
    Q_EMIT dateChanged(d->m_date);
}

void KDateComboBox::assignDate(const QDate &date)
{
    d->m_date = date;
}

bool KDateComboBox::isValid() const
{
    d->parseDate();
    return d->isInDateRange(d->m_date);
}

bool KDateComboBox::isNull() const
{
    return lineEdit()->text().isEmpty();
}

KDateComboBox::Options KDateComboBox::options() const
{
    return d->m_options;
}

void KDateComboBox::setOptions(Options options)
{
    if (options != d->m_options) {
        d->m_options = options;
        d->initDateWidget();
        d->updateDateWidget();
    }
}

QDate KDateComboBox::minimumDate() const
{
    return d->m_minDate;
}

void KDateComboBox::setMinimumDate(const QDate &minDate, const QString &minWarnMsg)
{
    if (minDate.isValid()) {
        d->setDateRange(minDate, d->m_maxDate, minWarnMsg, d->m_maxWarnMsg);
    }
}

void KDateComboBox::resetMinimumDate()
{
    d->setDateRange(QDate(), d->m_maxDate, QString(), d->m_maxWarnMsg);
}

QDate KDateComboBox::maximumDate() const
{
    return d->m_maxDate;
}

void KDateComboBox::setMaximumDate(const QDate &maxDate, const QString &maxWarnMsg)
{
    if (maxDate.isValid()) {
        d->setDateRange(d->m_minDate, maxDate, d->m_minWarnMsg, maxWarnMsg);
    }
}

void KDateComboBox::resetMaximumDate()
{
    d->setDateRange(d->m_minDate, QDate(), d->m_minWarnMsg, QString());
}

void KDateComboBox::setDateRange(const QDate &minDate, const QDate &maxDate, const QString &minWarnMsg, const QString &maxWarnMsg)
{
    if (minDate.isValid() && maxDate.isValid()) {
        d->setDateRange(minDate, maxDate, minWarnMsg, maxWarnMsg);
    }
}

void KDateComboBox::resetDateRange()
{
    d->setDateRange(QDate(), QDate(), QString(), QString());
}

QLocale::FormatType KDateComboBox::displayFormat() const
{
    return d->m_displayFormat;
}

void KDateComboBox::setDisplayFormat(QLocale::FormatType format)
{
    if (format != d->m_displayFormat) {
        d->m_displayFormat = format;
        d->initDateWidget();
        d->updateDateWidget();
    }
}

QMap<QDate, QString> KDateComboBox::dateMap() const
{
    return d->m_dateMenu->dateMap();
}

void KDateComboBox::setDateMap(QMap<QDate, QString> dateMap)
{
    d->m_dateMenu->setDateMap(dateMap);
}

bool KDateComboBox::eventFilter(QObject *object, QEvent *event)
{
    return QComboBox::eventFilter(object, event);
}

void KDateComboBox::keyPressEvent(QKeyEvent *keyEvent)
{
    QDate temp;
    switch (keyEvent->key()) {
    case Qt::Key_Down:
        temp = d->m_date.addDays(-1);
        break;
    case Qt::Key_Up:
        temp = d->m_date.addDays(1);
        break;
    case Qt::Key_PageDown:
        temp = d->m_date.addMonths(-1);
        break;
    case Qt::Key_PageUp:
        temp = d->m_date.addMonths(1);
        break;
    default:
        QComboBox::keyPressEvent(keyEvent);
        return;
    }
    if (d->isInDateRange(temp)) {
        d->enterDate(temp);
    }
}

void KDateComboBox::focusOutEvent(QFocusEvent *event)
{
    d->parseDate();
    d->warnDate();
    if (d->m_edited) {
        d->m_edited = false;
        Q_EMIT dateChanged(d->m_date);
    }
    QComboBox::focusOutEvent(event);
}

void KDateComboBox::showPopup()
{
    if (!isEditable() || !d->m_dateMenu //
        || (d->m_options & KDateComboBox::SelectDate) != KDateComboBox::SelectDate) {
        return;
    }

    d->m_dateMenu->setDate(d->m_date);

    const QRect desk = screen()->geometry();

    QPoint popupPoint = mapToGlobal(QPoint(0, 0));

    const int dateFrameHeight = d->m_dateMenu->sizeHint().height();
    if (popupPoint.y() + height() + dateFrameHeight > desk.bottom()) {
        popupPoint.setY(popupPoint.y() - dateFrameHeight);
    } else {
        popupPoint.setY(popupPoint.y() + height());
    }

    const int dateFrameWidth = d->m_dateMenu->sizeHint().width();
    if (popupPoint.x() + dateFrameWidth > desk.right()) {
        popupPoint.setX(desk.right() - dateFrameWidth);
    }

    if (popupPoint.x() < desk.left()) {
        popupPoint.setX(desk.left());
    }

    if (popupPoint.y() < desk.top()) {
        popupPoint.setY(desk.top());
    }

    d->m_dateMenu->popup(popupPoint);
}

void KDateComboBox::hidePopup()
{
    QComboBox::hidePopup();
}

void KDateComboBox::mousePressEvent(QMouseEvent *event)
{
    QComboBox::mousePressEvent(event);
}

void KDateComboBox::wheelEvent(QWheelEvent *event)
{
    QDate temp;
    if (event->angleDelta().y() < 0) {
        temp = d->m_date.addDays(-1);
    } else {
        temp = d->m_date.addDays(1);
    }
    if (d->isInDateRange(temp)) {
        d->enterDate(temp);
    }
}

void KDateComboBox::focusInEvent(QFocusEvent *event)
{
    QComboBox::focusInEvent(event);
}

void KDateComboBox::resizeEvent(QResizeEvent *event)
{
    QComboBox::resizeEvent(event);
}

#include "moc_kdatecombobox.cpp"
