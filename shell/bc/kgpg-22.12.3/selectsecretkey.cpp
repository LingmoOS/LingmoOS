/*

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "selectsecretkey.h"

#include "kgpgsettings.h"
#include "core/images.h"
#include "core/KGpgRootNode.h"
#include "model/kgpgitemmodel.h"
#include "model/selectkeyproxymodel.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

using namespace KgpgCore;

KgpgSelectSecretKey::KgpgSelectSecretKey(QWidget *parent, KGpgItemModel *model, const int countkey, const bool allowLocal, const bool allowTerminal)
	: QDialog(parent),
	m_localsign(nullptr),
	m_terminalsign(nullptr),
	m_signtrust(nullptr),
	m_proxy(new SelectSecretKeyProxyModel(this))
{
	setWindowTitle(i18n("Private Key List"));
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	QWidget *mainWidget = new QWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(mainWidget);
	m_okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_okButton->setDefault(true);
	m_okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &KgpgSelectSecretKey::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KgpgSelectSecretKey::reject);
	m_okButton->setDefault(true);
	QWidget *page = new QWidget(this);

	QLabel *label = new QLabel(i18n("Choose secret key for signing:"), page);

	m_proxy->setKeyModel(model);

	m_keyslist = new QTableView(page);
	m_keyslist->setModel(m_proxy);
	m_keyslist->setSortingEnabled(true);
	m_keyslist->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_keyslist->resizeColumnsToContents();

	QVBoxLayout *vbox = new QVBoxLayout(page);
	vbox->addWidget(label);
	vbox->addWidget(m_keyslist);

	if (countkey > 0) {
		QLabel *signchecklabel = new QLabel(i18np("How carefully have you checked that the key really "
						"belongs to the person with whom you wish to communicate:",
						"How carefully have you checked that the %1 keys really "
						"belong to the people with whom you wish to communicate:", countkey), page);
		signchecklabel->setWordWrap(true);

		m_signtrust = new QComboBox(page);
		m_signtrust->addItem(i18n("I Will Not Answer"));
		m_signtrust->addItem(i18n("I Have Not Checked at All"));
		m_signtrust->addItem(i18n("I Have Done Casual Checking"));
		m_signtrust->addItem(i18n("I Have Done Very Careful Checking"));

		vbox->addWidget(signchecklabel);
		vbox->addWidget(m_signtrust);
		if (allowLocal){
			m_localsign = new QCheckBox(i18n("Local signature (cannot be exported)"), page);
			vbox->addWidget(m_localsign);
		}
		if (allowTerminal && (countkey == 1)) {
			m_terminalsign = new QCheckBox(i18n("Do not sign all user id's (open terminal)"), page);
			vbox->addWidget(m_terminalsign);
		}
	}

	KGpgNode *nd = model->getRootNode()->findKey(KGpgSettings::defaultKey());
	if (nd != nullptr) {
		QModelIndex sidx = model->nodeIndex(nd);
		QModelIndex pidx = m_proxy->mapFromSource(sidx);
		m_keyslist->selectionModel()->setCurrentIndex(pidx, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
	}

	setMinimumSize(550, 200);
	slotSelectionChanged();
	mainLayout->addWidget(page);
	mainLayout->addWidget(buttonBox);

	connect(m_keyslist->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KgpgSelectSecretKey::slotSelectionChanged);
	connect(m_keyslist, &QTableView::doubleClicked, this, &KgpgSelectSecretKey::slotOk);
}

KgpgSelectSecretKey::~KgpgSelectSecretKey()
{
}

QString KgpgSelectSecretKey::getKeyID() const
{
    if (!m_keyslist->selectionModel()->hasSelection())
        return QString();
    return m_proxy->nodeForIndex(m_keyslist->selectionModel()->selectedIndexes().at(0))->getId();
}

QString KgpgSelectSecretKey::getKeyMail() const
{
    if (!m_keyslist->selectionModel()->hasSelection())
        return QString();
    return m_proxy->nodeForIndex(m_keyslist->selectionModel()->selectedIndexes().at(0))->getEmail();
}

int KgpgSelectSecretKey::getSignTrust() const
{
    if (m_signtrust)
        return m_signtrust->currentIndex();
    return -1;
}

bool KgpgSelectSecretKey::isLocalSign() const
{
    return m_localsign && m_localsign->isChecked();
}

bool KgpgSelectSecretKey::isTerminalSign() const
{
    return m_terminalsign && m_terminalsign->isChecked();
}

void KgpgSelectSecretKey::slotSelectionChanged()
{
    m_okButton->setEnabled(m_keyslist->selectionModel()->hasSelection());
}

void KgpgSelectSecretKey::slotOk()
{
    if (m_keyslist->selectionModel()->hasSelection())
        accept();
}
