// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNIT_FILE_INFO_H
#define UNIT_FILE_INFO_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QtDBus>

class UnitFileInfoData;
class QDebug;

/**
 * @brief The UnitFileInfo class
 */
class UnitFileInfo
{
public:
    /**
     * @brief Default constructor
     */
    UnitFileInfo();
    /**
     * @brief Overloaded constructor
     * @param name Unit file name
     * @param status Unit file status
     */
    UnitFileInfo(const QString &name, const QString &status);
    /**
     * @brief Copy constructor
     */
    UnitFileInfo(const UnitFileInfo &);
    /**
     * @brief operator = Copy assignment operator
     * @return Copied version
     */
    UnitFileInfo &operator=(const UnitFileInfo &);
    ~UnitFileInfo();

    /**
     * @brief operator == Compare this UnitFileInfo with others
     * @param other Other UnitFileInfo
     * @return Compare result
     */
    bool operator==(const UnitFileInfo &other) const;

    /**
     * @brief getName Get unit file name
     * @return Unit file name
     */
    QString getName() const;
    /**
    * @brief setName Set unit file name
    * @param name Unit file name
    */
    void setName(const QString &name);

    /**
     * @brief getStatus Get unit file status
     * @return Unit file status
     */
    QString getStatus() const;
    /**
     * @brief setStatus Set unit file status
     * @param status Unit file Status
     */
    void setStatus(const QString &status);

    /**
     * @brief registerMetaType Register UnitFileInfo meta type
     */
    static void registerMetaType();

    /**
     * @brief operator << Print UnitFileInfo object to debug stream
     * @param debug Debug stream
     * @param unit Unit file info
     * @return Debug stream
     */
    friend QDebug &operator<<(QDebug debug, const UnitFileInfo &unit);
    /**
     * @brief operator << Print UnitFileInfo object to DBus argument
     * @param argument DBus argument object
     * @param unit Unit file info
     * @return DBus argument object
     */
    friend QDBusArgument &operator<<(QDBusArgument &argument, const UnitFileInfo &unit);
    /**
     * @brief operator << Print UnitFileInfo object to data stream
     * @param stream Data stream
     * @param unit Unit file info
     * @return Data stream
     */
    friend QDataStream &operator<<(QDataStream &stream, const UnitFileInfo &unit);
    /**
     * @brief operator >> Read UnitFileInfo object from DBus argument
     * @param argument DBus argument
     * @param unit Unit file info
     * @return DBus argument
     */
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, UnitFileInfo &unit);
    /**
     * @brief operator >> Read UnitFileInfo object from data stream
     * @param stream Data stream
     * @param unit Unit file info
     * @return Data stream
     */
    friend const QDataStream &operator>>(QDataStream &stream, UnitFileInfo &unit);

private:
    QExplicitlySharedDataPointer<UnitFileInfoData> data;
};

typedef QList<UnitFileInfo> UnitFileInfoList;

Q_DECLARE_METATYPE(UnitFileInfo)
Q_DECLARE_METATYPE(UnitFileInfoList)

/**
 * @brief operator << Print list of UnitFileInfo objects to DBus argument
 * @param argument DBus argument
 * @param list List of UnitFileInfo object
 * @return DBus argument
 */
QDBusArgument &operator<<(QDBusArgument &argument, const UnitFileInfoList &list);
/**
 * @brief operator >> Read list of UnitFileInfo objects from DBus argument
 * @param argument DBus argument
 * @param list List of UnitFileInfo object
 * @return DBus argument
 */
const QDBusArgument &operator>>(const QDBusArgument &argument, UnitFileInfoList &list);

#endif  // UNIT_FILE_INFO_H
