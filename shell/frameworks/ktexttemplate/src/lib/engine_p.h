/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_ENGINE_P_H
#define KTEXTTEMPLATE_ENGINE_P_H

#include "engine.h"
#include "filter.h"
#include "pluginpointer_p.h"
#include "taglibraryinterface.h"

namespace KTextTemplate
{

class ScriptableTagLibrary;

class ScriptableLibraryContainer : public TagLibraryInterface
{
public:
    ScriptableLibraryContainer(const QHash<QString, AbstractNodeFactory *> &factories, const QHash<QString, Filter *> &filters)
        : m_nodeFactories(factories)
        , m_filters(filters)
    {
    }

    void setNodeFactories(const QHash<QString, AbstractNodeFactory *> &factories)
    {
        m_nodeFactories = factories;
    }

    void setFilters(const QHash<QString, Filter *> &filters)
    {
        m_filters = filters;
    }

    // Warning: should only be called by Engine::loadDefaultLibraries
    void clear()
    {
        qDeleteAll(m_nodeFactories);
        qDeleteAll(m_filters);
        m_nodeFactories.clear();
        m_filters.clear();
    }

    QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = {}) override
    {
        Q_UNUSED(name);
        return m_nodeFactories;
    }

    QHash<QString, Filter *> filters(const QString &name = {}) override
    {
        Q_UNUSED(name);
        return m_filters;
    }

private:
    QHash<QString, AbstractNodeFactory *> m_nodeFactories;
    QHash<QString, Filter *> m_filters;
};

class EnginePrivate
{
    explicit EnginePrivate(Engine *engine);

    TagLibraryInterface *loadLibrary(const QString &name);
    QString getScriptLibraryName(const QString &name) const;
#ifdef QT_QML_LIB
    ScriptableLibraryContainer *loadScriptableLibrary(const QString &name);
#endif
    PluginPointer<TagLibraryInterface> loadCppLibrary(const QString &name);

    Q_DECLARE_PUBLIC(Engine)
    Engine *const q_ptr;

    QHash<QString, PluginPointer<TagLibraryInterface>> m_libraries;
#ifdef QT_QML_LIB
    QHash<QString, ScriptableLibraryContainer *> m_scriptableLibraries;
#endif

    QList<QSharedPointer<AbstractTemplateLoader>> m_loaders;
    QStringList m_pluginDirs;
    QStringList m_defaultLibraries;
#ifdef QT_QML_LIB
    ScriptableTagLibrary *m_scriptableTagLibrary;
#endif
    bool m_smartTrimEnabled;
};
}

#endif
