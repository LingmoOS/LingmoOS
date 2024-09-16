// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCHEDULEYPE_H
#define DSCHEDULEYPE_H

#include "dtypecolor.h"

#include <QFlag>
#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QSharedPointer>

class DAccount;
//日程类型
class DScheduleType
{
public:
    enum Privilege {
        None = 0x0, //不可读不可写
        Read = 0x1, //可读
        Write = 0x2, //可编辑
        Delete = 0x4, //可删除
        System = Read, //系统权限
        User = Read | Write | Delete, //用户权限
    };
    Q_DECLARE_FLAGS(Privileges, Privilege)

    enum ShowState {
        Hide, //日程隐藏
        Show, //日程显示
    };

    typedef QSharedPointer<DScheduleType> Ptr;
    typedef QVector<DScheduleType::Ptr> List;

    DScheduleType();
    explicit DScheduleType(const QString &accountID);
    QString accountID() const;
    void setAccountID(const QString &accountID);

    Privileges privilege() const;
    void setPrivilege(const Privileges &privilege);

    DTypeColor typeColor() const;
    void setTypeColor(const DTypeColor &typeColor);

    void setColorID(const QString &colorID);
    QString getColorID() const;
    void setColorCode(const QString &colorCode);
    QString getColorCode() const;

    QString typeID() const;
    void setTypeID(const QString &typeID);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    ShowState showState() const;
    void setShowState(const ShowState &showState);

    QString typeName() const;
    void setTypeName(const QString &typeName);

    QString typePath() const;
    void setTypePath(const QString &typePath);

    QString description() const;
    void setDescription(const QString &description);

    QDateTime dtCreate() const;
    void setDtCreate(const QDateTime &dtCreate);

    QDateTime dtUpdate() const;
    void setDtUpdate(const QDateTime &dtUpdate);

    QDateTime dtDelete() const;
    void setDtDelete(const QDateTime &dtDelete);

    int deleted() const;
    void setDeleted(int deleted);

    static bool fromJsonString(DScheduleType::Ptr &scheduleType, const QString &jsonStr);
    static bool toJsonString(const DScheduleType::Ptr &scheduleType, QString &jsonStr);
    static bool fromJsonListString(DScheduleType::List &stList, const QString &jsonStr);
    static bool toJsonListString(const DScheduleType::List &stList, QString &jsonStr);

    int syncTag() const;
    void setSyncTag(int syncTag);

    friend bool operator<(const DScheduleType::Ptr &st1, const DScheduleType::Ptr &st2);

private:
    QString m_accountID;
    QString m_typeID;
    QString m_typeName; //类型名称
    QString m_displayName; //类型显示名称
    QString m_typePath; //类型云端路径
    DTypeColor m_typeColor; //相关颜色信息
    QString m_description; //类型相关描述
    Privileges m_privilege; //类型权限
    QDateTime m_dtCreate; //创建时间
    QDateTime m_dtUpdate; //更新时间
    QDateTime m_dtDelete; //删除时间
    ShowState m_showState; //类型下日程显示状态
    int m_deleted; //是否被删除
    int m_syncTag; //同步标识
};

Q_DECLARE_METATYPE(DScheduleType)
Q_DECLARE_OPERATORS_FOR_FLAGS(DScheduleType::Privileges)

#endif // DSCHEDULEYPE_H
