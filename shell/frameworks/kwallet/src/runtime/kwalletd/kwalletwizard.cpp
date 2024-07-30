/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwalletwizard.h"

#include "ui_kwalletwizardpageexplanation.h"
#include "ui_kwalletwizardpageintro.h"
#include "ui_kwalletwizardpageoptions.h"
#include "ui_kwalletwizardpagepassword.h"
#ifdef HAVE_GPGMEPP
#include "ui_kwalletwizardpagegpgkey.h"
#include "ui_kwalletwizardpagepasswordgpg.h"
#endif

#include <KLocalizedString>
#include <QButtonGroup>
#include <QDebug>
#include <QIcon>

#ifdef HAVE_GPGMEPP
#include <KMessageBox>
#include <QComboBox>
#include <gpgme++/context.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>
#endif

class PageIntro : public QWizardPage
{
public:
    explicit PageIntro(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);

        ui.ktitlewidget->setText("<h1>" + i18n("KWallet") + "</h1>");
        ui.ktitlewidget->setIcon(QIcon::fromTheme(QStringLiteral("kwalletmanager")));

        bg = new QButtonGroup(this);
        bg->setExclusive(true);
        bg->addButton(ui._basic, 0);
        bg->addButton(ui._advanced, 1);

        // force the "basic" button to be selected
        ui._basic->setChecked(true);
    }

    QButtonGroup *bg;

private:
    Ui::KWalletWizardPageIntro ui;
};

class PagePassword : public QWizardPage
{
public:
    explicit PagePassword(KWalletWizard *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);

        registerField(QStringLiteral("useWallet"), ui._useWallet);
        registerField(QStringLiteral("pass1"), ui._pass1);
        registerField(QStringLiteral("pass2"), ui._pass2);
#ifdef HAVE_GPGMEPP
        registerField(QStringLiteral("useGPG"), ui._radioGpg);
        registerField(QStringLiteral("useBlowfish"), ui._radioBlowfish);
        connect(ui._radioBlowfish, &QRadioButton::toggled, parent, &KWalletWizard::passwordPageUpdate);
#endif

        connect(ui._useWallet, &QCheckBox::clicked, parent, &KWalletWizard::passwordPageUpdate);
        connect(ui._pass1, &QLineEdit::textChanged, parent, &KWalletWizard::passwordPageUpdate);
        connect(ui._pass2, &QLineEdit::textChanged, parent, &KWalletWizard::passwordPageUpdate);
        ui._useWallet->setChecked(true);
    }

    int nextId() const override
    {
#ifdef HAVE_GPGMEPP
        int nextId = -1;
        if (field(QStringLiteral("useWallet")).toBool()) {
            if (field(QStringLiteral("useBlowfish")).toBool()) {
                nextId = static_cast<KWalletWizard *>(wizard())->wizardType() == KWalletWizard::Basic
                    ? -1
                    : KWalletWizard::PageOptionsId; // same as non GPGMEPP case
            } else {
                nextId = KWalletWizard::PageGpgKeyId;
            }
        }

        return nextId;
#else
        return static_cast<KWalletWizard *>(wizard())->wizardType() == KWalletWizard::Basic ? -1 : KWalletWizard::PageOptionsId;
#endif
    }

    void setMatchLabelText(const QString &text)
    {
        ui._matchLabel->setText(text);
    }

private:
#ifdef HAVE_GPGMEPP
    Ui::KWalletWizardPagePasswordGpg ui;
#else
    Ui::KWalletWizardPagePassword ui;
#endif
};

#ifdef HAVE_GPGMEPP
typedef std::vector<GpgME::Key> KeysVector;
Q_DECLARE_METATYPE(GpgME::Key)

struct AddKeyToCombo {
    QComboBox *_list;
    AddKeyToCombo(QComboBox *list)
        : _list(list)
    {
    }
    void operator()(const GpgME::Key &k)
    {
        QString text = QStringLiteral("%1 (%2)").arg(k.shortKeyID(), k.userID(0).email());
        QVariant varKey;
        varKey.setValue(k);
        _list->addItem(text, varKey);
    }
};

class PageGpgKey : public QWizardPage
{
public:
    explicit PageGpgKey(QWidget *parent)
        : QWizardPage(parent)
        , userHasGpgKeys(false)
    {
        ui.setupUi(this);

        registerField(QStringLiteral("gpgKey"), ui._gpgKey);

        KeysVector keys;
        GpgME::initializeLibrary();
        GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
        if (err) {
            qDebug() << "OpenPGP not supported on your system!";
            KMessageBox::error(
                this,
                i18n("The GpgME library failed to initialize for the OpenPGP protocol. Please check your system's configuration then try again."));
        } else {
            std::shared_ptr<GpgME::Context> ctx(GpgME::Context::createForProtocol(GpgME::OpenPGP));
            if (nullptr == ctx) {
                KMessageBox::error(
                    this,
                    i18n("The GpgME library failed to initialize for the OpenPGP protocol. Please check your system's configuration then try again."));
            } else {
                ctx->setKeyListMode(GpgME::KeyListMode::Local);
                err = ctx->startKeyListing();
                while (!err) {
                    GpgME::Key k = ctx->nextKey(err);
                    if (err) {
                        break;
                    }
                    if (!k.isInvalid() && k.canEncrypt() && (k.ownerTrust() == GpgME::Key::Ultimate)) {
                        keys.push_back(k);
                    }
                }
                ctx->endKeyListing();
            }
        }
        std::for_each(keys.begin(), keys.end(), AddKeyToCombo(ui._gpgKey));

        userHasGpgKeys = keys.size() > 0;
        if (userHasGpgKeys) {
            ui.stackedWidget->setCurrentWidget(ui._pageWhenHasKeys);
        } else {
            ui.stackedWidget->setCurrentWidget(ui._pageNoKeys);
            setFinalPage(true);
        }
        Q_EMIT completeChanged();
    }

    int nextId() const override
    {
        return static_cast<KWalletWizard *>(wizard())->wizardType() == KWalletWizard::Basic ? -1 : KWalletWizard::PageOptionsId;
    }

    bool isComplete() const override
    {
        return userHasGpgKeys;
    }

    bool hasGpgKeys() const
    {
        return userHasGpgKeys;
    }

    GpgME::Key gpgKey() const
    {
        QVariant varKey = ui._gpgKey->itemData(field(QStringLiteral("gpgKey")).toInt());
        return varKey.value<GpgME::Key>();
    }

private:
    Ui::KWalletWizardPageGpgKey ui;
    bool userHasGpgKeys;
};
#endif

class PageOptions : public QWizardPage
{
public:
    explicit PageOptions(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);

        registerField(QStringLiteral("closeWhenIdle"), ui._closeIdle);
        registerField(QStringLiteral("networkWallet"), ui._networkWallet);
    }

private:
    Ui::KWalletWizardPageOptions ui;
};

class PageExplanation : public QWizardPage
{
public:
    PageExplanation(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);
        setFinalPage(true);
    }

private:
    Ui::KWalletWizardPageExplanation ui;
};

KWalletWizard::KWalletWizard(QWidget *parent)
    : QWizard(parent)
{
    setOption(HaveFinishButtonOnEarlyPages);

    m_pageIntro = new PageIntro(this);
    setPage(PageIntroId, m_pageIntro);
    m_pagePasswd = new PagePassword(this);
    setPage(PagePasswordId, m_pagePasswd);
#ifdef HAVE_GPGMEPP
    m_pageGpgKey = new PageGpgKey(this);
    setPage(PageGpgKeyId, m_pageGpgKey);
#endif
    setPage(PageOptionsId, new PageOptions(this));
    setPage(PageExplanationId, new PageExplanation(this));

    resize(500, 420);
}

void KWalletWizard::passwordPageUpdate()
{
    bool complete = true;
    if (field(QStringLiteral("useWallet")).toBool()) {
#ifdef HAVE_GPGMEPP
        if (field(QStringLiteral("useBlowfish")).toBool()) {
            m_pagePasswd->setFinalPage(wizardType() == Basic);
            button(NextButton)->setVisible(wizardType() != Basic);
#endif
            if (field(QStringLiteral("pass1")).toString() == field(QStringLiteral("pass2")).toString()) {
                if (field(QStringLiteral("pass1")).toString().isEmpty()) {
                    m_pagePasswd->setMatchLabelText(i18n("<qt>Password is empty.  <b>(WARNING: Insecure)</b></qt>"));
                } else {
                    m_pagePasswd->setMatchLabelText(i18n("Passwords match."));
                }
            } else {
                m_pagePasswd->setMatchLabelText(i18n("Passwords do not match."));
                complete = false;
            }
#ifdef HAVE_GPGMEPP
        } else {
            m_pagePasswd->setFinalPage(false);
            button(NextButton)->setEnabled(true);
            return;
        }
#endif
    } else {
        m_pagePasswd->setMatchLabelText(QString());
    }
    button(wizardType() == Basic ? FinishButton : NextButton)->setEnabled(complete);
}

KWalletWizard::WizardType KWalletWizard::wizardType() const
{
    return (KWalletWizard::WizardType)m_pageIntro->bg->checkedId();
}

void KWalletWizard::initializePage(int id)
{
    switch (id) {
    case PagePasswordId: {
        bool islast = m_pageIntro->bg->checkedId() == 0;
        m_pagePasswd->setFinalPage(islast);
        button(NextButton)->setVisible(!islast);
        break;
    }
    }
}

#ifdef HAVE_GPGMEPP
GpgME::Key KWalletWizard::gpgKey() const
{
    return m_pageGpgKey->gpgKey();
}
#endif

#include "moc_kwalletwizard.cpp"
