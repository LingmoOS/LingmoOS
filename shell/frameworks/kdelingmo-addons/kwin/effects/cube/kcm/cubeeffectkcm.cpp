/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "cubeeffectkcm.h"

#include <KActionCollection>
#include <KConfigGroup>
#include <KConfigLoader>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileDialog>

K_PLUGIN_CLASS(CubeEffectConfig)

CubeEffectConfig::CubeEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());

    QFile xmlFile(QStringLiteral(":/main.xml"));
    KConfigGroup cg = KSharedConfig::openConfig(QStringLiteral("kwinrc"))->group("Effect-cube");
    m_configLoader = new KConfigLoader(cg, &xmlFile, this);
    addConfig(m_configLoader, widget());

    auto actionCollection = new KActionCollection(this, QStringLiteral("kwin"));
    actionCollection->setComponentDisplayName(i18n("KWin"));
    actionCollection->setConfigGroup(QStringLiteral("Cube"));
    actionCollection->setConfigGlobal(true);

    const QKeySequence defaultToggleShortcut = Qt::META | Qt::Key_C;
    QAction *toggleAction = actionCollection->addAction(QStringLiteral("Cube"));
    toggleAction->setText(i18n("Toggle Cube"));
    toggleAction->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(toggleAction, {defaultToggleShortcut});
    KGlobalAccel::self()->setShortcut(toggleAction, {defaultToggleShortcut});

    ui.shortcutsEditor->addCollection(actionCollection);
    connect(ui.shortcutsEditor, &KShortcutsEditor::keyChange, this, &CubeEffectConfig::markAsChanged);

    connect(ui.button_SelectSkyBox, &QPushButton::clicked, this, [this]() {
        auto dialog = new QFileDialog(widget());
        dialog->setFileMode(QFileDialog::ExistingFile);
        connect(dialog, &QFileDialog::fileSelected, ui.kcfg_SkyBox, &QLineEdit::setText);
        dialog->open();
    });

    connect(ui.button_Color, &QPushButton::toggled, this, &CubeEffectConfig::updateBackgroundFromUi);
    connect(ui.button_SkyBox, &QPushButton::toggled, this, &CubeEffectConfig::updateBackgroundFromUi);
    connect(ui.slider_DistanceFactor, &QSlider::sliderMoved, this, &CubeEffectConfig::updateDistanceFactorFromUi);
}

void CubeEffectConfig::load()
{
    KCModule::load();
    updateUiFromConfig();
    updateUnmanagedState();
}

void CubeEffectConfig::save()
{
    ui.shortcutsEditor->save();
    m_configLoader->save();

    KCModule::save();
    updateUnmanagedState();

    QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                     QStringLiteral("/Effects"),
                                                                     QStringLiteral("org.kde.kwin.Effects"),
                                                                     QStringLiteral("reconfigureEffect"));
    reconfigureMessage.setArguments({QStringLiteral("cube")});
    QDBusConnection::sessionBus().call(reconfigureMessage);
}

void CubeEffectConfig::defaults()
{
    KCModule::defaults();
    updateUiFromDefaultConfig();
    updateUnmanagedState();
}

void CubeEffectConfig::updateUiFromConfig()
{
    setUiBackground(m_configLoader->findItemByName(QStringLiteral("Background"))->property().toInt());
    setDistanceFactor(m_configLoader->findItemByName(QStringLiteral("DistanceFactor"))->property().toDouble());
}

void CubeEffectConfig::updateUiFromDefaultConfig()
{
    setUiBackground(m_configLoader->findItemByName(QStringLiteral("Background"))->property().toInt());
    setDistanceFactor(m_configLoader->findItemByName(QStringLiteral("DistanceFactor"))->property().toDouble());
    ui.shortcutsEditor->allDefault();
}

int CubeEffectConfig::uiBackground() const
{
    if (ui.button_SkyBox->isChecked()) {
        return 1;
    } else {
        return 0;
    }
}

void CubeEffectConfig::setUiBackground(int mode)
{
    switch (mode) {
    case 1:
        ui.button_SkyBox->setChecked(true);
        break;
    case 0:
    default:
        ui.button_Color->setChecked(true);
        break;
    }
}

void CubeEffectConfig::updateBackgroundFromUi()
{
    m_configLoader->findItemByName(QStringLiteral("Background"))->setProperty(uiBackground());
    updateUnmanagedState();
}

qreal CubeEffectConfig::distanceFactor() const
{
    return ui.slider_DistanceFactor->value() / 100.0;
}

void CubeEffectConfig::setDistanceFactor(qreal factor)
{
    ui.slider_DistanceFactor->setValue(std::round(factor * 100));
}

void CubeEffectConfig::updateDistanceFactorFromUi()
{
    m_configLoader->findItemByName(QStringLiteral("DistanceFactor"))->setProperty(distanceFactor());
    updateUnmanagedState();
}

void CubeEffectConfig::updateUnmanagedState()
{
    const auto backgroundItem = m_configLoader->findItemByName(QStringLiteral("Background"));
    const auto distanceFactorItem = m_configLoader->findItemByName(QStringLiteral("DistanceFactor"));

    unmanagedWidgetChangeState(backgroundItem->isSaveNeeded() || distanceFactorItem->isSaveNeeded());
    unmanagedWidgetDefaultState(backgroundItem->isDefault() || distanceFactorItem->isDefault());
}

#include "cubeeffectkcm.moc"
