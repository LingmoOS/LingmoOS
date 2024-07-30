/*
    SPDX-FileCopyrightText: 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "spellcheck_config.h"

#include <QGridLayout>
#include <QProcess>

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

// For the macro
#include <KRunner/AbstractRunner>

SpellCheckConfigForm::SpellCheckConfigForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

SpellCheckConfig::SpellCheckConfig(QObject *parent, const KPluginMetaData &metaData)
    : KCModule(parent, metaData)
{
    m_ui = new SpellCheckConfigForm(widget());

    QGridLayout *layout = new QGridLayout(widget());

    layout->addWidget(m_ui, 0, 0);

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(m_ui->m_requireTriggerWord, &QCheckBox::stateChanged, this, &SpellCheckConfig::markAsChanged);
#else
    connect(m_ui->m_requireTriggerWord, &QCheckBox::checkStateChanged, this, &SpellCheckConfig::markAsChanged);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(m_ui->m_requireTriggerWord, &QCheckBox::stateChanged, this, &SpellCheckConfig::toggleTriggerWord);
#else
    connect(m_ui->m_requireTriggerWord, &QCheckBox::checkStateChanged, this, &SpellCheckConfig::toggleTriggerWord);
#endif
    connect(m_ui->m_triggerWord, &QLineEdit::textChanged, this, &SpellCheckConfig::markAsChanged);
    connect(m_ui->m_openKcmButton, &QPushButton::clicked, this, &SpellCheckConfig::openKcm);

    m_ui->m_openKcmButton->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
}

SpellCheckConfig::~SpellCheckConfig()
{
}

void SpellCheckConfig::toggleTriggerWord(int state)
{
    m_ui->m_triggerWord->setEnabled(state == Qt::Checked);
}

void SpellCheckConfig::openKcm()
{
    QProcess::startDetached(QStringLiteral("kcmshell6"), {QStringLiteral("spellchecking")});
}

void SpellCheckConfig::load()
{
    KCModule::load();

    // FIXME: This shouldn't be hardcoded!
    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    const KConfigGroup grp = cfg->group("Runners").group(KRUNNER_PLUGIN_NAME);

    const bool requireTrigger = grp.readEntry("requireTriggerWord", true);
    const QString trigger = grp.readEntry("trigger", i18n("spell"));

    if (!requireTrigger) {
        m_ui->m_triggerWord->setEnabled(false);
    }

    m_ui->m_requireTriggerWord->setCheckState((requireTrigger) ? Qt::Checked : Qt::Unchecked);
    m_ui->m_triggerWord->setText(trigger);

    setNeedsSave(false);
}

void SpellCheckConfig::save()
{
    // FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners").group(KRUNNER_PLUGIN_NAME);

    bool requireTrigger = m_ui->m_requireTriggerWord->checkState() == Qt::Checked;
    if (requireTrigger) {
        grp.writeEntry("trigger", m_ui->m_triggerWord->text());
    }
    grp.writeEntry("requireTriggerWord", requireTrigger);
    grp.sync();

    setNeedsSave(false);
}

void SpellCheckConfig::defaults()
{
    m_ui->m_requireTriggerWord->setCheckState(Qt::Checked);
    m_ui->m_triggerWord->setText(i18n("spell"));
    setNeedsSave(true);
}

K_PLUGIN_CLASS(SpellCheckConfig)

#include "spellcheck_config.moc"
