/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SELECTEXPIRYDATE_H
#define SELECTEXPIRYDATE_H

#include <QDateTime>
#include <QDialog>

class KDatePicker;
class QCheckBox;
class QPushButton;

/**
 * @brief shows a dialog to choose expiry date
 *
 * This dialog allows the user to choose a expiry date or set a
 * checkbox to select that the given item will never expire.
 */
class SelectExpiryDate : public QDialog
{
	Q_OBJECT

public:
	explicit SelectExpiryDate(QWidget *parent = nullptr, QDateTime date = QDateTime());

	QDateTime date() const;

private Q_SLOTS:
	void slotCheckDate(const QDate &date);
	void slotEnableDate(const bool ison);

private:
	QCheckBox *m_unlimited;
	QPushButton *okButton;
	KDatePicker *m_datepicker;
};

#endif /* SELECTEXPIRYDATE_H */
