// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SEMANTICANALYSISTASK_H
#define SEMANTICANALYSISTASK_H

#include <QObject>
#include <QDateTime>
#include "../data/jsondata.h"

class QJsonObject;

class semanticAnalysisTask : public QObject
{
    Q_OBJECT
public:
    explicit semanticAnalysisTask(QObject *parent = nullptr);
    ~semanticAnalysisTask();
    bool resolveTaskJson(const QString &semantic);
    QString Intent() const;
    void setIntent(const QString &Intent);
    JsonData *getJsonData() const;
    void setJsonData(JsonData *JsonData);

private:
    void deleteJsonData();
    JsonData *createJsonDataFactory(const QString &Intent);
    void setShouldEndSession(bool isEnd);

signals:

public slots:
private:
    QString m_Intent;
    JsonData *m_JsonData {nullptr};
};

#endif // SEMANTICANALYSISTASK_H
