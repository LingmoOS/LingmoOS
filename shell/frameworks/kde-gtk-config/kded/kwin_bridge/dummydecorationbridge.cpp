/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QCoreApplication>
#include <QMouseEvent>

#include <KConfigGroup>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/Private/DecoratedClientPrivate>
#include <KDecoration2/Private/DecorationSettingsPrivate>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KSharedConfig>

#include "decorationpainter.h"
#include "dummydecoratedclient.h"
#include "dummydecorationbridge.h"
#include "dummydecorationsettings.h"

namespace KDecoration2
{
DummyDecorationBridge::DummyDecorationBridge(const QString &decorationTheme, QObject *parent)
    : DecorationBridge(parent)
    , m_decorationsConfigFileName()
    , m_factory()
    , m_decoration()
    , m_client()
{
    // HACK:
    // Some window decorations use button fade-in and fade-out animations.
    // These animations are very slow, and they are preventing this bridge
    // to correctly draw hover states of the buttons, when we pass a "hover"
    // event to them. To avoid this harmful side effect we use a hack:
    // We disable the animations via user configuration file temporary if
    // they were enabled, draw a buttons and then enable them again.
    if (decorationTheme == QStringLiteral("Oxygen")) {
        m_decorationsConfigFileName = QStringLiteral("oxygenrc");
    } else { // for Breeze window decorations and its forks
        m_decorationsConfigFileName = QStringLiteral("breezerc");
    }

    disableAnimations();

    const QString pluginPath = windowDecorationPluginPath(decorationTheme);
    m_pluginLoader.setFileName(pluginPath);
    m_factory = qobject_cast<KPluginFactory *>(m_pluginLoader.instance());
    if (m_factory) {
        const QVariantMap args({{QStringLiteral("bridge"), QVariant::fromValue(this)}});
        m_decoration = m_factory->create<KDecoration2::Decoration>(m_factory, QVariantList({args}));
    }

    auto decorationSettings = std::make_shared<KDecoration2::DecorationSettings>(this);
    m_decoration->setSettings(decorationSettings);
    m_decoration->init();

    // Update decoration settings, e.g. Breeze's "Draw a circle around close button"
    if (m_settings) {
        Q_EMIT m_settings->decorationSettings()->reconfigured();
    }
    enableAnimations();
}

DummyDecorationBridge::~DummyDecorationBridge()
{
    m_pluginLoader.unload();
}

std::unique_ptr<KDecoration2::DecorationSettingsPrivate> DummyDecorationBridge::settings(KDecoration2::DecorationSettings *parent)
{
    auto newSettings = std::unique_ptr<DummyDecorationSettings>(new DummyDecorationSettings(parent));
    m_settings = newSettings.get();
    return newSettings;
}

std::unique_ptr<KDecoration2::DecoratedClientPrivate> DummyDecorationBridge::createClient(KDecoration2::DecoratedClient *client,
                                                                                          KDecoration2::Decoration *decoration)
{
    auto ptr = std::unique_ptr<DummyDecoratedClient>(new DummyDecoratedClient(client, decoration));
    m_client = ptr.get();
    return ptr;
}

void DummyDecorationBridge::paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState)
{
    disableAnimations();
    std::unique_ptr<KDecoration2::DecorationButton> button{
        m_factory->create<KDecoration2::DecorationButton>(m_decoration,
                                                          QVariantList({
                                                              QVariant::fromValue(strToButtonType(buttonType)),
                                                              QVariant::fromValue(m_decoration),
                                                          }))};

    if (button == nullptr) {
        return;
    }

    button->setGeometry(DecorationPainter::ButtonGeometry);

    if (buttonType == QStringLiteral("maximized")) {
        // Different decorations use different ways to know if the window is maximized
        // For example Breeze uses 'checked' property, but Oxygen uses client's 'isMaximized' method
        button->setChecked(true);
        if (m_client) {
            dynamic_cast<DummyDecoratedClient *>(m_client)->setMaximized(true);
        }
    }

    if (buttonState.contains(QStringLiteral("active"))) {
        passMousePressEventToButton(button.get());
    } else if (buttonState.contains(QStringLiteral("hover"))) {
        passMouseHoverEventToButton(button.get());
    }

    if (buttonState.contains(QStringLiteral("backdrop"))) {
        if (m_client) {
            dynamic_cast<DummyDecoratedClient *>(m_client)->setActive(false);
        }
    } else {
        if (m_client) {
            dynamic_cast<DummyDecoratedClient *>(m_client)->setActive(true);
        }
    }

    button->paint(&painter, DecorationPainter::ButtonGeometry);
    enableAnimations();
}

void DummyDecorationBridge::disableAnimations()
{
    KSharedConfig::Ptr decorationConfig = KSharedConfig::openConfig(m_decorationsConfigFileName, KConfig::NoGlobals);
    if (decorationConfig) {
        KConfigGroup group = decorationConfig->group(QStringLiteral("Windeco"));
        group.writeEntry(QStringLiteral("AnimationsEnabled"), false, KConfig::WriteConfigFlags());
    }

    // In case decoration is using global animation settings
    KSharedConfig::Ptr globalConfig = KSharedConfig::openConfig();
    if (globalConfig) {
        auto group = globalConfig->group(QStringLiteral("KDE"));
        globalAnimationEntryValue = group.readEntry(QStringLiteral("AnimationDurationFactor"), 1.0);
        group.writeEntry(QStringLiteral("AnimationDurationFactor"), 0, KConfig::WriteConfigFlags());
    }
}

void DummyDecorationBridge::enableAnimations()
{
    KSharedConfig::Ptr globalConfig = KSharedConfig::openConfig();
    if (globalConfig) {
        auto group = globalConfig->group(QStringLiteral("KDE"));
        group.writeEntry(QStringLiteral("AnimationDurationFactor"), globalAnimationEntryValue, KConfig::WriteConfigFlags());
    }
}

QString DummyDecorationBridge::windowDecorationPluginPath(const QString &decorationTheme) const
{
    const auto decorationPlugins = KPluginMetaData::findPlugins(QStringLiteral("org.kde.kdecoration2"));

    QString defaultPluginPath;

    for (const auto &pluginMetaData : decorationPlugins) {
        if (pluginMetaData.pluginId() == QLatin1String("org.kde.breeze")) {
            defaultPluginPath = pluginMetaData.fileName();
        }

        if (pluginMetaData.name() == decorationTheme) {
            return pluginMetaData.fileName();
        }
    }
    return defaultPluginPath;
}

void DummyDecorationBridge::passMouseHoverEventToButton(KDecoration2::DecorationButton *button) const
{
    QHoverEvent event{QEvent::HoverEnter,
                      {
                          DecorationPainter::ButtonGeometry.width() / 2.0,
                          DecorationPainter::ButtonGeometry.height() / 2.0,
                      },
                      {
                          (DecorationPainter::ButtonGeometry.width() / 2.0) - 1,
                          (DecorationPainter::ButtonGeometry.height() / 2.0) - 1,
                      },
                      Qt::NoModifier};
    QCoreApplication::instance()->sendEvent(button, &event);
}

void DummyDecorationBridge::passMousePressEventToButton(KDecoration2::DecorationButton *button) const
{
    QMouseEvent event{QEvent::MouseButtonPress,
                      {
                          DecorationPainter::ButtonGeometry.width() / 2.0,
                          DecorationPainter::ButtonGeometry.height() / 2.0,
                      },
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier};
    QCoreApplication::instance()->sendEvent(button, &event);
}

KDecoration2::DecorationButtonType DummyDecorationBridge::strToButtonType(const QString &type) const
{
    if (type == QStringLiteral("minimize")) {
        return KDecoration2::DecorationButtonType::Minimize;
    } else if (type == QStringLiteral("close")) {
        return KDecoration2::DecorationButtonType::Close;
    } else {
        return KDecoration2::DecorationButtonType::Maximize;
    }
}

}

#include "moc_dummydecorationbridge.cpp"
