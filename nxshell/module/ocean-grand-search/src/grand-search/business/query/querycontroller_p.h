// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYCONTROLLER_P_H
#define QUERYCONTROLLER_P_H

#include "querycontroller.h"

#include <QObject>
#include <QTimer>

namespace GrandSearch {

class DaemonGrandSearchInterface;
class QueryControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QueryControllerPrivate(QueryController *parent = nullptr);

public slots:
    // 发起新的搜索后，在搜索完成或结束前，周期调用后端心跳接口
    void keepAlive();

    // 接收后端搜索完成信号，停止心跳发送
    void onSearchCompleted(const QString &missionId);

public:
    QueryController *q_p = nullptr;
    QString m_missionId;
    QString m_searchText;
    QTimer *m_keepAliveTimer = nullptr;

    DaemonGrandSearchInterface *m_daemonDbus = nullptr;
};

}

#endif // QUERYCONTROLLER_P_H
