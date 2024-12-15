// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QStandardPaths>
#include <QDir>
#include <QMap>
#include <QVariant>

#include "dsearch_p.h"
#include "dsearch.h"
#include "maincontroller/maincontroller.h"

DSEARCH_BEGIN_NAMESPACE

DSearchPrivate::DSearchPrivate(DSearch *qq, quint32 maxCount, SearchFlags flags)
    : q_ptr(qq),
      maxResultCount(maxCount),
      searchFlags(flags),
      mainController(new MainController(this))
{
    connect(mainController, &MainController::matched, q_ptr, &DSearch::matched);
    connect(mainController, &MainController::completed, q_ptr, &DSearch::completed);
}

QVariantMap DSearchPrivate::packOptions()
{
    QVariantMap options;
    options.insert("MaxResult", maxResultCount);
    options.insert("SearchFlags", QVariant::fromValue(searchFlags));
    options.insert("IndexPath", indexPath);
    options.insert("ResultFilterFunc", QVariant::fromValue(resultFilter));

    return options;
}

DSearch::DSearch(QObject *parent)
    : QObject(parent),
      d_ptr(new DSearchPrivate(this))
{
}

DSearch::DSearch(quint32 maxResultCount, SearchFlags flags, QObject *parent)
    : QObject(parent),
      d_ptr(new DSearchPrivate(this, maxResultCount, flags))
{
}

DSearch::~DSearch()
{
}

bool DSearch::search(const QString &path, const QString &keyword)
{
    Q_D(DSearch);

    if (path.isEmpty() || keyword.isEmpty() || d->indexPath.isEmpty())
        return false;

    auto options = d->packOptions();
    options.insert("Path", path);
    options.insert("Keyword", keyword);

    return d->mainController->doSearchTask(options);
}

bool DSearch::stop()
{
    Q_D(DSearch);

    d->mainController->stop();
    emit stoped();
    return true;
}

QStringList DSearch::matchedResults() const
{
    Q_D(const DSearch);

    return d->mainController->getResults();
}

void DSearch::setResultFilter(const ResultFilterFunc &filter)
{
    Q_D(DSearch);

    d->resultFilter = filter;
}

void DSearch::setMaxResultCount(quint32 maxResultCount)
{
    Q_D(DSearch);

    d->maxResultCount = maxResultCount;
}

quint32 DSearch::maxResultCount() const
{
    Q_D(const DSearch);

    return d->maxResultCount;
}

void DSearch::setSearchFlags(SearchFlags flags)
{
    Q_D(DSearch);

    d->searchFlags = flags;
}

SearchFlags DSearch::searchFlags() const
{
    Q_D(const DSearch);

    return d->searchFlags;
}

void DSearch::setIndexPath(const QString &path)
{
    Q_D(DSearch);

    d->indexPath = path;
}

QString DSearch::indexPath() const
{
    Q_D(const DSearch);

    return d->indexPath;
}

DSEARCH_END_NAMESPACE
