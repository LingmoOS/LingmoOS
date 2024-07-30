/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINWIDGET_H
#define KPLUGINWIDGET_H

#include <QList>
#include <QWidget>

#include <KPluginMetaData>
#include <KSharedConfig>
#include <kcmutils_export.h>

#include <memory>

class QPushButton;
class KPluginWidgetPrivate;

/**
 * @class KPluginWidget kpluginwidget.h KPluginWidget
 * A widget that shows a list of available plugins and allows to disable/enable them and open their configuration UI.
 *
 * Plugins that get added to the KPluginWidget need to define the @c X-KDE-ConfigModule property.
 * The value for this property is the namespace and file name of the KCM for the plugin.
 * An example value is "kf6/krunner/kcms/kcm_krunner_charrunner", "kf6/krunner/kcms" is the namespace
 * and "kcm_krunner_charrunner" the file name. The loaded KCMs don't need any embedded json metadata.
 * @since 5.89
 */
class KCMUTILS_EXPORT KPluginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KPluginWidget(QWidget *parent = nullptr);

    ~KPluginWidget();

    /**
     * Adds the plugins with the given label to the widget
     */
    void addPlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel);

    /**
     * Set the config object that will be used to store the enabled state of the plugins.
     * When porting away from KPluginSelector, the "Plugins" group from the config root should
     * be used. For example:
     * @code
     * KSharedConfig::openConfig(QStringLiteral("krunnerrc"))->group("Plugins")
     * @endcode
     */
    void setConfig(const KConfigGroup &config);

    /**
     * Clears all the added plugins and any unsaved changes.
     */
    void clear();

    /**
     * Saves the changes to the config set by @ref setConfig.
     */
    void save();

    /**
     * Loads the enabled state of the plugins from the config set by setConfig()
     * and clears any changes by the user.
     * @since 5.91
     */
    void load();

    /**
     * Resets the enabled state of the plugins to their defaults
     * @see KPluginMetaData::isEnabledByDefault
     */
    void defaults();

    /**
     * Returns @c true if the enabled state of each plugin is the same as that plugin's default state.
     */
    bool isDefault() const;

    /**
     * Returns true if the plugin selector has any changes that are not yet saved to configuration.
     * @see save()
     */
    bool isSaveNeeded() const;

    /**
     * Sets the @p arguments with which the configuration modules will be initialized
     */
    void setConfigurationArguments(const QVariantList &arguments);

    /**
     * Returns the configuration arguments that will be used
     */
    QVariantList configurationArguments() const;

    /**
     * Shows the configuration dialog for the plugin @p pluginId if it's available
     */
    void showConfiguration(const QString &pluginId);

    /**
     * Shows an indicator when a plugin status is different from default
     */
    void setDefaultsIndicatorsVisible(bool isVisible);

    /**
     * Add additional widgets to each row of the plugin selector
     * @param handler returns the additional button that should be displayed in the row;
     * the handler can return a null pointer if no button should be displayed
     */
    void setAdditionalButtonHandler(const std::function<QPushButton *(const KPluginMetaData &)> &handler);

Q_SIGNALS:
    /**
     * Emitted when any of the plugins are changed.
     * @param pluginId id of the changed plugin
     * @param enabled if the given plugin is currently enabled or disabled
     */
    void pluginEnabledChanged(const QString &pluginId, bool enabled);

    /**
     * Emitted when any of the plugins are changed.
     * @param changed if the KPluginWidget object contains changes
     * @see needsSave
     */
    void changed(bool enabled);

    /**
     * Emitted after the config of an embedded KCM has been saved. The
     * argument is the name of the parent component that needs to reload
     * its config.
     */
    void pluginConfigSaved(const QString &pluginId);

    /**
     * Emitted after configuration is changed.
     *
     * @p isDefault @c true if the configuration state is the default, @c false otherwise
     */
    void defaulted(bool isDefault);

private:
    std::unique_ptr<KPluginWidgetPrivate> const d;
};

#endif
