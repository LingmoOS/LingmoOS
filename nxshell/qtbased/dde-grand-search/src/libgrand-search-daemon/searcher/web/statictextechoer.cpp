// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "statictextechoer.h"
#include "global/builtinsearch.h"
#include "statictextworker.h"

#include <QDebug>

using namespace GrandSearch;

StaticTextEchoer::StaticTextEchoer(QObject *parent): Searcher(parent)
{

}

QString StaticTextEchoer::name() const
{
    return GRANDSEARCH_CLASS_WEB_STATICTEXT;
}

bool StaticTextEchoer::isActive() const
{
    return true;
}

bool StaticTextEchoer::activate()
{
    return false;
}

ProxyWorker *StaticTextEchoer::createWorker() const
{
    return new StaticTextWorker(name());
}

bool StaticTextEchoer::action(const QString &action, const QString &item)
{
    Q_UNUSED(item);
    qWarning() << "no such action:" << action << ".";
    return false;
}
