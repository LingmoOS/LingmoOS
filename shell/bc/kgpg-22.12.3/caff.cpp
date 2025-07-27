/*
    SPDX-FileCopyrightText: 2009, 2010, 2012, 2013, 2014, 2015, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "caff.h"
#include "caff_p.h"

#include "kgpg_general_debug.h"
#include "kgpginterface.h"
#include "kgpgsettings.h"
#include "core/KGpgKeyNode.h"
#include "transactions/kgpgdeluid.h"
#include "transactions/kgpgencrypt.h"
#include "transactions/kgpgexport.h"
#include "transactions/kgpgimport.h"
#include "transactions/kgpgsignuid.h"
#include "gpgproc.h"

#include <KLocalizedString>
#include <KMessageBox>


#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

KGpgCaffPrivate::KGpgCaffPrivate(KGpgCaff *parent, const KGpgSignableNode::List &ids, const QStringList &signers,
		const KGpgCaff::OperationFlags flags, const KGpgSignTransactionHelper::carefulCheck checklevel)
	: QObject(parent),
	q_ptr(parent),
	m_signers(signers),
	m_flags(flags),
	m_checklevel(checklevel),
	m_gpgVersion(GPGProc::gpgVersion(GPGProc::gpgVersionString(QString()))),
	m_allids(ids)
{
	const QString gpgCfg = KGpgSettings::gpgConfigPath();
	const QString secring = KgpgInterface::getGpgSetting(QLatin1String( "secret-keyring" ), gpgCfg);

	QFileInfo fn(gpgCfg);
	if (!secring.isEmpty()) {
		m_secringfile = secring;
	} else {
		fn.setFile(fn.dir(), QLatin1String("secring.gpg"));
		m_secringfile = QDir::toNativeSeparators(fn.absoluteFilePath());
	}

	fn.setFile(fn.dir(), QLatin1String("private-keys-v1.d"));
	m_secringdir = QDir::toNativeSeparators(fn.absoluteFilePath());
}

KGpgCaffPrivate::~KGpgCaffPrivate()
{
}

void
KGpgCaffPrivate::reexportKey(const KGpgSignableNode *key)
{
	Q_Q(KGpgCaff);

	Q_ASSERT(m_tempdir.isNull());

	// find out if the given id can be used for encryption
	const KGpgKeyNode *k;
	if (key->getType() & KgpgCore::ITYPE_PAIR)
		k = key->toKeyNode();
	else
		k = key->getParentKeyNode()->toKeyNode();

	// skip if not
	if (!k->canEncrypt()) {
		m_noEncIds << key;
		m_allids.removeFirst();
		checkNextLoop();
		return;
	}

	m_tempdir.reset(new QTemporaryDir());

	if (m_gpgVersion >= 0x20100) {
		/* see https://lists.gnupg.org/pipermail/gnupg-devel/2014-December/029296.html */
		QFile seclink(m_secringdir);

		if (!seclink.link(m_tempdir->path() + QLatin1String("private-keys-v1.d"))) {
			KMessageBox::error(qobject_cast<QWidget *>(q->parent()),
					i18n("This function is not available on this system. The symbolic link to the private GnuPG keys cannot be created."));
			return;
		}
	}

	// export all keys necessary for signing
	QStringList exportkeys(m_signers);
	exportkeys << key->getKeyNode()->getId();

	KGpgImport *imp = new KGpgImport(this);

	QStringList expOptions(QLatin1String( "--export-options" ));
	expOptions << QLatin1String( "export-clean,export-attribute" );
	KGpgExport *exp = new KGpgExport(this, exportkeys, expOptions);
	exp->setOutputTransaction(imp);

	imp->setGnuPGHome(m_tempdir->path());

	connect(imp, &KGpgImport::done, this, &KGpgCaffPrivate::slotReimportDone);
	imp->start();
}

void
KGpgCaffPrivate::slotReimportDone(int result)
{
	KGpgImport *imp = qobject_cast<KGpgImport *>(sender());

	if (result != KGpgTransaction::TS_OK) {
		abortOperation(result);
	} else {
		if (imp->getImportedIds(0x1).count() != 1 + m_signers.count()) {
			abortOperation(-1);
		} else {
			KGpgSignUid *signuid = new KGpgSignUid(this, m_signers.first(), m_allids.first(), false, m_checklevel);
			signuid->setGnuPGHome(m_tempdir->path());
			if (m_gpgVersion < 0x20100)
				signuid->setSecringFile(m_secringfile);
			connect(signuid, &KGpgSignUid::done, this, &KGpgCaffPrivate::slotSigningFinished);

			signuid->start();
		}
	}

	sender()->deleteLater();
}

void
KGpgCaffPrivate::abortOperation(int result)
{
	Q_Q(KGpgCaff);

	qCDebug(KGPG_LOG_GENERAL) << "transaction" << sender() << "failed, result" << result;
	m_tempdir.reset();

	Q_EMIT q->aborted();
}

void
KGpgCaffPrivate::checkNextLoop()
{
	Q_Q(KGpgCaff);

	m_tempdir.reset();

	if (m_allids.isEmpty()) {
		if (!m_noEncIds.isEmpty()) {
			QStringList ids;

			for (const KGpgSignableNode *nd : qAsConst(m_noEncIds))
				if (nd->getEmail().isEmpty())
					ids << i18nc("%1 is the key id, %2 is the name and comment of the key or uid",
							"%1: %2", nd->getId(), nd->getNameComment());
				else
					ids << i18nc("%1 is the key id, %2 is the name and comment of the key or uid, %3 is the email address of the uid",
							"%1: %2 &lt;%3&gt;", nd->getId(), nd->getNameComment(), nd->getEmail());

			KMessageBox::detailedError(qobject_cast<QWidget *>(q->parent()),
					i18np("No mail was sent for the following user id because it belongs to a key without encryption capability:",
							"No mail was sent for the following user ids because they belong to keys without encryption capability:",
							m_noEncIds.count()),
					ids.join(QLatin1String("\n")));
		}

		if (!m_alreadyIds.isEmpty()) {
			QStringList ids;

			for (const KGpgSignableNode *nd : qAsConst(m_alreadyIds))
				if (nd->getEmail().isEmpty())
					ids << i18nc("%1 is the key id, %2 is the name and comment of the key or uid",
							"%1: %2", nd->getId(), nd->getNameComment());
				else
					ids << i18nc("%1 is the key id, %2 is the name and comment of the key or uid, %3 is the email address of the uid",
							"%1: %2 &lt;%3&gt;", nd->getId(), nd->getNameComment(), nd->getEmail());

			KMessageBox::detailedError(qobject_cast<QWidget *>(q->parent()),
					i18np("No mail was sent for the following user id because it was already signed:",
							"No mail was sent for the following user ids because they were already signed:",
							m_alreadyIds.count()),
					ids.join(QLatin1String("\n")));
		}

		Q_EMIT q->done();
	} else {
		reexportKey(m_allids.first());
	}
}

void
KGpgCaffPrivate::slotSigningFinished(int result)
{
	sender()->deleteLater();

	if (result != KGpgTransaction::TS_OK) {
		if ((result == KGpgSignTransactionHelper::TS_ALREADY_SIGNED) && (m_flags & KGpgCaff::IgnoreAlreadySigned)) {
			m_alreadyIds << m_allids.takeFirst();
			checkNextLoop();
		} else {
			abortOperation(result);
		}
		return;
	}

	const KGpgSignableNode *uid = m_allids.first();

	// if there is no email address we can't send this out anyway, so don't bother.
	// could be improved: if this is the only selected uid from this key go and select
	// a proper mail address to send this to
	if (uid->getEmail().isEmpty()) {
		m_allids.removeFirst();
		checkNextLoop();
	}

	const KGpgKeyNode *key = uid->getKeyNode();

	int uidnum;

	if (uid == key) {
		uidnum = -1;
	} else {
		uidnum = -uid->getId().toInt();
	}

	KGpgDelUid::RemoveMode removeMode;
	switch (KGpgSettings::mailUats()) {
	case 0:
		removeMode = KGpgDelUid::RemoveWithEmail;
		break;
	case 1:
		if (uid == key) {
			removeMode = KGpgDelUid::RemoveWithEmail;
		} else {
			// check if this is the first uid with email address
			const KGpgSignableNode *otherUid;
			int index = 1;
			removeMode = KGpgDelUid::RemoveAllOther;

			while ( (otherUid = key->getUid(index++)) != nullptr) {
				if (otherUid == uid) {
					removeMode = KGpgDelUid::RemoveWithEmail;
					break;
				}
				if (!otherUid->getEmail().isEmpty())
					break;
			}
		}
		break;
	case 2:
		removeMode = KGpgDelUid::RemoveAllOther;
		break;
	default:
		Q_ASSERT(0);
		return;
	}

	KGpgDelUid *deluid = new KGpgDelUid(this, key, uidnum, removeMode);

	deluid->setGnuPGHome(m_tempdir->path());

	connect(deluid, &KGpgDelUid::done, this, &KGpgCaffPrivate::slotDelUidFinished);

	deluid->start();
}

void
KGpgCaffPrivate::slotDelUidFinished(int result)
{
	sender()->deleteLater();

	const KGpgSignableNode *uid = m_allids.first();
	const KGpgKeyNode *key = uid->getKeyNode();

	if (result != KGpgTransaction::TS_OK) {
		// it's no error if we tried to delete all other ids but there is no other id
		if ((uid != key) || (result != KGpgDelUid::TS_NO_SUCH_UID)) {
			abortOperation(result);
			return;
		}
	}

	QStringList expOptions(QLatin1String( "--export-options" ));
	expOptions << QLatin1String( "export-attribute" );

	KGpgExport *exp = new KGpgExport(this, QStringList(key->getId()), expOptions);

	exp->setGnuPGHome(m_tempdir->path());

	connect(exp, &KGpgExport::done, this, &KGpgCaffPrivate::slotExportFinished);

	exp->start();
}

void
KGpgCaffPrivate::slotExportFinished(int result)
{
	sender()->deleteLater();

	if (result != KGpgTransaction::TS_OK) {
		abortOperation(result);
		return;
	}

	const KGpgSignableNode *uid = m_allids.first();
	const KGpgKeyNode *key = uid->getKeyNode();

	KGpgExport *exp = qobject_cast<KGpgExport *>(sender());
	Q_ASSERT(exp != nullptr);

	QString body = KGpgSettings::emailTemplate();
	body.replace(QLatin1Char( '%' ) + i18nc("Email template placeholder for key id", "KEYID") + QLatin1Char( '%' ), key->getId());
	body.replace(QLatin1Char( '%' ) + i18nc("Email template placeholder for key id", "UIDNAME") + QLatin1Char( '%' ), uid->getNameComment());

	body += QLatin1Char( '\n' ) + QLatin1String( exp->getOutputData() );

	KGpgEncrypt *enc = new KGpgEncrypt(this, QStringList(key->getId()), body, KGpgEncrypt::AsciiArmored | KGpgEncrypt::AllowUntrustedEncryption);

	// Set the home directory to make sure custom encrypt options
	// as well as the "always encrypt to" setting are not honored.
	enc->setGnuPGHome(m_tempdir->path());

	connect(enc, &KGpgEncrypt::done, this, &KGpgCaffPrivate::slotTextEncrypted);

	enc->start();
}

void
KGpgCaffPrivate::slotTextEncrypted(int result)
{
	sender()->deleteLater();

	switch (result) {
	case KGpgTransaction::TS_OK: {
		KGpgEncrypt *enc = qobject_cast<KGpgEncrypt *>(sender());
		Q_ASSERT(enc != nullptr);

		const QString text = enc->encryptedText().join(QLatin1String("\n"));

		const KGpgSignableNode *uid = m_allids.takeFirst();

		const QString email = uid->getEmail();
		const QString keyid = uid->getKeyNode()->getId();

		QDesktopServices::openUrl(QUrl(QLatin1String("mailto:") + email +
				QLatin1String("?subject=") + i18nc("%1 is 64 bit key id (in hex), text is used as email subject", "Your key %1", keyid) +
				QLatin1String("&body=") + text));
		break;
		}
	default:
		abortOperation(result);
		break;
	case KGpgTransaction::TS_USER_ABORTED:
		m_allids.clear();
		break;
	}

	checkNextLoop();
}

KGpgCaff::KGpgCaff(QObject *parent, const KGpgSignableNode::List &ids, const QStringList &signids,
		const int checklevel, const OperationFlags flags)
	: QObject(parent),
	d_ptr(new KGpgCaffPrivate(this, ids, signids, flags, static_cast<KGpgSignTransactionHelper::carefulCheck>(checklevel)))
{
}

void
KGpgCaff::run()
{
	Q_D(KGpgCaff);

	d->reexportKey(d->m_allids.first());
}
