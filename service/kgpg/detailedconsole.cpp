/*
    SPDX-FileCopyrightText: 2003, 2004 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2010, 2012, 2014, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikoans <andrius@stikonas.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "detailedconsole.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>

KgpgDetailedInfo::KgpgDetailedInfo(QWidget *parent, const QString &boxLabel, const QString &errormessage,
		const QStringList &keysList, const QString &caption)
	: QDialog(parent)
{
	if (!caption.isEmpty())
		setWindowTitle(caption);
	else
		setWindowTitle(i18n("Info"));

	// FIXME: hopefully KMessageBox will provide a helper for this one day...
	// code copied from KMessageBox for now
	QPushButton *detailsButton = new QPushButton;
	detailsButton->setObjectName(QStringLiteral("detailsButton"));
	detailsButton->setText(QApplication::translate("KMessageBox", "&Details") + QStringLiteral(" >>"));
	detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
	buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
	buttonBox->addButton(QDialogButtonBox::Ok);
	buttonBox->button(QDialogButtonBox::Ok)->setFocus();

	QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &KgpgDetailedInfo::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KgpgDetailedInfo::reject);
	okButton->setDefault(true);
	setModal(true);
	KMessageBox::createKMessageBox(this, buttonBox, QMessageBox::Information, boxLabel, keysList, QString(), nullptr,
			KMessageBox::Options(), errormessage);
}
