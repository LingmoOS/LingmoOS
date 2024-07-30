/* SPDX-FileCopyrightText: 2010 Anton Kreuzkamp <akreuzkamp@web.de>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "charrunner_config.h"
#include "config_keys.h"

// KF
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KRunner/AbstractRunner>
#include <KSharedConfig>

CharacterRunnerConfigForm::CharacterRunnerConfigForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

CharacterRunnerConfig::CharacterRunnerConfig(QObject *parent, const KPluginMetaData &metaData)
    : KCModule(parent, metaData)
{
    m_ui = new CharacterRunnerConfigForm(widget());

    QGridLayout *layout = new QGridLayout(widget());
    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->edit_trigger, &QLineEdit::textChanged, this, &CharacterRunnerConfig::markAsChanged);
    connect(m_ui->addItem, &QPushButton::clicked, this, &CharacterRunnerConfig::addItem);
    connect(m_ui->addItem, &QPushButton::clicked, this, &CharacterRunnerConfig::markAsChanged);
    connect(m_ui->deleteItem, &QPushButton::clicked, this, &CharacterRunnerConfig::deleteItem);
    connect(m_ui->deleteItem, &QPushButton::clicked, this, &CharacterRunnerConfig::markAsChanged);
    connect(m_ui->list, &QTreeWidget::itemSelectionChanged, this, &CharacterRunnerConfig::validateDeleteButton);
    connect(m_ui->edit_alias, &QLineEdit::textChanged, this, &CharacterRunnerConfig::validateAddButton);
    connect(m_ui->edit_hex, &QLineEdit::textChanged, this, &CharacterRunnerConfig::validateAddButton);
}

void CharacterRunnerConfig::load()
{
    KCModule::load();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners").group(KRUNNER_PLUGIN_NAME);

    m_ui->edit_trigger->setText(grp.readEntry(CONFIG_TRIGGERWORD, DEFAULT_TRIGGERWORD.toString()));
    const auto aliasList = grp.readEntry(CONFIG_ALIASES, QStringList());
    const auto codeList = grp.readEntry(CONFIG_CODES, QStringList());
    if (aliasList.size() == codeList.size()) {
        for (int i = 0; i < aliasList.size(); ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->list, 2);
            item->setText(0, aliasList[i]);
            item->setText(1, codeList[i]);
            m_ui->list->addTopLevelItem(item);
        }
    } else {
        const auto msg = new KMessageWidget(i18nc("Message that config is corrupted", //
                                                  "Config entries for alias list and code list have different sizes, ignoring all."),
                                            widget());
        m_ui->verticalLayout->insertWidget(0, msg);
    }
}

void CharacterRunnerConfig::save()
{
    KCModule::save();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners").group(KRUNNER_PLUGIN_NAME);

    grp.writeEntry(CONFIG_TRIGGERWORD, m_ui->edit_trigger->text().isEmpty() ? DEFAULT_TRIGGERWORD.toString() : m_ui->edit_trigger->text());

    QList<QString> aliasList;
    QList<QString> codeList;
    for (int i = 0; i < m_ui->list->topLevelItemCount(); ++i) {
        aliasList.append(m_ui->list->topLevelItem(i)->text(0));
        codeList.append(m_ui->list->topLevelItem(i)->text(1));
    }
    grp.writeEntry(CONFIG_ALIASES, aliasList);
    grp.writeEntry(CONFIG_CODES, codeList);
    grp.sync();
}

void CharacterRunnerConfig::defaults()
{
    KCModule::defaults();

    m_ui->edit_trigger->setText(DEFAULT_TRIGGERWORD.toString());
    m_ui->list->clear();

    markAsChanged();
}

void CharacterRunnerConfig::addItem()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->list, 2);
    item->setText(0, m_ui->edit_alias->text());
    item->setText(1, m_ui->edit_hex->text());
    m_ui->list->addTopLevelItem(item);
    m_ui->edit_alias->clear();
    m_ui->edit_hex->clear();
}

void CharacterRunnerConfig::deleteItem()
{
    m_ui->list->takeTopLevelItem(m_ui->list->indexOfTopLevelItem(m_ui->list->currentItem()));
}

void CharacterRunnerConfig::validateAddButton()
{
    m_ui->addItem->setDisabled(m_ui->edit_alias->text().isEmpty() || m_ui->edit_hex->text().isEmpty());
}

void CharacterRunnerConfig::validateDeleteButton()
{
    m_ui->deleteItem->setDisabled(!m_ui->list->selectedItems().count());
}

K_PLUGIN_FACTORY(CharacterRunnerConfigFactory, registerPlugin<CharacterRunnerConfig>();)

#include "charrunner_config.moc"
