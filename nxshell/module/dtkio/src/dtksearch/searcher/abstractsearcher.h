// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTSEARCHER_H
#define ABSTRACTSEARCHER_H

#include <QObject>
#include <QVariantMap>

#include "dtksearch_global.h"
#include "dtksearchtypes.h"

DSEARCH_BEGIN_NAMESPACE

class AbstractSearcher : public QObject
{
    Q_OBJECT
public:
    enum Status {
        Ready,
        Runing,
        Completed,
        Terminated
    };

    explicit AbstractSearcher(const QVariantMap &options, QObject *parent = nullptr);
    virtual void search() = 0;
    virtual void stop() = 0;
    virtual bool hasItem() const = 0;
    virtual QStringList takeAll() = 0;

private:
    void unpackOptions(const QVariantMap &options);

Q_SIGNALS:
    void unearthed(AbstractSearcher *searcher);

protected:
    QString m_indexPath;
    QString m_searchPath;
    QString m_keyword;

    ResultFilterFunc m_filterFunc { nullptr };

    quint32 m_maxResult;
    SearchFlags m_flags;
};

DSEARCH_END_NAMESPACE

#endif   // ABSTRACTSEARCHER_H
