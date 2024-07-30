/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "engine.h"
#include "engine_p.h"

#include "exception.h"
#include "ktexttemplate_config_p.h"
#ifdef QT_QML_LIB
#include "scriptabletags.h"
#endif
#include "template_p.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <QTextStream>

using namespace KTextTemplate;

static const char s_scriptableLibName[] = "ktexttemplate_scriptabletags";

Engine::Engine(QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginePrivate(this))
{
    d_ptr->m_defaultLibraries << QStringLiteral("ktexttemplate_defaulttags") << QStringLiteral("ktexttemplate_loadertags")
                              << QStringLiteral("ktexttemplate_defaultfilters");

    d_ptr->m_pluginDirs = QCoreApplication::libraryPaths();
    d_ptr->m_pluginDirs << QString::fromLocal8Bit(KTEXTTEMPLATE_PLUGIN_PATH);
}

Engine::~Engine()
{
#ifdef QT_QML_LIB
    qDeleteAll(d_ptr->m_scriptableLibraries);
#endif
    d_ptr->m_libraries.clear();
    delete d_ptr;
}

QList<QSharedPointer<AbstractTemplateLoader>> Engine::templateLoaders()
{
    Q_D(Engine);
    return d->m_loaders;
}

void Engine::addTemplateLoader(QSharedPointer<AbstractTemplateLoader> loader)
{
    Q_D(Engine);
    d->m_loaders << loader;
}

std::pair<QString, QString> Engine::mediaUri(const QString &fileName) const
{
    Q_D(const Engine);

    std::pair<QString, QString> uri;
    for (auto &loader : d->m_loaders) {
        uri = loader->getMediaUri(fileName);
        if (!uri.second.isEmpty())
            break;
    }
    return uri;
}

void Engine::setPluginPaths(const QStringList &dirs)
{
    Q_D(Engine);
    d->m_pluginDirs = dirs;
}

void Engine::addPluginPath(const QString &dir)
{
    Q_D(Engine);
    QStringList temp;
    temp << dir;
    temp << d->m_pluginDirs;
    d->m_pluginDirs = temp;
}

void Engine::removePluginPath(const QString &dir)
{
    Q_D(Engine);
    d->m_pluginDirs.removeAll(dir);
}

QStringList Engine::pluginPaths() const
{
    Q_D(const Engine);
    return d->m_pluginDirs;
}

QStringList Engine::defaultLibraries() const
{
    Q_D(const Engine);
    return d->m_defaultLibraries;
}

void Engine::addDefaultLibrary(const QString &libName)
{
    Q_D(Engine);
    d->m_defaultLibraries << libName;
}

void Engine::removeDefaultLibrary(const QString &libName)
{
    Q_D(Engine);
    d->m_defaultLibraries.removeAll(libName);
}

void Engine::loadDefaultLibraries()
{
    Q_D(Engine);

#ifdef QT_QML_LIB
    // Make sure we can load default scriptable libraries if we're supposed to.
    if (d->m_defaultLibraries.contains(QLatin1String(s_scriptableLibName)) && !d->m_scriptableTagLibrary) {
        d->m_scriptableTagLibrary = new ScriptableTagLibrary(this);

// It would be better to load this as a plugin, but that is not currently
// possible with webkit/javascriptcore
// so we new the library directly.
// https://bugs.webkit.org/show_bug.cgi?id=38193
#if 0
    d->loadCppLibrary( s_scriptableLibName );
    PluginPointer<TagLibraryInterface> library = d->loadCppLibrary( s_scriptableLibName );
    if ( !library )
      throw KTextTemplate::Exception( TagSyntaxError, QStringLiteral("Could not load scriptable tags library") );
#endif
    }
#endif

    for (const QString &libName : std::as_const(d->m_defaultLibraries)) {
        if (libName == QLatin1String(s_scriptableLibName))
            continue;

        // already loaded by the engine.
        if (d->m_libraries.contains(libName))
            continue;

#ifdef QT_QML_LIB
        // Although we don't use scripted libaries here, we need to
        // recognize them being first in the search path and not load a
        // c++ plugin of the same name in that case.
        auto scriptableLibrary = d->loadScriptableLibrary(libName);
        if (scriptableLibrary) {
            scriptableLibrary->clear();
            break;
        }
#endif

        auto library = d->loadCppLibrary(libName);
        if (library) {
            break;
        }
    }
}

TagLibraryInterface *Engine::loadLibrary(const QString &name)
{
    Q_D(Engine);

#ifdef QT_QML_LIB
    if (name == QLatin1String(s_scriptableLibName))
        return nullptr;
#endif

    // already loaded by the engine.
    if (d->m_libraries.contains(name))
        return d->m_libraries.value(name).data();

    auto library = d->loadLibrary(name);
    if (library) {
        return library;
    }
    throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Plugin library '%1' not found.").arg(name));
    return nullptr;
}

TagLibraryInterface *EnginePrivate::loadLibrary(const QString &name)
{
#ifdef QT_QML_LIB
    auto scriptableLibrary = loadScriptableLibrary(name);
    if (scriptableLibrary)
        return scriptableLibrary;

// else this is not a scriptable library.
#endif

    return loadCppLibrary(name).data();
}

EnginePrivate::EnginePrivate(Engine *engine)
    : q_ptr(engine)
#ifdef QT_QML_LIB
    , m_scriptableTagLibrary(nullptr)
#endif
    , m_smartTrimEnabled(false)
{
}

QString EnginePrivate::getScriptLibraryName(const QString &name) const
{
    auto pluginIndex = 0;
    const QString prefix = QStringLiteral("/kf6/ktexttemplate/");
    while (m_pluginDirs.size() > pluginIndex) {
        const auto nextDir = m_pluginDirs.at(pluginIndex++);
        const QString libFileName = nextDir + prefix + name + QStringLiteral(".qs");

        const QFile file(libFileName);
        if (!file.exists())
            continue;
        return libFileName;
    }
    auto it = m_loaders.constBegin();
    const auto end = m_loaders.constEnd();
    for (; it != end; ++it) {
        const auto pair = (*it)->getMediaUri(prefix + name + QStringLiteral(".qs"));

        if (!pair.first.isEmpty() && !pair.second.isEmpty()) {
            return pair.first + pair.second;
        }
    }
    return {};
}

#ifdef QT_QML_LIB
ScriptableLibraryContainer *EnginePrivate::loadScriptableLibrary(const QString &name)
{
    if (!m_scriptableTagLibrary)
        return nullptr;

#if 0
  if ( !m_libraries.contains( s_scriptableLibName ) )
    return 0;
#endif

    const auto libFileName = getScriptLibraryName(name);

    if (libFileName.isEmpty())
        return nullptr;

    const auto it = m_scriptableLibraries.constFind(libFileName);
    if (it != m_scriptableLibraries.constEnd()) {
        auto library = it.value();
        library->setNodeFactories(m_scriptableTagLibrary->nodeFactories(libFileName));
        library->setFilters(m_scriptableTagLibrary->filters(libFileName));
        return library;
    }
#if 0
  PluginPointer<TagLibraryInterface> scriptableTagLibrary = m_libraries.value( s_scriptableLibName );
#endif

    const auto factories = m_scriptableTagLibrary->nodeFactories(libFileName);
    const auto filters = m_scriptableTagLibrary->filters(libFileName);

    auto library = new ScriptableLibraryContainer(factories, filters);
    m_scriptableLibraries.insert(libFileName, library);
    return library;
}
#endif

PluginPointer<TagLibraryInterface> EnginePrivate::loadCppLibrary(const QString &name)
{
    auto pluginIndex = 0;

    while (m_pluginDirs.size() > pluginIndex) {
        const auto nextDir = m_pluginDirs.at(pluginIndex++);
        const QString pluginDirString = nextDir + QStringLiteral("/kf6/ktexttemplate/");

        const QDir pluginDir(pluginDirString);

        if (!pluginDir.exists())
            continue;

        auto list = pluginDir.entryList(QStringList(name
#if PLUGINS_PREFER_DEBUG_POSTFIX
                                                    + QLatin1Char('d')
#endif
                                                    + QLatin1Char('*')));

        if (list.isEmpty()) {
#if PLUGINS_PREFER_DEBUG_POSTFIX
            list = pluginDir.entryList(QStringList(name + QLatin1Char('*')));
            if (list.isEmpty())
                continue;
#else
            continue;
#endif
        }

        auto pluginPath = pluginDir.absoluteFilePath(list.first());
        auto plugin = PluginPointer<TagLibraryInterface>(pluginPath);

        if (plugin) {
            m_libraries.insert(name, plugin);
            return plugin;
        }
    }
    return nullptr;
}

Template Engine::loadByName(const QString &name) const
{
    Q_D(const Engine);

    for (auto &loader : d->m_loaders) {
        if (!loader->canLoadTemplate(name))
            continue;

        const auto t = loader->loadByName(name, this);

        if (t) {
            return t;
        }
    }
    auto t = Template(new TemplateImpl(this));
    t->setObjectName(name);
    t->d_ptr->m_error = TagSyntaxError;
    t->d_ptr->m_errorString = QStringLiteral("Template not found, %1").arg(name);
    return t;
}

Template Engine::newTemplate(const QString &content, const QString &name) const
{
    Q_D(const Engine);
    auto t = Template(new TemplateImpl(this, d->m_smartTrimEnabled));
    t->setObjectName(name);
    t->setContent(content);
    return t;
}

void Engine::setSmartTrimEnabled(bool enabled)
{
    Q_D(Engine);
    d->m_smartTrimEnabled = enabled;
}

bool Engine::smartTrimEnabled() const
{
    Q_D(const Engine);
    return d->m_smartTrimEnabled;
}

#include "moc_engine.cpp"
