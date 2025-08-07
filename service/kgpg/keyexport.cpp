/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2011 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2011 Luis Ángel Fernández Fernández <laffdez@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyexport.h"
#include <KMessageBox>

KeyExport::KeyExport(QWidget *parent, const QStringList &keyservers)
	: QDialog(parent),
	Ui_KeyExport()
{
	setupUi(this);

	buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &KeyExport::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KeyExport::reject);
	newFilename->setWindowTitle(i18n("Save File"));
	newFilename->setMode(KFile::File);

	if (!keyservers.isEmpty()) {
		checkServer->setEnabled(true);
		checkServer->setToolTip(QString());
		destServer->addItems(keyservers);
	}

	connect(checkServer, &QRadioButton::toggled, destServer, &QComboBox::setEnabled);
        connect(checkFile, &QRadioButton::toggled, newFilename, &KUrlRequester::setEnabled);
}

void KeyExport::accept()
{
	if (checkFile->isChecked()) {
		if (QFile::exists(newFilename->url().path().simplified())) {
			const QString message = i18n("Overwrite existing file %1?", newFilename->url().fileName());
			int result = KMessageBox::warningContinueCancel(this, message, QString(), KStandardGuiItem::overwrite());
			if (KMessageBox::Cancel == result)
				return;
		}
	}

	QDialog::accept();
}
