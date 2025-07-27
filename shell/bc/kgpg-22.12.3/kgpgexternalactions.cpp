/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2011, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikoans <andrius@stikonas.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgexternalactions.h"

#include "detailedconsole.h"
#include "foldercompressjob.h"
#include "keyservers.h"
#include "keysmanager.h"
#include "kgpgfirstassistant.h"
#include "kgpginterface.h"
#include "kgpgsettings.h"
#include "kgpgtextinterface.h"
#include "selectpublickeydialog.h"
#include "selectsecretkey.h"
#include "core/images.h"
#include "editor/kgpgeditor.h"
#include "editor/kgpgtextedit.h"
#include "transactions/kgpgdecrypt.h"
#include "transactions/kgpgencrypt.h"
#include "transactions/kgpgsigntext.h"
#include "transactions/kgpgtransactionjob.h"
#include "transactions/kgpgverify.h"

#include <KActionCollection>
#include <KHelpClient>
#include <KMessageBox>

#include <KJobTrackerInterface>
#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QProcess>
#include <QStringListModel>
#include <QTemporaryFile>
#include <kio/global.h>
#include <kio/renamedialog.h>

KGpgExternalActions::KGpgExternalActions(KeysManager *parent, KGpgItemModel *model)
	: QObject(parent),
	compressionScheme(0),
	m_model(model),
	m_kgpgfoldertmp(nullptr),
	m_keysmanager(parent)
{
	readOptions();
}

KGpgExternalActions::~KGpgExternalActions()
{
	delete m_kgpgfoldertmp;
}

void KGpgExternalActions::encryptFiles(KeysManager *parent, const QList<QUrl> &urls)
{
	Q_ASSERT(!urls.isEmpty());

	KGpgExternalActions *encActions = new KGpgExternalActions(parent, parent->getModel());

	KgpgSelectPublicKeyDlg *dialog = new KgpgSelectPublicKeyDlg(parent, parent->getModel(), encActions->goDefaultKey(), false, urls);
	connect(dialog, &KgpgSelectPublicKeyDlg::accepted, encActions, &KGpgExternalActions::slotEncryptionKeySelected);
	connect(dialog, &KgpgSelectPublicKeyDlg::rejected, dialog, &KgpgSelectPublicKeyDlg::deleteLater);
	connect(dialog, &KgpgSelectPublicKeyDlg::rejected, encActions, &KGpgExternalActions::deleteLater);
	dialog->show();
}

void KGpgExternalActions::slotEncryptionKeySelected()
{
	KgpgSelectPublicKeyDlg *dialog = qobject_cast<KgpgSelectPublicKeyDlg *>(sender());
	Q_ASSERT(dialog != nullptr);
	sender()->deleteLater();

	QStringList opts;
	QString defaultKey;

	if (KGpgSettings::encryptFilesTo()) {
		if (KGpgSettings::pgpCompatibility())
			opts << QLatin1String( "--pgp6" );

		defaultKey = KGpgSettings::fileEncryptionKey();
	}

	KGpgEncrypt::EncryptOptions eopt = KGpgEncrypt::DefaultEncryption;

	if (dialog->getUntrusted())
		eopt |= KGpgEncrypt::AllowUntrustedEncryption;
	if (dialog->getArmor())
		eopt |= KGpgEncrypt::AsciiArmored;
	if (dialog->getHideId())
		eopt |= KGpgEncrypt::HideKeyId;

	if (KGpgSettings::allowCustomEncryptionOptions()) {
		const QString customopts = dialog->getCustomOptions();

		if (!customopts.isEmpty())
			opts << customopts.split(QLatin1Char(' '), Qt::SkipEmptyParts);
	}

	QStringList keys = dialog->selectedKeys();

	if (!defaultKey.isEmpty() && !keys.contains(defaultKey))
		keys.append(defaultKey);

	if (dialog->getSymmetric())
		keys.clear();

	KGpgEncrypt *enc = new KGpgEncrypt(dialog->parent(), keys, dialog->getFiles(), eopt, opts);
	KGpgTransactionJob *encjob = new KGpgTransactionJob(enc);

	KIO::getJobTracker()->registerJob(encjob);
	encjob->start();

	deleteLater();
}

void KGpgExternalActions::encryptFolders(KeysManager *parent, const QList<QUrl> &urls)
{
	QTemporaryFile *tmpfolder = new QTemporaryFile();

	if (!tmpfolder->open()) {
		delete tmpfolder;
		KMessageBox::error(parent, i18n("Cannot create temporary file for folder compression."), i18n("Temporary File Creation"));
		return;
	}

	if (KMessageBox::Continue != KMessageBox::warningContinueCancel(parent,
				i18n("<qt>KGpg will now create a temporary archive file:<br /><b>%1</b> to process the encryption. "
				"The file will be deleted after the encryption is finished.</qt>",
				tmpfolder->fileName()), i18n("Temporary File Creation"), KStandardGuiItem::cont(),
				KStandardGuiItem::cancel(), QLatin1String( "FolderTmpFile" ))) {
		delete tmpfolder;
		return;
	}

	KGpgExternalActions *encActions = new KGpgExternalActions(parent, parent->getModel());
	KgpgSelectPublicKeyDlg *dialog = new KgpgSelectPublicKeyDlg(parent, parent->getModel(), encActions->goDefaultKey(), false, urls);
	encActions->m_kgpgfoldertmp = tmpfolder;

	QWidget *bGroup = new QWidget(dialog->optionsbox);
	QHBoxLayout *bGroupHBoxLayout = new QHBoxLayout(bGroup);
	bGroupHBoxLayout->setContentsMargins(0, 0, 0, 0);

	(void) new QLabel(i18n("Compression method for archive:"), bGroup);

	QComboBox *optionbx = new QComboBox(bGroup);
	bGroupHBoxLayout->addWidget(optionbx);
	optionbx->setModel(new QStringListModel(FolderCompressJob::archiveNames(), bGroup));

	connect(optionbx, QOverload<int>::of(&QComboBox::activated), encActions, &KGpgExternalActions::slotSetCompression);
	connect(dialog, &KgpgSelectPublicKeyDlg::accepted, encActions, &KGpgExternalActions::startFolderEncode);
	connect(dialog, &KgpgSelectPublicKeyDlg::rejected, encActions, &KGpgExternalActions::deleteLater);
	connect(dialog, &KgpgSelectPublicKeyDlg::rejected, dialog, &KgpgSelectPublicKeyDlg::deleteLater);
	
	dialog->show();
}

void KGpgExternalActions::slotSetCompression(int cp)
{
	compressionScheme = cp;
}

void KGpgExternalActions::startFolderEncode()
{
	KgpgSelectPublicKeyDlg *dialog = qobject_cast<KgpgSelectPublicKeyDlg *>(sender());
	Q_ASSERT(dialog != nullptr);
	dialog->deleteLater();

	const QList<QUrl> urls = dialog->getFiles();

	QStringList selec = dialog->selectedKeys();
	KGpgEncrypt::EncryptOptions encOptions = KGpgEncrypt::DefaultEncryption;
	const QStringList encryptOptions = dialog->getCustomOptions().split(QLatin1Char(' '),  Qt::SkipEmptyParts);
	if (dialog->getSymmetric()) {
		selec.clear();
	} else {
		Q_ASSERT(!selec.isEmpty());
	}

	QString extension = FolderCompressJob::extensionForArchive(compressionScheme);

	if (dialog->getArmor())
		extension += QLatin1String( ".asc" );
	else if (KGpgSettings::pgpExtension())
		extension += QLatin1String( ".pgp" );
	else
		extension += QLatin1String( ".gpg" );

	if (dialog->getArmor())
		encOptions |= KGpgEncrypt::AsciiArmored;
	if (dialog->getHideId())
		encOptions |= KGpgEncrypt::HideKeyId;
	if (dialog->getUntrusted())
		encOptions |= KGpgEncrypt::AllowUntrustedEncryption;

	QUrl encryptedFile(QUrl::fromLocalFile(urls.first().adjusted(QUrl::StripTrailingSlash).path() + extension));
	QFile encryptedFolder(encryptedFile.path());
	dialog->hide();
	if (encryptedFolder.exists()) {
		QPointer<KIO::RenameDialog> over = new KIO::RenameDialog(m_keysmanager, i18n("File Already Exists"),
				QUrl(), encryptedFile, KIO::RenameDialog_Overwrite);
		if (over->exec() == QDialog::Rejected) {
			dialog = nullptr;
			delete over;
			deleteLater();
			return;
		}
		encryptedFile = over->newDestUrl();
		delete over;
	}

	FolderCompressJob *trayinfo = new FolderCompressJob(m_keysmanager, urls, encryptedFile, m_kgpgfoldertmp,
			selec, encryptOptions, encOptions, compressionScheme);
	connect(trayinfo, &FolderCompressJob::result, this, &KGpgExternalActions::slotFolderFinished);
	KIO::getJobTracker()->registerJob(trayinfo);
	trayinfo->start();
}

void KGpgExternalActions::slotFolderFinished(KJob *job)
{
	FolderCompressJob *trayinfo = qobject_cast<FolderCompressJob *>(job);
	Q_ASSERT(trayinfo != nullptr);

	if (trayinfo->error())
		KMessageBox::error(m_keysmanager, trayinfo->errorString());

	deleteLater();
}

void KGpgExternalActions::verifyFile(QUrl url)
{
	// check file signature
	if (url.isEmpty())
		return;

	QString sigfile;
	// try to find detached signature.
	if (!url.fileName().endsWith(QLatin1String(".sig"))) {
		sigfile = url.path() + QLatin1String( ".sig" );
		if (!QFile::exists(sigfile)) {
			sigfile = url.path() + QLatin1String( ".asc" );
			// if no .asc or .sig signature file included, assume the file is internally signed
			if (!QFile::exists(sigfile))
				sigfile.clear();
		}
	} else {
		sigfile = url.path();
		sigfile.chop(4);
	}

	KGpgVerify *kgpv = new KGpgVerify(parent(), QList<QUrl>({QUrl(sigfile)}));
	connect(kgpv, &KGpgVerify::done, this, &KGpgExternalActions::slotVerificationDone);
	kgpv->start();
}

void KGpgExternalActions::slotVerificationDone(int result)
{
	KGpgVerify *kgpv = qobject_cast<KGpgVerify *>(sender());
	Q_ASSERT(kgpv != nullptr);
	kgpv->deleteLater();

	if (result == KGpgVerify::TS_MISSING_KEY) {
		KeyServer *kser = new KeyServer(m_keysmanager, m_model);
		kser->slotSetText(kgpv->missingId());
		kser->slotImport();
	} else {
		const QStringList messages = kgpv->getMessages();

		if (messages.isEmpty())
			return;

		QStringList msglist;
		for (QString rawmsg : messages)
			msglist << rawmsg.replace(QLatin1Char('<'), QLatin1String("&lt;"));

		(void) new KgpgDetailedInfo(m_keysmanager, KGpgVerify::getReport(messages, m_model),
				msglist.join(QLatin1String("<br/>")),
				QStringList(), i18nc("Caption of message box", "Verification Finished"));
	}
}

void KGpgExternalActions::signFiles(KeysManager* parent, const QList<QUrl>& urls)
{
	Q_ASSERT(!urls.isEmpty());

	KGpgExternalActions *signActions = new KGpgExternalActions(parent, parent->getModel());

	signActions->droppedUrls = urls;

	KgpgSelectSecretKey *keydlg = new KgpgSelectSecretKey(parent, parent->getModel(), false);
	connect(keydlg, &KgpgSelectSecretKey::accepted, signActions, &KGpgExternalActions::slotSignFiles);
	connect(keydlg, &KgpgSelectSecretKey::rejected, keydlg, &KgpgSelectSecretKey::deleteLater);
	connect(keydlg, &KgpgSelectSecretKey::rejected, signActions, &KGpgExternalActions::deleteLater);
	keydlg->show();
}

void KGpgExternalActions::slotSignFiles()
{
	KgpgSelectSecretKey *keydlg = qobject_cast<KgpgSelectSecretKey *>(sender());
	Q_ASSERT(keydlg != nullptr);
	sender()->deleteLater();

	const QString signKeyID = keydlg->getKeyID();

	QStringList Options;
	KGpgSignText::SignOptions sopts = KGpgSignText::DetachedSignature;
	if (KGpgSettings::asciiArmor()) {
		Options << QLatin1String( "--armor" );
		sopts |= KGpgSignText::AsciiArmored;
	}
	if (KGpgSettings::pgpCompatibility())
		Options << QLatin1String( "--pgp6" );

	if (droppedUrls.count() > 1) {
		KGpgTextInterface *signFileProcess = new KGpgTextInterface(parent(), signKeyID, Options);
		connect(signFileProcess, &KGpgTextInterface::fileSignFinished, signFileProcess, &KGpgTextInterface::deleteLater);
		signFileProcess->signFiles(droppedUrls);
	} else {
		KGpgSignText *signt = new KGpgSignText(parent(), signKeyID, droppedUrls, sopts);
		connect(signt, &KGpgSignText::done, signt, &KGpgSignText::deleteLater);
		signt->start();
	}

	deleteLater();
}

void KGpgExternalActions::decryptFiles(KeysManager* parent, const QList<QUrl> &urls)
{
	KGpgExternalActions *decActions = new KGpgExternalActions(parent, parent->getModel());

	decActions->decryptFile(urls);
}

void KGpgExternalActions::decryptFile(QList<QUrl> urls)
{
	if (urls.isEmpty()) {
		deleteLater();
		return;
	}

	while (!urls.first().isLocalFile()) {
		showDroppedFile(urls.takeFirst());
	}

	QUrl first = urls.first();

	QString oldname(first.fileName());
	if (oldname.endsWith(QLatin1String(".gpg"), Qt::CaseInsensitive) ||
			oldname.endsWith(QLatin1String(".asc"), Qt::CaseInsensitive) ||
			oldname.endsWith(QLatin1String(".pgp"), Qt::CaseInsensitive))
		oldname.chop(4);
	else
		oldname.append(QLatin1String( ".clear" ));

	QUrl swapname = QUrl::fromLocalFile(first.adjusted(QUrl::RemoveFilename).path() + oldname);
	QFile fgpg(swapname.path());
	if (fgpg.exists()) {
		QPointer<KIO::RenameDialog> over = new KIO::RenameDialog(m_keysmanager,
				i18n("File Already Exists"), QUrl(), swapname, KIO::RenameDialog_Overwrite);
		if (over->exec() != QDialog::Accepted) {
			delete over;
			urls.pop_front();
			decryptFile(urls);
			return;
		}

		swapname = over->newDestUrl();
		delete over;
	}

	droppedUrls = urls;
	KGpgDecrypt *decr = new KGpgDecrypt(this, droppedUrls.first(), swapname);
	connect(decr, &KGpgDecrypt::done, this, &KGpgExternalActions::slotDecryptionDone);
	decr->start();
}

void KGpgExternalActions::slotDecryptionDone(int status)
{
	KGpgDecrypt *decr = qobject_cast<KGpgDecrypt *>(sender());
	Q_ASSERT(decr != nullptr);

	if (status != KGpgTransaction::TS_OK)
		m_decryptionFailed << droppedUrls.first();

	decr->deleteLater();

	droppedUrls.pop_front();

	if (!droppedUrls.isEmpty()) {
		decryptFile(droppedUrls);
	} else {
		if (!m_decryptionFailed.isEmpty()) {
			QStringList failedFiles;
			for (const QUrl &url : qAsConst(m_decryptionFailed))
				failedFiles.append(url.toDisplayString());
			KMessageBox::errorList(nullptr,
					i18np("Decryption of this file failed:", "Decryption of these files failed:",
					m_decryptionFailed.count()), failedFiles,
					i18n("Decryption failed."));
		}
		deleteLater();
	}
}

void KGpgExternalActions::showDroppedFile(const QUrl &file)
{
    KgpgEditor *kgpgtxtedit = new KgpgEditor(m_keysmanager, m_model, {});
	connect(m_keysmanager, &KeysManager::fontChanged, kgpgtxtedit, &KgpgEditor::slotSetFont);

	kgpgtxtedit->m_editor->openDroppedFile(file, false);

	kgpgtxtedit->show();
}

void KGpgExternalActions::readOptions()
{
	if (KGpgSettings::firstRun()) {
		firstRun();
	} else if (KGpgSettings::gpgConfigPath().isEmpty()) {
                if (KMessageBox::Yes == KMessageBox::questionYesNo(nullptr,
				i18n("<qt>You have not set a path to your GnuPG config file.<br />This may cause some surprising results in KGpg's execution."
				"<br />Would you like to start KGpg's assistant to fix this problem?</qt>"),
				QString(), KGuiItem(i18n("Start Assistant")), KGuiItem(i18n("Do Not Start"))))
			startAssistant();
	}
}

void KGpgExternalActions::firstRun()
{
	QProcess *createConfigProc = new QProcess(this);
	QStringList args;
	args << QLatin1String( "--no-tty" ) << QLatin1String( "--list-secret-keys" );
	createConfigProc->start(QLatin1String( "gpg" ), args);	// start GnuPG so that it will create a config file
	createConfigProc->waitForFinished();
	startAssistant();
}

void KGpgExternalActions::startAssistant()
{
	if (m_assistant.isNull()) {
		m_assistant = new KGpgFirstAssistant(m_keysmanager);

		connect(m_assistant.data(), &KGpgFirstAssistant::accepted, this, &KGpgExternalActions::slotSaveOptionsPath);
		connect(m_assistant.data(), &KGpgFirstAssistant::rejected, m_assistant.data(), &KGpgFirstAssistant::deleteLater);
		connect(m_assistant->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &KGpgExternalActions::help);
	}

	m_assistant->show();
}

void KGpgExternalActions::slotSaveOptionsPath()
{
	KGpgSettings::setAutoStart(m_assistant->getAutoStart());
	KGpgSettings::setGpgConfigPath(m_assistant->getConfigPath());
	KGpgSettings::setFirstRun(false);

	const QString gpgConfServer(KgpgInterface::getGpgSetting(QLatin1String( "keyserver" ), KGpgSettings::gpgConfigPath()));
	if (!gpgConfServer.isEmpty()) {
		// The user already had configured a keyserver, set this one as default.
		QStringList serverList(KGpgSettings::keyServers());
		serverList.prepend(gpgConfServer);
		KGpgSettings::setKeyServers(serverList);
	}

	const QString defaultID(m_assistant->getDefaultKey());

	KGpgSettings::self()->save();
	Q_EMIT updateDefault(defaultID);
	if (m_assistant->runKeyGenerate())
		Q_EMIT createNewKey();
	m_assistant->deleteLater();
}

void KGpgExternalActions::help()
{
	KHelpClient::invokeHelp(QString(), QLatin1String( "kgpg" ));
}

QKeySequence KGpgExternalActions::goDefaultKey() const
{
	return QKeySequence(qobject_cast<QAction *>(m_keysmanager->actionCollection()->action(QLatin1String( "go_default_key" )))->shortcut());
}
