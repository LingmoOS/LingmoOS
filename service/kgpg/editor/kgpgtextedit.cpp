/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgtextedit.h"

#include "selectsecretkey.h"
#include "kgpgsettings.h"
#include "keyservers.h"
#include "selectpublickeydialog.h"
#include "detailedconsole.h"
#include "keysmanager.h"
#include "editor/kgpgeditor.h"
#include "transactions/kgpgdecrypt.h"
#include "transactions/kgpgencrypt.h"
#include "transactions/kgpgimport.h"
#include "transactions/kgpgsigntext.h"
#include "transactions/kgpgverify.h"

#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlMimeData>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QMimeData>
#include <QTemporaryFile>
#include <QTextStream>

#define SIGNEDMESSAGE_BEGIN  QLatin1String( "-----BEGIN PGP SIGNED MESSAGE-----" )
#define SIGNEDMESSAGE_END    QLatin1String( "-----END PGP SIGNATURE-----" )

KgpgTextEdit::KgpgTextEdit(QWidget *parent, KGpgItemModel *model, KeysManager *manager)
            : KTextEdit(parent),
            m_posstart(-1),
            m_posend(-1),
            m_model(model),
            m_keysmanager(manager)
{
    setCheckSpellingEnabled(true);
    setAcceptDrops(true);
    setReadOnly(false);
    setUndoRedoEnabled(true);
    setAcceptRichText(false);
}

KgpgTextEdit::~KgpgTextEdit()
{
}

void KgpgTextEdit::dragEnterEvent(QDragEnterEvent *e)
{
    // if a file is dragged into editor ...
    if (e->mimeData()->hasUrls() || e->mimeData()->hasText())
        e->acceptProposedAction();
}

void KgpgTextEdit::dropEvent(QDropEvent *e)
{
    // decode dropped file or dropped text
    QList<QUrl> uriList = KUrlMimeData::urlsFromMimeData(e->mimeData());
    if (!uriList.isEmpty())
        slotDroppedFile(uriList.first());
    else
    if (e->mimeData()->hasText())
        insertPlainText(e->mimeData()->text());
}

void KgpgTextEdit::slotDroppedFile(const QUrl &url)
{
	openDroppedFile(url, true);
}

void KgpgTextEdit::openDroppedFile(const QUrl &url, const bool probe)
{
	QUrl tmpurl;

	QString checkFile;
	if (url.isLocalFile()) {
		checkFile = url.path();
		tmpurl = url;
	} else {
		if (KMessageBox::warningContinueCancel(this, i18n("<qt><b>Remote file dropped</b>.<br />The remote file will now be copied to a temporary file to process requested operation. This temporary file will be deleted after operation.</qt>"), QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), QLatin1String( "RemoteFileWarning" )) != KMessageBox::Continue)
			return;

		QTemporaryFile tmpFile;
		tmpFile.open();
		auto copyJob = KIO::file_copy(url, QUrl::fromLocalFile(tmpFile.fileName()));
		KJobWidgets::setWindow(copyJob , this);
		copyJob->exec();
		if (copyJob->error()) {
			KMessageBox::error(this, i18n("Could not download file."));
			return;
		}
		tmpFile.setAutoRemove(false);
		checkFile = m_tempfile = tmpFile.fileName();
		tmpurl = QUrl::fromLocalFile(m_tempfile);
	}

	QString result;

	QFile qfile(checkFile);
	if (qfile.open(QIODevice::ReadOnly)) {
		QTextStream t(&qfile);
		result = t.readAll();
		qfile.close();
	}

	if (result.isEmpty())
		return;

	if (!probe || KGpgDecrypt::isEncryptedText(result, &m_posstart, &m_posend)) {
		// if pgp data found, decode it
		KGpgDecrypt *decr = new KGpgDecrypt(this, QList<QUrl>({tmpurl}));
		connect(decr, &KGpgDecrypt::done, this, &KgpgTextEdit::slotDecryptDone);
		decr->start();
		return;
	}
	// remove only here, as KGpgDecrypt will use and remove the file itself
	if(!m_tempfile.isEmpty())
		QFile::remove( m_tempfile );
	m_tempfile.clear();

	QString tmpinfo;

	switch (KGpgImport::isKey(result)) {
	case 1:
		tmpinfo = i18n("<qt>This file is a <b>public</b> key.<br />Do you want to import it instead of opening it in editor?</qt>");
		break;
	case 2:
		tmpinfo = i18n("<qt>This file is a <b>private</b> key.<br />Do you want to import it instead of opening it in editor?</qt>");
		break;
        }

	if (!tmpinfo.isEmpty()) {
		if (KMessageBox::questionYesNo(this, tmpinfo, i18n("Key file dropped on Editor")) != KMessageBox::Yes) {
			setPlainText(result);
		} else {
			KGpgImport *imp = new KGpgImport(this, result);
			connect(imp, &KGpgImport::done, m_keysmanager, &KeysManager::slotImportDone);
			imp->start();
		}
	} else {
		if (m_posstart != -1) {
			Q_ASSERT(m_posend != -1);
			QString fullcontent = toPlainText();
			fullcontent.replace(m_posstart, m_posend - m_posstart, result);
			setPlainText(fullcontent);
			m_posstart = -1;
			m_posend = -1;
		} else {
			setPlainText(result);
		}
	}
}

void KgpgTextEdit::slotEncode()
{
	QPointer<KgpgSelectPublicKeyDlg> dialog = new KgpgSelectPublicKeyDlg(this, m_model, m_keysmanager->goDefaultShortcut(), true);
	if (dialog->exec() == QDialog::Accepted) {
		QStringList options;
		KGpgEncrypt::EncryptOptions opts = KGpgEncrypt::DefaultEncryption;

		if (dialog->getArmor())
			opts |= KGpgEncrypt::AsciiArmored;

		if (dialog->getUntrusted())
			opts |= KGpgEncrypt::AllowUntrustedEncryption;

		if (dialog->getHideId())
			opts |= KGpgEncrypt::HideKeyId;

		if (KGpgSettings::allowCustomEncryptionOptions()) {
			const QString customoptions = dialog->getCustomOptions();
			if (!customoptions.isEmpty())
				options << customoptions.split(QLatin1Char(' '), Qt::SkipEmptyParts);
		}

		if (KGpgSettings::pgpCompatibility())
			options << QLatin1String( "--pgp6" );

		QStringList listkeys;
		if (!dialog->getSymmetric())
			listkeys = dialog->selectedKeys();

		KGpgEncrypt *encr = new KGpgEncrypt(this, listkeys, toPlainText(), opts, options);
		encr->start();
		connect(encr, &KGpgEncrypt::done, this, &KgpgTextEdit::slotEncodeUpdate);
	}
	delete dialog;
}

void KgpgTextEdit::slotDecode()
{
	const QString fullcontent = toPlainText();

	if (!KGpgDecrypt::isEncryptedText(fullcontent, &m_posstart, &m_posend))
		return;

	KGpgDecrypt *decr = new KGpgDecrypt(this, fullcontent.mid(m_posstart, m_posend - m_posstart));
	connect(decr, &KGpgDecrypt::done, this, &KgpgTextEdit::slotDecryptDone);
	decr->start();
}

void KgpgTextEdit::slotSign(const QString &message)
{
    QString signkeyid;

    QPointer<KgpgSelectSecretKey> opts = new KgpgSelectSecretKey(this, m_model);
    if (opts->exec() == QDialog::Accepted)
        signkeyid = opts->getKeyID();
    else
    {
        delete opts;
        return;
    }

    delete opts;

	KGpgSignText *signt = new KGpgSignText(this, signkeyid, message);
	connect(signt, &KGpgSignText::done, this, &KgpgTextEdit::slotSignUpdate);
	signt->start();
}

void KgpgTextEdit::slotVerify(const QString &message)
{
    const QString startmsg(SIGNEDMESSAGE_BEGIN);
    const QString endmsg(SIGNEDMESSAGE_END);

    int posstart = message.indexOf(startmsg);
    if (posstart == -1)
        return;

    int posend = message.indexOf(endmsg, posstart);
    if (posend == -1)
        return;
    posend += endmsg.length();

    KGpgVerify *verify = new KGpgVerify(this, message.mid(posstart, posend - posstart));
    connect(verify, &KGpgVerify::done, this, &KgpgTextEdit::slotVerifyDone);
    verify->start();
}

void KgpgTextEdit::slotDecryptDone(int result)
{
	KGpgDecrypt *decr = qobject_cast<KGpgDecrypt *>(sender());
	Q_ASSERT(decr != nullptr);

	if (!m_tempfile.isEmpty()) {
		QFile::remove(m_tempfile);
		m_tempfile.clear();
	}

	const QChar lf = QLatin1Char('\n');
	if (result == KGpgTransaction::TS_OK) {
		// FIXME choose codec
		setPlainText(decr->decryptedText().join(lf) + lf);
	} else if (result != KGpgTransaction::TS_USER_ABORTED) {
		KMessageBox::detailedError(this, i18n("Decryption failed."), decr->getMessages().join(lf));
	}

	decr->deleteLater();
}

void KgpgTextEdit::slotEncodeUpdate(int result)
{
	KGpgEncrypt *enc = qobject_cast<KGpgEncrypt *>(sender());
	Q_ASSERT(enc != nullptr);

	if (result == KGpgTransaction::TS_OK) {
		const QChar lf = QLatin1Char('\n');
		setPlainText(enc->encryptedText().join(lf) + lf);
	} else {
		KMessageBox::error(this, i18n("The encryption failed with error code %1", result),
				i18n("Encryption failed."));
	}

	sender()->deleteLater();
}

void KgpgTextEdit::slotSignUpdate(int result)
{
	const KGpgSignText * const signt = qobject_cast<KGpgSignText *>(sender());
	sender()->deleteLater();
	Q_ASSERT(signt != nullptr);

	if (result != KGpgTransaction::TS_OK) {
		KMessageBox::error(this, i18n("Signing not possible: bad passphrase or missing key"));
		return;
	}

	const QChar lf = QLatin1Char('\n');
	const QString content = signt->signedText().join(lf) + lf;

    setPlainText(content);
    Q_EMIT resetEncoding(false);
}

void KgpgTextEdit::slotVerifyDone(int result)
{
	const KGpgVerify * const verify = qobject_cast<KGpgVerify *>(sender());
	sender()->deleteLater();
	Q_ASSERT(verify != nullptr);

	Q_EMIT verifyFinished();

	if (result == KGpgVerify::TS_MISSING_KEY) {
		verifyKeyNeeded(verify->missingId());
		return;
	}

	const QStringList messages = verify->getMessages();

	if (messages.isEmpty())
		return;

	QStringList msglist;
	for (QString rawmsg : messages)
		msglist << rawmsg.replace(QLatin1Char('<'), QLatin1String("&lt;"));

	(void) new KgpgDetailedInfo(this, KGpgVerify::getReport(messages, m_model),
			msglist.join(QLatin1String("<br/>")),
			QStringList(), i18nc("Caption of message box", "Verification Finished"));
}

void KgpgTextEdit::verifyKeyNeeded(const QString &id)
{
    KGuiItem importitem = KStandardGuiItem::yes();
    importitem.setText(i18n("&Import"));
    importitem.setToolTip(i18n("Import key in your list"));

    KGuiItem noimportitem = KStandardGuiItem::no();
    noimportitem.setText(i18n("Do &Not Import"));
    noimportitem.setToolTip(i18n("Will not import this key in your list"));

    if (KMessageBox::questionYesNo(this, i18n("<qt><b>Missing signature:</b><br />Key id: %1<br /><br />Do you want to import this key from a keyserver?</qt>", id), i18n("Missing Key"), importitem, noimportitem) == KMessageBox::Yes)
    {
        KeyServer *kser = new KeyServer(nullptr, m_model, true);
        kser->slotSetText(id);
        kser->slotImport();
    }
}

void KgpgTextEdit::slotSignVerify()
{
	signVerifyText(toPlainText());
}

void KgpgTextEdit::signVerifyText(const QString &message)
{
	if (message.contains(SIGNEDMESSAGE_BEGIN))
		slotVerify(message);
	else
		slotSign(message);
}

void KgpgTextEdit::slotHighlightText(const QString &, const int matchingindex, const int matchedlength)
{
	highlightWord(matchedlength, matchingindex);
}
