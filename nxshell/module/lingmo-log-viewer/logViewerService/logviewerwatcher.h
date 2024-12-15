// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGVIEWERWATCHER_H
#define LOGVIEWERWATCHER_H

#include <QObject>
#include <QTimer>

/**
 * @class LogViewerWatcher
 * @brief 监控客户端类
 */
class LogViewerWatcher :public QObject
{
    Q_OBJECT
public:
    explicit LogViewerWatcher();

public Q_SLOTS:
    void onTimeOut();
private:
    QString executCmd(const QString &strCmd, const QStringList &args = QStringList());
private:
    QTimer *m_Timer=nullptr;
};

#endif // LOGVIEWERWATCHER_H
