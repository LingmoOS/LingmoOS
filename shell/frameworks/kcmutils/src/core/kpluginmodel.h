/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMODEL_H
#define KPLUGINMODEL_H

#include "kcmutilscore_export.h"

#include <QAbstractListModel>
#include <QList>

#include <KPluginMetaData>
#include <memory>

class KConfigGroup;
class KPluginModelPrivate;

/**
 * @class KPluginModel kpluginmodel.h KPluginModel
 * A model that provides a list of available plugins and allows to disable/enable them.
 *
 * Plugins need to define the @c X-KDE-ConfigModule property for their config modules to be found.
 * The value for this property is the namespace and file name of the KCM for the plugin.
 * An example value is "kf6/krunner/kcms/kcm_krunner_charrunner", "kf6/krunner/kcms" is the namespace
 * and "kcm_krunner_charrunner" the file name. The loaded KCMs don't need any embedded JSON metadata.
 *
 * @see KPluginWidget
 *
 * @since 5.94
 */
class KCMUTILSCORE_EXPORT KPluginModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        IconRole = Qt::DecorationRole,
        EnabledRole = Qt::CheckStateRole,
        DescriptionRole = Qt::UserRole + 1,
        IsChangeableRole,
        MetaDataRole,
        ConfigRole,
        IdRole,
        EnabledByDefaultRole,
        SortableRole, /// @internal
    };

    explicit KPluginModel(QObject *parent = nullptr);
    ~KPluginModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    /**
     * Append plugins to the model. This will not replace existing entries.
     *
     * @param plugins the plugins to be added.
     * @param categoryLabel a user-visible label for the section the plugins are added to.
     *
     */
    void addPlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel);

    /**
     * Add plugins that should not be sorted automatically based on their name
     * This is useful in case your app has a custom sorting mechanism or implements reordering of plugins
     *
     * @since 6.0
     */
    void addUnsortablePlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel);

    /// @since 6.0
    void removePlugin(const KPluginMetaData &data);

    /**
     * Removes all plugins.
     */
    void clear();

    /**
     * Set the KConfigGroup that is used to load/save the enabled state.
     */
    void setConfig(const KConfigGroup &config);

    /**
     * Save the enabled state of the plugins to the config group set by @ref setConfig.
     */
    void save();

    /**
     * Load the enabled state of the plugins from the config group set by @ref setConfig.
     */
    void load();

    /**
     * Reset the enabled state of the plugins to its defaults.
     */
    void defaults();

    /**
     * Whether or not there are unsaved changes to the enabled state of the plugins.
     */
    bool isSaveNeeded();

    /**
     * Returns the KPluginMetaData object of the plugin's config module. If no plugin is found or the plugin does not have a config, the resulting
     * KPluginMetaData object will be invalid.
     * @since 5.94
     */
    KPluginMetaData findConfigForPluginId(const QString &pluginId) const;

    /**
     * Emitted when the enabled state matches the default changes.
     *
     * @see defaults
     */
    Q_SIGNAL void defaulted(bool isDefaulted);

    /**
     * Emitted when @ref isSaveNeeded is changed.
     */
    Q_SIGNAL void isSaveNeededChanged();

private:
    const std::unique_ptr<KPluginModelPrivate> d;
    friend class KPluginProxyModel;
    QStringList getOrderedCategoryLabels();
};
#endif
