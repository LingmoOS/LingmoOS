// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTQUERY_H
#define FULLTEXTQUERY_H

#include "searcher/semantic/semantichelper.h"

#include <QObject>

namespace GrandSearch {

class FileResultsHandler;
class FullTextQueryPrivate;
class FullTextQuery : public QObject
{
    Q_OBJECT
    friend class FullTextQueryPrivate;
public:
    explicit FullTextQuery(QObject *parent = nullptr);
    ~FullTextQuery();
    static void run(void *ptr, PushItemCallBack callBack, void *pdata);
    void setEntity(const QList<SemanticEntity> &entity);
    void setFileHandler(FileResultsHandler *handler);
signals:

public slots:
private:
    FullTextQueryPrivate *d;
};
}

#endif // FULLTEXTQUERY_H
