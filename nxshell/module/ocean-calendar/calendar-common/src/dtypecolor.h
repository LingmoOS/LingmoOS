// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTYPECOLOR_H
#define DTYPECOLOR_H

#include <QString>
#include <QSharedPointer>
#include <QVector>
#include <QDateTime>

/**
 * @brief The DTypeColor class
 * 类型颜色
 */
class DTypeColor
{
public:
    //用户自定义的可读可编辑
    //系统默认只读
    enum Privilege {
        PriSystem = 1, //系统默认
        PriUser = 7 //用户自定义
    };

    typedef QSharedPointer<DTypeColor> Ptr;
    typedef QVector<Ptr> List;

    DTypeColor();
    DTypeColor(const DTypeColor &typeColor);

    QString colorCode() const;
    void setColorCode(const QString &colorCode);

    QString colorID() const;
    void setColorID(const QString &colorID);

    Privilege privilege() const;
    void setPrivilege(const Privilege &privilege);

    bool isSysColorInfo();

    bool operator!=(const DTypeColor &color) const;

    static List fromJsonString(const QString &colorJson);
    static QString toJsonString(const List &colorList);

    QDateTime dtCreate() const;
    void setDtCreate(const QDateTime &dtCreate);

    friend bool operator<(const DTypeColor::Ptr &tc1, const DTypeColor::Ptr &tc2);

private:
    QString m_colorID;
    QString m_colorCode; //颜色码
    Privilege m_privilege;
    QDateTime m_dtCreate;
};

#endif // DTYPECOLOR_H
