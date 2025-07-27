/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "selectexpirydate.h"

#include <QCheckBox>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KDatePicker>
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QPushButton>

SelectExpiryDate::SelectExpiryDate(QWidget* parent, QDateTime date)
	: QDialog(parent)
{
	setWindowTitle(i18n("Choose New Expiration"));
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	QWidget *mainWidget = new QWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(mainWidget);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &SelectExpiryDate::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &SelectExpiryDate::reject);
	okButton->setDefault(true);

	QWidget *page = new QWidget(this);
	m_unlimited = new QCheckBox(i18nc("Key has unlimited lifetime", "Unlimited"), page);
	m_unlimited->setChecked(date.isNull());

	if (date.isNull())
		date = QDateTime::currentDateTime();

	m_datepicker = new KDatePicker(date.date(), page);
	if (date.isNull()) {
		m_datepicker->setEnabled(false);
		m_unlimited->setChecked(true);
	}

	QVBoxLayout *layout = new QVBoxLayout(page);
	layout->setSpacing(3);
	layout->addWidget(m_datepicker);
	layout->addWidget(m_unlimited);

	connect(m_unlimited, &QCheckBox::toggled, this, &SelectExpiryDate::slotEnableDate);
	connect(m_datepicker, &KDatePicker::dateChanged, this, &SelectExpiryDate::slotCheckDate);
	connect(m_datepicker, &KDatePicker::dateEntered, this, &SelectExpiryDate::slotCheckDate);

	mainLayout->addWidget(page);
	mainLayout->addWidget(buttonBox);
	show();

	slotEnableDate(m_unlimited->isChecked());
}

QDateTime SelectExpiryDate::date() const
{
	if (m_unlimited->isChecked())
		return QDateTime();
	else
		return QDateTime(m_datepicker->date());
}

void SelectExpiryDate::slotCheckDate(const QDate& date)
{
	okButton->setEnabled(QDateTime(date) >= QDateTime::currentDateTime());
}

void SelectExpiryDate::slotEnableDate(const bool ison)
{
	m_datepicker->setEnabled(!ison);
	if (ison)
		okButton->setEnabled(true);
	else
		slotCheckDate(m_datepicker->date());
}
