// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNIT_INFO_H
#define UNIT_INFO_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QtDBus>

class UnitInfoData;
class QDebug;

/**
 * @brief The UnitInfo class
 */
class UnitInfo
{
public:
    /**
     * @brief UnitInfo constructor
     */
    UnitInfo();
    /**
     * @brief Constructor with params
     * @param name Unit name
     * @param description Unit description
     * @param loadState Unit load state
     * @param activeState Unit active state
     * @param subState Unit sub state
     * @param followedBy Unit followedBy
     * @param unitObjectPath Unit object path
     * @param jobId Unit job Id
     * @param jobType Unit job type
     * @param jobObjectPath Job object path
     */
    UnitInfo(const QString &name,
             const QString &description,
             const QString &loadState,
             const QString &activeState,
             const QString &subState,
             const QString &followedBy,
             const QString &unitObjectPath,
             quint32 jobId,
             const QString &jobType,
             const QString &jobObjectPath);
    /**
     * @brief Copy constructor
     */
    UnitInfo(const UnitInfo &);
    /**
     * @brief operator = Copy assignment
     * @return Copied version
     */
    UnitInfo &operator=(const UnitInfo &);
    ~UnitInfo();

    /**
     * @brief operator == Compare UnitInfo with others
     * @param other Other UnitInfo object
     * @return Compare result
     */
    bool operator==(const UnitInfo &other) const;

    /**
     * @brief getName Get unit name
     * @return Unit name
     */
    QString getName() const;
    /**
    * @brief setName Set unit name
    * @param name Unit name
    */
    void setName(const QString &name);

    /**
     * @brief getDescription Get unit description
     * @return Unit description
     */
    QString getDescription() const;
    /**
     * @brief setDescription Set unit description
     * @param description Unit description
     */
    void setDescription(const QString &description);

    /**
     * @brief getLoadState Get unit load state
     * @return Unit load state
     */
    QString getLoadState() const;
    /**
    * @brief setLoadState Set unit load state
    * @param loadState Unit load state
    */
    void setLoadState(const QString &loadState);

    /**
     * @brief getActiveState Get unit active state
     * @return Unit active state
     */
    QString getActiveState() const;
    /**
    * @brief setActiveState Set unit active state
    * @param activeState Set unit active state
    */
    void setActiveState(const QString &activeState);

    /**
     * @brief getSubState Get unit sub state
     * @return Unit sub state
     */
    QString getSubState() const;
    /**
     * @brief setSubState Set unit sub state
     * @param subState Unit sub state
     */
    void setSubState(const QString &subState);

    /**
     * @brief getFollowedBy Get unit followedBy
     * @return Unit's followedBy
     */
    QString getFollowedBy() const;
    /**
     * @brief setFollowedBy Set unit followedBy
     * @param followedBy Unit's followedBy
     */
    void setFollowedBy(const QString &followedBy);

    /**
     * @brief getUnitObjectPath Get unit object path
     * @return Unit object path
     */
    QString getUnitObjectPath() const;
    /**
     * @brief setUnitObjectPath Set unit object path
     * @param unitObjectPath Unit object path
     */
    void setUnitObjectPath(const QString &unitObjectPath);

    /**
     * @brief getJobId Get unit job Id
     * @return Unit job Id
     */
    quint32 getJobId() const;
    /**
     * @brief setJobId Set Unit job Id
     * @param jobId Unit Job Id
     */
    void setJobId(quint32 jobId);

    /**
     * @brief getJobType Get Unit job type
     * @return Unit job type
     */
    QString getJobType() const;
    /**
     * @brief setJobType Set unit job type
     * @param jobType Unit job type
     */
    void setJobType(const QString &jobType);

    /**
     * @brief getJobObjectPath Get job object path
     * @return Job object path
     */
    QString getJobObjectPath() const;
    /**
     * @brief setJobObjectPath Set job object path
     * @param jobObjectPath Job object path
     */
    void setJobObjectPath(const QString &jobObjectPath);

    /**
     * @brief registerMetaType Register UnitInfo meta type
     */
    static void registerMetaType();

    /**
     * @brief operator << Print UnitInfo object to debug stream
     * @param debug Debug stream
     * @param unit UnitInfo object
     * @return Debug stream
     */
    friend QDebug &operator<<(QDebug &debug, const UnitInfo &unit);
    /**
     * @brief operator << Output UnitInfo object to DBus argument
     * @param argument DBus argument
     * @param unit UnitInfo object
     * @return DBus argument
     */
    friend QDBusArgument &operator<<(QDBusArgument &argument, const UnitInfo &unit);
    /**
     * @brief operator << Output UnitInfo object to data stream
     * @param stream Data stream
     * @param unit UnitInfo object
     * @return Data stream
     */
    friend QDataStream &operator<<(QDataStream &stream, const UnitInfo &unit);
    /**
     * @brief operator >> Read UnitInfo object from DBus argument
     * @param argument DBus argument
     * @param unit UnitInfo object
     * @return DBus argument
     */
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, UnitInfo &unit);
    /**
     * @brief operator >> Read UnitInfo object from data stream
     * @param stream Data stream
     * @param unit UnitInfo object
     * @return Data stream
     */
    friend const QDataStream &operator>>(QDataStream &stream, UnitInfo &unit);

private:
    QExplicitlySharedDataPointer<UnitInfoData> data;
};

typedef QList<UnitInfo> UnitInfoList;

Q_DECLARE_METATYPE(UnitInfo)
Q_DECLARE_METATYPE(UnitInfoList)

/**
 * @brief operator << Output list of UnitInfo objects to DBus argument
 * @param argument DBus argument
 * @param list List of UnitInfo objects
 * @return DBus argument
 */
QDBusArgument &operator<<(QDBusArgument &argument, const UnitInfoList &list);
/**
 * @brief operator >> Read list of UnitInfo object from DBus argument
 * @param argument DBus argument
 * @param list List of UnitInfo object
 * @return DBus argument
 */
const QDBusArgument &operator>>(const QDBusArgument &argument, UnitInfoList &list);

#endif  // UNIT_INFO_H
