// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CDBUSPENDINGCALLWATCHER_H
#define CDBUSPENDINGCALLWATCHER_H

#include <QDBusPendingCallWatcher>

/**
 * @brief The CallMessge struct
 * dbus请求回调数据
 */
struct CallMessge {
    int code; //返回码，0:请求成功，大于0:请求失败
    QString msg; //返回码说明，根据实际需求可返回任意字符串
};

/**
 * @brief CallbackFunc
 * dbus请求回调函数类型
 */
typedef std::function<void(CallMessge)> CallbackFunc;

//继承QDbus回调观察者，将部分自定义的数据包装在回调类中
class CDBusPendingCallWatcher : public QDBusPendingCallWatcher
{
    Q_OBJECT
public:
    explicit CDBusPendingCallWatcher(const QDBusPendingCall &call, QString member, QObject *parent = nullptr);

    //设置回调函数
    void setCallbackFunc(CallbackFunc func);
    //获取回调函数
    CallbackFunc getCallbackFunc();

    //设置调用方法名
    QString getmember();

signals:
    void signalCallFinished(CDBusPendingCallWatcher *);

public slots:

private:
    QString m_member; //调用的dbus方法名
    CallbackFunc m_func = nullptr; //回调函数指针
};

#endif // CDBUSPENDINGCALLWATCHER_H
