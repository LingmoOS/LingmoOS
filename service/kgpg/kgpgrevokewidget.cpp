/*
    SPDX-FileCopyrightText: 2002, 2003 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2010, 2012, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "kgpgrevokewidget.h"

#include "core/KGpgKeyNode.h"
#include "kgpgsettings.h"

#include <QUrl>
#include <QDir>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

KgpgRevokeWidget::KgpgRevokeWidget(QWidget* parent)
	: QWidget(parent),
	Ui_KgpgRevokeWidget()
{
	setupUi(this);
	connect(cbSave, &QCheckBox::toggled, this, &KgpgRevokeWidget::cbSave_toggled);
}

void KgpgRevokeWidget::cbSave_toggled(bool isOn)
{
	outputFile->setEnabled(isOn);
}

KGpgRevokeDialog::KGpgRevokeDialog(QWidget* parent, const KGpgKeyNode *node)
	: QDialog(parent),
	m_revWidget(new KgpgRevokeWidget(this)),
	m_id(node->getId())
{
	setWindowTitle(i18n("Create Revocation Certificate"));
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	QWidget *mainWidget = new QWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(mainWidget);
	QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &KGpgRevokeDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KGpgRevokeDialog::reject);
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	setModal(true);

	m_revWidget->keyID->setText(i18nc("<Name> (<Email>) ID: <KeyId>", "%1 (%2) ID: %3",
				node->getName(), node->getEmail(), m_id));
	m_revWidget->outputFile->setUrl(revokeUrl(node->getName(), node->getEmail()));
	m_revWidget->outputFile->setMode(KFile::File);

	setMinimumSize(m_revWidget->sizeHint());
	mainLayout->addWidget(m_revWidget);
	mainLayout->addWidget(buttonBox);
}

QString KGpgRevokeDialog::getDescription() const
{
	return m_revWidget->textDescription->toPlainText();
}

int KGpgRevokeDialog::getReason() const
{
	return m_revWidget->comboBox1->currentIndex();
}

QUrl KGpgRevokeDialog::saveUrl() const
{
	if (m_revWidget->cbSave->isChecked())
		return m_revWidget->outputFile->url();
	else
		return QUrl();
}

QString KGpgRevokeDialog::getId() const
{
	return m_id;
}

bool KGpgRevokeDialog::printChecked()
{
	return m_revWidget->cbPrint->isChecked();
}

bool KGpgRevokeDialog::importChecked()
{
	return m_revWidget->cbImport->isChecked();
}

QUrl KGpgRevokeDialog::revokeUrl(const QString& name, const QString& email)
{
	QString revurl;
	const QString gpgPath(KGpgSettings::gpgConfigPath());
	if (!gpgPath.isEmpty())
		revurl = QUrl::fromLocalFile(gpgPath).adjusted(QUrl::RemoveFilename).path();
	else
		revurl = QDir::homePath() + QLatin1Char('/');

	if (!email.isEmpty())
		revurl += email.section(QLatin1Char('@'), 0, 0);
	else
		revurl += name.section(QLatin1Char(' '), 0, 0);

	return QUrl::fromLocalFile(revurl + QLatin1String( ".revoke" ));
}
