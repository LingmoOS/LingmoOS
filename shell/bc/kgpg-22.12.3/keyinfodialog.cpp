/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2011 Philip Greggory Lee <rocketman768@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyinfodialog.h"

#include "kgpgchangekey.h"
#include <kgpgsettings.h>
#include "selectexpirydate.h"
#include "core/convert.h"
#include "core/images.h"
#include "core/kgpgkey.h"
#include "model/kgpgitemmodel.h"
#include "model/kgpgitemnode.h"
#include "transactions/kgpgchangepass.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLocale>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KgpgCore;

KgpgTrustLabel::KgpgTrustLabel(QWidget *parent)
	: QWidget(parent),
	m_text_w(new QLabel(this)),
	m_color_w(new QLabel(this))
{
    m_text_w->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_color_w->setLineWidth(1);
    m_color_w->setFrameShape(QFrame::Box);
    m_color_w->setAutoFillBackground(true);
    m_color_w->setMinimumWidth(64);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_text_w);
    layout->addWidget(m_color_w);

    change();
}

void KgpgTrustLabel::setText(const QString &text)
{
    m_text = text;
    change();
}

void KgpgTrustLabel::setColor(const QColor &color)
{
    m_color = color;
    change();
}

QString KgpgTrustLabel::text() const
{
    return m_text;
}

QColor KgpgTrustLabel::color() const
{
    return m_color;
}

void KgpgTrustLabel::change()
{
    m_text_w->setText(m_text);

    QPalette palette = m_color_w->palette();
    palette.setColor(m_color_w->backgroundRole(), m_color);
    m_color_w->setPalette(palette);
}

KgpgKeyInfo::KgpgKeyInfo(KGpgKeyNode *node, KGpgItemModel *model, QWidget *parent)
	: QDialog(parent),
	keychange(new KGpgChangeKey(node, this)),
	m_node(node),
	m_model(model),
	m_trust(new KgpgTrustLabel(this)),
	m_keywaschanged(false),
	m_closewhendone(false)
{
	Q_ASSERT(m_model != nullptr);
	Q_ASSERT(m_node != nullptr);

    setupUi(this);
    m_owtrust->addItem(Convert::toString(GPGME_VALIDITY_UNDEFINED));
    m_owtrust->addItem(Convert::toString(GPGME_VALIDITY_NEVER));
    m_owtrust->addItem(Convert::toString(GPGME_VALIDITY_MARGINAL));
    m_owtrust->addItem(Convert::toString(GPGME_VALIDITY_FULL));
    m_owtrust->addItem(Convert::toString(GPGME_VALIDITY_ULTIMATE));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QWidget *mainWidget = new QWidget(this);
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Apply);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KgpgKeyInfo::okButtonClicked);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &KgpgKeyInfo::applyButtonClicked);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &KgpgKeyInfo::cancelButtonClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KgpgKeyInfo::reject);
    okButton->setDefault(true);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    m_email->setUnderline(false);
    int trustRow;
    formLayout_keyproperties->getWidgetPosition(tl_trust, &trustRow, nullptr);
    formLayout_keyproperties->setWidget(trustRow, QFormLayout::FieldRole, m_trust);

    // Hide some widgets if this is not a secret node.
    if ( ! m_node->isSecret() ) {
        m_expirationbtn->hide();
        m_password->hide();
    }

    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    connect(m_owtrust, QOverload<int>::of(&QComboBox::activated), this, &KgpgKeyInfo::slotChangeTrust);
    connect(m_photoid, &QComboBox::textActivated, this, &KgpgKeyInfo::slotLoadPhoto);
    connect(m_email, &KUrlLabel::leftClickedUrl, this, &KgpgKeyInfo::slotOpenUrl);
    connect(keychange, &KGpgChangeKey::done, this, &KgpgKeyInfo::slotApplied);
    connect(m_disable, &QCheckBox::toggled, this, &KgpgKeyInfo::slotDisableKey);
    connect(m_expirationbtn, &QPushButton::clicked, this, &KgpgKeyInfo::slotChangeDate);
    connect(m_password, &QPushButton::clicked, this, &KgpgKeyInfo::slotChangePass);

    displayKey();
    adjustSize();
    gr_fingerprint->setMinimumHeight(gr_fingerprint->height());
}

KgpgKeyInfo::~KgpgKeyInfo()
{
	if (keychange) {
		// make sure anything that happens as result of the selfdestruct does not call
		// out into a method of this object again, as it is under destruction and that
		// can cause crashes (see bug 373910). Since ~QObject has not been run yet the
		// connections are not yet removed automatically.
		disconnect(keychange, &KGpgChangeKey::done, this, &KgpgKeyInfo::slotApplied);
		keychange->selfdestruct(false);
	}
}

void KgpgKeyInfo::reloadNode()
{
	const QString kid(m_node->getId());

	// this will delete m_node
	m_model->refreshKey(m_node);

	m_node = m_model->getRootNode()->findKey(kid);
	if (m_node != nullptr) {
		displayKey();
	} else {
		KMessageBox::error(this, i18n("<qt>The requested key is not present in the keyring anymore.<br />Perhaps it was deleted by another application</qt>"), i18n("Key not found"));
		m_keywaschanged = false;
		close();
	}
}

void KgpgKeyInfo::displayKey()
{
    const QString name = m_node->getName();
    setWindowTitle(name);
    m_name->setText(QLatin1String( "<qt><b>" ) + name + QLatin1String( "</b></qt>" ));

    const QString email = m_node->getEmail();
    if (email.isEmpty()) {
        m_email->setText(i18nc("no email address", "none"));
        m_email->setUrl(QString());
        m_email->setEnabled(false);
    } else {
        m_email->setText(QLatin1String( "<qt><b>&lt;" ) + email + QLatin1String( "&gt;</b></qt>" ));
        m_email->setUrl(QLatin1String( "mailto:" ) + name + QLatin1Char( '<' ) + email + QLatin1Char( '>' ));
    }

    const KgpgKey *key = m_node->getKey();
    m_caps->setText(Convert::toString(key->keytype()));

    QString trust;
    QColor trustcolor;

    if (key->valid()) {
        QModelIndex idx = m_model->nodeIndex(m_node, KEYCOLUMN_TRUST);
        trust = m_model->data(idx, Qt::AccessibleTextRole).toString();
        trustcolor = m_model->data(idx, Qt::BackgroundRole).value<QColor>();
    } else {
        trust = Convert::toString(TRUST_DISABLED);
        trustcolor = KGpgSettings::colorBad();
    }

    m_id->setText(m_node->getId().right(16));
    m_algorithm->setText(Convert::toString(key->algorithm()) + QLatin1String( " / " ) + Convert::toString(key->encryptionAlgorithm()));
    m_algorithm->setWhatsThis(i18n("<qt>The left part is the algorithm used by the <b>signature</b> key. The right part is the algorithm used by the <b>encryption</b> key.</qt>"));
    m_creation->setText(QLocale().toString(m_node->getCreation().date(), QLocale::ShortFormat));
    if (m_node->getExpiration().isNull())
        m_expiration->setText(i18nc("Unlimited key lifetime", "Unlimited"));
    else
        m_expiration->setText(QLocale().toString(m_node->getExpiration().date(), QLocale::ShortFormat));
    m_trust->setText(trust);
    m_trust->setColor(trustcolor);
    m_length->setText(m_node->getSize());
    m_length->setWhatsThis(i18n("<qt>The left part is the size of the <b>signature</b> key. The right part is the size of the <b>encryption</b> key.</qt>"));
    m_fingerprint->setText(m_node->getBeautifiedFingerprint());

    const QString comment = m_node->getComment();
    if (comment.isEmpty()) {
        m_comment->setText(i18nc("no key comment", "<em>none</em>"));
        m_comment->setTextFormat(Qt::RichText);
    } else {
        m_comment->setText(comment);
        m_comment->setTextFormat(Qt::PlainText);
    }

	switch (key->ownerTrust()) {
	case GPGME_VALIDITY_NEVER:
		m_owtrust->setCurrentIndex(1);
		break;
	case GPGME_VALIDITY_MARGINAL:
		m_owtrust->setCurrentIndex(2);
		break;
	case GPGME_VALIDITY_FULL:
		m_owtrust->setCurrentIndex(3);
		break;
	case GPGME_VALIDITY_ULTIMATE:
		m_owtrust->setCurrentIndex(4);
		break;
	case GPGME_VALIDITY_UNDEFINED:
	default:
		m_owtrust->setCurrentIndex(0);
		break;
	}

    if (!key->valid())
        m_disable->setChecked(true);

    connect(m_node, &KGpgKeyNode::expanded, this, &KgpgKeyInfo::slotKeyExpanded);
    m_node->expand();
    m_photoid->clear();
}

void KgpgKeyInfo::slotOpenUrl()
{
    QDesktopServices::openUrl(QUrl(m_email->url()));
}

void KgpgKeyInfo::slotLoadPhoto(const QString &uid)
{
	int i = uid.toInt();
	QPixmap pixmap = m_node->getUid(i)->toUatNode()->getPixmap();
	QImage img = pixmap.toImage();
	pixmap = QPixmap::fromImage(img.scaled(m_photo->width(), m_photo->height(), Qt::KeepAspectRatio));
	m_photo->setPixmap(pixmap);
}

void KgpgKeyInfo::slotChangeDate()
{
	QPointer<SelectExpiryDate> dialog = new SelectExpiryDate(this, m_node->getExpiration());
	if (dialog->exec() == QDialog::Accepted) {
		keychange->setExpiration(dialog->date());
		buttonBox->button(QDialogButtonBox::Apply)->setEnabled(keychange->wasChanged());
	}
	delete dialog;
}

void KgpgKeyInfo::slotDisableKey(const bool ison)
{
	keychange->setDisable(ison);
	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(keychange->wasChanged());
}

void KgpgKeyInfo::slotChangePass()
{
	KGpgChangePass *cp = new KGpgChangePass(this, m_node->getId());

	connect(cp, &KGpgChangePass::done, this, &KgpgKeyInfo::slotInfoPasswordChanged);

	cp->start();
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
}

void KgpgKeyInfo::slotInfoPasswordChanged(int result)
{
	sender()->deleteLater();

	QApplication::restoreOverrideCursor();

	switch (result) {
	case KGpgTransaction::TS_OK:
		// when pinentry is used it is indistinguishable if everything went fine
		// or the user cancelled the password entries, so simply show nothing
		break;
	case KGpgTransaction::TS_BAD_PASSPHRASE:
		KMessageBox::error(this, i18n("Bad old passphrase, the passphrase for the key was not changed"), i18n("Could not change passphrase"));
		break;
	case KGpgTransaction::TS_USER_ABORTED:
		break;
	default:
		KMessageBox::error(this, i18n("KGpg was unable to change the passphrase."));
	}
}

void KgpgKeyInfo::slotChangeTrust(const int newtrust)
{
	keychange->setOwTrust(static_cast<gpgme_validity_t>(newtrust + 1));
	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(keychange->wasChanged());
}

void KgpgKeyInfo::setControlEnable(const bool b)
{
    m_owtrust->setEnabled(b);
    m_disable->setEnabled(b);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(b && keychange->wasChanged());

    if (m_expirationbtn)
        m_expirationbtn->setEnabled(b);
    if (m_password)
        m_password->setEnabled(b);

    if (b)
        QApplication::restoreOverrideCursor();
    else
        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
}

void KgpgKeyInfo::okButtonClicked()
{
	m_closewhendone = true;
	applyButtonClicked();
}

void KgpgKeyInfo::applyButtonClicked()
{
	setControlEnable(false);
	keychange->apply();
}

void KgpgKeyInfo::cancelButtonClicked()
{
	if (m_keywaschanged && m_node)
		Q_EMIT keyNeedsRefresh(m_node);
	reject();
}

void KgpgKeyInfo::slotApplied(int result)
{
	if (result) {
		KMessageBox::error(this, i18n("Changing key properties failed."), i18n("Key properties"));
	} else {
		m_keywaschanged = true;
		Q_EMIT keyNeedsRefresh(m_node);
		reloadNode();
	}
	setControlEnable(true);

	if (m_closewhendone)
		accept();
}

void KgpgKeyInfo::slotKeyExpanded()
{
	// the counting starts at 1 and that is the primary uid which can't be a photo id
	unsigned int i = 2;
	const KGpgSignableNode *uat;

	while ((uat = m_node->getUid(i++)) != nullptr) {
		if (uat->getType() != KgpgCore::ITYPE_UAT)
			continue;

		m_photoid->addItem(uat->getId());
	}

	bool hasphoto = (m_photoid->count() > 0);

	m_photoid->setVisible(hasphoto);
	m_photoid->setEnabled(hasphoto);
	if (hasphoto)
		slotLoadPhoto(m_photoid->currentText());
}
