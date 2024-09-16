// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDir>

#include "abstractsearcher.h"

DSEARCH_BEGIN_NAMESPACE

AbstractSearcher::AbstractSearcher(const QVariantMap &options, QObject *parent)
    : QObject(parent)
{
    unpackOptions(options);
}

void AbstractSearcher::unpackOptions(const QVariantMap &options)
{
    m_maxResult = options.value("MaxResult").toUInt();
    m_indexPath = options.value("IndexPath").toString();
    m_searchPath = options.value("Path").toString();
    m_keyword = options.value("Keyword").toString();
    m_flags = options.value("SearchFlags").value<SearchFlags>();
    m_filterFunc = options.value("ResultFilterFunc").value<ResultFilterFunc>();
}

DSEARCH_END_NAMESPACE
