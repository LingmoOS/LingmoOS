/*
 *   SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "preferencesdlg.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "configurationmanager.h"
#include <KLocalizedString>
#include <sonnet/configwidget.h>

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setFaceType(List);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    m_pageMisc = new MiscPage(this);
    KPageWidgetItem *page = new KPageWidgetItem(m_pageMisc, i18n("General options"));
    page->setIcon(QIcon::fromTheme(QStringLiteral("kmenuedit")));
    addPage(page);

    m_pageSpellChecking = new SpellCheckingPage(this);
    page = new KPageWidgetItem(m_pageSpellChecking, i18n("Spell Checking"));
    page->setHeader(i18n("Spell checking Options"));
    page->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
    addPage(page);

    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &PreferencesDialog::slotSave);
}

void PreferencesDialog::slotSave()
{
    m_pageSpellChecking->saveOptions();
    m_pageMisc->saveOptions();
}

SpellCheckingPage::SpellCheckingPage(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    m_confPage = new Sonnet::ConfigWidget(this);
    lay->addWidget(m_confPage);
    setLayout(lay);
}

void SpellCheckingPage::saveOptions()
{
    m_confPage->save();
}

MiscPage::MiscPage(QWidget *parent)
    : QWidget(parent)
{
    // general group
    QGroupBox *generalGroup = new QGroupBox(i18n("General"));
    QVBoxLayout *generalGroupLayout = new QVBoxLayout(generalGroup);
    m_showHiddenEntries = new QCheckBox(i18n("Show hidden entries"));
    generalGroupLayout->addWidget(m_showHiddenEntries);

    // add groups
    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->addWidget(generalGroup);
    pageLayout->addStretch();

    // update displayed config
    m_showHiddenEntries->setChecked(ConfigurationManager::getInstance()->hiddenEntriesVisible());
}

void MiscPage::saveOptions()
{
    ConfigurationManager::getInstance()->setHiddenEntriesVisible(m_showHiddenEntries->isChecked());
}
