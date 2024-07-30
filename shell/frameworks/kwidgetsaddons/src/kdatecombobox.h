/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATECOMBOBOX_H
#define KDATECOMBOBOX_H

#include <kwidgetsaddons_export.h>

#include <QComboBox>
#include <QLocale>
#include <memory>

/**
 * @class KDateComboBox kdatecombobox.h KDateComboBox
 *
 * @short A combobox for dates.
 */
class KWIDGETSADDONS_EXPORT KDateComboBox : public QComboBox
{
    Q_OBJECT

    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate RESET resetMinimumDate)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate RESET resetMaximumDate)
    Q_PROPERTY(Options options READ options WRITE setOptions)

public:
    /**
     * Options provided by the widget
     * @see options()
     * @see setOptions()
     * @see Options
     */
    enum Option {
        EditDate = 0x0001, /**< Allow the user to manually edit the date in the combo line edit */
        SelectDate = 0x0002, /**< Allow the user to select the date from a drop-down menu */
        DatePicker = 0x0004, /**< Show a date picker in the drop-down */
        DateKeywords = 0x0008, /**< Show date keywords in the drop-down */
        WarnOnInvalid = 0x0010, /**< Show a warning on focus out if the date is invalid */
    };
    /**
     * Stores a combination of #Option values.
     */
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /**
     * Create a new KDateComboBox widget
     *
     * By default the EditDate, SelectDate, DatePicker and DateKeywords options
     * are enabled, the ShortDate format is used and the date is set to the
     * current date.
     */
    explicit KDateComboBox(QWidget *parent = nullptr);

    /**
     * Destroy the widget
     */
    ~KDateComboBox() override;

    /**
     * Return the currently selected date
     *
     * @return the currently selected date
     */
    QDate date() const;

    /**
     * Return if the current user input is valid
     *
     * If the user input is null then it is not valid
     *
     * @return if the current user input is valid
     *
     * @see isNull()
     */
    bool isValid() const;

    /**
     * Return if the current user input is null
     *
     * @return if the current user input is null
     *
     * @see isValid()
     */
    bool isNull() const;

    /**
     * Return the currently set widget options
     *
     * @return the currently set widget options
     */
    Options options() const;

    /**
     * Return the currently set date display format
     *
     * By default this is the Short Format
     *
     * @return the currently set date format
     */
    QLocale::FormatType displayFormat() const;

    /**
     * Return the current minimum date
     *
     * @return the current minimum date
     */
    QDate minimumDate() const;

    /**
     * Return the current maximum date
     *
     * @return the current maximum date
     */
    QDate maximumDate() const;

    /**
     * Return the map of dates listed in the drop-down and their displayed
     * string forms.
     *
     * @return the select date map
     *
     * @see setDateMap()
     */
    QMap<QDate, QString> dateMap() const;

Q_SIGNALS:

    /**
     * Signal if the date has been manually entered (by typing a date and losing focus, or pressing Enter)
     * or selected by the user (using the popup selector, or up, down, page up, page down keys, or the mouse wheel).
     *
     * The emitted date may be invalid.
     *
     * @param date the new date
     */
    void dateEntered(const QDate &date);

    /**
     * Signal if the date has been changed either manually by the user
     * or programmatically.
     *
     * The emitted date may be invalid.
     *
     * @param date the new date
     */
    void dateChanged(const QDate &date);

    /**
     * Signal if the date is being manually edited by the user.
     *
     * The emitted date may be invalid, or may not yet be what the user intends as the final date.
     *
     * @param date the new date
     */
    void dateEdited(const QDate &date);

public Q_SLOTS:

    /**
     * Set the currently selected date
     *
     * You can set an invalid date or a date outside the valid range, validity
     * checking is only done via isValid().
     *
     * @param date the new date
     */
    void setDate(const QDate &date);

    /**
     * Set the new widget options
     *
     * @param options the new widget options
     */
    void setOptions(Options options);

    /**
     * Sets the date format to display.
     *
     * By default is the Short Format.
     *
     * @param format the date format to use
     */
    void setDisplayFormat(QLocale::FormatType format);

    /**
     * Set the valid date range to be applied by isValid().
     *
     * Both dates must be valid and the minimum date must be less than or equal
     * to the maximum date, otherwise the date range will not be set.
     *
     * @param minDate the minimum date
     * @param maxDate the maximum date
     * @param minWarnMsg the minimum warning message
     * @param maxWarnMsg the maximum warning message
     */
    void setDateRange(const QDate &minDate, const QDate &maxDate, const QString &minWarnMsg = QString(), const QString &maxWarnMsg = QString());

    /**
     * Reset the minimum and maximum date to the default values.
     * @see setDateRange()
     */
    void resetDateRange();

    /**
     * Set the minimum allowed date.
     *
     * If the date is invalid, or greater than current maximum,
     * then the minimum will not be set.
     *
     * @param minDate the minimum date
     * @param minWarnMsg the minimum warning message
     *
     * @see minimumDate()
     * @see maximumDate()
     * @see setMaximumDate()
     * @see setDateRange()
     */
    void setMinimumDate(const QDate &minDate, const QString &minWarnMsg = QString());

    /**
     * Reset the minimum date to the default.
     *
     * The default is to have no minimum date.
     */
    void resetMinimumDate();

    /**
     * Set the maximum allowed date.
     *
     * If the date is invalid, or less than current minimum,
     * then the maximum will not be set.
     *
     * @param maxDate the maximum date
     * @param maxWarnMsg the maximum warning message
     *
     * @see minimumDate()
     * @see maximumDate()
     * @see setMaximumDate()
     * @see setDateRange()
     */
    void setMaximumDate(const QDate &maxDate, const QString &maxWarnMsg = QString());

    /**
     * Reset the maximum date to the default
     *
     * The default is to have no maximum date.
     */
    void resetMaximumDate();

    /**
     * Set the list of dates able to be selected from the drop-down and the
     * string form to display for those dates, e.g. "2010-01-01" and "Yesterday".
     *
     * Any invalid or duplicate dates will be used, the list will NOT be
     * sorted, and the minimum and maximum date will not be affected.
     *
     * The @p dateMap is keyed by the date to be listed and the value is the
     * string to be displayed.  If you want the date to be displayed in the
     * default date format then the string should be null.  If you want a
     * separator to be displayed then set the string to "separator".
     *
     * @param dateMap the map of dates able to be selected
     *
     * @see dateMap()
     */
    void setDateMap(QMap<QDate, QString> dateMap);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void showPopup() override;
    void hidePopup() override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    /**
     * Assign the date for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param date the new date
     */
    virtual void assignDate(const QDate &date);

private:
    friend class KDateComboBoxPrivate;
    std::unique_ptr<class KDateComboBoxPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDateComboBox::Options)

#endif // KDATECOMBOBOX_H
