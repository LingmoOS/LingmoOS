/*
 *   SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *   SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "basictab.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KIconButton>
#include <KKeySequenceWidget>
#include <KLocalizedString>
#include <KShell>
#include <KUrlRequester>
#include <QLineEdit>

#include "globalaccel.h"

#include "klinespellchecking.h"
#include "menuinfo.h"

BasicTab::BasicTab(QWidget *parent)
    : QTabWidget(parent)
{
    initGeneralTab();
    initAdvancedTab();
    initConnections();

    setDocumentMode(true);

    slotDisableAction();
}

void BasicTab::initGeneralTab()
{
    // general tab
    QWidget *generalTab = new QWidget();
    QGridLayout *generalTabLayout = new QGridLayout(generalTab);
    generalTab->setAcceptDrops(false);

    // name
    _nameLabel = new QLabel(i18n("&Name:"));
    generalTabLayout->addWidget(_nameLabel, 0, 0);
    _nameEdit = new QLineEdit();
    _nameEdit->setAcceptDrops(false);
    _nameEdit->setClearButtonEnabled(true);
    _nameLabel->setBuddy(_nameEdit);
    generalTabLayout->addWidget(_nameEdit, 0, 1, 1, 1);

    // description
    _descriptionLabel = new QLabel(i18n("&Description:"));
    generalTabLayout->addWidget(_descriptionLabel, 1, 0);
    _descriptionEdit = new KLineSpellChecking();
    _descriptionEdit->setAcceptDrops(false);
    _descriptionLabel->setBuddy(_descriptionEdit);
    generalTabLayout->addWidget(_descriptionEdit, 1, 1, 1, 1);

    // comment
    _commentLabel = new QLabel(i18n("&Comment:"));
    generalTabLayout->addWidget(_commentLabel, 2, 0);
    _commentEdit = new KLineSpellChecking();
    _commentEdit->setAcceptDrops(false);
    _commentLabel->setBuddy(_commentEdit);
    generalTabLayout->addWidget(_commentEdit, 2, 1, 1, 2);

    // environment variables
    _envarsLabel = new QLabel(i18nc("@label:textfield", "Environment Variables:"));
    generalTabLayout->addWidget(_envarsLabel, 3, 0);
    _envarsEdit = new QLineEdit();
    _envarsLabel->setBuddy(_envarsEdit);
    generalTabLayout->addWidget(_envarsEdit, 3, 1, 1, 2);

    // command
    _programLabel = new QLabel(i18nc("@label:textbox the name or path to a command-line program", "Program:"));
    generalTabLayout->addWidget(_programLabel, 4, 0);
    _programEdit = new KUrlRequester();
    _programEdit->lineEdit()->setAcceptDrops(false);
    _programLabel->setBuddy(_programEdit);
    generalTabLayout->addWidget(_programEdit, 4, 1, 1, 2);

    _argsLabel = new QLabel(i18nc("@label:textfield", "Command-Line Arguments:"));
    generalTabLayout->addWidget(_argsLabel, 5, 0);
    _argsEdit = new QLineEdit();
    _argsEdit->setWhatsThis(
        i18n("Following the command, you can have several place holders which will be replaced "
             "with the actual values when the actual program is run:\n"
             "%f - a single file name\n"
             "%F - a list of files; use for applications that can open several local files at once\n"
             "%u - a single URL\n"
             "%U - a list of URLs\n"
             "%d - the folder of the file to open\n"
             "%D - a list of folders\n"
             "%i - the icon\n"
             "%m - the mini-icon\n"
             "%c - the caption"));
    _argsLabel->setBuddy(_argsEdit);
    generalTabLayout->addWidget(_argsEdit, 5, 1, 1, 2);

    // launch feedback
    _launchCB = new QCheckBox(i18n("Enable &launch feedback"));
    generalTabLayout->addWidget(_launchCB, 6, 0, 1, 3);

    // KDE visibility
    _onlyShowInKdeCB = new QCheckBox(i18n("Only show when logged into a Plasma session"));
    generalTabLayout->addWidget(_onlyShowInKdeCB, 7, 0, 1, 3);

    // hidden entry
    _hiddenEntryCB = new QCheckBox(i18n("Hidden entry"));
    _hiddenEntryCB->hide();
    generalTabLayout->addWidget(_hiddenEntryCB, 8, 0, 1, 3);

    // icon
    _iconButton = new KIconButton();
    _iconButton->setFixedSize(56, 56);
    _iconButton->setIconSize(32);
    generalTabLayout->addWidget(_iconButton, 0, 2, 2, 1);
    generalTabLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 8, 0, 1, 3);

    // add the general group
    addTab(generalTab, i18n("General"));
}

void BasicTab::initAdvancedTab()
{
    // advanced tab
    QWidget *advancedTab = new QWidget();
    QVBoxLayout *advancedTabLayout = new QVBoxLayout(advancedTab);

    // work path
    _workPathGroup = new QGroupBox();
    QHBoxLayout *workPathGroupLayout = new QHBoxLayout(_workPathGroup);
    _pathLabel = new QLabel(i18n("&Work path:"));
    workPathGroupLayout->addWidget(_pathLabel);
    _pathEdit = new KUrlRequester();
    _pathEdit->setMode(KFile::Directory | KFile::LocalOnly);
    _pathEdit->lineEdit()->setAcceptDrops(false);
    _pathLabel->setBuddy(_pathEdit);
    workPathGroupLayout->addWidget(_pathEdit);
    advancedTabLayout->addWidget(_workPathGroup);

    // terminal CB
    _terminalGroup = new QGroupBox();
    QVBoxLayout *terminalGroupLayout = new QVBoxLayout(_terminalGroup);
    _terminalCB = new QCheckBox(i18n("Run in term&inal"));
    terminalGroupLayout->addWidget(_terminalCB);
    // terminal options
    QWidget *terminalOptionsGroup = new QWidget();
    QHBoxLayout *terminalOptionsGroupLayout = new QHBoxLayout(terminalOptionsGroup);
    _terminalOptionsLabel = new QLabel(i18n("Terminal &options:"));
    terminalOptionsGroupLayout->addWidget(_terminalOptionsLabel);
    _terminalOptionsEdit = new QLineEdit();
    _terminalOptionsEdit->setClearButtonEnabled(true);
    _terminalOptionsEdit->setAcceptDrops(false);
    _terminalOptionsEdit->setEnabled(false);
    _terminalOptionsLabel->setBuddy(_terminalOptionsEdit);
    terminalOptionsGroupLayout->addWidget(_terminalOptionsEdit);
    terminalGroupLayout->addWidget(terminalOptionsGroup);
    advancedTabLayout->addWidget(_terminalGroup);

    // user name CB
    _userGroup = new QGroupBox();
    QVBoxLayout *userGroupLayout = new QVBoxLayout(_userGroup);
    _userCB = new QCheckBox(i18n("&Run as a different user"));
    userGroupLayout->addWidget(_userCB);
    // user name
    QWidget *userNameGroup = new QWidget();
    QHBoxLayout *userNameGroupLayout = new QHBoxLayout(userNameGroup);
    _userNameLabel = new QLabel(i18n("&Username:"));
    userNameGroupLayout->addWidget(_userNameLabel);
    _userNameEdit = new QLineEdit();
    _userNameEdit->setClearButtonEnabled(true);
    _userNameEdit->setAcceptDrops(false);
    _userNameEdit->setEnabled(false);
    _userNameLabel->setBuddy(_userNameEdit);
    userNameGroupLayout->addWidget(_userNameEdit);
    userGroupLayout->addWidget(userNameGroup);
    advancedTabLayout->addWidget(_userGroup);

    // key binding
    _keyBindingGroup = new QGroupBox();
    QHBoxLayout *keyBindingGroupLayout = new QHBoxLayout(_keyBindingGroup);
    _keyBindingLabel = new QLabel(i18n("Current shortcut &key:"));
    keyBindingGroupLayout->addWidget(_keyBindingLabel);
    _keyBindingEdit = new KKeySequenceWidget();
    _keyBindingEdit->setMultiKeyShortcutsAllowed(false);
    _keyBindingLabel->setBuddy(_keyBindingEdit);
    keyBindingGroupLayout->addWidget(_keyBindingEdit);
    advancedTabLayout->addWidget(_keyBindingGroup);

    // push components to the top
    advancedTabLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    // add the general group
    addTab(advancedTab, i18n("Advanced"));
}

void BasicTab::initConnections()
{
    // general tab's components
    connect(_nameEdit, &QLineEdit::textChanged, this, &BasicTab::slotChanged);
    connect(_descriptionEdit, &KLineSpellChecking::textChanged, this, &BasicTab::slotChanged);
    connect(_commentEdit, &KLineSpellChecking::textChanged, this, &BasicTab::slotChanged);
    connect(_programEdit, &KUrlRequester::textChanged, this, &BasicTab::slotChanged);
    connect(_envarsEdit, &QLineEdit::textChanged, this, &BasicTab::slotChanged);
    connect(_argsEdit, &QLineEdit::textChanged, this, &BasicTab::slotChanged);
    connect(_programEdit, &KUrlRequester::urlSelected, this, &BasicTab::slotExecSelected);
    connect(_launchCB, &QCheckBox::clicked, this, &BasicTab::launchcb_clicked);
    connect(_onlyShowInKdeCB, &QCheckBox::clicked, this, &BasicTab::onlyshowcb_clicked);
    connect(_hiddenEntryCB, &QCheckBox::clicked, this, &BasicTab::hiddenentrycb_clicked);
    connect(_iconButton, &KIconButton::iconChanged, this, &BasicTab::slotChanged);

    // advanced tab's components
    connect(_pathEdit, &KUrlRequester::textChanged, this, &BasicTab::slotChanged);
    connect(_terminalCB, &QCheckBox::clicked, this, &BasicTab::termcb_clicked);
    connect(_terminalOptionsEdit, &QLineEdit::textChanged, this, &BasicTab::slotChanged);
    connect(_userCB, &QCheckBox::clicked, this, &BasicTab::uidcb_clicked);
    connect(_userNameEdit, &QLineEdit::textChanged, this, &BasicTab::slotChanged);
    connect(_keyBindingEdit, &KKeySequenceWidget::keySequenceChanged, this, &BasicTab::slotCapturedKeySequence);
}

void BasicTab::slotDisableAction()
{
    // disable all group at the beginning.
    // because there is not file selected.
    _nameEdit->setEnabled(false);
    _descriptionEdit->setEnabled(false);
    _commentEdit->setEnabled(false);
    _programEdit->setEnabled(false);
    _envarsEdit->setEnabled(false);
    _argsEdit->setEnabled(false);
    _launchCB->setEnabled(false);
    _onlyShowInKdeCB->setEnabled(false);
    _hiddenEntryCB->setEnabled(false);
    _nameLabel->setEnabled(false);
    _descriptionLabel->setEnabled(false);
    _commentLabel->setEnabled(false);
    _programLabel->setEnabled(false);
    _envarsLabel->setEnabled(false);
    _argsLabel->setEnabled(false);
    _workPathGroup->setEnabled(false);
    _terminalGroup->setEnabled(false);
    _userGroup->setEnabled(false);
    _iconButton->setEnabled(false);
    // key binding part
    _keyBindingGroup->setEnabled(false);
}

void BasicTab::enableWidgets(bool isDF, bool isDeleted)
{
    // set only basic attributes if it is not a .desktop file
    _nameEdit->setEnabled(!isDeleted);
    _descriptionEdit->setEnabled(!isDeleted);
    _commentEdit->setEnabled(!isDeleted);
    _iconButton->setEnabled(!isDeleted);
    _programEdit->setEnabled(isDF && !isDeleted);
    _envarsEdit->setEnabled(isDF && !isDeleted);
    _argsEdit->setEnabled(isDF && !isDeleted);
    _launchCB->setEnabled(isDF && !isDeleted);
    _onlyShowInKdeCB->setEnabled(isDF && !isDeleted);
    _hiddenEntryCB->setEnabled(isDF && !isDeleted);
    _nameLabel->setEnabled(!isDeleted);
    _descriptionLabel->setEnabled(!isDeleted);
    _commentLabel->setEnabled(!isDeleted);
    _programLabel->setEnabled(isDF && !isDeleted);
    _envarsLabel->setEnabled(isDF && !isDeleted);
    _argsLabel->setEnabled(isDF && !isDeleted);

    _workPathGroup->setEnabled(isDF && !isDeleted);
    _terminalGroup->setEnabled(isDF && !isDeleted);
    _userGroup->setEnabled(isDF && !isDeleted);
    _keyBindingGroup->setEnabled(isDF && !isDeleted);

    _terminalOptionsEdit->setEnabled(isDF && !isDeleted && _terminalCB->isChecked());
    _terminalOptionsLabel->setEnabled(isDF && !isDeleted && _terminalCB->isChecked());

    _userNameEdit->setEnabled(isDF && !isDeleted && _userCB->isChecked());
    _userNameLabel->setEnabled(isDF && !isDeleted && _userCB->isChecked());
}

void BasicTab::setFolderInfo(MenuFolderInfo *folderInfo)
{
    blockSignals(true);
    _menuFolderInfo = folderInfo;
    _menuEntryInfo = 0;

    _nameEdit->setText(folderInfo->caption);
    _descriptionEdit->setText(folderInfo->genericname);
    _descriptionEdit->setCursorPosition(0);
    _commentEdit->setText(folderInfo->comment);
    _commentEdit->setCursorPosition(0);
    _iconButton->setIcon(folderInfo->icon);

    // clean all disabled fields and return
    _programEdit->lineEdit()->clear();
    _envarsEdit->clear();
    _argsEdit->clear();
    _pathEdit->lineEdit()->clear();
    _terminalOptionsEdit->clear();
    _userNameEdit->clear();
    _launchCB->setChecked(false);
    _terminalCB->setChecked(false);
    _onlyShowInKdeCB->setChecked(false);
    _hiddenEntryCB->setChecked(false);
    _userCB->setChecked(false);
    _keyBindingEdit->clearKeySequence();

    enableWidgets(false, folderInfo->hidden);
    blockSignals(false);
}

void BasicTab::setEntryInfo(MenuEntryInfo *entryInfo)
{
    blockSignals(true);
    _menuFolderInfo = 0;
    _menuEntryInfo = entryInfo;

    if (!entryInfo) {
        _nameEdit->clear();
        _descriptionEdit->clear();
        _commentEdit->clear();
        _iconButton->setIcon(QString());

        // key binding part
        _keyBindingEdit->clearKeySequence();

        _programEdit->lineEdit()->clear();
        _envarsEdit->clear();
        _argsEdit->clear();
        _onlyShowInKdeCB->setChecked(false);
        _hiddenEntryCB->setChecked(false);

        _pathEdit->lineEdit()->clear();
        _terminalOptionsEdit->clear();
        _userNameEdit->clear();

        _launchCB->setChecked(false);
        _terminalCB->setChecked(false);
        _userCB->setChecked(false);
        enableWidgets(true, true);
        blockSignals(false);
        return;
    }

    KDesktopFile *df = entryInfo->desktopFile();

    _nameEdit->setText(df->readName());
    _descriptionEdit->setText(df->readGenericName());
    _descriptionEdit->setCursorPosition(0);
    _commentEdit->setText(df->readComment());
    _commentEdit->setCursorPosition(0);
    _iconButton->setIcon(df->readIcon());

    // key binding part
    if (!entryInfo->shortcut().isEmpty()) {
        _keyBindingEdit->setKeySequence(entryInfo->shortcut());
    } else {
        _keyBindingEdit->clearKeySequence();
    }

    QStringList execLine = KShell::splitArgs(df->desktopGroup().readEntry("Exec"));
    QStringList enVars = {};
    if (!execLine.isEmpty()) {
        // check for apps that use the env executable
        // to set the environment
        if (execLine[0] == QLatin1String("env")) {
            execLine.pop_front();
        }
        for (auto env : execLine) {
            if (!env.contains(QLatin1String("="))) {
                break;
            }
            enVars += env;
            execLine.pop_front();
        }

        _programEdit->lineEdit()->setText(execLine.takeFirst());
    } else {
        _programEdit->lineEdit()->clear();
    }
    _argsEdit->setText(KShell::joinArgs(execLine));
    _envarsEdit->setText(KShell::joinArgs(enVars));

    _pathEdit->lineEdit()->setText(df->readPath());
    _terminalOptionsEdit->setText(df->desktopGroup().readEntry("TerminalOptions"));
    _userNameEdit->setText(df->desktopGroup().readEntry("X-KDE-Username"));

    if (df->desktopGroup().hasKey("StartupNotify")) {
        _launchCB->setChecked(df->desktopGroup().readEntry("StartupNotify", true));
    } else { // backwards comp.
        _launchCB->setChecked(df->desktopGroup().readEntry("X-KDE-StartupNotify", true));
    }

    // or maybe enable only if it contains nothing but KDE?
    _onlyShowInKdeCB->setChecked(df->desktopGroup().readXdgListEntry("OnlyShowIn").contains(QLatin1String("KDE")));

    if (df->desktopGroup().hasKey("NoDisplay")) {
        _hiddenEntryCB->setChecked(df->desktopGroup().readEntry("NoDisplay", true));
    } else {
        _hiddenEntryCB->setChecked(false);
    }

    _terminalCB->setChecked(df->desktopGroup().readEntry("Terminal", false));

    _userCB->setChecked(df->desktopGroup().readEntry("X-KDE-SubstituteUID", false));

    enableWidgets(true, entryInfo->hidden);
    blockSignals(false);
}

void BasicTab::apply()
{
    if (_menuEntryInfo) {
        _menuEntryInfo->setDirty();
        _menuEntryInfo->setCaption(_nameEdit->text());
        _menuEntryInfo->setDescription(_descriptionEdit->text());
        _menuEntryInfo->setIcon(_iconButton->icon());

        KDesktopFile *df = _menuEntryInfo->desktopFile();
        KConfigGroup dg = df->desktopGroup();
        dg.writeEntry("Comment", _commentEdit->text());
        
        QStringList command;
        QStringList envars = KShell::splitArgs(_envarsEdit->text());
        if (!envars.isEmpty()) {
            envars.push_front(QLatin1String("env"));
            command << KShell::joinArgs(envars);
        }
        command << _programEdit->lineEdit()->text();
        const QStringList args = KShell::splitArgs(_argsEdit->text());
        if (!args.isEmpty()) {
            command << KShell::joinArgs(args);
        }
        dg.writeEntry("Exec", command.join(QLatin1Char(' ')));

        // NOT writePathEntry, it writes the entry with non-XDG-compliant flag: Path[$e]
        dg.writeEntry("Path", _pathEdit->lineEdit()->text());

        dg.writeEntry("Terminal", _terminalCB->isChecked());
        dg.writeEntry("TerminalOptions", _terminalOptionsEdit->text());
        dg.writeEntry("X-KDE-SubstituteUID", _userCB->isChecked());
        dg.writeEntry("X-KDE-Username", _userNameEdit->text());
        dg.writeEntry("StartupNotify", _launchCB->isChecked());
        dg.writeEntry("NoDisplay", _hiddenEntryCB->isChecked());

        QStringList onlyShowIn = df->desktopGroup().readXdgListEntry("OnlyShowIn");
        /* the exact semantics of this checkbox are unclear if there is more than just KDE in the list...
         * For example: - The list is "Gnome;", the user enables "Only show when logged into a Plasma session" - should we remove Gnome?
         *              - The list is "Gnome;KDE;", the user unchecks the box - should we keep Gnome?
         */
        if (_onlyShowInKdeCB->isChecked() && !onlyShowIn.contains(QLatin1String("KDE"))) {
            onlyShowIn << QStringLiteral("KDE");
        } else if (!_onlyShowInKdeCB->isChecked() && onlyShowIn.contains(QLatin1String("KDE"))) {
            onlyShowIn.removeAll(QStringLiteral("KDE"));
        }
        if (onlyShowIn.isEmpty()) {
            dg.deleteEntry("OnlyShowIn");
        } else {
            dg.writeXdgListEntry("OnlyShowIn", onlyShowIn);
        }
    } else {
        _menuFolderInfo->setCaption(_nameEdit->text());
        _menuFolderInfo->setGenericName(_descriptionEdit->text());
        _menuFolderInfo->setComment(_commentEdit->text());
        _menuFolderInfo->setIcon(_iconButton->icon());
    }
}

void BasicTab::slotChanged()
{
    if (signalsBlocked()) {
        return;
    }
    apply();
    if (_menuEntryInfo) {
        emit changed(_menuEntryInfo);
    } else {
        emit changed(_menuFolderInfo);
    }
}

void BasicTab::launchcb_clicked()
{
    slotChanged();
}

void BasicTab::onlyshowcb_clicked()
{
    slotChanged();
}

void BasicTab::hiddenentrycb_clicked()
{
    slotChanged();
}

void BasicTab::termcb_clicked()
{
    _terminalOptionsEdit->setEnabled(_terminalCB->isChecked());
    _terminalOptionsLabel->setEnabled(_terminalCB->isChecked());
    slotChanged();
}

void BasicTab::uidcb_clicked()
{
    _userNameEdit->setEnabled(_userCB->isChecked());
    _userNameLabel->setEnabled(_userCB->isChecked());
    slotChanged();
}

void BasicTab::slotExecSelected()
{
    QString path = _programEdit->lineEdit()->text();
    if (!path.startsWith(QLatin1Char('\''))) {
        _programEdit->lineEdit()->setText(KShell::quoteArg(path));
    }
}

void BasicTab::slotCapturedKeySequence(const QKeySequence &seq)
{
    if (signalsBlocked()) {
        return;
    }
    QKeySequence cut(seq);
    if (_menuEntryInfo->isShortcutAvailable(cut)) {
        _menuEntryInfo->setShortcut(cut);
    } else {
        // We will not assign the shortcut so reset the visible key sequence
        _keyBindingEdit->setKeySequence(QKeySequence());
    }
    if (_menuEntryInfo) {
        emit changed(_menuEntryInfo);
    }
}

void BasicTab::updateHiddenEntry(bool show)
{
    if (show) {
        _hiddenEntryCB->show();
    } else {
        _hiddenEntryCB->hide();
    }
}
