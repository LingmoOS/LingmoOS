// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FEATUREQUERY_H
#define FEATUREQUERY_H

#include "searcher/semantic/semantichelper.h"

#include <QObject>

namespace GrandSearch {

class FileResultsHandler;
class FeatureQueryPrivate;
class FeatureQuery : public QObject
{
    Q_OBJECT
    friend class FeatureQueryPrivate;
public:
    explicit FeatureQuery(QObject *parent = nullptr);
    ~FeatureQuery();
    static void run(void *ptr, PushItemCallBack callBack, void *pdata);
    void setEntity(const QList<SemanticEntity> &entity);
    void setFileHandler(FileResultsHandler *handler);
signals:

public slots:
private:
    FeatureQueryPrivate *d;
};

}

#endif // FEATUREQUERY_H
