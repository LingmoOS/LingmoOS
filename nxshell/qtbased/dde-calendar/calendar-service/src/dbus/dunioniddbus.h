// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DUNIONIDDBUS_H
#define DUNIONIDDBUS_H

#include "daccount.h"

#include <QtDBus/QtDBus>
#include <QSharedPointer>

class DUnionIDDbus : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.utcloud.Daemon";
    }

public:
    explicit DUnionIDDbus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~DUnionIDDbus();

public slots:
    /**
     * @brief login     调用系统用户登入接口
     * @param
     * @return
     */
    inline QDBusPendingReply<> login()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Login"), argumentList);
    }
    /**
     * @brief logout     调用系统用户登出接口
     * @param
     * @return
     */
    inline QDBusPendingReply<> logout()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Logout"), argumentList);
    }
    /**
     * @brief Upload     云同步上传接口
     * @param key  数据标识或文件绝对路径等
     * @return data  上传成功数据的元信息ID值
     *         err   错误信息
     */
    inline QDBusPendingReply<QString> Upload(const QString &key)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(key);
        return asyncCallWithArgumentList(QStringLiteral("Upload"), argumentList);
    }

    /**
     * @brief Download     云同步下载接口
     * @param key  数据标识或文件绝对路径等
     *        path1 指定缓存数据的路径
     * @return path2 下载数据的路径
     *         err   错误信息
     */
    inline QDBusPendingReply<QString> Download(const QString &key, const QString &path1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(key) << QVariant::fromValue(path1);
        return asyncCallWithArgumentList(QStringLiteral("Download"), argumentList);
    }

    /**
     * @brief Delete     云同步删除接口
     * @param key  数据标识或文件绝对路径等
     * @return id  删除id
     *         err   错误信息
     */
    inline QDBusPendingReply<QString> Delete(const QString &key)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(key);
        return asyncCallWithArgumentList(QStringLiteral("Delete"), argumentList);
    }

    /**
     * @brief Metadata     元数据获取接口
     * @param key  数据标识或文件绝对路径等
     * @return meta  元数据信息
     *         err   错误信息
     */
    inline QDBusPendingReply<QByteArray> Metadata(const QString &key)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(key);
        return asyncCallWithArgumentList(QStringLiteral("Metadata"), argumentList);
    }

    /**
     * @brief SwitcherGet     获取总开关状态
     * @param
     * @return
     *
     */
    inline QDBusPendingReply<QString> SwitcherDump()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("SwitcherDump"), argumentList);
    }

    /**
     * @brief SwitcherGet     获取指定应用的开关状态
     * @param arg   指定应用云同步的绝对路径
     * @return
     *
     */
    inline QDBusPendingReply<bool> SwitcherGet(const QString &arg)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(arg);
        return asyncCallWithArgumentList(QStringLiteral("SwitcherGet"), argumentList);
    }


};


namespace com {
namespace deepin {
namespace sync {
typedef ::DUnionIDDbus cloudopt;
}
}
}

#endif // DUNIONIDDBUS_H
