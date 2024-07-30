/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2021-2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINFACTORY_H
#define KPLUGINFACTORY_H

#include "kcoreaddons_export.h"
#include "kpluginmetadata.h"

#include <QObject>
#include <QVariant>

#include <memory>
#include <type_traits>

class QWidget;
class KPluginFactoryPrivate;

namespace KParts
{
class Part;
}

#define KPluginFactory_iid "org.kde.KPluginFactory"

// Internal macro that generated the KPluginFactory subclass
#define __K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations, ...)                                                                                          \
    class name : public KPluginFactory                                                                                                                         \
    {                                                                                                                                                          \
        Q_OBJECT                                                                                                                                               \
        Q_INTERFACES(KPluginFactory)                                                                                                                           \
        Q_PLUGIN_METADATA(__VA_ARGS__)                                                                                                                         \
    public:                                                                                                                                                    \
        explicit name()                                                                                                                                        \
        {                                                                                                                                                      \
            pluginRegistrations                                                                                                                                \
        }                                                                                                                                                      \
        ~name(){};                                                                                                                                             \
    };

/**
 * @relates KPluginFactory
 *
 * Create a KPluginFactory subclass and export it as the root plugin object.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @param pluginRegistrations code to be inserted into the constructor of the
 * class. Usually a series of registerPlugin() calls.
 *
 * @note K_PLUGIN_FACTORY declares the subclass including a Q_OBJECT macro.
 * So you need to make sure to have Qt's moc run also for the source file
 * where you use the macro. E.g. in projects using CMake and it's automoc feature,
 * as usual you need to have a line
 * @code
 * #include <myplugin.moc>
 * @endcode
 * in the same source file when that one has the name "myplugin.cpp".
 *
 * Example:
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_FACTORY(MyPluginFactory, registerPlugin<MyPlugin>();)
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * If you want to compile a .json file into the plugin, use K_PLUGIN_FACTORY_WITH_JSON.
 *
 * @see K_PLUGIN_FACTORY_WITH_JSON
 * @see K_PLUGIN_FACTORY_DECLARATION
 * @see K_PLUGIN_FACTORY_DEFINITION
 */
#define K_PLUGIN_FACTORY(name, pluginRegistrations) __K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations, IID KPluginFactory_iid)

/**
 * @relates KPluginFactory
 *
 * Create a KPluginFactory subclass and export it as the root plugin object with
 * JSON metadata.
 *
 * This macro does the same as K_PLUGIN_FACTORY, but adds a JSON file as plugin
 * metadata.  See Q_PLUGIN_METADATA() for more information.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @param pluginRegistrations code to be inserted into the constructor of the
 * class. Usually a series of registerPlugin() calls.
 *
 * @param jsonFile name of the JSON file to be compiled into the plugin as metadata
 *
 * @note K_PLUGIN_FACTORY_WITH_JSON declares the subclass including a Q_OBJECT macro.
 * So you need to make sure to have Qt's moc run also for the source file
 * where you use the macro. E.g. in projects using CMake and its automoc feature,
 * as usual you need to have a line
 * @code
 * #include <myplugin.moc>
 * @endcode
 * in the same source file when that one has the name "myplugin.cpp".
 *
 * Example:
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_FACTORY_WITH_JSON(MyPluginFactory,
 *                  "metadata.json",
 *                  registerPlugin<MyPlugin>();
 *                 )
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * @see K_PLUGIN_FACTORY
 * @see K_PLUGIN_FACTORY_DECLARATION
 * @see K_PLUGIN_FACTORY_DEFINITION
 *
 * @since 5.0
 */
#define K_PLUGIN_FACTORY_WITH_JSON(name, jsonFile, pluginRegistrations)                                                                                        \
    __K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations, IID KPluginFactory_iid FILE jsonFile)

/**
 * @relates KPluginFactory
 *
 * Create a KPluginFactory subclass and export it as the root plugin object with
 * JSON metadata.
 *
 * This macro does the same as K_PLUGIN_FACTORY_WITH_JSON, but you only have to pass the class name and the json file.
 * The factory name and registerPlugin call are deduced from the class name.
 *
 * @code
 * #include <myplugin.moc>
 * @endcode
 * in the same source file when that one has the name "myplugin.cpp".
 *
 * Example:
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_CLASS_WITH_JSON(MyPlugin, "metadata.json")
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * @see K_PLUGIN_FACTORY_WITH_JSON
 *
 * @since 5.44
 */
#ifdef KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME
#define K_PLUGIN_CLASS_WITH_JSON(classname, jsonFile)                                                                                                          \
    K_PLUGIN_FACTORY_WITH_JSON(KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME, jsonFile, registerPlugin<classname>();)
#else
#define K_PLUGIN_CLASS_WITH_JSON(classname, jsonFile) K_PLUGIN_FACTORY_WITH_JSON(classname##Factory, jsonFile, registerPlugin<classname>();)
#endif

/**
 * @relates KPluginFactory
 *
 * Creates a KPluginFactory subclass and exports it as the root plugin object.
 * Unlike @ref K_PLUGIN_CLASS_WITH_JSON, this macro does not require json meta data.
 *
 * This macro does the same as K_PLUGIN_FACTORY, but you only have to pass the class name.
 * The factory name and registerPlugin call are deduced from the class name.
 * This is also useful if you want to use static plugins, see the kcoreaddons_add_plugin CMake method.
 * @since 5.90
 */
#ifdef KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME
#define K_PLUGIN_CLASS(classname) K_PLUGIN_FACTORY(KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME, registerPlugin<classname>();)
#else
#define K_PLUGIN_CLASS(classname) K_PLUGIN_FACTORY(classname##Factory, registerPlugin<classname>();)
#endif

/**
 * @class KPluginFactory kpluginfactory.h <KPluginFactory>
 *
 * KPluginFactory provides a convenient way to provide factory-style plugins.
 * Qt plugins provide a singleton object, but a common pattern is for plugins
 * to generate as many objects of a particular type as the application requires.
 * By using KPluginFactory, you can avoid implementing the factory pattern
 * yourself.
 *
 * KPluginFactory also allows plugins to provide multiple different object
 * types, indexed by keywords.
 *
 * The objects created by KPluginFactory must inherit QObject, and must have a
 * standard constructor pattern:
 * @li if the object is a KPart::Part, it must be of the form
 * @code
 * T(QWidget *parentWidget, QObject *parent, const QVariantList &args)
 * @endcode
 * or
 * @code
 * T(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 * @endcode
 * @li if it is a QWidget, it must be of the form
 * @code
 * T(QWidget *parent, const QVariantList &args)
 * @endcode
 * or
 * @code
 * T(QWidget *parent, const KPluginMetaData &metaData, const QVariantList &args)
 * @endcode
 * @li otherwise it must be of the form
 * @code
 * T(QObject *parent, const QVariantList &args)
 * @endcode
 * or
 * @code
 * T(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 * @endcode
 *
 * You should typically use either K_PLUGIN_CLASS() or
 * K_PLUGIN_CLASS_WITH_JSON() in your plugin code to generate a factory.
 * The typical pattern is:
 *
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_CLASS(MyPlugin)
 * #include <myplugin.moc>
 * @endcode
 *
 * If you want to write a custom KPluginFactory not using the standard macro(s)
 * you can reimplement the
 * create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args)
 * method.
 *
 * Example:
 * @code
 * class SomeScriptLanguageFactory : public KPluginFactory
 * {
 *     Q_OBJECT
 * public:
 *     SomeScriptLanguageFactory()
 *     {}
 *
 * protected:
 *     virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args)
 *     {
 *         // Create an identifier based on the iface and given pluginId
 *         const QString identifier = QLatin1String(iface) + QLatin1Char('_') + metaData().pluginId();
 *         // load scripting language module from the information in identifier and return it:
 *         return object;
 *     }
 * };
 * @endcode
 *
 * To load the KPluginFactory from an installed plugin you can use @ref loadFactory and for
 * directly creating a plugin instance from it @ref instantiatePlugin
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @author Bernhard Loos <nhuh.put@web.de>
 * @author Alexander Lohnau <alexander.lohnau@gmx.de>
 */
class KCOREADDONS_EXPORT KPluginFactory : public QObject
{
    Q_OBJECT

public:
    /**
     * This constructor creates a factory for a plugin.
     */
    explicit KPluginFactory();

    /**
     * This destroys the PluginFactory.
     */
    ~KPluginFactory() override;

    /// @since 5.86
    enum ResultErrorReason {
        NO_PLUGIN_ERROR = 0,
        INVALID_PLUGIN,
        INVALID_FACTORY,
        INVALID_KPLUGINFACTORY_INSTANTIATION,
    };
    /**
     * Holds the result of a plugin load operation, i.e. the loaded plugin on success or information about the error on failure
     * @since 5.86
     */
    template<typename T>
    class Result
    {
    public:
        T *plugin = nullptr;
        /// translated, user-visible error string
        QString errorString;
        /// untranslated error text
        QString errorText;
        ResultErrorReason errorReason = NO_PLUGIN_ERROR;
        explicit operator bool() const
        {
            return plugin != nullptr;
        }
    };

    /**
     * Attempts to load the KPluginFactory from the given metadata.
     * The errors will be logged using the `kf.coreaddons` debug category.
     * @param data KPluginMetaData from which the plugin should be loaded
     * @return Result object which contains the plugin instance and potentially error information
     * @since 5.86
     */
    static Result<KPluginFactory> loadFactory(const KPluginMetaData &data);

    /**
     * Attempts to load the KPluginFactory and create a @p T instance from the given metadata
     * KCoreAddons will log error messages automatically, meaning you only need to implement your
     * own logging in case you want to give it more context info or have a custom category.
     * @code
        if (auto result = KPluginFactory::instantiatePlugin<MyClass>(metaData, parent, args)) {
            // The plugin is valid and result.plugin contains the object
        } else {
            // We can access the error related properties, but result.plugin is a nullptr
            qCWarning(MYCATEGORY) << result.errorString;
        }
     * @endcode
     * If there is no extra error handling needed the plugin can be directly accessed and checked if it is a nullptr
     * @code
        if (auto plugin = KPluginFactory::instantiatePlugin<MyClass>(metaData, parent, args).plugin) {
        }
     * @endcode
     * @param data KPluginMetaData from which the plugin should be loaded
     * @param args arguments which get passed to the plugin's constructor
     * @return Result object which contains the plugin instance and potentially error information
     * @since 5.86
     */
    template<typename T>
    static Result<T> instantiatePlugin(const KPluginMetaData &data, QObject *parent = nullptr, const QVariantList &args = {})
    {
        Result<T> result;
        KPluginFactory::Result<KPluginFactory> factoryResult = loadFactory(data);
        if (!factoryResult.plugin) {
            result.errorString = factoryResult.errorString;
            result.errorText = factoryResult.errorText;
            result.errorReason = factoryResult.errorReason;
            return result;
        }
        T *instance = factoryResult.plugin->create<T>(parent, args);
        if (!instance) {
            const QLatin1String className(T::staticMetaObject.className());
            result.errorString = tr("KPluginFactory could not create a %1 instance from %2").arg(className, data.fileName());
            result.errorText = QStringLiteral("KPluginFactory could not create a %1 instance from %2").arg(className, data.fileName());
            result.errorReason = INVALID_KPLUGINFACTORY_INSTANTIATION;
            logFailedInstantiationMessage(T::staticMetaObject.className(), data);
        } else {
            result.plugin = instance;
        }
        return result;
    }

    /**
     * Use this method to create an object. It will try to create an object which inherits
     * @p T. If it has multiple choices it's not defined which object will be returned, so be careful
     * to request a unique interface or use keywords.
     *
     * @tparam T the interface for which an object should be created. The object will inherit @p T.
     * @param parent the parent of the object. If @p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * @param args additional arguments which will be passed to the object.
     * @returns pointer to the created object is returned, or @c nullptr if an error occurred.
     */
    template<typename T>
    T *create(QObject *parent = nullptr, const QVariantList &args = {});

    /**
     * Use this method to create an object. It will try to create an object which inherits @p T
     * This overload has an additional @p parentWidget argument, which is used by some plugins (e.g. Parts).

     * @tparam T the interface for which an object should be created. The object will inherit @p T.
     * @param parentWidget an additional parent widget.
     * @param parent the parent of the object. If @p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * @param args additional arguments which will be passed to the object. Since 5.93 this has a default arg.
     * @returns pointer to the created object is returned, or @c nullptr if an error occurred.
     */
    template<typename T>
    T *create(QWidget *parentWidget, QObject *parent, const QVariantList &args = {});

    /**
     * @returns the metadata of the plugin
     *
     * @since 5.77
     */
    KPluginMetaData metaData() const;

    /**
     * Set the metadata about the plugin this factory generates.
     *
     * @param metaData  the metadata about the plugin
     *
     * @since 5.77
     */
    void setMetaData(const KPluginMetaData &metaData);

protected:
    /**
     * Function pointer type to a function that instantiates a plugin
     * For plugins that don't support a KPluginMetaData parameter it is discarded
     * @since 5.77
     */
    using CreateInstanceWithMetaDataFunction = QObject *(*)(QWidget *, QObject *, const KPluginMetaData &, const QVariantList &);

    /**
     * This is used to detect the arguments need for the constructor of metadata-taking plugin classes.
     * You can inherit it, if you want to add new classes and still keep support for the old ones.
     */
    template<class impl>
    struct InheritanceWithMetaDataChecker {
        /// property to control the availability of the registerPlugin overload taking default values
        static constexpr bool enabled = std::is_constructible<impl, QWidget *, QObject *, KPluginMetaData, QVariantList>::value // KParts
            || std::is_constructible<impl, QWidget *, QObject *, KPluginMetaData>::value
            || std::is_constructible<impl, QWidget *, KPluginMetaData, QVariantList>::value // QWidgets
            || std::is_constructible<impl, QWidget *, KPluginMetaData>::value
            || std::is_constructible<impl, QObject *, KPluginMetaData, QVariantList>::value // Nomal QObjects
            || std::is_constructible<impl, QObject *, KPluginMetaData>::value;

        CreateInstanceWithMetaDataFunction createInstanceFunction(KParts::Part *)
        {
            return &createPartWithMetaDataInstance<impl>;
        }
        CreateInstanceWithMetaDataFunction createInstanceFunction(QWidget *)
        {
            return &createWithMetaDataInstance<impl, QWidget>;
        }
        CreateInstanceWithMetaDataFunction createInstanceFunction(...)
        {
            return &createWithMetaDataInstance<impl, QObject>;
        }
    };

    /**
     * This is used to detect the arguments need for the constructor of metadata-less plugin classes.
     * You can inherit it, if you want to add new classes and still keep support for the old ones.
     */
    template<class impl>
    struct InheritanceChecker {
        /// property to control the availability of the registerPlugin overload taking default values
        static constexpr bool _canConstruct = std::is_constructible<impl, QWidget *, QVariantList>::value // QWidget plugin
            || std::is_constructible<impl, QWidget *>::value //
            || std::is_constructible<impl, QObject *, QVariantList>::value // QObject plugins
            || std::is_constructible<impl, QObject *>::value;
        static constexpr bool enabled = _canConstruct && !InheritanceWithMetaDataChecker<impl>::enabled; // Avoid ambiguity in case of default arguments

        CreateInstanceWithMetaDataFunction createInstanceFunction(QWidget *)
        {
            return &createInstance<impl, QWidget>;
        }
        CreateInstanceWithMetaDataFunction createInstanceFunction(...)
        {
            return &createInstance<impl, QObject>;
        }
    };

    // Use std::enable_if_t once C++14 can be relied on
    template<bool B, class T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;

    /**
     * Uses a default instance creation function depending on the type of interface. If the
     * interface inherits from
     * @li @c KParts::Part the function will call
     * @code
     * new T(QWidget *parentWidget, QObject *parent, const QVariantList &args)
     * @endcode
     * @li @c QWidget the function will call
     * @code
     * new T(QWidget *parent, const QVariantList &args)
     * @endcode
     * @li else the function will call
     * @code
     * new T(QObject *parent, const QVariantList &args)
     * @endcode
     *
     * If those constructor methods are not callable this overload is not available.
     */
    template<class T, enable_if_t<InheritanceChecker<T>::enabled, int> = 0>
    void registerPlugin()
    {
        CreateInstanceWithMetaDataFunction instanceFunction = InheritanceChecker<T>().createInstanceFunction(static_cast<T *>(nullptr));
        registerPlugin(&T::staticMetaObject, instanceFunction);
    }

    /**
     * Uses a default instance creation function depending on the type of interface. If the
     * interface inherits from
     * @li @c KParts::Part the function will call
     * @code
     * new T(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
     * @endcode
     * @li @c QWidget the function will call
     * @code
     * new T(QWidget *parent, const KPluginMetaData &metaData, const QVariantList &args)
     * @endcode
     * @li else the function will call
     * @code
     * new T(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
     * @endcode
     *
     * If those constructor methods are not callable this overload is not available.
     */
    template<class T, enable_if_t<InheritanceWithMetaDataChecker<T>::enabled, int> = 0>
    void registerPlugin()
    {
        CreateInstanceWithMetaDataFunction instanceFunction = InheritanceWithMetaDataChecker<T>().createInstanceFunction(static_cast<T *>(nullptr));
        registerPlugin(&T::staticMetaObject, instanceFunction);
    }

    /**
     * Registers a plugin with the factory. Call this function from the constructor of the
     * KPluginFactory subclass to make the create function able to instantiate the plugin when asked
     * for an interface the plugin implements.
     *
     * @param T the name of the plugin class
     * @param instanceFunction A function pointer to a function that creates an instance of the plugin.
     * @since 5.96
     */
    template<class T>
    void registerPlugin(CreateInstanceWithMetaDataFunction instanceFunction)
    {
        registerPlugin(&T::staticMetaObject, instanceFunction);
    }

    /**
     * This function is called when the factory asked to create an Object.
     *
     * You may reimplement it to provide a very flexible factory. This is especially useful to
     * provide generic factories for plugins implemented using a scripting language.
     *
     * @param iface the staticMetaObject::className() string identifying the plugin interface that
     * was requested. E.g. for KCModule plugins this string will be "KCModule".
     * @param parentWidget only used if the requested plugin is a KPart.
     * @param parent the parent object for the plugin object.
     * @param args a plugin specific list of arbitrary arguments.
     */
    virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args);

    template<class impl, class ParentType>
    static QObject *createInstance(QWidget * /*parentWidget*/, QObject *parent, const KPluginMetaData & /*metaData*/, const QVariantList &args)
    {
        ParentType *p = nullptr;
        if (parent) {
            p = qobject_cast<ParentType *>(parent);
            Q_ASSERT(p);
        }
        if constexpr (std::is_constructible<impl, ParentType *, QVariantList>::value) {
            return new impl(p, args);
        } else {
            return new impl(p);
        }
    }

    template<class impl, class ParentType>
    static QObject *createWithMetaDataInstance(QWidget * /*parentWidget*/, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    {
        ParentType *p = nullptr;
        if (parent) {
            p = qobject_cast<ParentType *>(parent);
            Q_ASSERT(p);
        }
        if constexpr (std::is_constructible<impl, ParentType *, KPluginMetaData, QVariantList>::value) {
            return new impl(p, metaData, args);
        } else {
            return new impl(p, metaData);
        }
    }

    template<class impl>
    static QObject *createPartWithMetaDataInstance(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    {
        if constexpr (std::is_constructible<impl, QWidget *, QObject *, KPluginMetaData, QVariantList>::value) {
            return new impl(parentWidget, parent, metaData, args);
        } else {
            return new impl(parentWidget, parent, metaData);
        }
    }

private:
    friend KPluginFactoryPrivate;
    std::unique_ptr<KPluginFactoryPrivate> const d;
    void registerPlugin(const QMetaObject *metaObject, CreateInstanceWithMetaDataFunction instanceFunction);
    // The logging categories are not part of the public API, consequently this needs to be a private function
    static void logFailedInstantiationMessage(KPluginMetaData data);
    static void logFailedInstantiationMessage(const char *className, KPluginMetaData data);
};

template<typename T>
inline T *KPluginFactory::create(QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent) : nullptr, parent, args);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

template<typename T>
inline T *KPluginFactory::create(QWidget *parentWidget, QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parentWidget, parent, args);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

Q_DECLARE_INTERFACE(KPluginFactory, KPluginFactory_iid)

#endif // KPLUGINFACTORY_H
