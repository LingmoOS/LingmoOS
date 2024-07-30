/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGDIALOGMANAGER_H
#define KCONFIGDIALOGMANAGER_H

#include <kconfigwidgets_export.h>

#include <QHash>
#include <QObject>
#include <memory>

class KConfigDialogManagerPrivate;

class KCoreConfigSkeleton;
class KConfigSkeleton;
class KConfigSkeletonItem;
class QWidget;

/**
 * @class KConfigDialogManager kconfigdialogmanager.h KConfigDialogManager
 *
 * @short Provides a means of automatically retrieving,
 * saving and resetting KConfigSkeleton based settings in a dialog.
 *
 * The KConfigDialogManager class provides a means of automatically
 * retrieving, saving and resetting basic settings.
 * It also can emit signals when settings have been changed
 * (settings were saved) or modified (the user changes a checkbox
 * from on to off).
 *
 * The object names of the widgets to be managed have to correspond to the names of the
 * configuration entries in the KConfigSkeleton object plus an additional
 * "kcfg_" prefix. For example a widget with the object name "kcfg_MyOption"
 * would be associated to the configuration entry "MyOption".
 *
 * The widget classes of Qt and KDE Frameworks are supported out of the box,
 * for other widgets see below:
 *
 * @par Using Custom Widgets
 * @parblock
 * Custom widget classes are supported if they have a Q_PROPERTY defined for the
 * property representing the value edited by the widget. By default the property
 * is used for which "USER true" is set. For using another property, see below.
 *
 * Example:
 *
 * A class ColorEditWidget is used in the settings UI to select a color. The
 * color value is set and read as type QColor. For that it has a definition of
 * the value property similar to this:
 * \code
 * Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
 * \endcode
 * And of course it has the definition and implementation of the respective
 * read & write methods and the notify signal.
 * This class then can be used directly with KConfigDialogManager and does not need
 * further setup.
 * @endparblock
 *
 * @par Using Other Properties than The USER Property
 * @parblock
 * To use a widget's property that is not the USER property, the property to use
 * can be selected by setting onto the widget instance a property with the key
 * "kcfg_property" and as the value the name of the property:
 * \code
 * ColorEditWidget *myWidget = new ColorEditWidget;
 * myWidget->setProperty("kcfg_property", QByteArray("redColorPart"));
 * \endcode
 * This selection of the property to use is just valid for this widget instance.
 * When using a UI file, the "kcfg_property" property can also be set using Qt Designer.
 * @endparblock
 *
 * @par Configuring Classes to use Other Properties Globally
 * @parblock
 * Alternatively a non-USER property can be defined for a widget class globally
 * by registering it for the class in the KConfigDialogManager::propertyMap().
 * This global registration has lower priority than any "kcfg_property" property
 * set on a class instance though, so the latter overrules this global setting.
 * Note: setting the property in the propertyMap affects any instances of that
 * widget class in the current application, so use only when needed and prefer
 * instead the "kcfg_property" property. Especially with software with many
 * libraries and 3rd-party plugins in one process there is a chance of
 * conflicting settings.
 *
 * Example:
 *
 * If the ColorEditWidget has another property redColor defined by
 * \code
 * Q_PROPERTY(int redColorPart READ redColorPart WRITE setRedColorPart NOTIFY redColorPartChanged)
 * \endcode
 * and this one should be used in the settings, call somewhere in the code before
 * using the settings:
 * \code
 * KConfigDialogManager::propertyMap()->insert("ColorEditWidget", QByteArray("redColorPart"));
 * \endcode
 * @endparblock
 *
 * @par Using Different Signals than The NOTIFY Signal
 * @parblock
 * If some non-default signal should be used, e.g. because the property to use does not
 * have a NOTIFY setting, for a given widget instance the signal to use can be set
 * by a property with the key "kcfg_propertyNotify" and as the value the signal signature.
 * This will take priority over the signal noted by NOTIFY for the chosen property
 * as well as the content of KConfigDialogManager::changedMap(). Since 5.32.
 *
 * Example:
 *
 * If for a class OtherColorEditWidget there was no NOTIFY set on the USER property,
 * but some signal colorSelected(QColor) defined which would be good enough to reflect
 * the settings change, defined by
 * \code
 * Q_PROPERTY(QColor color READ color WRITE setColor USER true)
 * Q_SIGNALS:
 *     void colorSelected(const QColor &color);
 * \endcode
 * the signal to use would be defined by this:
 * \code
 * OtherColorEditWidget *myWidget = new OtherColorEditWidget;
 * myWidget->setProperty("kcfg_propertyNotify", QByteArray(SIGNAL(colorSelected(QColor))));
 * \endcode
 * @endparblock
 *
 * @author Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
 * @author Waldo Bastian <bastian@kde.org>
 */
class KCONFIGWIDGETS_EXPORT KConfigDialogManager : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    /**
     * One or more of the settings have been saved (such as when the user
     * clicks on the Apply button).  This is only emitted by updateSettings()
     * whenever one or more setting were changed and consequently saved.
     */
    void settingsChanged(); // clazy:exclude=overloaded-signal

    /**
     * If retrieveSettings() was told to track changes then if
     * any known setting was changed this signal will be emitted.  Note
     * that a settings can be modified several times and might go back to the
     * original saved state. hasChanged() will tell you if anything has
     * actually changed from the saved values.
     */
    void widgetModified();

public:
    /**
     * Constructor.
     * @param parent  Dialog widget to manage
     * @param conf Object that contains settings
     */
    KConfigDialogManager(QWidget *parent, KCoreConfigSkeleton *conf);

    /**
     * Destructor.
     */
    ~KConfigDialogManager() override;

    /**
     * Add additional widgets to manage
     * @param widget Additional widget to manage, including all its children
     */
    void addWidget(QWidget *widget);

    /**
     * Returns whether the current state of the known widgets are
     * different from the state in the config object.
     */
    bool hasChanged() const;

    /**
     * Returns whether the current state of the known widgets are
     * the same as the default state in the config object.
     */
    bool isDefault() const;

    /**
     * Retrieve the map between widgets class names and the
     * USER properties used for the configuration values.
     */
    static QHash<QString, QByteArray> *propertyMap();

public Q_SLOTS:
    /**
     * Traverse the specified widgets, saving the settings of all known
     * widgets in the settings object.
     *
     * Example use: User clicks Ok or Apply button in a configure dialog.
     */
    void updateSettings();

    /**
     * Traverse the specified widgets, sets the state of all known
     * widgets according to the state in the settings object.
     *
     * Example use: Initialisation of dialog.
     * Example use: User clicks Reset button in a configure dialog.
     */
    void updateWidgets();

    /**
     * Traverse the specified widgets, sets the state of all known
     * widgets according to the default state in the settings object.
     *
     * Example use: User clicks Defaults button in a configure dialog.
     */
    void updateWidgetsDefault();

    /**
     * Show or hide an indicator when settings have changed from their default value.
     * Update all widgets to show or hide the indicator accordingly.
     *
     * @since 5.73
     */
    void setDefaultsIndicatorsVisible(bool enabled);

protected:
    /**
     * @param trackChanges - If any changes by the widgets should be tracked
     * set true.  This causes the emitting the modified() signal when
     * something changes.
     * TODO: @return bool - True if any setting was changed from the default.
     */
    void init(bool trackChanges);

    /**
     * Recursive function that finds all known children.
     * Goes through the children of widget and if any are known and not being
     * ignored, stores them in currentGroup.  Also checks if the widget
     * should be disabled because it is set immutable.
     * @param widget - Parent of the children to look at.
     * @param trackChanges - If true then tracks any changes to the children of
     * widget that are known.
     * @return bool - If a widget was set to something other than its default.
     */
    bool parseChildren(const QWidget *widget, bool trackChanges);

    /**
     * Finds the USER property name using Qt's MetaProperty system, and caches
     * it in the property map (the cache could be retrieved by propertyMap() ).
     */
    QByteArray getUserProperty(const QWidget *widget) const;

    /**
     * Find the property to use for a widget by querying the "kcfg_property"
     * property of the widget. Like a widget can use a property other than the
     * USER property.
     * @since 4.3
     */
    QByteArray getCustomProperty(const QWidget *widget) const;

    /**
     * Finds the changed signal of the USER property using Qt's MetaProperty system.
     * @since 5.32
     */
    QByteArray getUserPropertyChangedSignal(const QWidget *widget) const;

    /**
     * Find the changed signal of the property to use for a widget by querying
     * the "kcfg_propertyNotify" property of the widget. Like a widget can use a
     * property change signal other than the one for USER property, if there even is one.
     * @since 5.32
     */
    QByteArray getCustomPropertyChangedSignal(const QWidget *widget) const;

    /**
     * Set a property
     */
    void setProperty(QWidget *w, const QVariant &v);

    /**
     * Retrieve a property
     */
    QVariant property(QWidget *w) const;

    /**
     * Setup secondary widget properties
     */
    void setupWidget(QWidget *widget, KConfigSkeletonItem *item);

    /**
     * Initializes the property maps
     */
    static void initMaps();

private:
    /**
     * KConfigDialogManager KConfigDialogManagerPrivate class.
     */
    std::unique_ptr<KConfigDialogManagerPrivate> const d;
    friend class KConfigDialogManagerPrivate;

    Q_DISABLE_COPY(KConfigDialogManager)
    Q_PRIVATE_SLOT(d, void onWidgetModified())
};

#endif // KCONFIGDIALOGMANAGER_H
