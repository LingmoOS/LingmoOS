// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchergroup.h"
#include "searchergroup_p.h"

#include <QDebug>

using namespace GrandSearch;

SearcherGroupPrivate::SearcherGroupPrivate(SearcherGroup *parent)
    : QObject(parent)
    , q(parent)
{
}

void SearcherGroupPrivate::initBuiltin()
{
    Q_ASSERT(m_builtin.isEmpty());

#ifdef ENABLE_DEEPINANYTHING
    qInfo() << "create FileNameSearcher";
    auto fileSearcher = new FileNameSearcher(this);
    m_builtin << fileSearcher;
#endif

#ifdef ENABLE_FSEARCH
    qInfo() << "create FsSearcher.";
    auto fSearcher = new FsSearcher(this);
    fSearcher->asyncInitDataBase();
    m_builtin << fSearcher;
#endif

    qInfo() << "create DesktopAppSearcher.";
    auto appSearcher = new DesktopAppSearcher(this);
    appSearcher->asyncInit();
    m_builtin << appSearcher;

    qInfo() << "create StaticTextEchoer.";
    auto stWebSearcher = new StaticTextEchoer(this);
    m_builtin << stWebSearcher;

#ifdef ENABLE_DEEPINANYTHING
    qInfo() << "create SemanticSearcher.";
    auto semanticSearcher = new SemanticSearcher(this);
    m_builtin << semanticSearcher;
#endif

}

bool SearcherGroupPrivate::addExtendSearcher(const SearchPluginInfo &pluginInfo)
{
    //在插件模块需对以下信息做严格的校验，此处不再做有效性检查
    if (Q_UNLIKELY(pluginInfo.name.isEmpty() || pluginInfo.address.isEmpty()
            || pluginInfo.service.isEmpty() || pluginInfo.interface.isEmpty()
            || pluginInfo.ifsVersion.isEmpty()))
        return false;

    if (q->searcher(pluginInfo.name)) {
        qWarning() << "searcher has existed." << pluginInfo.name;
        return false;
    }

    qDebug() << "cretate ExtendSearcher" << pluginInfo.name;
    ExtendSearcher *extend = new ExtendSearcher(pluginInfo.name, this);
    extend->setService(pluginInfo.service, pluginInfo.address,
                         pluginInfo.interface, pluginInfo.ifsVersion);

    //自动启动的插件设置为可激活
    if (pluginInfo.mode == SearchPluginInfo::Auto) {
        extend->setActivatable(ExtendSearcher::InnerActivation);

        //激活操作,必须直连
        connect(extend, &ExtendSearcher::activateRequest, this, &SearcherGroupPrivate::onActivatePlugin, Qt::DirectConnection);
    } else if (pluginInfo.mode == SearchPluginInfo::Trigger) { //dbus触发启动
        extend->setActivatable(ExtendSearcher::Trigger);
    }

    m_extend << extend;
    return true;
}

void SearcherGroupPrivate::onActivatePlugin(const QString &name, bool &ret)
{
    Q_ASSERT(m_pluginManager);
    ret = m_pluginManager->activatePlugin(name);
}

bool SearcherGroupPrivate::initPluinManager()
{
    Q_ASSERT(m_pluginManager == nullptr);

    m_pluginManager = new PluginManager(this);
    return m_pluginManager->loadPlugin();
}

void SearcherGroupPrivate::initExtendSearcher()
{
    Q_ASSERT(m_pluginManager);

    QList<SearchPluginInfo> plugins = m_pluginManager->plugins();
    for (const SearchPluginInfo &info : plugins)
        if (!addExtendSearcher(info))
            qWarning() << "create ExtendSearcher error:" << info.name;
}

SearcherGroup::SearcherGroup(QObject *parent)
    : QObject(parent)
    , d(new SearcherGroupPrivate(this))
{
}

bool SearcherGroup::init()
{
    //初始化内置搜索项
    d->initBuiltin();

    //初始化插件
    if (!d->initPluinManager()) {
        qCritical() << "error: fail to init PluinManager.";
        return false;
    }

    //创建扩展搜索项
    d->initExtendSearcher();

    //启动高优先级插件
    d->m_pluginManager->autoActivate();

    return true;
}

QList<Searcher *> SearcherGroup::searchers() const
{
    return d->m_builtin + d->m_extend;
}

Searcher *SearcherGroup::searcher(const QString &name) const
{
    auto allSearcher = searchers();

    auto iter = std::find_if(allSearcher.begin(), allSearcher.end(), [name](const Searcher *s) {
        return s->name() == name;
    });

    if (iter != allSearcher.end())
        return *iter;
    else
        return nullptr;
}

void SearcherGroup::dormancy()
{
    if (d->m_pluginManager) {
        QList<SearchPluginInfo> plugins = d->m_pluginManager->plugins();
        for (const SearchPluginInfo &plugin : plugins) {
            //停用低优先级的Auto类型插件
            if (plugin.mode == SearchPluginInfo::Auto
                    && plugin.priority == SearchPluginInfo::Low) {
                d->m_pluginManager->inactivate(plugin.name);
            }
        }
    }
}

