/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QPluginLoader>
#include <QString>

#include <KDecoration2/Decoration>
#include <KDecoration2/Private/DecorationBridge>
#include <kdecoration2/decorationdefines.h>


class KPluginFactory;

namespace KDecoration2
{
class DecorationSettings;
class DecoratedClientPrivate;
class DecorationSettingsPrivate;
class DummyDecorationSettings;

class DummyDecorationBridge : public DecorationBridge
{
    Q_OBJECT
public:
    DummyDecorationBridge(const QString &decorationTheme, QObject *parent = nullptr);
    ~DummyDecorationBridge() override;

    std::unique_ptr<KDecoration2::DecorationSettingsPrivate> settings(KDecoration2::DecorationSettings *parent) override;
    std::unique_ptr<KDecoration2::DecoratedClientPrivate> createClient(KDecoration2::DecoratedClient *client, KDecoration2::Decoration *decoration) override;

    void paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState);

private:
    void disableAnimations();
    void enableAnimations();

    QString windowDecorationPluginPath(const QString &decorationTheme) const;

    void passMouseHoverEventToButton(KDecoration2::DecorationButton *button) const;
    void passMousePressEventToButton(KDecoration2::DecorationButton *button) const;

    KDecoration2::DecorationButtonType strToButtonType(const QString &type) const;

    QString m_decorationsConfigFileName;
    double globalAnimationEntryValue;

    QPluginLoader m_pluginLoader;
    KPluginFactory *m_factory;
    KDecoration2::Decoration *m_decoration;
    KDecoration2::DecoratedClientPrivate *m_client;
    KDecoration2::DummyDecorationSettings *m_settings;
};

}
