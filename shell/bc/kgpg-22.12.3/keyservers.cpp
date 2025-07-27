/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2006, 2007, 2008, 2009, 2010, 2012, 2013, 2014, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyservers.h"

#include "core/convert.h"
#include "detailedconsole.h"
#include "gpgproc.h"
#include "kgpgsettings.h"
#include "model/keylistproxymodel.h"
#include "model/kgpgitemmodel.h"
#include "transactions/kgpgimport.h"
#include "transactions/kgpgkeyservergettransaction.h"
#include "transactions/kgpgkeyserversearchtransaction.h"
#include "transactions/kgpgsendkeys.h"

#include <algorithm>

#include <KLocalizedString>
#include <KMessageBox>
#include <QCursor>
#include <QRegularExpression>

KeyServer::KeyServer(QWidget *parent, KGpgItemModel *model, const bool autoclose)
	: QDialog(parent),
	m_dialogserver(nullptr),
	m_searchproc(nullptr),
	page(new keyServerWidget()),
	m_listpop(nullptr),
	m_itemmodel(new KeyListProxyModel(this, KeyListProxyModel::SingleColumnIdFirst))
{
	setWindowTitle(i18n("Key Server"));

	m_autoclose = autoclose;
	m_resultmodel.setSortCaseSensitivity(Qt::CaseInsensitive);
	m_resultmodel.setDynamicSortFilter(true);
	m_resultmodel.setFilterKeyColumn(0);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(page);

	const QStringList serverlist(getServerList());
	page->kCBexportks->addItems(serverlist);
	page->kCBimportks->addItems(serverlist);
	page->qLEimportid->setFocus();
	page->Buttonsearch->setDefault(true);

	connect(page->buttonBox, &QDialogButtonBox::rejected, this, &KeyServer::accept);
	connect(page->Buttonimport, &QPushButton::clicked, this, &KeyServer::slotImport);
	connect(page->Buttonsearch, &QPushButton::clicked, this, &KeyServer::slotSearch);
	connect(page->Buttonexport, &QPushButton::clicked, this, &KeyServer::slotPreExport);
	connect(page->buttonBox, &QDialogButtonBox::accepted, this, &KeyServer::slotOk);
	connect(page->cBproxyI, &QCheckBox::toggled, this, &KeyServer::slotEnableProxyI);
	connect(page->cBproxyE, &QCheckBox::toggled, this, &KeyServer::slotEnableProxyE);
	connect(page->qLEimportid, &QLineEdit::textChanged, this, &KeyServer::slotTextChanged);

	page->cBproxyI->setChecked(KGpgSettings::useProxy());
	page->cBproxyE->setChecked(KGpgSettings::useProxy());

	const QString httpproxy(QLatin1String( qgetenv("http_proxy") ));
	if (!httpproxy.isEmpty()) {
		page->cBproxyI->setEnabled(true);
		page->cBproxyE->setEnabled(true);
		page->kLEproxyI->setText(httpproxy);
		page->kLEproxyE->setText(httpproxy);
	}

	page->Buttonimport->setEnabled(!page->qLEimportid->text().isEmpty());
	page->Buttonsearch->setEnabled(!page->qLEimportid->text().isEmpty());
	setMinimumSize(sizeHint());

	m_itemmodel->setKeyModel(model);
	m_itemmodel->setTrustFilter(KgpgCore::TRUST_UNDEFINED);
	page->kCBexportkey->setModel(m_itemmodel);
}

KeyServer::~KeyServer()
{
	delete page;
}

void KeyServer::slotImport()
{
	if (page->kCBimportks->currentText().isEmpty())
		return;

	if (page->qLEimportid->text().isEmpty()) {
		KMessageBox::error(this, i18n("You must enter a search string."));
		return;
	}

	startImport(page->qLEimportid->text().simplified().split(QLatin1Char( ' ' )), page->kCBimportks->currentText(), page->kLEproxyI->text());
}

void KeyServer::startImport(const QStringList &keys, QString server, const QString &proxy)
{
	if (server.isEmpty()) {
		const QStringList kservers = KeyServer::getServerList();
		if (kservers.isEmpty()) {
			KMessageBox::error(this, i18n("You need to configure keyservers before trying to download keys."),
					i18n("No keyservers defined"));
			return;
		}

		server = kservers.first();
	}

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

	KGpgReceiveKeys *proc = new KGpgReceiveKeys(this, server, keys, true, proxy);
	connect(proc, &KGpgReceiveKeys::done, this, &KeyServer::slotDownloadKeysFinished);

	proc->start();
}

void KeyServer::slotDownloadKeysFinished(int resultcode)
{
	QApplication::restoreOverrideCursor();

	KGpgKeyserverGetTransaction *t = qobject_cast<KGpgKeyserverGetTransaction *>(sender());
	const QStringList log(t->getLog());

	t->deleteLater();

	if (resultcode == KGpgTransaction::TS_USER_ABORTED) {
		Q_EMIT importFailed();
		return;
	}

	const QStringList keys(KGpgImport::getImportedIds(log));
	const QString resultmessage(KGpgImport::getImportMessage(log));

	if (!keys.empty())
		Q_EMIT importFinished(keys);

	(void) new KgpgDetailedInfo(this, resultmessage, log.join(QLatin1String("\n")),
			KGpgImport::getDetailedImportMessage(log).split(QLatin1Char( '\n' )),
			i18nc("Caption of message box", "Key Import Finished"));
}

void KeyServer::slotExport(const QStringList &keyIds)
{
	if (page->kCBexportks->currentText().isEmpty())
		return;

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

	KGpgSendKeys *nk = new KGpgSendKeys(this, page->kCBexportks->currentText(), keyIds, expattr, true, page->kLEproxyI->text());
	connect(nk, &KGpgSendKeys::done, this, &KeyServer::slotUploadKeysFinished);

	nk->start();
}

void KeyServer::slotUploadKeysFinished(int resultcode)
{
	KGpgSendKeys *nk = qobject_cast<KGpgSendKeys *>(sender());
	Q_ASSERT(nk != nullptr);

	const QStringList message(nk->getLog());
	nk->deleteLater();

	QApplication::restoreOverrideCursor();

	QString title;
	if (resultcode == KGpgTransaction::TS_OK)
		title = i18n("Upload to keyserver finished without errors");
	else
		title = i18n("Upload to keyserver failed");
	KMessageBox::informationList(this, title, message);
}

void KeyServer::slotSearch()
{
	if (page->kCBimportks->currentText().isEmpty())
		return;

	if (page->qLEimportid->text().isEmpty()) {
		KMessageBox::error(this, i18n("You must enter a search string."));
		return;
	}

	page->Buttonsearch->setEnabled(false);
	if (m_searchproc)
		return;

	m_resultmodel.resetSourceModel();
	m_resultmodel.setFilterRegularExpression(QRegularExpression());
	m_resultmodel.setFilterByValidity(true);

	m_dialogserver = new QDialog(this);
	m_dialogserver->setWindowTitle(i18nc("Window title", "Import Key From Keyserver"));

	QVBoxLayout *mainLayout = new QVBoxLayout(m_dialogserver);
	m_dialogserver->setLayout(mainLayout);

	m_listpop = new searchRes(m_dialogserver);
	m_listpop->buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("&Import"));
	m_listpop->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	m_listpop->kLVsearch->setModel(&m_resultmodel);
	m_listpop->kLVsearch->setColumnWidth(0, 180);
	m_listpop->validFilterCheckbox->setChecked(m_resultmodel.filterByValidity());
	m_listpop->statusText->setText(i18n("Connecting to the server..."));

	connect(m_listpop->filterEdit, &QLineEdit::textChanged, this, &KeyServer::slotSetFilterString);
	connect(m_listpop->kLVsearch->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KeyServer::transferKeyID);
	connect(m_listpop->validFilterCheckbox, &QCheckBox::toggled, &m_resultmodel, &KGpgSearchResultModel::setFilterByValidity);
	connect(m_listpop->validFilterCheckbox, &QCheckBox::toggled, this, &KeyServer::slotUpdateLabelOnFilterChange);
	connect(m_listpop->buttonBox, &QDialogButtonBox::accepted, this, &KeyServer::slotPreImport);
	connect(m_listpop->kLVsearch, &QTreeView::activated, m_dialogserver, &QDialog::accepted);
	connect(m_listpop->buttonBox, &QDialogButtonBox::rejected, this, &KeyServer::handleQuit);
	connect(m_listpop->qLEID, &QLineEdit::textChanged, this, [&] (const QString & text) {
		if (text.isEmpty())
			m_listpop->kLVsearch->selectionModel()->clearSelection();
		});

	m_listpop->kLVsearch->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_readmessage.clear();

	const QString keyserv(page->kCBimportks->currentText());

	bool useproxy = page->cBproxyI->isChecked();
	QString proxy;
	if (useproxy)
		proxy = page->kLEproxyI->text();

	m_searchproc = new KGpgKeyserverSearchTransaction(this, keyserv, page->qLEimportid->text().simplified(),
			true, proxy);
	connect(m_searchproc, &KGpgKeyserverSearchTransaction::done, this, &KeyServer::slotSearchResult);
	connect(m_searchproc, &KGpgKeyserverSearchTransaction::newKey, &m_resultmodel, &KGpgSearchResultModel::slotAddKey);
	m_searchproc->start();

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	mainLayout->addWidget(m_listpop);
	m_listpop->setMinimumSize(m_listpop->sizeHint());
	m_dialogserver->exec();
}

void KeyServer::slotSearchResult(int result)
{
	Q_ASSERT(sender() == m_searchproc);
	m_searchproc->deleteLater();
	m_searchproc = nullptr;
	page->Buttonsearch->setEnabled(true);
	QApplication::restoreOverrideCursor();

	if (result == KGpgTransaction::TS_USER_ABORTED) {
		delete m_dialogserver;
		m_dialogserver = nullptr;
		return;
	}

	m_listpop->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

	const int keys = m_resultmodel.sourceRowCount(QModelIndex());
	if (keys > 0) {
		m_listpop->kLVsearch->selectionModel()->setCurrentIndex(m_resultmodel.index(0, 0),
				QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
	}
	slotUpdateLabelOnFilterChange();
}

void KeyServer::slotSetText(const QString &text)
{
	page->qLEimportid->setText(text);
}

void KeyServer::slotTextChanged(const QString &text)
{
	page->Buttonimport->setEnabled(!text.isEmpty());
	page->Buttonsearch->setEnabled(!text.isEmpty() && (m_searchproc == nullptr));
}

void KeyServer::slotSetExportAttribute(const QString &state)
{
	if (!state.isEmpty())
		expattr = state;
	else
		expattr.clear();
}

void KeyServer::slotEnableProxyI(const bool on)
{
	page->kLEproxyI->setEnabled(on);
}

void KeyServer::slotEnableProxyE(const bool on)
{
	page->kLEproxyE->setEnabled(on);
}

void KeyServer::transferKeyID()
{
	QSet<QString> ids;

	const QModelIndexList indexes = m_listpop->kLVsearch->selectionModel()->selectedIndexes();
	for (const QModelIndex &index : indexes)
		ids << m_resultmodel.idForIndex(index);

	const QStringList idlist(ids.values());
	m_listpop->qLEID->setText(idlist.join( QLatin1String( " " )));
}

void KeyServer::slotPreImport()
{
	transferKeyID();
	if (m_listpop->qLEID->text().isEmpty()) {
		KMessageBox::error(this, i18n("You must choose a key."));
		return;
	}
	const QStringList keys = m_listpop->qLEID->text().simplified().split(QLatin1Char(' '));
	m_dialogserver->close();
	startImport(keys, page->kCBimportks->currentText(), page->kLEproxyI->text());
}

void KeyServer::slotPreExport()
{
	slotExport(QStringList(page->kCBexportkey->currentText().section(QLatin1Char( ':' ), 0, 0)));
}

void KeyServer::slotOk()
{
	accept();
}

QStringList KeyServer::getServerList()
{
	QStringList serverList(KGpgSettings::keyServers()); // From kgpg config
	if (!serverList.isEmpty()) {
		const QString defaultServer(serverList.takeFirst());
		std::sort(serverList.begin(), serverList.end());
		serverList.prepend(defaultServer);
	}

	return serverList;
}

void KeyServer::handleQuit()
{
	if (m_searchproc != nullptr) {
		QApplication::restoreOverrideCursor();
        disconnect(m_searchproc, nullptr, nullptr, nullptr);
		m_searchproc->deleteLater();
		m_searchproc = nullptr;
	}
	m_dialogserver->close();
	page->Buttonsearch->setEnabled(true);
}

void KeyServer::slotSetKeyserver(const QString &server)
{
	page->kCBimportks->setCurrentIndex(page->kCBimportks->findText(server));
}

void KeyServer::slotSetFilterString(const QString &expression)
{
	m_resultmodel.setFilterRegularExpression(QRegularExpression(expression, QRegularExpression::CaseInsensitiveOption));
	slotUpdateLabelOnFilterChange();
}

void KeyServer::slotUpdateLabelOnFilterChange()
{
	const int keys = m_resultmodel.sourceRowCount(QModelIndex());
	const int keysShown = m_resultmodel.rowCount(QModelIndex());
	Q_ASSERT(keysShown <= keys);

	if (keys == 0) {
		m_listpop->statusText->setText(i18n("No matching keys found"));
	} else {
		if (keysShown == keys) {
			m_listpop->statusText->setText(i18np("Found 1 matching key", "Found %1 matching keys", keys));
		} else {
			if (keys == 1 && keysShown == 0) {
				m_listpop->statusText->setText(i18n("Found 1 matching key (not shown)"));
			} else {
				m_listpop->statusText->setText(i18n("Found %1 matching keys (%2 shown)", keys, keysShown));
			}
		}
	}
}
