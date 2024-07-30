/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLETAGS_H
#define SCRIPTABLETAGS_H

#include "node.h"
#include "taglibraryinterface.h"

#include <QJSValue>

class QJSEngine;

namespace KTextTemplate
{
class Engine;

class ScriptableHelperFunctions : public QObject
{
    Q_OBJECT
    QJSEngine *m_scriptEngine;

public:
    explicit ScriptableHelperFunctions(QJSEngine *scriptEngine)
        : m_scriptEngine(scriptEngine)
    {
    }

    Q_INVOKABLE QJSValue markSafeFunction(QJSValue inputValue);
    Q_INVOKABLE QJSValue ScriptableFilterExpressionConstructor(QString name, QObject *parserObj = {});
    Q_INVOKABLE QJSValue ScriptableNodeConstructor(QJSValue callContext);
    Q_INVOKABLE QJSValue ScriptableVariableConstructor(QString name);
    Q_INVOKABLE QJSValue ScriptableTemplateConstructor(QString content, QString name, QObject *parent);
};

class ScriptableTagLibrary : public QObject, public TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextTemplate::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.kde.KTextTemplate.TagLibraryInterface")
public:
    ScriptableTagLibrary(QObject *parent = {});

    QHash<QString, AbstractNodeFactory *> nodeFactories(const QString &name = {}) override;

    QHash<QString, Filter *> filters(const QString &name = {}) override;

public Q_SLOTS:
    void addFactory(const QString &factoryName, const QString &tagname);
    void addFilter(const QString &filterName);

protected:
    bool evaluateScript(const QString &name);
    QHash<QString, AbstractNodeFactory *> getFactories();
    QHash<QString, Filter *> getFilters();

private:
    QJSEngine *m_scriptEngine;
    QJSValue m_functions;
    QHash<QString, AbstractNodeFactory *> m_nodeFactories;
    QHash<QString, QString> m_factoryNames;
    QStringList m_filterNames;
    QHash<QString, Filter *> m_filters;
};
}

#endif
