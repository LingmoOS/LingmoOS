/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007-2022 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2011 Luis Ángel Fernández Fernández <laffdez@gmail.com>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keysmanager.h"

#include "caff.h"
#include "core/images.h"
#include "detailedconsole.h"
#include "gpgproc.h"
#include "groupedit.h"
#include "keyadaptor.h"
#include "keyexport.h"
#include "keyinfodialog.h"
#include "keyservers.h"
#include "keytreeview.h"
#include "kgpg_general_debug.h"
#include "kgpg.h"
#include "kgpgchangekey.h"
#include "kgpgkeygenerate.h"
#include "kgpgoptions.h"
#include "kgpgrevokewidget.h"
#include "kgpgsettings.h"
#include "newkey.h"
#include "selectpublickeydialog.h"
#include "selectsecretkey.h"
#include "sourceselect.h"
#include "editor/kgpgeditor.h"
#include "editor/kgpgtextedit.h"
#include "model/keylistproxymodel.h"
#include "transactions/kgpgaddphoto.h"
#include "transactions/kgpgadduid.h"
#include "transactions/kgpgdecrypt.h"
#include "transactions/kgpgdelkey.h"
#include "transactions/kgpgdelsign.h"
#include "transactions/kgpgdeluid.h"
#include "transactions/kgpgencrypt.h"
#include "transactions/kgpgexport.h"
#include "transactions/kgpggeneratekey.h"
#include "transactions/kgpggeneraterevoke.h"
#include "transactions/kgpgimport.h"
#include "transactions/kgpgkeyservergettransaction.h"
#include "transactions/kgpgprimaryuid.h"
#include "transactions/kgpgsignkey.h"
#include "transactions/kgpgsignuid.h"
#include "transactions/kgpgtransactionjob.h"

#include <algorithm>
#include <akonadi-contact_version.h>
#if AKONADICONTACT_VERSION > QT_VERSION_CHECK(5, 19, 40)
#include <Akonadi/ContactEditor>
#include <Akonadi/ContactEditorDialog>
#include <Akonadi/ContactSearchJob>
#else
#include <Akonadi/Contact/ContactEditor>
#include <Akonadi/Contact/ContactEditorDialog>
#include <Akonadi/Contact/ContactSearchJob>
#endif
#include <KActionCollection>
#include <KContacts/AddresseeList>
// #include <KContacts/Key> TODO
#include <kwidgetsaddons_version.h>
#include <KIO/Global>
#include <KIO/ApplicationLauncherJob>
#include <KIO/OpenUrlJob>
#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
#include <KIO/JobUiDelegateFactory>
#else
#include <KIO/JobUiDelegate>
#endif
#include <KJobTrackerInterface>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KSelectAction>
#include <KService>
#include <KSharedConfig>
#include <KStandardAction>
#include <KStandardGuiItem>
#include <KStandardShortcut>
#include <KStatusNotifierItem>
#include <KTipDialog>
#include <KToggleAction>

#include <QApplication>
#include <QDBusConnection>

#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMetaObject>
#include <QNetworkConfigurationManager>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QProcess>
#include <QRegularExpression>
#include <QStatusBar>
#include <QUrl>
#include <QWidget>
#include <QWidgetAction>

using namespace KgpgCore;

KeysManager::KeysManager(QWidget *parent)
           : KXmlGuiWindow(parent),
	   imodel(new KGpgItemModel(this)),
	   m_adduid(nullptr),
	   m_genkey(nullptr),
	   m_delkey(nullptr),
	   terminalkey(nullptr),
	   m_trayicon(nullptr)
{
	new KeyAdaptor(this);
	QDBusConnection::sessionBus().registerObject(QLatin1String( "/KeyInterface" ), this);

	setAttribute(Qt::WA_DeleteOnClose, false);
	setWindowTitle(i18n("Key Management"));

	KStandardAction::quit(this, &KeysManager::quitApp, actionCollection());
	actionCollection()->addAction(KStandardAction::Preferences, QLatin1String( "options_configure" ), this, SLOT(showOptions()));

	openEditor = actionCollection()->addAction(QLatin1String("kgpg_editor"), this, &KeysManager::slotOpenEditor);
	openEditor->setIcon(QIcon::fromTheme( QLatin1String( "accessories-text-editor" )));
	openEditor->setText(i18n("&Open Editor"));

	kserver = actionCollection()->addAction( QLatin1String("key_server"), this, &KeysManager::showKeyServer);
	kserver->setText( i18n("&Key Server Dialog") );
	kserver->setIcon( QIcon::fromTheme( QLatin1String( "network-server" )) );

	goToDefaultKey = actionCollection()->addAction(QLatin1String("go_default_key"), this, &KeysManager::slotGotoDefaultKey);
	goToDefaultKey->setIcon(QIcon::fromTheme( QLatin1String( "go-home" )));
	goToDefaultKey->setText(i18n("&Go to Default Key"));
	actionCollection()->setDefaultShortcut(goToDefaultKey, QKeySequence(Qt::CTRL | Qt::Key_Home));

	s_kgpgEditor = new KgpgEditor(this, imodel, Qt::Dialog);
	s_kgpgEditor->setAttribute(Qt::WA_DeleteOnClose, false);

	// this must come after kserver, preferences, and openEditor are created
	// because they are used to set up the tray icon context menu
	readOptions();

	if (showTipOfDay)
		installEventFilter(this);

	QAction *action;

	action = actionCollection()->addAction(QLatin1String("help_tipofday"), this, &KeysManager::slotTip);
	action->setIcon( QIcon::fromTheme( QLatin1String( "help-hint" )) );
	action->setText( i18n("Tip of the &Day") );

	action = actionCollection()->addAction(QLatin1String("gpg_man"), this, &KeysManager::slotManpage);
	action->setText( i18n("View GnuPG Manual") );
	action->setIcon( QIcon::fromTheme( QLatin1String( "help-contents" )) );

	action = actionCollection()->addAction(QLatin1String("key_refresh"), this, &KeysManager::refreshkey);
	action->setIcon(QIcon::fromTheme( QLatin1String( "view-refresh" )));
	action->setText(i18n("&Refresh List"));
	actionCollection()->setDefaultShortcuts(action, KStandardShortcut::reload());

	longId = actionCollection()->add<KToggleAction>(QLatin1String("show_long_keyid"), this, &KeysManager::slotShowLongId);
	longId->setText(i18n("Show &Long Key Id"));
	longId->setChecked(KGpgSettings::showLongKeyId());

	QAction *infoKey = actionCollection()->addAction(QLatin1String("key_info"), this, &KeysManager::keyproperties);
	infoKey->setIcon(QIcon::fromTheme( QLatin1String( "document-properties-key" )));
	infoKey->setText(i18n("K&ey Properties"));

	QAction *openKeyUrl = actionCollection()->addAction(QLatin1String("key_url"), this, &KeysManager::slotOpenKeyUrl);
	openKeyUrl->setIcon(QIcon::fromTheme(QLatin1String("internet-services")));
	openKeyUrl->setText(i18n("&Open Key URL"));

	editKey = actionCollection()->addAction(QLatin1String("key_edit"), this, &KeysManager::slotedit);
	editKey->setIcon(QIcon::fromTheme( QLatin1String( "utilities-terminal" )));
	editKey->setText(i18n("Edit Key in &Terminal"));
	actionCollection()->setDefaultShortcut(editKey, QKeySequence(Qt::ALT | Qt::Key_Return));

	QAction *generateKey = actionCollection()->addAction(QLatin1String("key_gener"), this, &KeysManager::slotGenerateKey);
	generateKey->setIcon(QIcon::fromTheme( QLatin1String( "key-generate-pair" )));
	generateKey->setText(i18n("&Generate Key Pair..."));
	actionCollection()->setDefaultShortcuts(generateKey, KStandardShortcut::shortcut(KStandardShortcut::New));

	exportPublicKey = actionCollection()->addAction(QLatin1String("key_export"), this, &KeysManager::slotexport);
	exportPublicKey->setIcon(QIcon::fromTheme( QLatin1String( "document-export-key" )));
	actionCollection()->setDefaultShortcuts(exportPublicKey, KStandardShortcut::shortcut(KStandardShortcut::Copy));

	QAction *importKey = actionCollection()->addAction(QLatin1String("key_import"), this, &KeysManager::slotPreImportKey);
	importKey->setIcon(QIcon::fromTheme( QLatin1String( "document-import-key" )));
	importKey->setText(i18n("&Import Key..."));
	actionCollection()->setDefaultShortcuts(importKey, KStandardShortcut::shortcut(KStandardShortcut::Paste));

	m_sendEmail = actionCollection()->addAction(QLatin1String("send_mail"), this, &KeysManager::slotSendEmail);
	m_sendEmail->setIcon(QIcon::fromTheme(QLatin1String("mail-send")));
	m_sendEmail->setText(i18n("Send Ema&il"));

	QAction *newContact = actionCollection()->addAction(QLatin1String("add_kab"), this, &KeysManager::addToKAB);
	newContact->setIcon(QIcon::fromTheme( QLatin1String( "contact-new" )));
	newContact->setText(i18n("&Create New Contact in Address Book"));

	createGroup = actionCollection()->addAction(QLatin1String("create_group"), this, &KeysManager::createNewGroup);
	createGroup->setIcon(Images::group());

	editCurrentGroup = actionCollection()->addAction(QLatin1String("edit_group"), this, &KeysManager::editGroup);
	editCurrentGroup->setText(i18n("&Edit Group..."));

	delGroup = actionCollection()->addAction(QLatin1String("delete_group"), this, &KeysManager::deleteGroup);
	delGroup->setText(i18n("&Delete Group"));
	delGroup->setIcon(QIcon::fromTheme( QLatin1String( "edit-delete" )));

	m_groupRename = actionCollection()->addAction(QLatin1String("rename_group"), this, &KeysManager::renameGroup);
	m_groupRename->setText(i18n("&Rename Group"));
	m_groupRename->setIcon(QIcon::fromTheme( QLatin1String( "edit-rename" )));
	actionCollection()->setDefaultShortcut(m_groupRename, QKeySequence(Qt::Key_F2));

	deleteKey = actionCollection()->addAction(QLatin1String("key_delete"), this, &KeysManager::confirmdeletekey);
	deleteKey->setIcon(QIcon::fromTheme( QLatin1String( "edit-delete" )));
	actionCollection()->setDefaultShortcut(deleteKey, QKeySequence(Qt::Key_Delete));

	setDefaultKey = actionCollection()->addAction(QLatin1String("key_default"), this, &KeysManager::slotSetDefKey);
	setDefaultKey->setText(i18n("Set as De&fault Key"));

	QAction *addPhoto = actionCollection()->addAction(QLatin1String("add_photo"), this, &KeysManager::slotAddPhoto);
	addPhoto->setText(i18n("&Add Photo..."));

	QAction *addUid = actionCollection()->addAction(QLatin1String("add_uid"), this, &KeysManager::slotAddUid);
	addUid->setText(i18n("&Add User Id..."));

	QAction *exportSecretKey = actionCollection()->addAction(QLatin1String("key_sexport"), this, &KeysManager::slotexportsec);
	exportSecretKey->setText(i18n("Export Secret Key..."));

	QAction *deleteKeyPair = actionCollection()->addAction(QLatin1String("key_pdelete"), this, &KeysManager::deleteseckey);
	deleteKeyPair->setText(i18n("Delete Key Pair"));
	deleteKeyPair->setIcon(QIcon::fromTheme( QLatin1String( "edit-delete" )));

	m_revokeKey = actionCollection()->addAction(QLatin1String("key_revoke"), this, &KeysManager::revokeWidget);
	m_revokeKey->setText(i18n("Revoke Key..."));

	QAction *regeneratePublic = actionCollection()->addAction(QLatin1String("key_regener"), this, &KeysManager::slotregenerate);
	regeneratePublic->setText(i18n("&Regenerate Public Key"));

	delUid = actionCollection()->addAction(QLatin1String("del_uid"), this, &KeysManager::slotDelUid);
	delUid->setIcon(QIcon::fromTheme( QLatin1String( "edit-delete" )));

	setPrimUid = actionCollection()->addAction(QLatin1String("prim_uid"), this, &KeysManager::slotPrimUid);
	setPrimUid->setText(i18n("Set User Id as &Primary"));

	QAction *openPhoto = actionCollection()->addAction(QLatin1String("key_photo"), this, &KeysManager::slotShowPhoto);
	openPhoto->setIcon(QIcon::fromTheme( QLatin1String( "image-x-generic" )));
	openPhoto->setText(i18n("&Open Photo"));

	QAction *deletePhoto = actionCollection()->addAction(QLatin1String("delete_photo"), this, &KeysManager::slotDeletePhoto);
	deletePhoto->setIcon(QIcon::fromTheme( QLatin1String( "edit-delete" )));
	deletePhoto->setText(i18n("&Delete Photo"));

	delSignKey = actionCollection()->addAction(QLatin1String("key_delsign"), this, &KeysManager::delsignkey);
	delSignKey->setIcon(QIcon::fromTheme( QLatin1String( "edit-delete" )));
	delSignKey->setEnabled(false);

	importAllSignKeys = actionCollection()->addAction(QLatin1String("key_importallsign"), this, &KeysManager::importallsignkey);
	importAllSignKeys->setIcon(QIcon::fromTheme( QLatin1String( "document-import" )));
	importAllSignKeys->setText(i18n("Import &Missing Signatures From Keyserver"));

	refreshKey = actionCollection()->addAction(QLatin1String("key_server_refresh"), this, &KeysManager::refreshKeyFromServer);
	refreshKey->setIcon(QIcon::fromTheme( QLatin1String( "view-refresh" )));

	signKey = actionCollection()->addAction(QLatin1String("key_sign"), this, &KeysManager::signkey);
	signKey->setIcon(QIcon::fromTheme( QLatin1String( "document-sign-key" )));

	signUid = actionCollection()->addAction(QLatin1String("key_sign_uid"), this, &KeysManager::signuid);
	signUid->setIcon(QIcon::fromTheme( QLatin1String( "document-sign-key" )));

	signMailUid = actionCollection()->addAction(QLatin1String("key_sign_mail_uid"), this, &KeysManager::caff);
	signMailUid->setIcon(QIcon::fromTheme( QLatin1String( "document-sign-key" )));

	importSignatureKey = actionCollection()->addAction(QLatin1String("key_importsign"), this, &KeysManager::preimportsignkey);
	importSignatureKey->setIcon(QIcon::fromTheme( QLatin1String( "document-import-key" )));

	sTrust = actionCollection()->add<KToggleAction>(QLatin1String("show_trust"), this, &KeysManager::slotShowTrust);
	sTrust->setText(i18n("Trust"));

	sSize = actionCollection()->add<KToggleAction>(QLatin1String("show_size"), this, &KeysManager::slotShowSize);
	sSize->setText(i18n("Size"));

	sCreat = actionCollection()->add<KToggleAction>(QLatin1String("show_creat"), this, &KeysManager::slotShowCreation);
	sCreat->setText(i18n("Creation"));

	sExpi = actionCollection()->add<KToggleAction>(QLatin1String("show_expi"), this, &KeysManager::slotShowExpiration);
	sExpi->setText(i18n("Expiration"));

	photoProps = actionCollection()->add<KSelectAction>(QLatin1String( "photo_settings" ));
	photoProps->setIcon(QIcon::fromTheme( QLatin1String( "image-x-generic" )));
	photoProps->setText(i18n("&Photo ID's"));

	// Keep the list in kgpg.kcfg in sync with this one!
	QStringList list;
	list.append(i18n("Disable"));
	list.append(i18nc("small picture", "Small"));
	list.append(i18nc("medium picture", "Medium"));
	list.append(i18nc("large picture", "Large"));
	photoProps->setItems(list);

	trustProps = actionCollection()->add<KSelectAction>(QLatin1String( "trust_filter_settings" ));
	trustProps->setText(i18n("Minimum &Trust"));

	QStringList tlist;
	tlist.append(i18nc("no filter: show all keys", "&None"));
	tlist.append(i18nc("show only active keys", "&Active"));
	tlist.append(i18nc("show only keys with at least marginal trust", "&Marginal"));
	tlist.append(i18nc("show only keys with at least full trust", "&Full"));
	tlist.append(i18nc("show only ultimately trusted keys", "&Ultimate"));

	trustProps->setItems(tlist);

	iproxy = new KeyListProxyModel(this);
	iproxy->setKeyModel(imodel);
	connect(this, &KeysManager::readAgainOptions, iproxy, &KeyListProxyModel::settingsChanged);

	iview = new KeyTreeView(this, iproxy);
	connect(iview, &KeyTreeView::doubleClicked, this, QOverload<const QModelIndex &>::of(&KeysManager::defaultAction));
	connect(iview, &KeyTreeView::importDrop, this, QOverload<const QList<QUrl> &>::of(&KeysManager::slotImport));
	iview->setSelectionMode(QAbstractItemView::ExtendedSelection);
	setCentralWidget(iview);
	iview->resizeColumnsToContents();
	iview->setAlternatingRowColors(true);
	iview->setSortingEnabled(true);
	connect(iview, &KeyTreeView::customContextMenuRequested, this, &KeysManager::slotMenu);
	iview->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(iview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KeysManager::checkList);

	connect(iview, &KeyTreeView::returnPressed, this, &KeysManager::slotDefaultAction);

	hPublic = actionCollection()->add<KToggleAction>(QLatin1String("show_secret"), iproxy, &KeyListProxyModel::setOnlySecret);
	hPublic->setIcon(QIcon::fromTheme( QLatin1String( "view-key-secret" )));
	hPublic->setText(i18n("&Show Only Secret Keys"));
	hPublic->setChecked(KGpgSettings::showSecret());

	int psize = KGpgSettings::photoProperties();
	photoProps->setCurrentItem(psize);
	slotSetPhotoSize(psize);
	psize = KGpgSettings::trustLevel();
	trustProps->setCurrentItem(psize);
	slotSetTrustFilter(psize);
	slotShowLongId(KGpgSettings::showLongKeyId());

	m_popuppub = new QMenu(this);
	m_popuppub->addAction(exportPublicKey);
	m_popuppub->addAction(m_sendEmail);
	m_popuppub->addAction(signMailUid);
	m_popuppub->addAction(signKey);
	m_popuppub->addAction(signUid);
	m_popuppub->addAction(deleteKey);
	m_popuppub->addAction(infoKey);
	m_popuppub->addAction(openKeyUrl);
	m_popuppub->addAction(editKey);
	m_popuppub->addAction(refreshKey);
	m_popuppub->addAction(createGroup);
	m_popuppub->addSeparator();
	m_popuppub->addAction(importAllSignKeys);

	m_popupsec = new QMenu(this);
	m_popupsec->addAction(exportPublicKey);
	m_popupsec->addAction(m_sendEmail);
	m_popupsec->addAction(signKey);
	m_popupsec->addAction(signUid);
	m_popupsec->addAction(signMailUid);
	m_popupsec->addAction(infoKey);
	m_popupsec->addAction(openKeyUrl);
	m_popupsec->addAction(editKey);
	m_popupsec->addAction(refreshKey);
	m_popupsec->addAction(setDefaultKey);
	m_popupsec->addSeparator();
	m_popupsec->addAction(importAllSignKeys);
	m_popupsec->addSeparator();
	m_popupsec->addAction(addPhoto);
	m_popupsec->addAction(addUid);
	m_popupsec->addAction(exportSecretKey);
	m_popupsec->addAction(deleteKeyPair);

	m_popupgroup = new QMenu(this);
	m_popupgroup->addAction(editCurrentGroup);
	m_popupgroup->addAction(m_groupRename);
	m_popupgroup->addAction(delGroup);
	m_popupgroup->addAction(refreshKey);

	m_popupout = new QMenu(this);
	m_popupout->addAction(importKey);

	m_popupsig = new QMenu(this);
	m_popupsig->addAction(importSignatureKey);
	m_popupsig->addAction(delSignKey);

	m_popupphoto = new QMenu(this);
	m_popupphoto->addAction(openPhoto);
	m_popupphoto->addAction(signUid);
	m_popupphoto->addAction(signMailUid);
	m_popupphoto->addAction(deletePhoto);

	m_popupuid = new QMenu(this);
	m_popupuid->addAction(m_sendEmail);
	m_popupuid->addAction(signMailUid);
	m_popupuid->addAction(signUid);
	m_popupuid->addAction(delUid);
	m_popupuid->addAction(setPrimUid);

	m_popuporphan = new QMenu(this);
	m_popuporphan->addAction(regeneratePublic);
	m_popuporphan->addAction(deleteKeyPair);

	exportPublicKey->setEnabled(false);

	KConfigGroup cg = KConfigGroup(KSharedConfig::openConfig().data(), "KeyView");
	iview->restoreLayout(cg);

	connect(photoProps, &KSelectAction::indexTriggered, this, &KeysManager::slotSetPhotoSize);
	connect(trustProps, &KSelectAction::indexTriggered, this, &KeysManager::slotSetTrustFilter);

	QLabel *searchLabel = new QLabel(i18n("Search:"), this);
	m_listviewsearch = new QLineEdit(this);
	m_listviewsearch->setClearButtonEnabled(true);

	QWidget *searchWidget = new QWidget(this);
	QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
	searchLayout->setContentsMargins(0, 0, 0, 0);
	searchLayout->addWidget(searchLabel);
	searchLayout->addWidget(m_listviewsearch);
	searchLayout->addStretch();

	QWidgetAction *searchLineAction = new QWidgetAction(/*i18nc("Name of the action that is a search line, shown for example in the toolbar configuration dialog",
			"Search Line"), */this);
	actionCollection()->addAction(QLatin1String( "search_line" ), searchLineAction);
	searchLineAction->setDefaultWidget(searchWidget);

	action = actionCollection()->addAction(QLatin1String("search_focus"), m_listviewsearch, QOverload<>::of(&QWidget::setFocus));
	action->setText(i18nc("Name of the action that gives the focus to the search line", "Focus Search Line"));
	actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::Key_F6));
	connect(m_listviewsearch, &QLineEdit::textChanged, iproxy, &KeyListProxyModel::setFilterFixedString);

	setActionDescriptions(1);

	// get all keys data
	setupGUI(KXmlGuiWindow::Create | Save | ToolBar | StatusBar | Keys, QLatin1String( "keysmanager.rc" ));

	sTrust->setChecked(KGpgSettings::showTrust());
	iview->setColumnHidden(2, !KGpgSettings::showTrust());
	sSize->setChecked(KGpgSettings::showSize());
	iview->setColumnHidden(3, !KGpgSettings::showSize());
	sCreat->setChecked(KGpgSettings::showCreat());
	iview->setColumnHidden(4, !KGpgSettings::showCreat());
	sExpi->setChecked(KGpgSettings::showExpi());
	iview->setColumnHidden(5, !KGpgSettings::showExpi());
	iproxy->setOnlySecret(KGpgSettings::showSecret());

	m_statusBarLabel.setAlignment(Qt::AlignCenter);
	statusBar()->addPermanentWidget(&m_statusBarLabel);

	cg = KConfigGroup(KSharedConfig::openConfig().data(), "MainWindow");
	setAutoSaveSettings(cg, true);
	applyMainWindowSettings(cg);

	connect(this, &KeysManager::fontChanged, s_kgpgEditor, &KgpgEditor::slotSetFont);

	QNetworkConfigurationManager *netmgr = new QNetworkConfigurationManager(this);
	connect(netmgr, &QNetworkConfigurationManager::onlineStateChanged, this, &KeysManager::toggleNetworkActions);

	toggleNetworkActions(netmgr->isOnline());
	importSignatureKey->setEnabled(false);

    stateChanged(QStringLiteral("empty_list"));

	QMetaObject::invokeMethod(this, "refreshkey", Qt::QueuedConnection);
}

KeysManager::~KeysManager()
{
}

void KeysManager::slotGenerateKey()
{
	if (m_genkey) {
		KMessageBox::error(this,
				i18n("Another key generation operation is still in progress.\nPlease wait a moment until this operation is complete."),
				i18n("Generating new key pair"));
		return;
	}

	QPointer<KgpgKeyGenerate> kg = new KgpgKeyGenerate(this);
	if (kg->exec() == QDialog::Accepted) {
		if (!kg->isExpertMode()) {
			KGpgGenerateKey *genkey = new KGpgGenerateKey(this, kg->name(), kg->email(),
					kg->comment(), kg->algo(), kg->size(), kg->days(), kg->expiration(),
					kg->caps());

			m_genkey = new KGpgTransactionJob(genkey);
			connect(m_genkey, &KGpgTransactionJob::result, this, &KeysManager::slotGenerateKeyDone);

			KIO::getJobTracker()->registerJob(m_genkey);
			m_genkey->start();
			QApplication::setOverrideCursor(Qt::BusyCursor);
		} else {
			KConfigGroup config(KSharedConfig::openConfig(), "General");

			QString terminalApp(config.readPathEntry("TerminalApplication", QLatin1String( "konsole" )));

			const QString gpgbin = KGpgSettings::gpgBinaryPath();
			QString gpg_args = gpgbin + QLatin1String(" --expert");
			if (GPGProc::gpgVersion(GPGProc::gpgVersionString(gpgbin)) > 0x20100)
				gpg_args += QLatin1String(" --full-gen-key ");
			else
				gpg_args += QLatin1String(" --gen-key ");

			gpg_args += GPGProc::getGpgHomeArguments(gpgbin).join(QLatin1Char(' '));

			QStringList args;
			args << QLatin1String( "-e" )
			     << gpg_args;

			QProcess *genKeyProc = new QProcess(this);
			genKeyProc->start(terminalApp, args);
			if (!genKeyProc->waitForStarted(-1)) {
				KMessageBox::error(this, i18n("Generating new key pair"),
						i18n("Can not start \"konsole\" application for expert mode."));
			} else {
				genKeyProc->waitForFinished(-1);
				refreshkey();
			}
		}
	}

	delete kg;
}

void KeysManager::showKeyManager()
{
	show();
}

void KeysManager::slotOpenEditor()
{
	KgpgEditor *kgpgtxtedit = new KgpgEditor(this, imodel, Qt::Window);

	connect(this, &KeysManager::fontChanged, kgpgtxtedit, &KgpgEditor::slotSetFont);

	kgpgtxtedit->show();
}

void KeysManager::changeMessage(const QString &msg, const bool keep)
{
	int timeout = keep ? 0 : 10000;

	statusBar()->showMessage(msg, timeout);
}

void KeysManager::updateStatusCounter()
{
	m_statusBarLabel.setText(imodel->statusCountMessage());
}

void KeysManager::slotGenerateKeyDone(KJob *job)
{
	changeMessage(i18nc("Application ready for user input", "Ready"));
	QApplication::restoreOverrideCursor();

	KGpgTransactionJob *tjob = qobject_cast<KGpgTransactionJob *>(job);

	const KGpgGenerateKey * const genkey = qobject_cast<const KGpgGenerateKey *>(tjob->getTransaction());
	int res = tjob->getResultCode();

	const QString infomessage(i18n("Generating new key pair"));

	switch (res) {
	case KGpgTransaction::TS_BAD_PASSPHRASE:
		KMessageBox::error(this, i18n("Bad passphrase. Cannot generate a new key pair."), infomessage);
		break;
	case KGpgTransaction::TS_USER_ABORTED:
		KMessageBox::error(this, i18n("Aborted by the user. Cannot generate a new key pair."), infomessage);
		break;
	case KGpgTransaction::TS_INVALID_EMAIL:
		KMessageBox::error(this, i18n("The email address is not valid. Cannot generate a new key pair."), infomessage);
		break;
	case KGpgGenerateKey::TS_INVALID_NAME:
		KMessageBox::error(this, i18n("The name is not accepted by gpg. Cannot generate a new key pair."), infomessage);
		break;
	case KGpgTransaction::TS_OK: {
		updateStatusCounter();

		QPointer<QDialog> keyCreated = new QDialog(this);
		keyCreated->setWindowTitle(i18n("New Key Pair Created"));

		QVBoxLayout *mainLayout = new QVBoxLayout(keyCreated);
		keyCreated->setLayout(mainLayout);

		newKey *page = new newKey(keyCreated);
		page->TLname->setText(QLatin1String( "<b>" ) + genkey->getName() + QLatin1String( "</b>" ));

		const QString email(genkey->getEmail());
		page->TLemail->setText(QLatin1String( "<b>" ) + email + QLatin1String( "</b>" ));

		page->kURLRequester1->setUrl(KGpgRevokeDialog::revokeUrl(genkey->getName(), email));

		const QString fingerprint(genkey->getFingerprint());
		page->TLid->setText(QLatin1String( "<b>" ) % fingerprint.rightRef(8) % QLatin1String( "</b>" ));
		page->LEfinger->setText(fingerprint);
		page->CBdefault->setChecked(true);
		page->show();
		mainLayout->addWidget(page);
		page->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
		connect(page->buttonBox, &QDialogButtonBox::accepted, keyCreated.data(), &QDialog::accept);

		keyCreated->exec();
		if (keyCreated.isNull())
			return;

		imodel->refreshKey(fingerprint);
		KGpgKeyNode *knode = imodel->getRootNode()->findKey(fingerprint);
		if (page->CBdefault->isChecked())
			imodel->setDefaultKey(knode);

		iview->selectNode(knode);

		if (page->CBsave->isChecked() || page->CBprint->isChecked()) {
			QUrl revurl;
			if (page->CBsave->isChecked())
				revurl = page->kURLRequester1->url();

			KGpgGenerateRevoke *genRev = new KGpgGenerateRevoke(this, fingerprint, revurl,
					0, i18n("backup copy"));

			connect(genRev, &KGpgGenerateRevoke::done, this, &KeysManager::slotRevokeGenerated);

			if (page->CBprint->isChecked())
				connect(genRev, &KGpgGenerateRevoke::revokeCertificate, this, &KeysManager::doPrint);

			genRev->start();
		}
		delete keyCreated;
		break;
	}
	default:
		KMessageBox::detailedError(this,
				i18n("gpg process did not finish. Cannot generate a new key pair."),
				genkey->gpgErrorMessage(), infomessage);
	}

	m_genkey = nullptr;
}

void KeysManager::slotShowTrust()
{
	bool b = !sTrust->isChecked();
	iview->setColumnHidden(KEYCOLUMN_TRUST, b);
	if (!b && (iview->columnWidth(KEYCOLUMN_TRUST) == 0))
		iview->resizeColumnToContents(KEYCOLUMN_TRUST);
}

void KeysManager::slotShowExpiration()
{
	bool b = !sExpi->isChecked();
	iview->setColumnHidden(KEYCOLUMN_EXPIR, b);
	if (!b && (iview->columnWidth(KEYCOLUMN_EXPIR) == 0))
		iview->resizeColumnToContents(KEYCOLUMN_EXPIR);
}

void KeysManager::slotShowSize()
{
	bool b = !sSize->isChecked();
	iview->setColumnHidden(KEYCOLUMN_SIZE, b);
	if (!b && (iview->columnWidth(KEYCOLUMN_SIZE) == 0))
		iview->resizeColumnToContents(KEYCOLUMN_SIZE);
}

void KeysManager::slotShowCreation()
{
	bool b = !sCreat->isChecked();
	iview->setColumnHidden(KEYCOLUMN_CREAT, b);
	if (!b && (iview->columnWidth(KEYCOLUMN_CREAT) == 0))
		iview->resizeColumnToContents(KEYCOLUMN_CREAT);
}

void KeysManager::slotShowLongId(bool b)
{
	iproxy->setIdLength(b ? 16 : 8);
}

void KeysManager::slotSetTrustFilter(int i)
{
	KgpgCore::KgpgKeyTrustFlag t;

	Q_ASSERT((i >= 0) && (i < 5));
	switch (i) {
	case 0:
		t = TRUST_MINIMUM;
		break;
	case 1:
		t = TRUST_UNDEFINED;
		break;
	case 2:
		t = TRUST_MARGINAL;
		break;
	case 3:
		t = TRUST_FULL;
		break;
	default:
		t = TRUST_ULTIMATE;
	}

	iproxy->setTrustFilter(t);
}

bool KeysManager::eventFilter(QObject *, QEvent *e)
{
	if ((e->type() == QEvent::Show) && (showTipOfDay)) {
		KTipDialog::showTip(this, QLatin1String("kgpg/tips"), false);
		showTipOfDay = false;
	}

	return false;
}

void KeysManager::slotGotoDefaultKey()
{
	iview->selectNode(imodel->getRootNode()->findKey(KGpgSettings::defaultKey()));
}

void KeysManager::refreshKeyFromServer()
{
	const auto keysList = iview->selectedNodes();
	if (keysList.empty())
		return;

	QStringList keyIDS;

	for (KGpgNode *item : keysList) {
		if (item->getType() == ITYPE_GROUP)
		{
			for (int j = 0; j < item->getChildCount(); j++)
				keyIDS << item->getChild(j)->getId();

			continue;
		}

		if (item->getType() & ITYPE_PAIR) {
			keyIDS << item->getId();
		} else {
			KMessageBox::error(this, i18n("You can only refresh primary keys. Please check your selection."));
			return;
		}
	}

	QString proxy;
	if (KGpgSettings::useProxy())
		proxy = QLatin1String( qgetenv("http_proxy") );

	KGpgRefreshKeys *t = new KGpgRefreshKeys(this, KGpgSettings::keyServers().first(), keyIDS, true, proxy);
	connect(t, &KGpgRefreshKeys::done, this, &KeysManager::slotKeyRefreshDone);
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	t->start();
}

void KeysManager::slotKeyRefreshDone(int result)
{
	KGpgRefreshKeys *t = qobject_cast<KGpgRefreshKeys *>(sender());
	Q_ASSERT(t != nullptr);

	if (result == KGpgTransaction::TS_USER_ABORTED) {
		t->deleteLater();
		QApplication::restoreOverrideCursor();
		return;
	}

	const QStringList log(t->getLog());
	const QStringList keys = KGpgImport::getImportedIds(log, 0xffff);
	const QStringList message(KGpgImport::getImportMessage(log));

	t->deleteLater();

	if (!keys.empty())
		imodel->refreshKeys(keys);

	QApplication::restoreOverrideCursor();
	(void) new KgpgDetailedInfo(this, message.join(QLatin1String("\n")), log.join(QLatin1String("<br/>")),
			KGpgImport::getDetailedImportMessage(log, imodel).split(QLatin1Char('\n')));
}

void KeysManager::slotDelUid()
{
	KGpgUidNode *nd = iview->selectedNode()->toUidNode();

	KGpgDelUid *deluid = new KGpgDelUid(this, nd);

	connect(deluid, &KGpgDelUid::done, this, &KeysManager::slotDelUidDone);
	deluid->start();
}

void KeysManager::slotDelUidDone(int result)
{
	KGpgDelUid * const deluid = qobject_cast<KGpgDelUid *>(sender());
	Q_ASSERT(deluid != nullptr);

	sender()->deleteLater();
	if (result == KGpgTransaction::TS_OK)
		imodel->refreshKey(deluid->getKeyId());
	// FIXME: do something useful with result if it is a failure
}

void KeysManager::slotPrimUid()
{
	KGpgPrimaryUid *puid = new KGpgPrimaryUid(this, iview->selectedNode()->toUidNode());

	connect(puid, &KGpgPrimaryUid::done, this, &KeysManager::slotPrimUidDone);

	puid->start();
}

void KeysManager::slotPrimUidDone(int result)
{
	const QString kid(qobject_cast<KGpgPrimaryUid *>(sender())->getKeyId());

	sender()->deleteLater();

	if (result == KGpgTransaction::TS_OK)
		imodel->refreshKey(kid);
	// FIXME: some error reporting
}

void KeysManager::slotregenerate()
{
	QString regID = iview->selectedNode()->getId();
	KProcess *p1, *p2, *p3;

	p1 = new KProcess(this);
	*p1 << KGpgSettings::gpgBinaryPath()
			<< QLatin1String("--no-secmem-warning")
			<< QLatin1String("--export-secret-key")
			<< regID;
	p1->setOutputChannelMode(KProcess::OnlyStdoutChannel);

	p2 = new KProcess(this);
	*p2 << QLatin1String("gpgsplit")
			<< QLatin1String("--no-split")
			<< QLatin1String("--secret-to-public");
	p2->setOutputChannelMode(KProcess::OnlyStdoutChannel);

	p3 = new KProcess(this);
	*p3 << KGpgSettings::gpgBinaryPath()
			<< QLatin1String("--import");

	p1->setStandardOutputProcess(p2);
	p2->setStandardOutputProcess(p3);

	p1->start();
	p2->start();
	p3->start();

	p1->waitForFinished();
	p2->waitForFinished();
	p3->waitForFinished();

	delete p1;
	delete p2;
	delete p3;

	imodel->refreshKey(regID);
}

void KeysManager::slotAddUid()
{
	if (m_adduid) {
		KMessageBox::error(this, i18n("Another operation is still in progress.\nPlease wait a moment until this operation is complete."),
				i18n("Add New User Id"));
		return;
	}

	addUidWidget = new QDialog(this);
	addUidWidget->setWindowTitle(i18n("Add New User Id"));
	QVBoxLayout *mainLayout = new QVBoxLayout(addUidWidget);
	addUidWidget->setLayout(mainLayout);
	keyUid = new AddUid(addUidWidget);
	mainLayout->addWidget(keyUid);

	keyUid->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	keyUid->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(keyUid->buttonBox, &QDialogButtonBox::accepted, addUidWidget, &QDialog::accept);
	connect(keyUid->buttonBox, &QDialogButtonBox::rejected, addUidWidget, &QDialog::reject);

	//keyUid->setMinimumSize(keyUid->sizeHint());
	keyUid->setMinimumWidth(300);

	connect(keyUid->qLineEdit1, &QLineEdit::textChanged, this, &KeysManager::slotAddUidEnable);
	if (addUidWidget->exec() != QDialog::Accepted)
		return;

	m_adduid = new KGpgAddUid(this, iview->selectedNode()->getId(), keyUid->qLineEdit1->text(),
			keyUid->qLineEdit2->text(), keyUid->qLineEdit3->text());
	connect(m_adduid, &KGpgAddUid::done, this, &KeysManager::slotAddUidFin);
	m_adduid->start();
}

void KeysManager::slotAddUidFin(int res)
{
	// TODO error reporting
	if (res == 0)
		imodel->refreshKey(m_adduid->getKeyid());
	m_adduid->deleteLater();
	m_adduid = nullptr;
}

void KeysManager::slotAddUidEnable(const QString & name)
{
	keyUid->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(name.length() > 4);
}

void KeysManager::slotAddPhoto()
{
	QString mess = i18n("The image must be a JPEG file. Remember that the image is stored within your public key, so "
	"if you use a very large picture, your key will become very large as well. The size should not exceed 6 KiB. "
	"An image size of around 240x288 is a good size to use.");

    if (KMessageBox::warningContinueCancel(nullptr, mess) != KMessageBox::Continue)
		return;

    QString imagepath = QFileDialog::getOpenFileName(nullptr, QString(), QString(), QLatin1String( "image/jpeg" ));
	if (imagepath.isEmpty())
		return;

	KGpgAddPhoto *addphoto = new KGpgAddPhoto(this, iview->selectedNode()->getId(), imagepath);
	connect(addphoto, &KGpgAddPhoto::done, this, &KeysManager::slotAddPhotoFinished);
	addphoto->start();
}

void KeysManager::slotAddPhotoFinished(int res)
{
	sender()->deleteLater();

	// TODO : add res == 3 (bad passphrase)

	if (res == 0)
		slotUpdatePhoto();
}

void KeysManager::slotDeletePhoto()
{
	KGpgNode *nd = iview->selectedNode();
	KGpgUatNode *und = nd->toUatNode();
	KGpgKeyNode *parent = und->getParentKeyNode();

	QString mess = i18n("<qt>Are you sure you want to delete Photo id <b>%1</b><br/>from key <b>%2 &lt;%3&gt;</b>?</qt>",
				und->getId(), parent->getName(), parent->getEmail());

	if (KMessageBox::warningContinueCancel(this, mess) != KMessageBox::Continue)
		return;

	KGpgDelUid *deluid = new KGpgDelUid(this, und);
	connect(deluid, &KGpgDelUid::done, this, &KeysManager::slotDelPhotoFinished);

	deluid->start();
}

void KeysManager::slotDelPhotoFinished(int res)
{
	sender()->deleteLater();

	// TODO : add res == 3 (bad passphrase)

	if (res == 0) {
		KGpgNode *nd = iview->selectedNode();
		imodel->refreshKey(nd->getParentKeyNode()->toKeyNode());
	}
}

void KeysManager::slotUpdatePhoto()
{
	KGpgNode *nd = iview->selectedNode();
	imodel->refreshKey(nd->toKeyNode());
}

void KeysManager::slotSetPhotoSize(int size)
{
	switch(size) {
	case 1:
		iproxy->setPreviewSize(22);
		break;
	case 2:
		iproxy->setPreviewSize(42);
		break;
	case 3:
		iproxy->setPreviewSize(65);
		break;
	default:
		iproxy->setPreviewSize(0);
		break;
	}
}

void KeysManager::addToKAB()
{
	KGpgNode *nd = iview->selectedNode();
	if (nd == nullptr)
		return;

	Akonadi::ContactSearchJob * const job = new Akonadi::ContactSearchJob();
	job->setLimit(1);
	job->setQuery(Akonadi::ContactSearchJob::Email, nd->getEmail());
	connect(job, &Akonadi::ContactSearchJob::result, this, &KeysManager::slotAddressbookSearchResult);

	m_addIds[job] = nd;
}

void KeysManager::slotAddressbookSearchResult(KJob *job)
{
	KGpgNode * const nd = m_addIds.value(job, nullptr);

	if (!nd)
		return;

	Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>(job);
	Q_ASSERT(searchJob);
	const KContacts::Addressee::List addresseeList = searchJob->contacts();

	m_addIds.take(job);

	Akonadi::ContactEditorDialog *dlg;
// 	KContacts::Key key; TODO
	if (!addresseeList.isEmpty()) {
		dlg = new Akonadi::ContactEditorDialog(Akonadi::ContactEditorDialog::EditMode, this);
		dlg->setContact(searchJob->items().at(0));
	} else {
		KContacts::Addressee addressee;
		addressee.setNameFromString(nd->getName());
		addressee.setEmails(QStringList(nd->getEmail()));
		dlg = new Akonadi::ContactEditorDialog(Akonadi::ContactEditorDialog::CreateMode, this);
		dlg->editor()->setContactTemplate(addressee);
	}

	connect(dlg, &Akonadi::ContactEditorDialog::finished, dlg, &Akonadi::ContactEditorDialog::deleteLater);
	dlg->show();
}

void KeysManager::slotManpage()
{
	const KService::Ptr helpCenter = KService::serviceByDesktopName(QStringLiteral("org.kde.khelpcenter"));
	auto job = new KIO::ApplicationLauncherJob(helpCenter);
	job->setUrls({QUrl(QStringLiteral("man:/gpg"))});
	job->start();
}

void KeysManager::slotTip()
{
	KTipDialog::showTip(this, QLatin1String("kgpg/tips"), true);
}

void KeysManager::showKeyServer()
{
	QPointer<KeyServer> ks = new KeyServer(this, imodel);
	connect(ks.data(), &KeyServer::importFinished, imodel, QOverload<const QStringList &>::of(&KGpgItemModel::refreshKeys));
	ks->exec();

	delete ks;
	refreshkey();
}

void KeysManager::checkList()
{
	const auto exportList = iview->selectedNodes();

	switch (exportList.size()) {
	case 0:
		stateChanged(QStringLiteral("empty_list"));
		return;
	case 1:
		if (exportList.at(0)->getType() == ITYPE_GROUP) {
			stateChanged(QLatin1String( "group_selected" ));
		} else {
			stateChanged(QLatin1String( "single_selected" ));
			m_revokeKey->setEnabled(exportList.at(0)->getType() == ITYPE_PAIR);
			if (terminalkey)
				editKey->setEnabled(false);
			m_sendEmail->setEnabled(!exportList[0]->getEmail().isEmpty());
			setDefaultKey->setEnabled(!imodel->isDefaultKey(exportList[0]));
		}
		break;
	default:
		stateChanged(QLatin1String( "multi_selected" ));
	}
	if (!m_online)
		refreshKey->setEnabled(false);

	switch (exportList.at(0)->getType()) {
	case ITYPE_PUBLIC:
		changeMessage(i18n("Public Key"));
		break;
	case ITYPE_SUB:
		changeMessage(i18n("Sub Key"));
		break;
	case ITYPE_PAIR:
		changeMessage(i18n("Secret Key Pair"));
		break;
	case ITYPE_GROUP:
		changeMessage(i18n("Key Group"));
		break;
	case ITYPE_SIGN:
		changeMessage(i18n("Signature"));
		break;
	case ITYPE_UID:
		changeMessage(i18n("User ID"));
		break;
	case ITYPE_REVSIGN:
		changeMessage(i18n("Revocation Signature"));
		break;
	case ITYPE_UAT:
		changeMessage(i18n("Photo ID"));
		break;
	case ITYPE_SECRET:
		changeMessage(i18n("Orphaned Secret Key"));
		break;
	case ITYPE_GPUBLIC:
	case ITYPE_GSECRET:
	case ITYPE_GPAIR:
		changeMessage(i18n("Group member"));
		break;
	default:
		qCDebug(KGPG_LOG_GENERAL) << "Oops, unmatched type value" << exportList.at(0)->getType();
	}
}

void KeysManager::quitApp()
{
	// close window
	saveToggleOpts();
	qApp->quit();
}

void KeysManager::saveToggleOpts(void)
{
	KConfigGroup cg = KConfigGroup(KSharedConfig::openConfig().data(), "KeyView");
	iview->saveLayout(cg);
	KGpgSettings::setPhotoProperties(photoProps->currentItem());
	KGpgSettings::setShowTrust(sTrust->isChecked());
	KGpgSettings::setShowExpi(sExpi->isChecked());
	KGpgSettings::setShowCreat(sCreat->isChecked());
	KGpgSettings::setShowSize(sSize->isChecked());
	KGpgSettings::setTrustLevel(trustProps->currentItem());
	KGpgSettings::setShowSecret(hPublic->isChecked());
	KGpgSettings::setShowLongKeyId(longId->isChecked());
	KGpgSettings::self()->save();
}

void KeysManager::readOptions()
{
	m_clipboardmode = QClipboard::Clipboard;
	if (KGpgSettings::useMouseSelection() && (qApp->clipboard()->supportsSelection()))
		m_clipboardmode = QClipboard::Selection;

	if (imodel != nullptr)
		updateStatusCounter();

	showTipOfDay = KGpgSettings::showTipOfDay();

	if (KGpgSettings::showSystray()) {
		setupTrayIcon();
	} else {
		delete m_trayicon;
		m_trayicon = nullptr;
	}
}

void KeysManager::showOptions()
{
	if (KConfigDialog::showDialog(QLatin1String( "settings" )))
		return;

	QPointer<kgpgOptions> optionsDialog = new kgpgOptions(this, imodel);
	connect(optionsDialog.data(), &kgpgOptions::settingsUpdated, this, &KeysManager::readAllOptions);
	connect(optionsDialog.data(), &kgpgOptions::homeChanged, imodel, &KGpgItemModel::refreshAllKeys);
	connect(optionsDialog.data(), &kgpgOptions::homeChanged, imodel, &KGpgItemModel::refreshGroups);
	connect(optionsDialog.data(), &kgpgOptions::refreshTrust, imodel, &KGpgItemModel::refreshTrust);
	connect(optionsDialog.data(), &kgpgOptions::changeFont, this, &KeysManager::fontChanged);
	optionsDialog->exec();
	delete optionsDialog;

	s_kgpgEditor->m_recentfiles->setMaxItems(KGpgSettings::recentFiles());
}

void KeysManager::readAllOptions()
{
	readOptions();
	Q_EMIT readAgainOptions();
}

void KeysManager::slotSetDefKey()
{
	setDefaultKeyNode(iview->selectedNode()->toKeyNode());
}

void KeysManager::slotSetDefaultKey(const QString &newID)
{
	KGpgKeyNode *ndef = imodel->getRootNode()->findKey(newID);

	if (ndef == nullptr) {
		KGpgSettings::setDefaultKey(newID);
		KGpgSettings::self()->save();
		return;
	}

	setDefaultKeyNode(ndef);
}

void KeysManager::setDefaultKeyNode(KGpgKeyNode *key)
{
	const QString &newID(key->getId());

	if (newID == KGpgSettings::defaultKey())
		return;

	KGpgSettings::setDefaultKey(newID);
	KGpgSettings::self()->save();

	imodel->setDefaultKey(key);
}

void
KeysManager::setActionDescriptions(int cnt)
{
	signUid->setText(i18np("&Sign User ID ...", "&Sign User IDs ...", cnt));
	signMailUid->setText(i18np("Sign and &Mail User ID ...", "Sign and &Mail User IDs ...", cnt));
	exportPublicKey->setText(i18np("E&xport Public Key...", "E&xport Public Keys...", cnt));
	refreshKey->setText(i18np("&Refresh Key From Keyserver", "&Refresh Keys From Keyserver", cnt));
	createGroup->setText(i18np("&Create Group with Selected Key...", "&Create Group with Selected Keys...", cnt));
	signKey->setText(i18np("&Sign Key...", "&Sign Keys...", cnt));
	delUid->setText(i18np("&Delete User ID", "&Delete User IDs", cnt));
	delSignKey->setText(i18np("Delete Sign&ature", "Delete Sign&atures", cnt));
	importSignatureKey->setText(i18np("Import Key From Keyserver", "Import Keys From Keyserver", cnt));
	deleteKey->setText(i18np("&Delete Key", "&Delete Keys", cnt));
}

void
KeysManager::slotMenu(const QPoint &pos)
{
	QPoint globpos = iview->mapToGlobal(pos);
	bool sametype;
	KgpgItemType itype;
	const auto ndlist = iview->selectedNodes(&sametype, &itype);
	bool unksig = false;
	QSet<QString> l;
	const int cnt = ndlist.size();

	// find out if an item has unknown signatures. Only check if the item has been
	// expanded before as expansion is very expensive and can take several seconds
	// that will freeze the UI meanwhile.
	for (KGpgNode *nd : ndlist) {
		if (!nd->hasChildren())
			continue;

		KGpgExpandableNode *exnd = nd->toExpandableNode();
		if (!exnd->wasExpanded()) {
			unksig = true;
			break;
		}
		getMissingSigs(l, exnd);
		if (!l.isEmpty()) {
			unksig = true;
			break;
		}
	}
	importAllSignKeys->setEnabled(unksig && m_online);

	signUid->setEnabled(!(itype & ~(ITYPE_PAIR | ITYPE_UID | ITYPE_UAT)));
	signMailUid->setEnabled(signUid->isEnabled());
	setActionDescriptions(cnt);

	if (itype == ITYPE_SIGN) {
		bool allunksig = true;
		for (KGpgNode *nd : ndlist) {
			allunksig = nd->toSignNode()->isUnknown();
			if (!allunksig)
				break;
		}

		importSignatureKey->setEnabled(allunksig && m_online);
		delSignKey->setEnabled( (cnt == 1) );
		m_popupsig->exec(globpos);
	} else if (itype == ITYPE_UID) {
		if (cnt == 1) {
			KGpgKeyNode *knd = ndlist.at(0)->toUidNode()->getParentKeyNode();
			setPrimUid->setEnabled(knd->getType() & ITYPE_SECRET);
		}
		m_popupuid->exec(globpos);
	} else if ((itype == ITYPE_UAT) && (cnt == 1)) {
		m_popupphoto->exec(globpos);
	} else if ((itype == ITYPE_PAIR) && (cnt == 1)) {
		m_popupsec->exec(globpos);
	} else if ((itype == ITYPE_SECRET) && (cnt == 1)) {
		m_popuporphan->exec(globpos);
	} else if (itype == ITYPE_GROUP) {
		delGroup->setEnabled( (cnt == 1) );
		editCurrentGroup->setEnabled( (cnt == 1) );
		m_groupRename->setEnabled( (cnt == 1) );
		m_popupgroup->exec(globpos);
	} else if (!(itype & ~(ITYPE_PAIR | ITYPE_GROUP))) {
		signKey->setEnabled(!(itype & ITYPE_GROUP));
		deleteKey->setEnabled(!(itype & ITYPE_GROUP));
		setDefaultKey->setEnabled( (cnt == 1) );
		m_popuppub->exec(globpos);
	} else if (!(itype & ~(ITYPE_UID | ITYPE_PAIR | ITYPE_UAT))) {
		setPrimUid->setEnabled(false);
		delUid->setEnabled(false);
		m_popupuid->exec(globpos);
	} else {
		m_popupout->exec(globpos);
	}
}

void KeysManager::revokeWidget()
{
	KGpgNode *nd = iview->selectedNode();
	QDialog *keyRevokeDialog = new KGpgRevokeDialog(this, nd->toKeyNode());

	connect(keyRevokeDialog, &QDialog::finished, this, &KeysManager::slotRevokeDialogFinished);

	keyRevokeDialog->open();
}

void KeysManager::slotRevokeDialogFinished(int result)
{
	sender()->deleteLater();

	if (result != QDialog::Accepted)
		return;

	KGpgRevokeDialog *keyRevokeDialog = qobject_cast<KGpgRevokeDialog *>(sender());

	KGpgGenerateRevoke *genRev = new KGpgGenerateRevoke(this, keyRevokeDialog->getId(), keyRevokeDialog->saveUrl(),
			keyRevokeDialog->getReason(), keyRevokeDialog->getDescription());

	connect(genRev, &KGpgGenerateRevoke::done, this, &KeysManager::slotRevokeGenerated);

	if (keyRevokeDialog->printChecked())
		connect(genRev, &KGpgGenerateRevoke::revokeCertificate, this, &KeysManager::doPrint);
	if (keyRevokeDialog->importChecked())
		connect(genRev, &KGpgGenerateRevoke::revokeCertificate, this, &KeysManager::slotImportRevokeTxt);

	genRev->start();
}

void KeysManager::slotRevokeGenerated(int result)
{
	KGpgGenerateRevoke *genRev = qobject_cast<KGpgGenerateRevoke *>(sender());

	genRev->deleteLater();

	switch (result) {
	case KGpgTransaction::TS_OK:
	case KGpgTransaction::TS_USER_ABORTED:
		break;
	default:
		KMessageBox::detailedError(this, i18n("Creation of the revocation certificate failed..."), genRev->getOutput());
		break;
	}
}

void KeysManager::slotImportRevokeTxt(const QString &revokeText)
{
	KGpgImport *import = new KGpgImport(this, revokeText);
	connect(import, &KGpgImport::done, this, &KeysManager::slotImportDone);
	import->start();
}

void KeysManager::slotexportsec()
{
	// export secret key
	const QString warn(i18n("<qt>Secret keys <b>should not</b> be saved in an unsafe place.<br/>"
				"If someone else can access this file, encryption with this key will be compromised.<br/>Continue key export?</qt>"));
	int result = KMessageBox::warningContinueCancel(this, warn);
	if (result != KMessageBox::Continue)
		return;
	KGpgNode *nd = iview->selectedNode();

	QString sname(nd->getEmail().section(QLatin1Char( '@' ), 0, 0).section(QLatin1Char( '.' ), 0, 0));
	if (sname.isEmpty())
		sname = nd->getName().section(QLatin1Char( ' ' ), 0, 0);
	sname.append(QLatin1String( ".asc" ));
	sname.prepend(QDir::homePath() + QLatin1Char( '/' ));
	QUrl url(QFileDialog::getSaveFileUrl(this, i18n("Export PRIVATE KEY As"), QUrl(sname), i18n( "*.asc|*.asc Files" )));

	if(!url.isEmpty()) {
		KGpgExport *exp = new KGpgExport(this, QStringList(nd->getId()), url.path(), QStringList(QLatin1String( "--armor" )), true);

		connect(exp, &KGpgExport::done, this, &KeysManager::slotExportSecFinished);

		exp->start();
	}
}

void KeysManager::slotExportSecFinished(int result)
{
	KGpgExport *exp = qobject_cast<KGpgExport *>(sender());
	Q_ASSERT(exp != nullptr);

	if (result == KGpgTransaction::TS_OK) {
		KMessageBox::information(this,
				i18n("<qt>Your <b>private</b> key \"%1\" was successfully exported to<br/>%2.<br/><b>Do not</b> leave it in an insecure place.</qt>",
				exp->getKeyIds().first(), exp->getOutputFile()));
	} else {
		KMessageBox::error(this, i18n("Your secret key could not be exported.\nCheck the key."));
	}
}

void KeysManager::slotexport()
{
	bool same;
	KgpgItemType tp;

	const auto ndlist = iview->selectedNodes(&same, &tp);
	if (ndlist.empty())
		return;
	if (!(tp & ITYPE_PUBLIC) || (tp & ~ITYPE_GPAIR))
		return;

	QString sname;

	if (ndlist.size() == 1) {
		sname = ndlist.at(0)->getEmail().section(QLatin1Char( '@' ), 0, 0).section(QLatin1Char( '.' ), 0, 0);
		if (sname.isEmpty())
			sname = ndlist.at(0)->getName().section(QLatin1Char(' '), 0, 0);
	} else
		sname = QLatin1String( "keyring" );

	QStringList klist;
	for (const auto k : ndlist)
		klist << k->getId();

	sname.append(QLatin1String( ".asc" ));
	sname.prepend(QDir::homePath() + QLatin1Char( '/' ));

	QStringList serverList(KGpgSettings::keyServers());
	serverList.replaceInStrings(QRegularExpression(QStringLiteral(" .*")), QString()); // Remove kde 3.5 (Default) tag.
	if (!serverList.isEmpty()) {
		QString defaultServer = serverList.takeFirst();
		std::sort(serverList.begin(), serverList.end());
		serverList.prepend(defaultServer);
	}

	QPointer<KeyExport> page = new KeyExport(this, serverList);

	page->newFilename->setUrl(QUrl(sname));

	if (!m_online)
		page->checkServer->setEnabled(false);

	if (page->exec() == QDialog::Accepted) {
		// export to file
		QString exportAttr;

		if (page->checkAttrAll->isChecked()) {
			// nothing
		} else if (page->checkAttrPhoto->isChecked()) {
			exportAttr = QLatin1String( "no-export-attributes" );
		} else {
			exportAttr = QLatin1String( "export-minimal" );
		}
		QStringList expopts;

		if (!exportAttr.isEmpty())
			expopts << QLatin1String( "--export-options" ) << exportAttr;

		if (page->checkServer->isChecked()) {
			KeyServer *expServer = new KeyServer(nullptr, imodel);
			expServer->slotSetExportAttribute(exportAttr);
			expServer->slotSetKeyserver(page->destServer->currentText());

			expServer->slotExport(klist);
		} else if (page->checkFile->isChecked()) {
			const QString expname(page->newFilename->url().path().simplified());
			if (!expname.isEmpty()) {

				expopts.append(QLatin1String( "--armor" ));

				KGpgExport *exp = new KGpgExport(this, klist, expname, expopts);

				connect(exp, &KGpgExport::done, this, &KeysManager::slotExportFinished);

				exp->start();
			}
		} else {
			KGpgExport *exp = new KGpgExport(this, klist, expopts);

			if (page->checkClipboard->isChecked())
				connect(exp, &KGpgExport::done, this, &KeysManager::slotProcessExportClip);
			else
				connect(exp, &KGpgExport::done, this, &KeysManager::slotProcessExportMail);

			exp->start();
		}
	}

	delete page;
}

void KeysManager::slotExportFinished(int result)
{
	KGpgExport *exp = qobject_cast<KGpgExport *>(sender());
	Q_ASSERT(exp != nullptr);

	if (result == KGpgTransaction::TS_OK) {
		KMessageBox::information(this,
				i18np("<qt>The public key was successfully exported to<br/>%2</qt>",
						"<qt>The %1 public keys were successfully exported to<br/>%2</qt>",
						exp->getKeyIds().count(), exp->getOutputFile()));
	} else {
		KMessageBox::error(this, i18n("Your public key could not be exported\nCheck the key."));
	}

	exp->deleteLater();
}

void KeysManager::slotProcessExportMail(int result)
{
	KGpgExport *exp = qobject_cast<KGpgExport *>(sender());
	Q_ASSERT(exp != nullptr);

	// start default Mail application
	if (result == KGpgTransaction::TS_OK) {
		QDesktopServices::openUrl(QUrl(QLatin1String("mailto:?body=") + QLatin1String(exp->getOutputData())));
	} else {
		KMessageBox::error(this, i18n("Your public key could not be exported\nCheck the key."));
	}

	exp->deleteLater();
}

void KeysManager::slotProcessExportClip(int result)
{
	KGpgExport *exp = qobject_cast<KGpgExport *>(sender());
	Q_ASSERT(exp != nullptr);

	if (result == KGpgTransaction::TS_OK) {
		qApp->clipboard()->setText(QLatin1String( exp->getOutputData() ), m_clipboardmode);
	} else {
		KMessageBox::error(this, i18n("Your public key could not be exported\nCheck the key."));
	}

	exp->deleteLater();
}

void KeysManager::showKeyInfo(const QString &keyID)
{
	KGpgKeyNode *key = imodel->getRootNode()->findKey(keyID);

	if (key == nullptr)
		return;

	showProperties(key);
}

void KeysManager::slotShowPhoto()
{
	KGpgNode *nd = iview->selectedNode();
	KGpgUatNode *und = nd->toUatNode();
	KGpgKeyNode *parent = und->getParentKeyNode();

	KProcess p;
	p << KGpgSettings::gpgBinaryPath()
			<< QLatin1String("--no-tty")
			<< QLatin1String("--edit-key")
			<< parent->getId()
			<< QLatin1String("uid")
			<< und->getId()
			<< QLatin1String("showphoto")
			<< QLatin1String("quit");
	p.startDetached();
}

void KeysManager::defaultAction(const QModelIndex &index)
{
	KGpgNode *nd = iproxy->nodeForIndex(index);

	defaultAction(nd);
}

void KeysManager::slotDefaultAction()
{
	defaultAction(iview->selectedNode());
}

void KeysManager::defaultAction(KGpgNode *nd)
{
	if (nd == nullptr)
		return;

	if (iview->isEditing())
		return;

	switch (nd->getType()) {
	case ITYPE_GROUP:
		editGroup();
		break;
	case ITYPE_UAT:
		slotShowPhoto();
		break;
	case ITYPE_SIGN:
	case ITYPE_GPUBLIC:
	case ITYPE_GSECRET:
	case ITYPE_GPAIR:
		iview->selectNode(nd->toRefNode()->getRefNode());
		break;
	case ITYPE_SECRET:
		slotregenerate();
		break;
	case ITYPE_PAIR:
	case ITYPE_PUBLIC:
		showProperties(nd);
		return;
        }
}

void
KeysManager::showProperties(KGpgNode *n)
{
	switch (n->getType()) {
	case ITYPE_UAT:
		return;
	case ITYPE_PUBLIC:
	case ITYPE_PAIR: {
		KGpgKeyNode *k = n->toKeyNode();
		QPointer<KgpgKeyInfo> opts = new KgpgKeyInfo(k, imodel, this);
		connect(opts.data(), &KgpgKeyInfo::keyNeedsRefresh, imodel, QOverload<KGpgKeyNode *>::of(&KGpgItemModel::refreshKey));
		connect(opts->keychange, &KGpgChangeKey::keyNeedsRefresh, imodel, QOverload<KGpgKeyNode *>::of(&KGpgItemModel::refreshKey));
		opts->exec();
		delete opts;
	}
	default:
		return;
	}
}

void KeysManager::keyproperties()
{
	KGpgNode *cur = iview->selectedNode();
	if (cur == nullptr)
		return;

	KGpgKeyNode *kn;

	switch (cur->getType()) {
	case ITYPE_SECRET:
	case ITYPE_GSECRET:
		if (KMessageBox::questionYesNo(this,
			i18n("<p>This key is an orphaned secret key (secret key without public key.) It is currently not usable.</p>"
					"<p>Would you like to regenerate the public key?</p>"),
					QString(), KGuiItem(i18n("Generate")), KGuiItem(i18n("Do Not Generate"))) == KMessageBox::Yes)
		slotregenerate();
		return;
	case ITYPE_PAIR:
	case ITYPE_PUBLIC: {
		kn = cur->toKeyNode();
		break;
	}
	case ITYPE_GPAIR:
	case ITYPE_GPUBLIC: {
		kn = cur->toGroupMemberNode()->getRefNode();
		break;
	}
	default:
		qCDebug(KGPG_LOG_GENERAL) << "Oops, called with invalid item type" << cur->getType();
		return;
	}

	QPointer<KgpgKeyInfo> opts = new KgpgKeyInfo(kn, imodel, this);
	connect(opts.data(), &KgpgKeyInfo::keyNeedsRefresh, imodel, QOverload<KGpgKeyNode *>::of(&KGpgItemModel::refreshKey));
	opts->exec();
	delete opts;
}

void KeysManager::deleteGroup()
{
	KGpgNode *nd = iview->selectedNode();
	if (!nd || (nd->getType() != ITYPE_GROUP))
		return;

	int result = KMessageBox::warningContinueCancel(this, i18n("<qt>Are you sure you want to delete group <b>%1</b> ?</qt>",
			nd->getName()), QString(), KGuiItem(i18n("Delete"), QLatin1String("edit-delete")));
	if (result != KMessageBox::Continue)
		return;

	nd->toGroupNode()->remove();
	imodel->delNode(nd);

	updateStatusCounter();
}

void KeysManager::renameGroup()
{
	if (iview->selectionModel()->selectedIndexes().isEmpty())
		return;

	QModelIndex selectedNodeIndex = iview->selectionModel()->selectedIndexes().at(0);

	iview->edit(selectedNodeIndex);
}

void KeysManager::createNewGroup()
{
	QStringList badkeys;
	KGpgKeyNode::List keysList;
	KgpgItemType tp;
	const auto ndlist = iview->selectedNodes(nullptr, &tp);

	if (ndlist.empty())
		return;
	if (tp & ~ITYPE_PAIR) {
		KMessageBox::error(this, i18n("<qt>You cannot create a group containing signatures, subkeys or other groups.</qt>"));
		return;
	}

	KgpgKeyTrustFlag mintrust;
	if (KGpgSettings::allowUntrustedGroupMembers()) {
		mintrust = KgpgCore::TRUST_UNDEFINED;
	} else {
		mintrust = KgpgCore::TRUST_FULL;
	}

	for (KGpgNode *nd : ndlist) {
		if (nd->getTrust() >= mintrust) {
			keysList.append(nd->toKeyNode());
		} else {
			badkeys += i18nc("<Name> (<Email>) ID: <KeyId>", "%1 (%2) ID: %3",
					nd->getName(), nd->getEmail(), nd->getId());
		}
	}

        QString groupName(QInputDialog::getText(this, i18n("Create New Group"),
			i18nc("Enter the name of the group you are creating now", "Enter new group name:")));
	if (groupName.isEmpty())
		return;
	if (!keysList.isEmpty()) {
		if (!badkeys.isEmpty())
			KMessageBox::informationList(this, i18n("Following keys are not valid or not trusted and will not be added to the group:"), badkeys);

		iview->selectNode(imodel->addGroup(groupName, keysList));
		updateStatusCounter();
        } else {
		KMessageBox::error(this,
				i18n("<qt>No valid or trusted key was selected. The group <b>%1</b> will not be created.</qt>",
				groupName));
	}
}

void KeysManager::editGroup()
{
	KGpgNode *nd = iview->selectedNode();
	if (!nd || (nd->getType() != ITYPE_GROUP))
		return;
	KGpgGroupNode *gnd = nd->toGroupNode();
	QPointer<QDialog> dialogGroupEdit = new QDialog(this );
	dialogGroupEdit->setWindowTitle(i18n("Group Properties"));
	QVBoxLayout *mainLayout = new QVBoxLayout(dialogGroupEdit);
	QWidget *mainWidget = new QWidget(this);
	mainLayout->addWidget(mainWidget);
	dialogGroupEdit->setLayout(mainLayout);


	QList<KGpgNode *> members(gnd->getChildren());

	groupEdit *gEdit = new groupEdit(dialogGroupEdit, &members, imodel);

	mainLayout->addWidget(gEdit);
	gEdit->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(gEdit->buttonBox, &QDialogButtonBox::accepted, dialogGroupEdit.data(), &QDialog::accept);
	connect(gEdit->buttonBox, &QDialogButtonBox::rejected, dialogGroupEdit.data(), &QDialog::reject);

	gEdit->show();

	if (dialogGroupEdit->exec() == QDialog::Accepted)
		imodel->changeGroup(gnd, members);

	delete dialogGroupEdit;
}

void KeysManager::signkey()
{
	// another sign operation is still running
	if (!signList.isEmpty())
		return;

	KgpgItemType tp;
	const auto tmplist = iview->selectedNodes(nullptr, &tp);
	if (tmplist.empty())
		return;

	if (tp & ~ITYPE_PAIR) {
		KMessageBox::error(this, i18n("You can only sign primary keys. Please check your selection."));
		return;
	}

	if (tmplist.size() == 1) {
		KGpgKeyNode *nd = tmplist.at(0)->toKeyNode();
		QString opt;

		if (nd->getEmail().isEmpty())
			opt = i18n("<qt>You are about to sign key:<br /><br />%1<br />ID: %2<br />Fingerprint: <br /><b>%3</b>.<br /><br />"
					"You should check the key fingerprint by phoning or meeting the key owner to be sure that someone "
					"is not trying to intercept your communications.</qt>",
					nd->getName(), nd->getId().right(8), nd->getBeautifiedFingerprint());
		else
			opt = i18n("<qt>You are about to sign key:<br /><br />%1 (%2)<br />ID: %3<br />Fingerprint: <br /><b>%4</b>.<br /><br />"
					"You should check the key fingerprint by phoning or meeting the key owner to be sure that someone "
					"is not trying to intercept your communications.</qt>",
					nd->getName(), nd->getEmail(), nd->getId().right(8), nd->getBeautifiedFingerprint());

		if (KMessageBox::warningContinueCancel(this, opt) != KMessageBox::Continue) {
			return;
		}
		signList.append(nd);
	} else {
		QStringList signKeyList;
		for (KGpgNode *n : tmplist) {
			const KGpgKeyNode *nd = n->toKeyNode();

			if (nd->getEmail().isEmpty())
				signKeyList += i18nc("Name: ID", "%1: %2", nd->getName(), nd->getBeautifiedFingerprint());
			else
				signKeyList += i18nc("Name (Email): ID", "%1 (%2): %3", nd->getName(), nd->getEmail(), nd->getBeautifiedFingerprint());

			signList.append(n->toSignableNode());
		}

		if (KMessageBox::Continue != KMessageBox::warningContinueCancelList(this,
				i18n("<qt>You are about to sign the following keys in one pass.<br/><b>If you have not carefully checked all fingerprints,"
						" the security of your communications may be compromised.</b></qt>"),
				signKeyList))
			return;
	}

	QPointer<KgpgSelectSecretKey> opts = new KgpgSelectSecretKey(this, imodel, signList.count());
	if (opts->exec() != QDialog::Accepted) {
		delete opts;
		signList.clear();
		return;
	}

	globalkeyID = QString(opts->getKeyID());
	const bool localsign = opts->isLocalSign();
	const int checklevel = opts->getSignTrust();
	bool isterminal = opts->isTerminalSign();
	delete opts;

	if (isterminal) {
		const QString keyid(signList.at(0)->getId());
		signList.clear();
		signKeyOpenConsole(globalkeyID, keyid, checklevel, localsign);
	} else {
		keyCount = 0;
		m_signuids = false;
		signLoop(localsign, checklevel);
	}
}

void KeysManager::signuid()
{
	// another sign operation is still running
	if (!signList.isEmpty())
		return;

	KgpgItemType tp;
	const auto tmplist = iview->selectedNodes(nullptr, &tp);
	if (tmplist.empty())
		return;

	if (tp & ~(ITYPE_PAIR | ITYPE_UID | ITYPE_UAT)) {
		KMessageBox::error(this, i18n("You can only sign user ids and photo ids. Please check your selection."));
		return;
	}

	if (tmplist.size() == 1) {
		KGpgSignableNode *nd = tmplist.at(0)->toSignableNode();
		KGpgKeyNode *pnd;
		if (tp & ITYPE_PUBLIC)
			pnd = nd->toKeyNode();
		else
			pnd = nd->getParentKeyNode()->toKeyNode();
		QString opt;

		if (nd->getEmail().isEmpty())
			opt = i18n("<qt>You are about to sign user id:<br /><br />%1<br />ID: %2<br />Fingerprint: <br /><b>%3</b>.<br /><br />"
			"You should check the key fingerprint by phoning or meeting the key owner to be sure that someone "
			"is not trying to intercept your communications.</qt>", nd->getName(), nd->getId(), pnd->getBeautifiedFingerprint());
		else
			opt = i18n("<qt>You are about to sign user id:<br /><br />%1 (%2)<br />ID: %3<br />Fingerprint: <br /><b>%4</b>.<br /><br />"
			"You should check the key fingerprint by phoning or meeting the key owner to be sure that someone "
			"is not trying to intercept your communications.</qt>", nd->getName(), nd->getEmail(), nd->getId(), pnd->getBeautifiedFingerprint());

		if (KMessageBox::warningContinueCancel(this, opt) != KMessageBox::Continue) {
			return;
		}
		signList.append(nd);
	} else {
		QStringList signKeyList;

		for (KGpgNode *nd : tmplist) {
			const KGpgKeyNode *pnd = (nd->getType() & (ITYPE_UID | ITYPE_UAT)) ?
					nd->getParentKeyNode()->toKeyNode() : nd->toKeyNode();

			if (nd->getEmail().isEmpty())
				signKeyList += i18nc("Name: ID", "%1: %2",
					nd->getName(), pnd->getBeautifiedFingerprint());
			else
				signKeyList += i18nc("Name (Email): ID", "%1 (%2): %3",
					nd->getName(), nd->getEmail(), pnd->getBeautifiedFingerprint());

			signList.append(nd->toSignableNode());
		}

		if (KMessageBox::warningContinueCancelList(this,
				i18n("<qt>You are about to sign the following user ids in one pass.<br/><b>If you have not carefully checked all fingerprints,"
						" the security of your communications may be compromised.</b></qt>"),
				signKeyList) != KMessageBox::Continue)
			return;
	}

	QPointer<KgpgSelectSecretKey> opts = new KgpgSelectSecretKey(this, imodel, signList.count());
	if (opts->exec() != QDialog::Accepted) {
		delete opts;
		signList.clear();
		return;
	}

	globalkeyID = QString(opts->getKeyID());
	const bool localsign = opts->isLocalSign();
	const int checklevel = opts->getSignTrust();
	bool isterminal = opts->isTerminalSign();
	delete opts;

	if (isterminal) {
		const QString keyid(signList.at(0)->getId());
		signList.clear();
		signKeyOpenConsole(globalkeyID, keyid, checklevel, localsign);
	} else {
		keyCount = 0;
		m_signuids = true;
		signLoop(localsign, checklevel);
	}
}

void KeysManager::signLoop(const bool localsign, const int checklevel)
{
	Q_ASSERT(keyCount < signList.count());

	KGpgSignableNode *nd = signList.at(keyCount);
	const KGpgSignTransactionHelper::carefulCheck cc = static_cast<KGpgSignTransactionHelper::carefulCheck>(checklevel);
	KGpgTransaction *sta;

	if (m_signuids) {
		sta = new KGpgSignUid(this, globalkeyID, nd, localsign, cc);
	} else {
		sta = new KGpgSignKey(this, globalkeyID, nd->toKeyNode(), localsign, cc);
	}

	connect(sta, &KGpgTransaction::done, this, &KeysManager::signatureResult);
	sta->start();
}

void KeysManager::signatureResult(int success)
{
	KGpgSignTransactionHelper *ta;
	KGpgSignUid *suid = qobject_cast<KGpgSignUid *>(sender());
	if (suid != nullptr) {
		ta = static_cast<KGpgSignTransactionHelper *>(suid);
	} else {
		ta = static_cast<KGpgSignTransactionHelper *>(static_cast<KGpgSignKey *>(sender()));
	}
	KGpgKeyNode *nd = const_cast<KGpgKeyNode *>(ta->getKey());
	const bool localsign = ta->getLocal();
	const int checklevel = ta->getChecking();
	const QString signer(ta->getSigner());
	sender()->deleteLater();

	switch (success) {
	case KGpgTransaction::TS_OK:
		if (refreshList.indexOf(nd) == -1)
			refreshList.append(nd);
		break;
	case KGpgTransaction::TS_BAD_PASSPHRASE:
		KMessageBox::error(this, i18n("<qt>Bad passphrase, key <b>%1 (%2)</b> not signed.</qt>",
				nd->getName(), nd->getEmail()));
		break;
	case KGpgSignTransactionHelper::TS_ALREADY_SIGNED:
		KMessageBox::error(this, i18n("<qt>The key <b>%1 (%2)</b> is already signed.</qt>",
				nd->getName(), nd->getEmail()));
		break;
	default:
		if (KMessageBox::questionYesNo(this,
				i18n("<qt>Signing key <b>%1</b> with key <b>%2</b> failed.<br />"
						"Do you want to try signing the key in console mode?</qt>",
						nd->getId(), signer)) == KMessageBox::Yes)
			signKeyOpenConsole(signer, nd->getId(), checklevel, localsign);
	}

	if (++keyCount == signList.count()) {
		signList.clear();
		if (!refreshList.isEmpty())
			imodel->refreshKeys(refreshList);
		refreshList.clear();
	} else {
		signLoop(localsign, checklevel);
	}
}

void KeysManager::caff()
{
	KgpgItemType tp;
	const auto tmplist = iview->selectedNodes(nullptr, &tp);
	KGpgSignableNode::List slist;
	if (tmplist.empty())
		return;

	if (tp & ~(ITYPE_PAIR | ITYPE_UID | ITYPE_UAT)) {
		KMessageBox::error(this, i18n("You can only sign user ids and photo ids. Please check your selection."));
		return;
	}

	for (KGpgNode *nd : tmplist) {
		switch (nd->getType()) {
		case KgpgCore::ITYPE_PAIR:
		case KgpgCore::ITYPE_PUBLIC: {
			KGpgKeyNode *knd = qobject_cast<KGpgKeyNode *>(nd);
			if (!knd->wasExpanded())
				knd->getChildCount();
			}
		}
		slist.append(nd->toSignableNode());
	}

	QPointer<KgpgSelectSecretKey> opts = new KgpgSelectSecretKey(this, imodel, slist.count(), false, false);
	if (opts->exec() != QDialog::Accepted) {
		delete opts;
		return;
	}

	KGpgCaff *ca = new KGpgCaff(this, slist, QStringList(opts->getKeyID()), opts->getSignTrust(), KGpgCaff::IgnoreAlreadySigned);
	delete opts;

	connect(ca, &KGpgCaff::done, this, &KeysManager::slotCaffDone);
	connect(ca, &KGpgCaff::aborted, this, &KeysManager::slotCaffDone);

	ca->run();
}

void KeysManager::slotCaffDone()
{
	Q_ASSERT(qobject_cast<KGpgCaff *>(sender()) != nullptr);

	sender()->deleteLater();
}

void KeysManager::signKeyOpenConsole(const QString &signer, const QString &keyid, const int checking, const bool local)
{
	KConfigGroup config(KSharedConfig::openConfig(), "General");

	KProcess process;
	process << config.readPathEntry("TerminalApplication", QLatin1String("konsole"))
			<< QLatin1String("-e")
			<< KGpgSettings::gpgBinaryPath()
			<< QLatin1String("--no-secmem-warning")
			<< QLatin1String("-u") << signer
			<< QLatin1String("--default-cert-level")
			<< QString(checking);

	if (!local)
		process << QLatin1String( "--sign-key" ) << keyid;
	else
		process << QLatin1String( "--lsign-key" ) << keyid;

	process.execute();
}

void KeysManager::getMissingSigs(QSet<QString> &missingKeys, const KGpgExpandableNode *nd)
{
	for (const KGpgNode *ch : nd->getChildren()) {
		if (ch->hasChildren()) {
			getMissingSigs(missingKeys, ch->toExpandableNode());
			continue;
		} else if (ch->getType() == ITYPE_SIGN) {
			if (ch->toSignNode()->isUnknown())
				missingKeys << ch->getId();
		}
	}
}

void KeysManager::importallsignkey()
{
	const auto sel = iview->selectedNodes();
	QSet<QString> missingKeys;

	if (sel.empty())
		return;

	for (const KGpgNode *nd : sel) {
		if (nd->hasChildren()) {
			getMissingSigs(missingKeys, nd->toExpandableNode());
		} else if (nd->getType() == ITYPE_SIGN) {
			const KGpgSignNode *sn = nd->toSignNode();

			if (sn->isUnknown())
				missingKeys << sn->getId();
		}
	}

	if (missingKeys.isEmpty()) {
		KMessageBox::information(this,
			i18np("All signatures for this key are already in your keyring",
			"All signatures for this keys are already in your keyring", sel.size()));
		return;
	}

	importRemoteKeys(missingKeys.values());
}

void KeysManager::preimportsignkey()
{
	const auto exportList = iview->selectedNodes();
	QStringList idlist;

	if (exportList.empty())
		return;

	for (const KGpgNode *nd : exportList)
		idlist << nd->getId();

	importRemoteKeys(idlist);
}

bool KeysManager::importRemoteKey(const QString &keyIDs)
{
	return importRemoteKeys(keyIDs.simplified().split(QLatin1Char( ' ' )), false);
}

bool KeysManager::importRemoteKeys(const QStringList &keyIDs, const bool dialog)
{
	const QStringList kservers = KeyServer::getServerList();
	if (kservers.isEmpty())
		return false;

	KGpgReceiveKeys *proc = new KGpgReceiveKeys(this, kservers.first(), keyIDs, dialog, QLatin1String( qgetenv("http_proxy") ));
	connect(proc, &KGpgReceiveKeys::done, this, &KeysManager::importRemoteFinished);

	proc->start();

	return true;
}

void KeysManager::importRemoteFinished(int result)
{
	KGpgReceiveKeys *t = qobject_cast<KGpgReceiveKeys *>(sender());
	Q_ASSERT(t != nullptr);

	const QStringList keys = KGpgImport::getImportedIds(t->getLog());

	t->deleteLater();

	if ((result == KGpgTransaction::TS_OK) && !keys.isEmpty())
		imodel->refreshKeys(keys);
}

void KeysManager::delsignkey()
{
	KGpgNode *nd = iview->selectedNode();
	if (nd == nullptr)
		return;

	QString uid;
	QString parentKey;

	KGpgExpandableNode *parent = nd->getParentKeyNode();
	switch (parent->getType()) {
	case ITYPE_PAIR:
	case ITYPE_PUBLIC:
		uid = QLatin1Char( '1' );
		parentKey = parent->getId();
		break;
	case ITYPE_UID:
	case ITYPE_UAT:
		uid = parent->getId();
		parentKey = parent->getParentKeyNode()->getId();
		break;
	default:
		Q_ASSERT(0);
		return;
	}

	const QString signID(nd->getId());
	QString signMail(nd->getNameComment());
	QString parentMail(parent->getNameComment());

	if (!parent->getEmail().isEmpty())
		parentMail += QLatin1String( " &lt;" ) + parent->getEmail() + QLatin1String( "&gt;" );
	if (!nd->getEmail().isEmpty())
		signMail += QLatin1String( " &lt;" ) + nd->getEmail() + QLatin1String( "&gt;" );

	if (parentKey == signID) {
		KMessageBox::error(this, i18n("Edit key manually to delete a self-signature."));
		return;
	}

	QString ask = i18n("<qt>Are you sure you want to delete signature<br /><b>%1</b><br />from user id <b>%2</b><br />of key: <b>%3</b>?</qt>",
			signMail, parentMail, parentKey);

	if (KMessageBox::questionYesNo(this, ask, QString(), KStandardGuiItem::del(), KStandardGuiItem::cancel()) != KMessageBox::Yes)
		return;

	KGpgDelSign *delsig = new KGpgDelSign(this, nd->toSignNode());
	connect(delsig, &KGpgDelSign::done, this, &KeysManager::delsignatureResult);
	delsig->start();
}

void KeysManager::delsignatureResult(int success)
{
	sender()->deleteLater();

	if (success == KGpgTransaction::TS_OK) {
		KGpgNode *nd = iview->selectedNode()->getParentKeyNode();

		while (!(nd->getType() & ITYPE_PAIR))
			nd = nd->getParentKeyNode();
		imodel->refreshKey(nd->toKeyNode());
	} else {
		KMessageBox::error(this, i18n("Requested operation was unsuccessful, please edit the key manually."));
	}
}

void KeysManager::slotSendEmail()
{
	QStringList maillist;

	const auto nodes = iview->selectedNodes();
	for (const KGpgNode *nd : nodes) {
		if (nd->getEmail().isEmpty())
			continue;

		maillist << QLatin1Char('"') + nd->getName() + QLatin1String("\" <") + nd->getEmail() + QLatin1Char('>');
	}

	if (maillist.isEmpty())
		return;

	QDesktopServices::openUrl(QUrl(QLatin1String("mailto:") + maillist.join(QLatin1String(", "))));
}

void KeysManager::slotedit()
{
	KGpgNode *nd = iview->selectedNode();
	Q_ASSERT(nd != nullptr);

	if (!(nd->getType() & ITYPE_PAIR))
		return;
	if (terminalkey)
		return;
	if ((m_delkey != nullptr) && m_delkey->keys.contains(nd->toKeyNode()))
		return;

	KProcess *kp = new KProcess(this);
	KConfigGroup config(KSharedConfig::openConfig(), "General");
	*kp << config.readPathEntry("TerminalApplication", QLatin1String("konsole"))
			<< QLatin1String("-e")
			<< KGpgSettings::gpgBinaryPath()
			<< QLatin1String("--no-secmem-warning")
			<< QLatin1String("--edit-key")
			<< nd->getId()
			<< QLatin1String("help");
	terminalkey = nd->toKeyNode();
	editKey->setEnabled(false);

	connect(kp, QOverload<int>::of(&KProcess::finished), this, &KeysManager::slotEditDone);
	kp->start();
}

void KeysManager::slotEditDone(int exitcode)
{
	if (exitcode == 0)
		imodel->refreshKey(terminalkey);

	terminalkey = nullptr;
	editKey->setEnabled(true);
}

void KeysManager::doPrint(const QString &txt)
{
	QPrinter prt;
	//qCDebug(KGPG_LOG_GENERAL) << "Printing..." ;
	QPointer<QPrintDialog> printDialog = new QPrintDialog(&prt, this);
	if (printDialog->exec() == QDialog::Accepted) {
		QPainter painter(&prt);
		int width = painter.device()->width();
		int height = painter.device()->height();
		painter.drawText(0, 0, width, height, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, txt);
	}
	delete printDialog;
}

void KeysManager::removeFromGroups(KGpgKeyNode *node)
{
	QStringList groupNames;

	const auto groups = node->getGroups();
	for (const KGpgGroupNode *gnd : groups)
		groupNames << gnd->getName();

	if (groupNames.isEmpty())
		return;

	const QString ask = i18np("<qt>The key you are deleting is a member of the following key group. Do you want to remove it from this group?</qt>",
			"<qt>The key you are deleting is a member of the following key groups. Do you want to remove it from these groups?</qt>",
			groupNames.count());

	if (KMessageBox::questionYesNoList(this, ask, groupNames, i18n("Delete key")) != KMessageBox::Yes)
		return;

	bool groupDeleted = false;

	const auto grefs = node->getGroupRefs();
	for (KGpgGroupMemberNode *gref : grefs) {
		KGpgGroupNode *group = gref->getParentKeyNode();

		bool deleteWholeGroup = (group->getChildCount() == 1) &&
				(group->getChild(0)->toGroupMemberNode() == gref);
		if (deleteWholeGroup)
			deleteWholeGroup = (KMessageBox::questionYesNo(this,
					i18n("You are removing the last key from key group %1.<br/>Do you want to delete the group, too?", group->getName()),
					i18n("Delete key")) == KMessageBox::Yes);

		if (!deleteWholeGroup) {
			imodel->deleteFromGroup(group, gref);
		} else {
			group->remove();
			imodel->delNode(group);
			groupDeleted = true;
		}
	}

	if (groupDeleted) {
		updateStatusCounter();
	}
}

void KeysManager::deleteseckey()
{
	KGpgKeyNode *nd = iview->selectedNode()->toKeyNode();
	Q_ASSERT(nd != nullptr);

	// delete a key
	int result = KMessageBox::warningContinueCancel(this,
			i18n("<p>Delete <b>secret</b> key pair <b>%1</b>?</p>Deleting this key pair means you will never be able to decrypt files encrypted with this key again.",
					nd->getNameComment()),
			QString(),
			KGuiItem(i18n("Delete"), QLatin1String( "edit-delete" )));
	if (result != KMessageBox::Continue)
		return;

	if (terminalkey == nd)
		return;
	if (m_delkey != nullptr) {
		KMessageBox::error(this,
				i18n("Another key delete operation is still in progress.\nPlease wait a moment until this operation is complete."),
				i18n("Delete key"));
		return;
	}

	removeFromGroups(nd);

	m_delkey = new KGpgDelKey(this, nd);
	connect(m_delkey, &KGpgDelKey::done, this, &KeysManager::secretKeyDeleted);
	m_delkey->start();
}

void KeysManager::secretKeyDeleted(int retcode)
{
	KGpgKeyNode *delkey = m_delkey->keys.at(0);
	if (retcode == 0) {
		KMessageBox::information(this, i18n("Key <b>%1</b> deleted.", delkey->getBeautifiedFingerprint()), i18n("Delete key"));
		imodel->delNode(delkey);
	} else {
		KMessageBox::error(this, i18n("Deleting key <b>%1</b> failed.", delkey->getBeautifiedFingerprint()), i18n("Delete key"));
	}
	m_delkey->deleteLater();
	m_delkey = nullptr;
}

void KeysManager::confirmdeletekey()
{
	if (m_delkey) {
		KMessageBox::error(this,
				i18n("Another key delete operation is still in progress.\nPlease wait a moment until this operation is complete."),
				i18n("Delete key"));
		return;
	}

	KgpgCore::KgpgItemType pt;
	bool same;
	const auto ndlist = iview->selectedNodes(&same, &pt);
	if (ndlist.empty())
		return;

	// do not delete a key currently edited in terminal
	if ((!(pt & ~ITYPE_PAIR)) && (ndlist.at(0) == terminalkey) && (ndlist.size() == 1)) {
		KMessageBox::error(this,
				i18n("Can not delete key <b>%1</b> while it is edited in terminal.",
				terminalkey->getBeautifiedFingerprint()), i18n("Delete key"));
		return;
	} else if (pt == ITYPE_GROUP) {
		deleteGroup();
		return;
	} else if (!(pt & ITYPE_GROUP) && (pt & ITYPE_SECRET) && (ndlist.size() == 1)) {
		deleteseckey();
		return;
	} else if ((pt == ITYPE_UID) && (ndlist.size() == 1)) {
		slotDelUid();
		return;
	} else if ((pt & ITYPE_GROUP) && !(pt & ~ITYPE_GPAIR)) {
		bool invalidDelete = std::any_of(ndlist.cbegin(), ndlist.cend(),
				[](const KGpgNode *nd) { return nd->getType() == ITYPE_GROUP; });

		// only allow removing group members if they belong to the same group
		if (!invalidDelete) {
			const KGpgNode * const group = ndlist.front()->getParentKeyNode();
			invalidDelete = std::any_of(ndlist.cbegin(), ndlist.cend(),
					[group] (const KGpgNode *nd) { return nd->getParentKeyNode() != group; });
		}

		if (!invalidDelete) {
			KGpgGroupNode *gnd = ndlist.front()->getParentKeyNode()->toGroupNode();

			QList<KGpgNode *> members = gnd->getChildren();

			for (KGpgNode *nd : ndlist) {
				int r = members.removeAll(nd);
				Q_ASSERT(r == 1);
				Q_UNUSED(r);
			}

			imodel->changeGroup(gnd, members);
			return;
		}
	}

	if (pt & ~ITYPE_PAIR) {
		KMessageBox::error(this,
				i18n("You have selected items that are not keys. They can not be deleted with this menu entry."),
				i18n("Delete key"));
		return;
	}

	QStringList keysToDelete;
	QStringList deleteIds;
	QStringList secList;
	KGpgKeyNode::List delkeys;

	bool secretKeyInside = (pt & ITYPE_SECRET);
	for (KGpgNode *nd : ndlist) {
		KGpgKeyNode *ki = nd->toKeyNode();

		if (ki->getType() & ITYPE_SECRET) {
			secList += ki->getNameComment();
		} else if (ki != terminalkey) {
			keysToDelete += ki->getNameComment();
			deleteIds << ki->getId();
			delkeys << ki;
		}
	}

	if (secretKeyInside) {
		int result = KMessageBox::warningContinueCancel(this,
				i18n("<qt>The following are secret key pairs:<br/><b>%1</b><br/>They will not be deleted.</qt>",
						secList.join( QLatin1String( "<br />" ))));
		if (result != KMessageBox::Continue)
			return;
	}

	if (keysToDelete.isEmpty())
		return;

	int result = KMessageBox::warningContinueCancelList(this,
			i18np("<qt><b>Delete the following public key?</b></qt>",
					"<qt><b>Delete the following %1 public keys?</b></qt>",
					keysToDelete.count()), keysToDelete, QString(),
					KStandardGuiItem::del());
	if (result != KMessageBox::Continue)
		return;

	for (KGpgNode *nd : ndlist)
		removeFromGroups(nd->toKeyNode());

	m_delkey = new KGpgDelKey(this, delkeys);
	connect(m_delkey, &KGpgDelKey::done, this, &KeysManager::slotDelKeyDone);
	m_delkey->start();
}

void KeysManager::slotDelKeyDone(int res)
{
	if (res == 0) {
		for (KGpgKeyNode *kn : m_delkey->keys)
			imodel->delNode(kn);
	}

	m_delkey->deleteLater();
	m_delkey = nullptr;

	updateStatusCounter();
}

void KeysManager::slotPreImportKey()
{
	QPointer<QDialog> dial = new QDialog(this);
	dial->setWindowTitle(i18n("Key Import"));
	QVBoxLayout *mainLayout = new QVBoxLayout(dial);
	QWidget *mainWidget = new QWidget(this);
	mainLayout->addWidget(mainWidget);
	dial->setLayout(mainLayout);

	SrcSelect *page = new SrcSelect();
	mainLayout->addWidget(page);
	page->newFilename->setWindowTitle(i18n("Open File"));
	page->newFilename->setMode(KFile::File);

	page->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(page->buttonBox, &QDialogButtonBox::accepted, dial.data(), &QDialog::accept);
	connect(page->buttonBox, &QDialogButtonBox::rejected, dial.data(), &QDialog::reject);

	if (dial->exec() == QDialog::Accepted) {
		if (page->checkFile->isChecked()) {
			QUrl impname = page->newFilename->url();
			if (!impname.isEmpty())
				slotImport(QList<QUrl>({impname}));
		} else if (page->checkServer->isChecked()) {
			const QString ids(page->keyIds->text().simplified());
			if (!ids.isEmpty())
				importRemoteKeys(ids.split(QLatin1Char( ' ' )));
		} else {
			slotImport(qApp->clipboard()->text(m_clipboardmode));
		}
	}

	delete dial;
}

void KeysManager::slotImport(const QString &text)
{
	if (text.isEmpty())
		return;

	KGpgImport *imp;

	if (!KGpgImport::isKey(text) && KGpgDecrypt::isEncryptedText(text)) {
		if (KMessageBox::questionYesNo(this,
				i18n("<qt>The text in the clipboard does not look like a key, but like encrypted text.<br />Do you want to decrypt it first"
						" and then try importing it?</qt>"),
						i18n("Import from Clipboard")) != KMessageBox::Yes)
			return;

		imp = new KGpgImport(this);
		KGpgDecrypt *decr = new KGpgDecrypt(this, text);
		imp->setInputTransaction(decr);
	} else {
		imp = new KGpgImport(this, text);
	}

	startImport(imp);
}

void KeysManager::slotImport(const QList<QUrl> &files)
{
	startImport(new KGpgImport(this, files));
}

void KeysManager::startImport(KGpgImport *import)
{
	changeMessage(i18n("Importing..."), true);
	connect(import, &KGpgImport::done, this, &KeysManager::slotImportDone);
	import->start();
}

void KeysManager::slotImportDone(int result)
{
	KGpgImport *import = qobject_cast<KGpgImport *>(sender());
	Q_ASSERT(import != nullptr);
	const QStringList rawmsgs(import->getMessages());

	if (result != 0) {
		KMessageBox::detailedError(this, i18n("Key importing failed. Please see the detailed log for more information."),
				rawmsgs.join(QLatin1String("\n")) , i18n("Key Import"));
	}

	QStringList keys(import->getImportedIds(0x1f));
	const bool needsRefresh = !keys.isEmpty();
	keys << import->getImportedIds(0);

	if (!keys.isEmpty()) {
		const QString msg(import->getImportMessage());
		const QStringList keynames(import->getImportedKeys());

		new KgpgDetailedInfo(this, msg, rawmsgs.join(QLatin1String("\n")), keynames, i18n("Key Import"));
		if (needsRefresh)
			imodel->refreshKeys(keys);
		else
			changeMessage(i18nc("Application ready for user input", "Ready"));
	} else{
		changeMessage(i18nc("Application ready for user input", "Ready"));
	}

	import->deleteLater();
}

void KeysManager::refreshkey()
{
	imodel->refreshAllKeys();
	updateStatusCounter();
}

KGpgItemModel *KeysManager::getModel()
{
	return imodel;
}

void
KeysManager::toggleNetworkActions(bool online)
{
	m_online = online;
	kserver->setEnabled(online);
	importSignatureKey->setEnabled(online);
	importAllSignKeys->setEnabled(online);
	refreshKey->setEnabled(online);
}

void
KeysManager::setupTrayIcon()
{
	bool newtray = (m_trayicon == nullptr);

	if (newtray) {
		m_trayicon = new KStatusNotifierItem(this);
		m_trayicon->setIconByName(QLatin1String( "kgpg" ));
		m_trayicon->setToolTip(QLatin1String( "kgpg" ), i18n("KGpg - encryption tool"), QString());
	}

	switch (KGpgSettings::leftClick()) {
	case KGpgSettings::EnumLeftClick::Editor:
		m_trayicon->setAssociatedWidget(s_kgpgEditor);
		break;
	case KGpgSettings::EnumLeftClick::KeyManager:
		m_trayicon->setAssociatedWidget(this);
		break;
	}

	m_trayicon->setCategory(KStatusNotifierItem::ApplicationStatus);

	if (!newtray)
		return;

	QMenu *conf_menu = m_trayicon->contextMenu();

	QAction *KgpgOpenManager = actionCollection()->addAction(QLatin1String("kgpg_manager"), this, &KeysManager::show);
	KgpgOpenManager->setIcon(QIcon::fromTheme( QLatin1String( "kgpg" )));
	KgpgOpenManager->setText(i18n("Ke&y Manager"));

	QAction *KgpgEncryptClipboard = actionCollection()->addAction(QLatin1String("clip_encrypt"), this, &KeysManager::clipEncrypt);
	KgpgEncryptClipboard->setText(i18n("&Encrypt Clipboard"));

	QAction *KgpgDecryptClipboard = actionCollection()->addAction(QLatin1String("clip_decrypt"), this, &KeysManager::clipDecrypt);
	KgpgDecryptClipboard->setText(i18n("&Decrypt Clipboard"));

	QAction *KgpgSignClipboard = actionCollection()->addAction(QLatin1String("clip_sign"), this, &KeysManager::clipSign);
	KgpgSignClipboard->setText(i18n("&Sign/Verify Clipboard"));
	KgpgSignClipboard->setIcon(QIcon::fromTheme( QLatin1String( "document-sign-key" )));

	QAction *KgpgPreferences = KStandardAction::preferences(this, &KeysManager::showOptions, actionCollection());

	conf_menu->addAction( KgpgEncryptClipboard );
	conf_menu->addAction( KgpgDecryptClipboard );
	conf_menu->addAction( KgpgSignClipboard );
	conf_menu->addAction( KgpgOpenManager );
	conf_menu->addAction( openEditor );
	conf_menu->addAction( kserver );
	conf_menu->addSeparator();
	conf_menu->addAction( KgpgPreferences );
}

void
KeysManager::showTrayMessage(const QString &message)
{
	if (m_trayicon == nullptr)
		return;

	m_trayicon->showMessage(QString(), message, QLatin1String( "kgpg" ));
}

QKeySequence
KeysManager::goDefaultShortcut() const
{
	return QKeySequence(goToDefaultKey->shortcut());
}

void
KeysManager::clipEncrypt()
{
	const QString cliptext(qApp->clipboard()->text(m_clipboardmode));

	if (cliptext.isEmpty()) {
		Q_ASSERT(m_trayicon != nullptr);
		m_trayicon->showMessage(QString(), i18n("Clipboard is empty."), QLatin1String( "kgpg" ));
		return;
	}

	QPointer<KgpgSelectPublicKeyDlg> dialog = new KgpgSelectPublicKeyDlg(this, imodel, QKeySequence(goToDefaultKey->shortcut()), true);
	if (dialog->exec() == QDialog::Accepted) {
		KGpgEncrypt::EncryptOptions encOptions = KGpgEncrypt::AsciiArmored;
		QStringList options;

		if (!dialog->getCustomOptions().isEmpty() && KGpgSettings::allowCustomEncryptionOptions())
			options = dialog->getCustomOptions().split(QLatin1Char(' '), Qt::SkipEmptyParts);

		if (dialog->getUntrusted())
			encOptions |= KGpgEncrypt::AllowUntrustedEncryption;
		if (dialog->getHideId())
			encOptions |= KGpgEncrypt::HideKeyId;

		if (KGpgSettings::pgpCompatibility())
			options.append(QLatin1String( "--pgp6" ));

		KGpgEncrypt *enc = new KGpgEncrypt(this, dialog->selectedKeys(), cliptext, encOptions, options);
		connect(enc, &KGpgEncrypt::done, this, &KeysManager::slotSetClip);

		m_trayicon->setStatus(KStatusNotifierItem::Active);
		enc->start();
	}

	delete dialog;
}

void
KeysManager::slotSetClip(int result)
{
	KGpgEncrypt *enc = qobject_cast<KGpgEncrypt *>(sender());
	Q_ASSERT(enc != nullptr);
	sender()->deleteLater();

	m_trayicon->setStatus(KStatusNotifierItem::Passive);

	if (result != KGpgTransaction::TS_OK)
		return;

	qApp->clipboard()->setText(enc->encryptedText().join(QLatin1String("\n")), m_clipboardmode);

	Q_ASSERT(m_trayicon != nullptr);
	m_trayicon->showMessage(QString(), i18n("Text successfully encrypted."), QLatin1String( "kgpg" ));
}

void
KeysManager::slotOpenKeyUrl()
{
	KGpgNode *cur = iview->selectedNode();
	if (cur == nullptr)
		return;

	QString id;

	switch (cur->getType()) {
	case ITYPE_PAIR:
	case ITYPE_PUBLIC: {
		id = cur->toKeyNode()->getFingerprint();
		break;
		}
	case ITYPE_GPAIR:
	case ITYPE_GPUBLIC: {
		id = cur->getId();
		break;
		}
	default:
		return;
	}

	const QStringList servers = KGpgSettings::infoServers();
	if (servers.isEmpty())
		return;

	QString url = servers.first();
	const QString idUC = id.toUpper();
	const QString idLC = id.toLower();

	url.replace(QLatin1String("$$ID8$$"), idUC.right(8));
	url.replace(QLatin1String("$$ID16$$"), idUC.right(16));
	url.replace(QLatin1String("$$FPR$$"), idUC);
	url.replace(QLatin1String("$$id8$$"), idLC.right(8));
	url.replace(QLatin1String("$$id16$$"), idLC.right(16));
	url.replace(QLatin1String("$$fpr$$"), idLC);

	auto *job = new KIO::OpenUrlJob(QUrl(url));
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
	job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
#else
	job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
#endif
	job->start();
}

void
KeysManager::clipDecrypt()
{
	const QString cliptext(qApp->clipboard()->text(m_clipboardmode).trimmed());

	if (cliptext.isEmpty()) {
		Q_ASSERT(m_trayicon != nullptr);
		m_trayicon->showMessage(QString(), i18n("Clipboard is empty."), QLatin1String( "kgpg" ));
		return;
	}

	KgpgEditor *kgpgtxtedit = new KgpgEditor(this, imodel,  {});
	kgpgtxtedit->setAttribute(Qt::WA_DeleteOnClose);
	connect(this, &KeysManager::fontChanged, kgpgtxtedit, &KgpgEditor::slotSetFont);
	kgpgtxtedit->m_editor->setPlainText(cliptext);
	kgpgtxtedit->m_editor->slotDecode();
	kgpgtxtedit->show();
}

void
KeysManager::clipSign()
{
	QString cliptext = qApp->clipboard()->text(m_clipboardmode);
	if (cliptext.isEmpty()) {
		Q_ASSERT(m_trayicon != nullptr);
		m_trayicon->showMessage(QString(), i18n("Clipboard is empty."), QLatin1String( "kgpg" ));
		return;
	}

        KgpgEditor *kgpgtxtedit = new KgpgEditor(this, imodel, {});
	kgpgtxtedit->setAttribute(Qt::WA_DeleteOnClose);
	connect(kgpgtxtedit->m_editor, &KgpgTextEdit::verifyFinished, kgpgtxtedit, &KgpgEditor::closeWindow);

	kgpgtxtedit->m_editor->signVerifyText(cliptext);
	kgpgtxtedit->show();
}
