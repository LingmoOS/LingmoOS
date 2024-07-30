/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGLOADER_H
#define KCONFIGLOADER_H

#include <QIODevice>

#include <kconfiggroup.h>
#include <kconfigskeleton.h>
#include <ksharedconfig.h>

#include <kconfiggui_export.h>

class ConfigLoaderPrivate;

/**
 * @class KConfigLoader kconfigloader.h <KConfigLoader>
 *
 * @short A KConfigSkeleton that populates itself based on KConfigXT XML
 *
 * This class allows one to ship an XML file and reconstitute it into a
 * KConfigSkeleton object at runtime. Common usage might look like this:
 *
 * \code
 * QFile file(xmlFilePath);
 * KConfigLoader appletConfig(configFilePath, &file);
 * \endcode
 *
 * Alternatively, any QIODevice may be used in place of QFile in the
 * example above.
 *
 * KConfigLoader is useful if it is not possible to use compiled code
 * and by that the kconfig compiler cannot be used. Common examples are
 * scripted plugins which want to provide a configuration interface.
 * With the help of KConfigLoader a dynamically loaded ui file can be
 * populated with the stored values and also stored back to the config
 * file.
 *
 * An example for populating a QDialog with a dynamically populated UI
 * with the help of a KConfigDialogManager:
 * \code
 * QDialog *dialog = new QDialog();
 * QFile xmlFile("path/to/kconfigxt.xml");
 * KConfigGroup cg = KSharedConfig::openConfig()->group(QString());
 * KConfigLoader *configLoader = new KConfigLoader(cg, &xmlFile, this);
 *
 * // load the ui file
 * QUiLoader *loader = new QUiLoader(this);
 * QFile uiFile("path/to/userinterface.ui");
 * uiFile.open(QFile::ReadOnly);
 * QWidget *customConfigForm = loader->load(&uiFile, dialog);
 * uiFile.close();
 *
 * KConfigDialogManager *manager = new KConfigDialogManager(customConfigForm, configLoader);
 * if (dialog->exec() == QDialog::Accepted) {
 *     manager->updateSettings();
 * }
 * \endcode
 *
 * Currently the following data types are supported:
 *
 * @li bools
 * @li colors
 * @li datetimes
 * @li enumerations
 * @li fonts
 * @li ints
 * @li passwords
 * @li paths
 * @li strings
 * @li stringlists
 * @li uints
 * @li urls
 * @li doubles
 * @li int lists
 * @li longlongs
 * @li path lists
 * @li points
 * @li pointfs
 * @li rects
 * @li rectfs
 * @li sizes
 * @li sizefs
 * @li ulonglongs
 * @li url lists
 **/
class KCONFIGGUI_EXPORT KConfigLoader : public KConfigSkeleton
{
public:
    /**
     * Creates a KConfigSkeleton populated using the definition found in
     * the XML data passed in.
     *
     * @param configFile path to the configuration file to use
     * @param xml the xml data; must be valid KConfigXT data
     * @param parent optional QObject parent
     **/
    KConfigLoader(const QString &configFile, QIODevice *xml, QObject *parent = nullptr);

    /**
     * Creates a KConfigSkeleton populated using the definition found in
     * the XML data passed in.
     *
     * @param config the configuration object to use
     * @param xml the xml data; must be valid KConfigXT data
     * @param parent optional QObject parent
     **/
    KConfigLoader(KSharedConfigPtr config, QIODevice *xml, QObject *parent = nullptr);

    /**
     * Creates a KConfigSkeleton populated using the definition found in
     * the XML data passed in.
     *
     * @param config the group to use as the root for configuration items
     * @param xml the xml data; must be valid KConfigXT data
     * @param parent optional QObject parent
     **/
    KConfigLoader(const KConfigGroup &config, QIODevice *xml, QObject *parent = nullptr);

    ~KConfigLoader() override;

    /**
     * Finds the item for the given group and key.
     *
     * @param group the group in the config file to look in
     * @param key the configuration key to find
     * @return the associated KConfigSkeletonItem, or @c nullptr if none
     */
    KConfigSkeletonItem *findItem(const QString &group, const QString &key) const;

    /**
     * Finds an item by its name
     */
    KConfigSkeletonItem *findItemByName(const QString &name) const;

    /**
     * Returns the property (variantized value) of the named item
     */
    QVariant property(const QString &name) const;

    /**
     * Check to see if a group exists
     *
     * @param group the name of the group to check for
     * @return true if the group exists, or false if it does not
     */
    bool hasGroup(const QString &group) const;

    /**
     * @return the list of groups defined by the XML
     */
    QStringList groupList() const;

protected:
    bool usrSave() override;

private:
    ConfigLoaderPrivate *const d;
};

#endif // multiple inclusion guard
