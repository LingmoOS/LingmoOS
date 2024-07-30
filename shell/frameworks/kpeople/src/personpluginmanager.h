/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2018 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSON_PLUGIN_MANAGER_H
#define PERSON_PLUGIN_MANAGER_H

#include <kpeople/kpeople_export.h>

#include <QHash>
#include <QVariant>

namespace KPeople
{
class AbstractPersonAction;
class BasePersonsDataSource;

/**
  This class allows applications to manage DataSource plugins

  It can be useful if an application wants to use custom DataSource,
  without exposing it to other KPeople-based applications
  (i.e. without installing a system-wide plugin)

  Another use-case is that it allows to pass custom arguments to DataSources
  (i.e. an ItemModel which will be used as a source of data)

  @since 5.51
 */
class KPEOPLE_EXPORT PersonPluginManager
{
public:
    /**
     * Use this if you explicitly don't want KPeople to autoload all the
     * available data source plugins.
     *
     * The default behavior is to autoload them
     */
    static void setAutoloadDataSourcePlugins(bool autoloadDataSourcePlugins);
    static QList<BasePersonsDataSource *> dataSourcePlugins();
    /**
     * Adds custom data source. If DataSource with such @p sourceId was already loaded, we override it
     *
     * Takes ownership of the @p source
     */
    static void addDataSource(const QString &sourceId, BasePersonsDataSource *source);
    static BasePersonsDataSource *dataSource(const QString &sourceId);

    /**
     * Instead of loading datasources from plugins, set sources manually
     * This is for unit tests only
     */
    static void setDataSourcePlugins(const QHash<QString, BasePersonsDataSource *> &dataSources);

    /**
     * Creates a contact with the specified @p properties
     * @returns if it could be done successfully
     *
     * @since 5.62
     */
    static bool addContact(const QVariantMap &properties);

    /**
     * Deletes a contact with the specified &p uri
     * @returns if it could be done successfully
     *
     * @since 5.62
     */
    static bool deleteContact(const QString &uri);
};
}

#endif // PERSON_PLUGIN_MANAGER_H
