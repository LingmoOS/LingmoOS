// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <functional>

#include <QObject>
#include <QHash>

#include "dtksearch_global.h"
#include "taskcommander.h"

DSEARCH_BEGIN_NAMESPACE

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();

    bool doSearchTask(const QVariantMap &options);
    void stop();
    QStringList getResults();

private Q_SLOTS:
    void onFinished();

Q_SIGNALS:
    void matched();
    void completed();

private:
    TaskCommander *task { nullptr };
};

DSEARCH_END_NAMESPACE

#endif   // MAINCONTROLLER_H
