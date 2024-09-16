// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYCONTROLLER_H
#define QUERYCONTROLLER_H

#include <QObject>
#include <QScopedPointer>

namespace GrandSearch {

class QueryControllerPrivate;
class QueryController : public QObject
{
    Q_OBJECT
public:
    explicit QueryController(QObject *parent = nullptr);
    ~QueryController();

    // 搜索文本发生变化，发起新的搜索请求
    void onSearchTextChanged(const QString &txt);

    // 前端主动终止搜索，调用接口通知后端
    void onTerminateSearch();

    // 返回当前任务ID，如果没有执行中的任务，则返回空
    QString getMissionID() const;
    bool isEmptySearchText() const;

signals:
    // 搜索文本改变后，发起新的搜索前，发出该任务ID改变信号
    void missionChanged(const QString &missionId, const QString &missionContent);

private:
    QScopedPointer<QueryControllerPrivate> d_p;
};

}

#endif // QUERYCONTROLLER_H
