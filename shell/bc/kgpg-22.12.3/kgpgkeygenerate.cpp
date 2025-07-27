/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2009, 2010, 2012, 2013, 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgkeygenerate.h"

#include "core/convert.h"
#include "core/emailvalidator.h"
#include "gpgproc.h"
#include "kgpgsettings.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QStringList>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QWidget>

using namespace KgpgCore;

KgpgKeyGenerate::KgpgKeyGenerate(QWidget *parent)
	: QDialog(parent),
	m_expert(false)
{
    setupUi(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KgpgKeyGenerate::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KgpgKeyGenerate::reject);

    user1Button->setText(i18n("&Expert Mode"));
    user1Button->setToolTip(i18n("Go to Expert Mode"));
    user1Button->setWhatsThis(i18n( "If you go to expert mode, you will use the command line to create your key." ));

    connect(m_kname, &QLineEdit::textChanged, this, &KgpgKeyGenerate::slotEnableOk);

    m_keyexp->setMinimumSize(m_keyexp->sizeHint());
    connect(m_keyexp, QOverload<int>::of(&QComboBox::activated), this, &KgpgKeyGenerate::slotEnableDays);

    m_keysize->addItem(i18n("1024"));
    m_keysize->addItem(i18n("2048"));
    m_keysize->addItem(i18n("4096"));
    m_keysize->setCurrentIndex(1); // 2048
    m_keysize->setMinimumSize(m_keysize->sizeHint());

    const QStringList pkAlgos = GPGProc::getGpgPubkeyAlgorithms(KGpgSettings::gpgBinaryPath());
    if (pkAlgos.contains(QLatin1String("RSA"))) {
	m_keykind->addItem(KgpgCore::Convert::toString(KgpgCore::ALGO_RSA_RSA));
	m_keykind->addItem(KgpgCore::Convert::toString(KgpgCore::ALGO_RSA));
    }
    if (pkAlgos.contains(QLatin1String("DSA")) && pkAlgos.contains(QLatin1String("ELG")))
	m_keykind->addItem(KgpgCore::Convert::toString(KgpgCore::ALGO_DSA_ELGAMAL));
    m_keykind->setCurrentIndex(0); // normally RSA+RSA
    slotEnableCaps(m_keykind->currentIndex());
    m_keykind->setMinimumSize(m_keykind->sizeHint());

    mainLayout->addWidget(vgroup);
    mainLayout->addWidget(buttonBox);

    slotEnableOk();
    updateGeometry();
    show();

    connect(okButton, &QPushButton::clicked, this, &KgpgKeyGenerate::slotOk);
    connect(user1Button, &QPushButton::clicked, this, &KgpgKeyGenerate::slotUser1);
    connect(m_keykind, QOverload<int>::of(&QComboBox::activated), this, &KgpgKeyGenerate::slotEnableCaps);
}

void KgpgKeyGenerate::slotOk()
{
    if (m_kname->text().simplified().isEmpty())
    {
        KMessageBox::error(this, i18n("You must give a name."));
        return;
    }

    if (m_kname->text().simplified().length() < 5)
    {
        KMessageBox::error(this, i18n("The name must have at least 5 characters"));
        return;
    }

    if (m_kname->text().simplified().at(0).isDigit())
    {
        KMessageBox::error(this, i18n("The name must not start with a digit"));
        return;
    }

    QString vmail = m_mail->text();
    if (vmail.isEmpty())
    {
        int result = KMessageBox::warningContinueCancel(this, i18n("You are about to create a key with no email address"));
        if (result != KMessageBox::Continue)
            return;
    } else {
        int pos = 0;
        if (EmailValidator().validate(vmail, pos) == QValidator::Invalid)
        {
            KMessageBox::error(this, i18n("Email address not valid"));
            return;
        }
    }

    accept();
}

void KgpgKeyGenerate::slotUser1()
{
    m_expert = true;
    accept();
}

void KgpgKeyGenerate::slotEnableDays(const int state)
{
    m_days->setDisabled(state == 0);
}

void KgpgKeyGenerate::slotEnableCaps(const int state)
{
    // currently only supported for standalone RSA keys
    capabilities->setDisabled(state != 2);
}

void KgpgKeyGenerate::slotEnableOk()
{
	okButton->setEnabled((m_kname->text().simplified().length() >= 5) &&
			!m_kname->text().simplified().at(0).isDigit());
}

bool KgpgKeyGenerate::isExpertMode() const
{
    return m_expert;
}

KgpgCore::KgpgKeyAlgo KgpgKeyGenerate::algo() const
{
	if (m_keykind->currentText() == KgpgCore::Convert::toString(KgpgCore::ALGO_RSA))
		return KgpgCore::ALGO_RSA;
	else if (m_keykind->currentText() == KgpgCore::Convert::toString(KgpgCore::ALGO_RSA_RSA))
		return KgpgCore::ALGO_RSA_RSA;
	else
		return KgpgCore::ALGO_DSA_ELGAMAL;
}

KgpgCore::KgpgSubKeyType KgpgKeyGenerate::caps() const
{
	KgpgCore::KgpgSubKeyType ret;

	if (!capabilities->isEnabled())
		return ret;

	if (capAuth->isChecked())
		ret |= KgpgCore::SKT_AUTHENTICATION;
	if (capCert->isChecked())
		ret |= KgpgCore::SKT_CERTIFICATION;
	if (capEncrypt->isChecked())
		ret |= KgpgCore::SKT_ENCRYPTION;
	if (capSign->isChecked())
		ret |= KgpgCore::SKT_SIGNATURE;

	return ret;
}

uint KgpgKeyGenerate::size() const
{
    return m_keysize->currentText().toUInt();
}

char KgpgKeyGenerate::expiration() const
{
	switch (m_keyexp->currentIndex()) {
	case 1:
		return 'd';
	case 2:
		return 'w';
	case 3:
		return 'm';
	case 4:
	default:
		return 'y';
	}
}

uint KgpgKeyGenerate::days() const
{
    if (m_days->text().isEmpty())
        return 0;
    return m_days->text().toUInt();
}

QString KgpgKeyGenerate::name() const
{
    if (m_kname->text().isEmpty())
        return QString();
    return m_kname->text();
}

QString KgpgKeyGenerate::email() const
{
    if (m_mail->text().isEmpty())
        return QString();
    return m_mail->text();
}

QString KgpgKeyGenerate::comment() const
{
    if (m_comment->text().isEmpty())
        return QString();
    return m_comment->text();
}
