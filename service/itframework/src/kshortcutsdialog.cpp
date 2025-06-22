/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kshortcutsdialog.h"
#include "kshortcutschemeshelper_p.h"
#include "kshortcutsdialog_p.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDomDocument>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "kactioncollection.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

/************************************************************************/
/* KShortcutsDialog                                                     */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialog there is almost no code left ;)                    */
/*                                                                      */
/************************************************************************/

QKeySequence primarySequence(const QList<QKeySequence> &sequences)
{
    return sequences.isEmpty() ? QKeySequence() : sequences.at(0);
}

QKeySequence alternateSequence(const QList<QKeySequence> &sequences)
{
    return sequences.size() <= 1 ? QKeySequence() : sequences.at(1);
}

class KShortcutsDialogPrivate
{
public:
    KShortcutsDialogPrivate(KShortcutsDialog *qq)
        : q(qq)
    {
    }

    QList<KActionCollection *> m_collections;

    void changeShortcutScheme(const QString &scheme)
    {
        if (m_keyChooser->isModified()
            && KMessageBox::questionTwoActions(q,
                                               i18n("The current shortcut scheme is modified. Save before switching to the new one?"),
                                               QString(),
                                               KStandardGuiItem::save(),
                                               KStandardGuiItem::discard())
                == KMessageBox::PrimaryAction) {
            m_keyChooser->save();
        } else {
            m_keyChooser->undo();
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        m_keyChooser->clearCollections();

        for (KActionCollection *collection : std::as_const(m_collections)) {
            // passing an empty stream forces the clients to reread the XML
            KXMLGUIClient *client = const_cast<KXMLGUIClient *>(collection->parentGUIClient());
            if (client) {
                client->setXMLGUIBuildDocument(QDomDocument());
            }
        }

        // get xmlguifactory
        if (!m_collections.isEmpty()) {
            const KXMLGUIClient *client = m_collections.first()->parentGUIClient();
            if (client) {
                KXMLGUIFactory *factory = client->factory();
                if (factory) {
                    factory->changeShortcutScheme(scheme);
                }
            }
        }

        for (KActionCollection *collection : std::as_const(m_collections)) {
            m_keyChooser->addCollection(collection);
        }

        QApplication::restoreOverrideCursor();
    }

    void undo()
    {
        m_keyChooser->undo();
    }

    void toggleDetails()
    {
        const bool isVisible = m_schemeEditor->isVisible();

        m_schemeEditor->setVisible(!isVisible);
        m_detailsButton->setText(detailsButtonText() + (isVisible ? QStringLiteral(" >>") : QStringLiteral(" <<")));
    }

    static QString detailsButtonText()
    {
        return i18n("Manage &Schemes");
    }

    void save()
    {
        m_keyChooser->save();
        Q_EMIT q->saved();
    }

    KShortcutsDialog *const q;
    KShortcutsEditor *m_keyChooser = nullptr; // ### move
    KShortcutSchemesEditor *m_schemeEditor = nullptr;
    QPushButton *m_detailsButton = nullptr;
    bool m_saveSettings = false;
};

KShortcutsDialog::KShortcutsDialog(QWidget *parent)
    : KShortcutsDialog(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, parent)
{
}

KShortcutsDialog::KShortcutsDialog(KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent)
    : QDialog(parent)
    , d(new KShortcutsDialogPrivate(this))
{
    setWindowTitle(i18nc("@title:window", "Configure Keyboard Shortcuts"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->m_keyChooser = new KShortcutsEditor(this, types, allowLetterShortcuts);
    layout->addWidget(d->m_keyChooser);

    d->m_schemeEditor = new KShortcutSchemesEditor(this);
    connect(d->m_schemeEditor, &KShortcutSchemesEditor::shortcutsSchemeChanged, this, [this](const QString &scheme) {
        d->changeShortcutScheme(scheme);
    });
    d->m_schemeEditor->hide();
    layout->addWidget(d->m_schemeEditor);

    d->m_detailsButton = new QPushButton;
    d->m_detailsButton->setText(KShortcutsDialogPrivate::detailsButtonText() + QLatin1String(" >>"));

    QPushButton *printButton = new QPushButton;
    KGuiItem::assign(printButton, KStandardGuiItem::print());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(d->m_detailsButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(printButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    layout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, d->m_keyChooser, &KShortcutsEditor::allDefault);
    connect(d->m_detailsButton, &QPushButton::clicked, this, [this]() {
        d->toggleDetails();
    });
    connect(printButton, &QPushButton::clicked, d->m_keyChooser, &KShortcutsEditor::printShortcuts);
    connect(buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        d->undo();
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    KConfigGroup group(KSharedConfig::openConfig(), "KShortcutsDialog Settings");
    resize(group.readEntry("Dialog Size", sizeHint()));
}

KShortcutsDialog::~KShortcutsDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), "KShortcutsDialog Settings");
    group.writeEntry("Dialog Size", size(), KConfigGroup::Persistent | KConfigGroup::Global);
}

void KShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
    d->m_keyChooser->addCollection(collection, title);
    d->m_collections << collection;
}

QList<KActionCollection *> KShortcutsDialog::actionCollections() const
{
    return d->m_collections;
}

// TODO KF6: remove this method, always save settings, and open the
// dialog with show() not exec()
bool KShortcutsDialog::configure(bool saveSettings)
{
    d->m_saveSettings = saveSettings;
    if (isModal()) {
        int retcode = exec();
        return retcode;
    } else {
        show();
        return false;
    }
}

void KShortcutsDialog::accept()
{
    if (d->m_saveSettings) {
        d->save();
    }
    QDialog::accept();
}

QSize KShortcutsDialog::sizeHint() const
{
    return QSize(600, 480);
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 84)
// static
int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent, bool saveSettings)
{
    // qDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )";
    auto *dlg = new KShortcutsDialog(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->addCollection(collection);
    return dlg->configure(saveSettings);
}
#endif

// static
void KShortcutsDialog::showDialog(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent)
{
    auto *dlg = new KShortcutsDialog(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    dlg->d->m_saveSettings = true; // Always save settings if the dialog is accepted

    dlg->addCollection(collection);
    dlg->show();
}

void KShortcutsDialog::importConfiguration(const QString &path)
{
    KConfig config(path);
    d->m_keyChooser->importConfiguration(static_cast<KConfigBase *>(&config));
}

void KShortcutsDialog::exportConfiguration(const QString &path) const
{
    KConfig config(path);
    d->m_keyChooser->exportConfiguration(static_cast<KConfigBase *>(&config));
}

void KShortcutsDialog::refreshSchemes()
{
    d->m_schemeEditor->refreshSchemes();
}

void KShortcutsDialog::addActionToSchemesMoreButton(QAction *action)
{
    d->m_schemeEditor->addMoreMenuAction(action);
}

#include "moc_kshortcutsdialog.cpp"
