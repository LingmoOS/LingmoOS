// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHERGROUP_P_H
#define SEARCHERGROUP_P_H

#include "searchergroup.h"
#include "searchplugin/pluginmanager.h"
#include "app/desktopappsearcher.h"
#include "extend/extendsearcher.h"
#include "web/statictextechoer.h"
#include "semantic/semanticsearcher.h"

#ifdef ENABLE_DEEPINANYTHING
    #include "file/filenamesearcher.h"
#endif

#ifdef ENABLE_FSEARCH
    #include "file/fssearcher.h"
#endif

namespace GrandSearch {

class SearcherGroupPrivate : public QObject
{
    Q_OBJECT
    friend class SearcherGroup;
public:
    explicit SearcherGroupPrivate(SearcherGroup *parent);

    void initBuiltin();
    bool initPluinManager();
    void initExtendSearcher();
    bool addExtendSearcher(const SearchPluginInfo &pluginInfo);
private slots:
    void onActivatePlugin(const QString &name, bool &ret);
private:
    SearcherGroup *q;
    QList<Searcher *> m_builtin;
    QList<Searcher *> m_extend;
    PluginManager *m_pluginManager = nullptr;
};

}

#endif // SEARCHERGROUP_P_H
