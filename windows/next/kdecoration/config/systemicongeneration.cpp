/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "systemicongeneration.h"
#include "breezeconfigwidget.h"
#include "presetsmodel.h"
#include <QPushButton>
#include <kicontheme.h>

namespace Breeze
{

SystemIconGeneration::SystemIconGeneration(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_SystemIconGeneration)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);
    this->setWindowTitle(i18n("System Icon Generation - Lingmo Settings"));
    populateComboBoxesWithSystemIconThemes();

    connect(m_ui->lingmoIconThemeInherits, &QComboBox::currentTextChanged, this, &SystemIconGeneration::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->lingmoDarkIconThemeInherits,
            &QComboBox::currentTextChanged,
            this,
            &SystemIconGeneration::updateChanged,
            Qt::ConnectionType::DirectConnection);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &SystemIconGeneration::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &SystemIconGeneration::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &SystemIconGeneration::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

SystemIconGeneration::~SystemIconGeneration()
{
    delete m_ui;
}

void SystemIconGeneration::load()
{
    m_loading = true;

    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_ui->lingmoIconThemeInherits->setCurrentText(m_internalSettings->lingmoIconThemeInherits());
    m_ui->lingmoDarkIconThemeInherits->setCurrentText(m_internalSettings->lingmoDarkIconThemeInherits());

    setChanged(false);

    m_loading = false;
    m_loaded = true;
}

void SystemIconGeneration::populateComboBoxesWithSystemIconThemes()
{
    QStringList iconThemes = KIconTheme::list();
    iconThemes.removeAll(QStringLiteral("lingmo"));
    iconThemes.removeAll(QStringLiteral("lingmo-dark"));
    iconThemes.sort(Qt::CaseSensitivity::CaseInsensitive);

    m_ui->lingmoIconThemeInherits->addItems(iconThemes);
    m_ui->lingmoDarkIconThemeInherits->addItems(iconThemes);
}

void SystemIconGeneration::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    m_internalSettings->setLingmoIconThemeInherits(m_ui->lingmoIconThemeInherits->currentText());
    m_internalSettings->setLingmoDarkIconThemeInherits(m_ui->lingmoDarkIconThemeInherits->currentText());

    m_internalSettings->save();
    setChanged(false);

    if (reloadKwinConfig) {
        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void SystemIconGeneration::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    populateComboBoxesWithSystemIconThemes();
    m_ui->lingmoIconThemeInherits->setCurrentText(m_internalSettings->lingmoIconThemeInherits());
    m_ui->lingmoDarkIconThemeInherits->setCurrentText(m_internalSettings->lingmoDarkIconThemeInherits());

    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool SystemIconGeneration::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("SystemIconGeneration"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void SystemIconGeneration::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void SystemIconGeneration::accept()
{
    save();
    QDialog::accept();
}

void SystemIconGeneration::reject()
{
    load();
    QDialog::reject();
}

void SystemIconGeneration::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_ui->lingmoIconThemeInherits->currentText() != m_internalSettings->lingmoIconThemeInherits())
        modified = true;
    else if (m_ui->lingmoDarkIconThemeInherits->currentText() != m_internalSettings->lingmoDarkIconThemeInherits())
        modified = true;

    setChanged(modified);
}

void SystemIconGeneration::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

}
