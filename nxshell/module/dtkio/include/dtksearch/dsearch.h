// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSEARCH_H
#define DSEARCH_H

#include <functional>

#include <QObject>

#include "dtksearch_global.h"
#include "dtksearchtypes.h"

DSEARCH_BEGIN_NAMESPACE

class DSearchPrivate;
class DSearch : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DSearch)

    Q_PROPERTY(quint32 maxResultCount READ maxResultCount WRITE setMaxResultCount)
    Q_PROPERTY(SearchFlags searchFlags READ searchFlags WRITE setSearchFlags)
    Q_PROPERTY(QString indexPath READ indexPath WRITE setIndexPath)

public:
    explicit DSearch(QObject *parent = nullptr);
    explicit DSearch(quint32 maxResultCount, SearchFlags flags, QObject *parent = nullptr);
    ~DSearch();

    bool search(const QString &path, const QString &keyword);
    bool stop();
    QStringList matchedResults() const;

    void setResultFilter(const ResultFilterFunc &filter);

    void setMaxResultCount(quint32 maxResultCount);
    quint32 maxResultCount() const;
    void setSearchFlags(SearchFlags flags);
    SearchFlags searchFlags() const;
    void setIndexPath(const QString &path);
    QString indexPath() const;

Q_SIGNALS:
    void matched();
    void completed();
    void stoped();

private:
    QScopedPointer<DSearchPrivate> d_ptr;
};

DSEARCH_END_NAMESPACE

#endif   // DSEARCH_H
