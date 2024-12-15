// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_H
#define TASKCOMMANDER_H

#include "global/matcheditem.h"

#include <QObject>

namespace GrandSearch {

class ProxyWorker;
class TaskCommanderPrivate;
class TaskCommander : public QObject
{
    Q_OBJECT
    friend class TaskCommanderPrivate;
public:
    explicit TaskCommander(const QString &content, QObject *parent = nullptr);
    QString taskID() const;
    QString content() const;
    void setContent(const QString &content);
    bool start();
    void stop();
    MatchedItemMap getResults() const;
    MatchedItemMap readBuffer() const;
    bool isEmptyBuffer() const;
    bool join(ProxyWorker *);
    void deleteSelf();
    bool isFinished() const;
protected:
    ~TaskCommander();
signals:
    void matched();
    void finished();
private:
    TaskCommanderPrivate *d;
};

}

#endif // TASKCOMMANDER_H
