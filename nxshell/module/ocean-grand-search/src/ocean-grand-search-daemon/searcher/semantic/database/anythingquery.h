// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANYTHINGQUERY_H
#define ANYTHINGQUERY_H

#include "searcher/semantic/semantichelper.h"

#include <QObject>

namespace GrandSearch {

class FileResultsHandler;
class AnythingQueryPrivate;
class AnythingQuery : public QObject
{
    Q_OBJECT
    friend class AnythingQueryPrivate;
public:
    explicit AnythingQuery(QObject *parent = nullptr);
    ~AnythingQuery();
    static void run(void *ptr, PushItemCallBack callBack, void *pdata);
    void setEntity(const QList<SemanticEntity> &entity);
    void setFileHandler(FileResultsHandler *handler);
private:
    AnythingQueryPrivate *d;
};

}

#endif // ANYTHINGQUERY_H
