/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "selectpublickeydialog.h"

#include "kgpgsettings.h"
#include "core/images.h"
#include "core/KGpgRootNode.h"
#include "model/kgpgitemmodel.h"
#include "model/selectkeyproxymodel.h"

#include <QAction>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>

using namespace KgpgCore;

KgpgSelectPublicKeyDlg::KgpgSelectPublicKeyDlg(QWidget *parent, KGpgItemModel *model, const QKeySequence &goDefaultKey, const bool hideasciioption, const QList<QUrl> &files)
	: QDialog(parent),
	m_customoptions(nullptr),
	imodel(model),
	m_files(files),
	m_hideasciioption(hideasciioption)
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mainLayout->addWidget(mainWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setDefault(true);
    m_okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_detailsButton = new QPushButton(this);
    m_detailsButton->setText(i18n("&Options") + QStringLiteral(" >>"));
    m_detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(m_detailsButton, &QPushButton::clicked, this, &KgpgSelectPublicKeyDlg::toggleDetails);
    buttonBox->addButton(m_detailsButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KgpgSelectPublicKeyDlg::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KgpgSelectPublicKeyDlg::reject);
    m_okButton->setDefault(true);

    int fcount = files.count();
    bool fmode = (fcount > 0);

    switch (fcount) {
    case 0:
	setWindowTitle(i18n("Select Public Key"));
	break;
    case 1:
	setWindowTitle(i18n("Select Public Key for %1", files.first().fileName()));
	break;
    default:
	setWindowTitle(i18np("Select Public Key for %2 and one more file", "Select Public Key for %2 and %1 more files", files.count() - 1, files.first().fileName()));
    }

    QWidget *page = new QWidget(this);

    m_searchbar = new QWidget(page);
    QHBoxLayout *searchbarHBoxLayout = new QHBoxLayout(m_searchbar);
    searchbarHBoxLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *searchlabel = new QLabel(i18n("&Search: "), m_searchbar);
    searchbarHBoxLayout->addWidget(searchlabel);

    m_searchlineedit = new QLineEdit(m_searchbar);
    searchbarHBoxLayout->addWidget(m_searchlineedit);
    m_searchlineedit->setClearButtonEnabled(true);
    searchlabel->setBuddy(m_searchlineedit);

    iproxy = new SelectKeyProxyModel(this);
    iproxy->setKeyModel(imodel);

    m_keyslist = new QTableView(page);
    m_keyslist->setSortingEnabled(true);
    m_keyslist->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_keyslist->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_keyslist->setModel(iproxy);
    m_keyslist->resizeColumnsToContents();
    m_keyslist->setWhatsThis(i18n("<b>Public keys list</b>: select the key that will be used for encryption."));
    connect(m_searchlineedit, &QLineEdit::textChanged, iproxy, &SelectKeyProxyModel::setFilterFixedString);

    optionsbox = new QWidget();
    QVBoxLayout *optionsboxVBoxLayout = new QVBoxLayout(optionsbox);
    optionsboxVBoxLayout->setContentsMargins(0, 0, 0, 0);
    optionsbox->hide();

    if (m_hideasciioption)
        m_cbarmor = nullptr;
    else
    {
        m_cbarmor = new QCheckBox(i18n("ASCII armored encryption"), optionsbox);
        optionsboxVBoxLayout->addWidget(m_cbarmor);
        m_cbarmor->setChecked(KGpgSettings::asciiArmor());
        m_cbarmor->setWhatsThis(i18n("<b>ASCII encryption</b>: makes it possible to open the encrypted file/message in a text editor"));
    }

    m_cbuntrusted = new QCheckBox(i18n("Allow encryption with untrusted keys"), optionsbox);
    optionsboxVBoxLayout->addWidget(m_cbuntrusted);
    // connect before setting the state so the model gets the right state from the start
    connect(m_cbuntrusted, &QCheckBox::toggled, this, &KgpgSelectPublicKeyDlg::slotUntrusted);
    m_cbuntrusted->setChecked(KGpgSettings::allowUntrustedKeys());
    m_cbuntrusted->setWhatsThis(i18n("<b>Allow encryption with untrusted keys</b>: when you import a public key, it is usually "
                    "marked as untrusted and you cannot use it unless you sign it in order to make it 'trusted'. Checking this "
                    "box enables you to use any key, even if it has not be signed."));

    m_cbhideid = new QCheckBox(i18n("Hide user id"), optionsbox);
    optionsboxVBoxLayout->addWidget(m_cbhideid);
    m_cbhideid->setChecked(KGpgSettings::hideUserID());
    m_cbhideid->setWhatsThis(i18n("<b>Hide user ID</b>: Do not put the keyid into encrypted packets. This option hides the receiver "
                    "of the message and is a countermeasure against traffic analysis. It may slow down the decryption process because "
                    "all available secret keys are tried."));

    m_cbsymmetric = new QCheckBox(i18n("Symmetrical encryption"), optionsbox);
    optionsboxVBoxLayout->addWidget(m_cbsymmetric);
    m_cbsymmetric->setWhatsThis(i18n("<b>Symmetrical encryption</b>: encryption does not use keys. You just need to give a password "
                                     "to encrypt/decrypt the file"));

    QVBoxLayout *dialoglayout = new QVBoxLayout(page);
    dialoglayout->setContentsMargins(0, 0, 0, 0);
    dialoglayout->addWidget(m_searchbar);
    dialoglayout->addWidget(m_keyslist);
    page->setLayout(dialoglayout);

    if (KGpgSettings::allowCustomEncryptionOptions())
    {
        QWidget *expertbox = new QWidget(page);
        QHBoxLayout *expertboxHBoxLayout = new QHBoxLayout(expertbox);
        expertboxHBoxLayout->setContentsMargins(0, 0, 0, 0);
        (void) new QLabel(i18n("Custom option:"), expertbox);

        m_customoptions = new QLineEdit(expertbox);
        expertboxHBoxLayout->addWidget(m_customoptions);
        m_customoptions->setText(KGpgSettings::customEncryptionOptions());
        m_customoptions->setWhatsThis(i18n("<b>Custom option</b>: for experienced users only, allows you to enter a gpg command line option, like: '--armor'"));

        dialoglayout->addWidget(expertbox);
    }

    KActionCollection *actcol = new KActionCollection(this);
    QAction *action = actcol->addAction(QLatin1String( "go_default_key" ));
    action->setText(i18n("&Go to Default Key"));
    actcol->setDefaultShortcut(action, goDefaultKey);

    connect(action, &QAction::triggered, this, &KgpgSelectPublicKeyDlg::slotGotoDefaultKey);
    connect(m_cbsymmetric, &QCheckBox::toggled, this, &KgpgSelectPublicKeyDlg::slotSymmetric);
    connect(m_keyslist->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KgpgSelectPublicKeyDlg::slotSelectionChanged);
    connect(m_keyslist, &QTableView::doubleClicked, this, &KgpgSelectPublicKeyDlg::slotOk);

    setMinimumSize(550, 200);
    updateGeometry();

    mainLayout->addWidget(page);
    mainLayout->addWidget(optionsbox);
    mainLayout->addWidget(buttonBox);
    slotSelectionChanged();

    if (fmode)
        slotGotoDefaultKey();
}

QStringList KgpgSelectPublicKeyDlg::selectedKeys() const
{
	if (getSymmetric())
		return QStringList();

	QStringList selectedKeys;

	const QModelIndexList indexes = m_keyslist->selectionModel()->selectedIndexes();
	for (const QModelIndex &idx : indexes) {
		if (idx.column() != 0)
			continue;
		KGpgNode *nd = iproxy->nodeForIndex(idx);
		if (nd->getType() == ITYPE_GROUP)
			selectedKeys << nd->getName();
		else
			selectedKeys << nd->getId();
	}

	return selectedKeys;
}

bool KgpgSelectPublicKeyDlg::getSymmetric() const
{
    return m_cbsymmetric->isChecked();
}

QString KgpgSelectPublicKeyDlg::getCustomOptions() const
{
    if (m_customoptions == nullptr)
        return QString();
    return m_customoptions->text().simplified();
}

bool KgpgSelectPublicKeyDlg::getUntrusted() const
{
    return m_cbuntrusted->isChecked();
}

bool KgpgSelectPublicKeyDlg::getArmor() const
{
    return m_hideasciioption || m_cbarmor->isChecked();
}

const QList<QUrl> &KgpgSelectPublicKeyDlg::getFiles() const
{
	return m_files;
}

bool KgpgSelectPublicKeyDlg::getHideId() const
{
    return m_cbhideid->isChecked();
}

void KgpgSelectPublicKeyDlg::slotOk()
{
    if (getSymmetric() || m_keyslist->selectionModel()->hasSelection())
        accept();
}

void KgpgSelectPublicKeyDlg::slotSelectionChanged()
{
    m_okButton->setEnabled(getSymmetric() || m_keyslist->selectionModel()->hasSelection());
}

void KgpgSelectPublicKeyDlg::slotSymmetric(const bool state)
{
    m_keyslist->setDisabled(state);
    m_cbuntrusted->setDisabled(state);
    m_cbhideid->setDisabled(state);
    m_searchbar->setDisabled(state);
    slotSelectionChanged();
}

void KgpgSelectPublicKeyDlg::slotUntrusted(const bool state)
{
	iproxy->setShowUntrusted(state);
}

void KgpgSelectPublicKeyDlg::slotGotoDefaultKey()
{
	KGpgNode *nd = imodel->getRootNode()->findKey(KGpgSettings::defaultKey());
	if (nd == nullptr)
		return;
	QModelIndex sidx = imodel->nodeIndex(nd);
	QModelIndex pidx = iproxy->mapFromSource(sidx);
	m_keyslist->selectionModel()->setCurrentIndex(pidx, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void KgpgSelectPublicKeyDlg::toggleDetails()
{
    const bool isVisible = optionsbox->isVisible();
    optionsbox->setVisible(!isVisible);
    m_detailsButton->setText(i18n("&Options") + (isVisible ? QStringLiteral(" >>") : QStringLiteral(" <<")));
}
