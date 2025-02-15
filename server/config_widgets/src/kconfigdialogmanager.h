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
 * further setup. For supporting also KDE Frameworks versions older than 5.32 see
 * below for how to register the property change signal.
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
 * @par Supporting Older Versions of KDE Frameworks
 * @parblock
 * Before version 5.32 of KDE Frameworks, the signal notifying about a change
 * of the property value in the widget had to be manually registered for any
 * custom widget, using KConfigDialogManager::changedMap(). The same also had
 * to be done for custom signals with widgets from Qt and KDE Frameworks.
 * So for code which needs to also work with older versions of the KDE Frameworks,
 * this still needs to be done.
 * Starting with version 5.32, where the new signal handling is effective, the
 * signal registered via KConfigDialogManager::changedMap() will take precedence over
 * the one read from the Q_PROPERTY declaration, but is overridden for a given
 * widget instance by the "kcfg_propertyNotify" property.
 *
 * Examples:
 *
 * For the class ColorEditWidget from the previous example this will register
 * the change signal as needed:
 * \code
 * KConfigDialogManager::changedMap()->insert("ColorEditWidget", QByteArray(SIGNAL(colorChanged(QColor))));
 * \endcode
 * For KDE Framework versions starting with 5.32 this will override then the signal
 * as read from the USER property, but as it is the same signal, nothing will break.
 *
 * If wants to reduce conflicts and also only add code to the build as needed,
 * one would add both a buildtime switch and a runtime switch like
 * \code
 * #include <kconfigwidgets_version.h>
 * #include <kcoreaddons.h>
 * // [...]
 * #if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5,32,0)
 * if (KCoreAddons::version() < QT_VERSION_CHECK(5,32,0)) {
 *     KConfigDialogManager::changedMap()->insert("ColorEditWidget", QByteArray(SIGNAL(colorChanged(QColor))));
 * }
 * #endif
 * \endcode
 * so support for the old variant would be only used when running against an older
 * KDE Frameworks, and this again only built in if also compiled against an older version.
 * Note: KCoreAddons::version() needs at least KDE Frameworks 5.20 though.
 *
 * For the class OtherColorEditWidget from the previous example for the support of
 * also older KDE Frameworks versions the change signal would be registered by this:
 * \code
 * KConfigDialogManager::changedMap()->insert("OtherColorEditWidget", QByteArray(SIGNAL(colorSelected(QColor))));
 * OtherColorEditWidget *myWidget = new OtherColorEditWidget;
 * myWidget->setProperty("kcfg_propertyNotify", QByteArray(SIGNAL(colorSelected(QColor))));
 * \endcode
 * Here for KDE Framework versions before 5.32 the "kcfg_propertyNotify" property would
 * be ignored and the signal taken from KConfigDialogManager::changedMap(), while
 * for newer versions it is taken from that property, which then overrides the latter.
 * But as it is the same signal, nothing will break.
 *
 * Again, using KConfigDialogManager::changedMap could be made to depend on the version,
 * so for newer versions any global conflicts are avoided:
 * \code
 * #include <kconfigwidgets_version.h>
 * #include <kcoreaddons.h>
 * // [...]
 * #if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5,32,0)
 * if (KCoreAddons::version() < QT_VERSION_CHECK(5,32,0)) {
 *     KConfigDialogManager::changedMap()->insert("OtherColorEditWidget", QByteArray(SIGNAL(colorSelected(QColor))));
 * }
 * #endif
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

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 82)
    /**
     * TODO: Verify
     * One or more of the settings have been changed.
     * @param widget - The widget group (pass in via addWidget()) that
     * contains the one or more modified setting.
     * @see settingsChanged()
     *
     * @deprecated since 5.82, use the KConfigDialogManager::settingsChanged() signal instead.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 82, "Use the KConfigDialogManager::settingsChanged() signal instead.")
    void settingsChanged(QWidget *widget); // clazy:exclude=overloaded-signal
#endif

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

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 84)
    // No deprecation warning by compiler here, as the replacement will be
    // automatically picked by the compiler in the future, being the method
    // overload using the base-class of the argument type.
    // Avoids the need to do extra-casting right now on the caller side.
    /**
     * Constructor.
     * @param parent  Dialog widget to manage
     * @param conf Object that contains settings
     * @deprecated since 5.84, use KConfigDialogManager(QWidget *parent, KCoreConfigSkeleton *conf)
     */
    KConfigDialogManager(QWidget *parent, KConfigSkeleton *conf);
#endif

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

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 32)
    /**
     * Retrieve the map between widgets class names and signals that are listened
     * to detect changes in the configuration values.
     * @deprecated Since 5.32, rely on the property change signal noted
     * by @c NOTIFY of the used property in the class definition
     * instead of setting it in this map. Or set the
     * "kcfg_propertyNotify" property on the widget instance.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 32, "See API docs")
    static QHash<QString, QByteArray> *changedMap();
#endif

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
