/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "decorationbutton.h"
#include <kdecoration2/kdecoration2_export.h>

#include <QFontMetricsF>
#include <QObject>

#include <memory>

namespace KDecoration2
{
class DecorationBridge;
class DecorationSettingsPrivate;

/**
 * @brief Common settings for the Decoration.
 *
 * This class gets injected into the Decoration and provides recommendations for the
 * Decoration. The Decoration is suggested to honor the settings, but may decide that some
 * settings don't fit the design and ignore them.
 *
 * @see Decoration
 **/
class KDECORATIONS2_EXPORT DecorationSettings : public QObject
{
    Q_OBJECT
    /**
     * Whether the feature to put a DecoratedClient on all desktops is available.
     *
     * If this feature is not available a Decoration might decide to not show the
     * DecorationButtonType::OnAllDesktops.
     **/
    Q_PROPERTY(bool onAllDesktopsAvailable READ isOnAllDesktopsAvailable NOTIFY onAllDesktopsAvailableChanged)
    /**
     * Whether the Decoration will be rendered with an alpha channel.
     *
     * If no alpha channel is available a Decoration should not use round borders.
     **/
    Q_PROPERTY(bool alphaChannelSupported READ isAlphaChannelSupported NOTIFY alphaChannelSupportedChanged)
    /**
     * Whether the Decoration should close the DecoratedClient when double clicking on the
     * DecorationButtonType::Menu.
     **/
    Q_PROPERTY(bool closeOnDoubleClickOnMenu READ isCloseOnDoubleClickOnMenu NOTIFY closeOnDoubleClickOnMenuChanged)
    /**
     * The suggested ordering of the decoration buttons on the left.
     **/
    Q_PROPERTY(QList<KDecoration2::DecorationButtonType> decorationButtonsLeft READ decorationButtonsLeft NOTIFY decorationButtonsLeftChanged)
    /**
     * The suggested ordering of the decoration buttons on the right.
     **/
    Q_PROPERTY(QList<KDecoration2::DecorationButtonType> decorationButtonsRight READ decorationButtonsRight NOTIFY decorationButtonsRightChanged)
    /**
     * The suggested border size.
     **/
    Q_PROPERTY(KDecoration2::BorderSize borderSize READ borderSize NOTIFY borderSizeChanged)
    /**
     * The fundamental unit of space that should be used for sizes, expressed in pixels.
     * Given the screen has an accurate DPI settings, it corresponds to a millimeter
     */
    Q_PROPERTY(int gridUnit READ gridUnit NOTIFY gridUnitChanged)
    /**
     * The recommended font for the Decoration's caption.
     **/
    Q_PROPERTY(QFont font READ font NOTIFY fontChanged)
    /**
     * smallSpacing is the amount of spacing that should be used around smaller UI elements,
     * for example as spacing in Columns. Internally, this size depends on the size of
     * the default font as rendered on the screen, so it takes user-configured font size and DPI
     * into account.
     */
    Q_PROPERTY(int smallSpacing READ smallSpacing NOTIFY spacingChanged)

    /**
     * largeSpacing is the amount of spacing that should be used inside bigger UI elements,
     * for example between an icon and the corresponding text. Internally, this size depends on
     * the size of the default font as rendered on the screen, so it takes user-configured font
     * size and DPI into account.
     */
    Q_PROPERTY(int largeSpacing READ largeSpacing NOTIFY spacingChanged)
public:
    explicit DecorationSettings(DecorationBridge *bridge, QObject *parent = nullptr);
    ~DecorationSettings() override;
    bool isOnAllDesktopsAvailable() const;
    bool isAlphaChannelSupported() const;
    bool isCloseOnDoubleClickOnMenu() const;
    QList<DecorationButtonType> decorationButtonsLeft() const;
    QList<DecorationButtonType> decorationButtonsRight() const;
    BorderSize borderSize() const;

    QFont font() const;
    /**
     * The fontMetrics for the recommended font.
     * @see font
     **/
    QFontMetricsF fontMetrics() const;

    int gridUnit() const;
    int smallSpacing() const;
    int largeSpacing() const;

Q_SIGNALS:
    void onAllDesktopsAvailableChanged(bool);
    void alphaChannelSupportedChanged(bool);
    void closeOnDoubleClickOnMenuChanged(bool);
    void decorationButtonsLeftChanged(const QList<KDecoration2::DecorationButtonType> &);
    void decorationButtonsRightChanged(const QList<KDecoration2::DecorationButtonType> &);
    void borderSizeChanged(KDecoration2::BorderSize size);
    void fontChanged(const QFont &font);
    void gridUnitChanged(int);
    void spacingChanged();

    /**
     * This signal is emitted when the backend got reconfigured.
     * If the plugin uses custom settings, it is recommended to re-read
     * them after this signal got emitted.
     **/
    void reconfigured();

private:
    const std::unique_ptr<DecorationSettingsPrivate> d;
};

}

Q_DECLARE_METATYPE(KDecoration2::BorderSize)
