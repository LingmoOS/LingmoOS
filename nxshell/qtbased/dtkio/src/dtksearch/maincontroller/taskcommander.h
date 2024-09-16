// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TASKCOMMANDER_H
#define TASKCOMMANDER_H

#include <QObject>

#include "dtksearch_global.h"

DSEARCH_BEGIN_NAMESPACE

class TaskCommanderPrivate;
class TaskCommander : public QObject
{
    Q_OBJECT

public:
    TaskCommander(const QVariantMap &options, QObject *parent = nullptr);

    QStringList getResults() const;
    bool start();
    void stop();
    void deleteSelf();

private:
    void createSearcher(const QVariantMap &options);

Q_SIGNALS:
    void matched();
    void finished();

private:
    TaskCommanderPrivate *d = nullptr;
};

DSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_H
