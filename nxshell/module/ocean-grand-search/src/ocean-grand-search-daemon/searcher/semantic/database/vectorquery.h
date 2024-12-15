// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VECTORQUERY_H
#define VECTORQUERY_H

#include "searcher/semantic/semantichelper.h"

#include <QObject>

namespace GrandSearch {

class FileResultsHandler;
class VectorQueryPrivate;
class SemanticParser;
class VectorQuery : public QObject
{
    Q_OBJECT
    friend class VectorQueryPrivate;
public:
    explicit VectorQuery(QObject *parent = nullptr);
    ~VectorQuery();
    static void run(void *ptr, PushItemCallBack callBack, void *pdata);
    void setFileHandler(FileResultsHandler *handler);
    void setParser(SemanticParser *);
    void setQuery(const QString &query);
signals:

public slots:
private:
    VectorQueryPrivate *d = nullptr;
};

}

#endif // VECTORQUERY_H
