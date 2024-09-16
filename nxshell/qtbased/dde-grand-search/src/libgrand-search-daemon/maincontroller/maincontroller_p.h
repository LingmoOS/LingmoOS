// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINCONTROLLER_P_H
#define MAINCONTROLLER_P_H

#include "maincontroller/maincontroller.h"
#include "searcher/searchergroup.h"
#include "task/taskcommander.h"

namespace GrandSearch {

class MainControllerPrivate : public QObject
{
    Q_OBJECT
    friend class MainController;
public:
    template<class BuildWorker>
    bool buildTask(const QString &context, BuildWorker worker) {
        //释放当前任务
        q->terminate();

        auto task = new TaskCommander(context);
        qInfo() << "new task:" << task << task->taskID();
        Q_ASSERT(task);

        //直连，防止被事件循环打乱时序
        connect(task, &TaskCommander::matched, q, &MainController::matched, Qt::DirectConnection);
        connect(task, &TaskCommander::finished, q, &MainController::searchCompleted, Qt::DirectConnection);

        worker(task);
        if (task->start()) {
            m_currentTask = task;

            //重新计时休眠
            m_dormancy.start();
            return true;
        }

        qWarning() << "fail to start task" << task << task->taskID();
        task->deleteSelf();
        return false;
    }
public:
    MainControllerPrivate(MainController *parent);
    ~MainControllerPrivate();
    void buildWorker(TaskCommander *task, const QSet<QString> &blankList = {});
    void buildWorker(TaskCommander *task, const QList<QString> &searchers);
    QStringList checkSearcher(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList);
    QString buildKeywordInJson(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList);
private slots:
    void dormancy();
private:
    MainController *q;
    SearcherGroup *m_searchers = nullptr;
    TaskCommander *m_currentTask = nullptr;
    QTimer m_dormancy;
};

}

#endif // MAINCONTROLLER_P_H
