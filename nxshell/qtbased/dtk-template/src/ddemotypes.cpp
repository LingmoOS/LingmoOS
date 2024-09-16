// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddemotypes_p.h"
#include <QDBusArgument>
#include <QDBusMetaType>

DDEMO_BEGIN_NAMESPACE

QDBusArgument &operator<<(QDBusArgument &arg, const UserPath_p &path)
{
    arg.beginStructure();
    arg << path.user_id;
    arg << path.path;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, UserPath_p &path)
{
    arg.beginStructure();
    arg >> path.user_id;
    arg >> path.path;
    arg.endStructure();
    return arg;
}

void UserPath_p::registerMetaType()
{
    qRegisterMetaType<UserPath_p>("UserPath_p");
    qRegisterMetaType<UserPathList_p>("UserPathList_p");
    qDBusRegisterMetaType<UserPath_p>();
    qDBusRegisterMetaType<UserPathList_p>();
}

UserPath_p::operator UserPath()
{
    return UserPath{.user_id = this->user_id, .path = [this]() {
                        const auto &list = this->path.path().split("/");
                        bool ok{false};
                        auto id = list.constLast().toLongLong(&ok);
                        return ok ? id : 0;
                    }()};
}

UserPath_p::operator UserPath() const
{
    return UserPath{.user_id = this->user_id, .path = [this]() {
                        const auto &list = this->path.path().split("/");
                        bool ok{false};
                        auto id = list.constLast().toLongLong(&ok);
                        return ok ? id : 0;
                    }()};
}

/*
struct UserPath_p
{
    uint user_id;
    QDBusObjectPath path;
};

typedef QList<UserPath> UserPathList;

UserPath ==> (uo) ==> struct of { uint, QDBusObjectPath}
UserPathList ==> a(uo) ==> Array of [struct of { uint, QDBusObjectPath}]

+---------------------------------------------------------------------+
|   xml    |      D-Bus Type                   | Qt DBUS Type         |
+---------------------------------------------------------------------+
|   y      |        BYTE                       |  uchar               |
+---------------------------------------------------------------------+
|   b      |        BOOLEAN                    |  bool                |
+---------------------------------------------------------------------+
|   n      |        INT16                      |  short               |
+---------------------------------------------------------------------+
|   q      |        UINT16                     |  ushort              |
+---------------------------------------------------------------------+
|   i      |        INT32                      |  int                 |
+---------------------------------------------------------------------+
|   u      |        UINT32                     |  uint                |
+---------------------------------------------------------------------+
|   x      |        INT64                      |  qlonglong           |
+---------------------------------------------------------------------+
|   t      |        UINT64                     |  qulonglong          |
+---------------------------------------------------------------------+
|   d      |        DOUBLE                     |  double              |
+---------------------------------------------------------------------+
|   s      |        STRING                     |  QString             |
+---------------------------------------------------------------------+
|   v      |        VARIANT                    |  QDBusVariant        |
+---------------------------------------------------------------------+
|   o      |        OBJECT_PATH                |  QDBusObjectPath     |
+---------------------------------------------------------------------+
|   g      |        SIGNATURE                  |  QDBusSignature      |
+---------------------------------------------------------------------+
|   as     |        Array of [string]          |  QStringList         |
+---------------------------------------------------------------------+
|   ai     |        Array of [int32]           |  QList<int>          |
+---------------------------------------------------------------------+
|   ay     |        Array of [BYTE]            |  QByteArray          |
+---------------------------------------------------------------------+
|   {sv}   |        Dict of {String,Variant}   |  QMap<string,Variant>|
+---------------------------------------------------------------------+
|   a      |        Array                      |  QList               |
+---------------------------------------------------------------------+
|   ()     |        struct of { }              |  QVariant            |
+---------------------------------------------------------------------+
|   {}     |        Dict                       |  QMap                |
+---------------------------------------------------------------------+

refs: https://blog.csdn.net/arv002/article/details/119784854
*/
DDEMO_END_NAMESPACE
