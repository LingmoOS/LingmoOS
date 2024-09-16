// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unit_info.h"

#include <QDebug>

/**
 * @brief The UnitInfoData class
 */
class UnitInfoData : public QSharedData
{
public:
    /**
     * @brief UnitInfoData constructor
     */
    UnitInfoData() {}
    /**
     * @brief Copy constructor
     * @param rhs Other UnitInfoData object
     */
    UnitInfoData(const UnitInfoData &rhs)
        : QSharedData(rhs)
        , m_jobId(rhs.m_jobId)
        , m_name(rhs.m_name)
        , m_description(rhs.m_description)
        , m_loadState(rhs.m_loadState)
        , m_activeState(rhs.m_activeState)
        , m_subState(rhs.m_subState)
        , m_followedBy(rhs.m_followedBy)
        , m_unitObjectPath(rhs.m_unitObjectPath)
        , m_jobType(rhs.m_jobType)
        , m_jobObjectPath(rhs.m_jobObjectPath)
    {
    }
    /**
     * @brief Copy assignment
     * @param rhs Other UnitInfoData object
     * @return Copied version
     */
    UnitInfoData &operator=(const UnitInfoData &rhs)
    {
        if (this != &rhs) {
            m_jobId = rhs.m_jobId;
            m_name = rhs.m_name;
            m_description = rhs.m_description;
            m_loadState = rhs.m_loadState;
            m_activeState = rhs.m_activeState;
            m_subState = rhs.m_subState;
            m_followedBy = rhs.m_followedBy;
            m_unitObjectPath = rhs.m_unitObjectPath;
            m_jobType = rhs.m_jobType;
            m_jobObjectPath = rhs.m_jobObjectPath;
        }
        return *this;
    }
    ~UnitInfoData() {}

    friend class UnitInfo;

private:
    // unit job Id
    quint32 m_jobId {0};
    // unit name
    QString m_name {};
    // unit description
    QString m_description {};
    // unit load state
    QString m_loadState {};
    // unit active state
    QString m_activeState {};
    // unit sub state
    QString m_subState {};
    // unit followedBy
    QString m_followedBy {};
    // unit object path
    QString m_unitObjectPath {};
    // unit job type
    QString m_jobType {};
    // job object path
    QString m_jobObjectPath {};
};

// Default constructor
UnitInfo::UnitInfo()
    : data(new UnitInfoData)
{
}

// Constructor with params
UnitInfo::UnitInfo(const QString &name,
                   const QString &description,
                   const QString &loadState,
                   const QString &activeState,
                   const QString &subState,
                   const QString &followedBy,
                   const QString &unitObjectPath,
                   quint32 jobId,
                   const QString &jobType,
                   const QString &jobObjectPath)
    : data(new UnitInfoData())
{
    data->m_name = name;
    data->m_description = description;
    data->m_loadState = loadState;
    data->m_activeState = activeState;
    data->m_subState = subState;
    data->m_followedBy = followedBy;
    data->m_unitObjectPath = unitObjectPath;
    data->m_jobId = jobId;
    data->m_jobType = jobType;
    data->m_jobObjectPath = jobObjectPath;
}

// Copy constructor
UnitInfo::UnitInfo(const UnitInfo &rhs)
    : data(rhs.data)
{
}

// Copy assignment
UnitInfo &UnitInfo::operator=(const UnitInfo &rhs)
{
    if (this != &rhs)
        data.operator = (rhs.data);
    return *this;
}

UnitInfo::~UnitInfo() {}

// Compare this UnitInfo object with others
bool UnitInfo::operator==(const UnitInfo &other) const
{
    return (
               data->m_name == other.getName() && data->m_description == other.getDescription() &&
               data->m_loadState == other.getLoadState() &&
               data->m_activeState == other.getActiveState() && data->m_subState == other.getSubState() &&
               data->m_followedBy == other.getFollowedBy() &&
               data->m_unitObjectPath == other.getUnitObjectPath() && data->m_jobId == other.getJobId() &&
               data->m_jobType == other.getJobType() && data->m_jobObjectPath == other.getJobObjectPath());
}

// Get unit name
QString UnitInfo::getName() const
{
    return data->m_name;
}

// Set unit name
void UnitInfo::setName(const QString &name)
{
    data->m_name = name;
}

// Get unit description
QString UnitInfo::getDescription() const
{
    return data->m_description;
}

// Set unit description
void UnitInfo::setDescription(const QString &description)
{
    data->m_description = description;
}

// Get unit load state
QString UnitInfo::getLoadState() const
{
    return data->m_loadState;
}
// Set unit load state
void UnitInfo::setLoadState(const QString &loadState)
{
    data->m_loadState = loadState;
}

// Get unit active state
QString UnitInfo::getActiveState() const
{
    return data->m_activeState;
}
// Set unit active state
void UnitInfo::setActiveState(const QString &activeState)
{
    data->m_activeState = activeState;
}

// Get unit sub state
QString UnitInfo::getSubState() const
{
    return data->m_subState;
}
// Set unit sub state
void UnitInfo::setSubState(const QString &subState)
{
    data->m_subState = subState;
}

// Get unit followedBy
QString UnitInfo::getFollowedBy() const
{
    return data->m_followedBy;
}
// Set unit followedBy
void UnitInfo::setFollowedBy(const QString &followedBy)
{
    data->m_followedBy = followedBy;
}

// Get unit object path
QString UnitInfo::getUnitObjectPath() const
{
    return data->m_unitObjectPath;
}
// Set unit object path
void UnitInfo::setUnitObjectPath(const QString &unitObjectPath)
{
    data->m_unitObjectPath = unitObjectPath;
}

// Get job Id
quint32 UnitInfo::getJobId() const
{
    return data->m_jobId;
}
// Set job Id
void UnitInfo::setJobId(quint32 jobId)
{
    data->m_jobId = jobId;
}

// Get job type
QString UnitInfo::getJobType() const
{
    return data->m_jobType;
}
// Set job type
void UnitInfo::setJobType(const QString &jobType)
{
    data->m_jobType = jobType;
}

// Get job object path
QString UnitInfo::getJobObjectPath() const
{
    return data->m_jobObjectPath;
}
// Set job object path
void UnitInfo::setJobObjectPath(const QString &jobObjectPath)
{
    data->m_jobObjectPath = jobObjectPath;
}

// Register UnitInfo meta type
void UnitInfo::registerMetaType()
{
    qRegisterMetaType<UnitInfo>("UnitInfo");
    qDBusRegisterMetaType<UnitInfo>();
    qRegisterMetaType<UnitInfoList>("UnitInfoList");
    qDBusRegisterMetaType<UnitInfoList>();
}

// Print UnitInfo object to debug stream
QDebug &operator<<(QDebug &debug, const UnitInfo &unit)
{
    debug << unit.getName() << unit.getDescription() << unit.getLoadState() << unit.getActiveState()
          << unit.getSubState() << unit.getFollowedBy() << unit.getUnitObjectPath()
          << unit.getJobId() << unit.getJobType() << unit.getJobObjectPath();
    return debug;
}

// Output UnitInfo object to DBus argument
QDBusArgument &operator<<(QDBusArgument &argument, const UnitInfo &unit)
{
    argument.beginStructure();
    argument << unit.getName() << unit.getDescription() << unit.getLoadState()
             << unit.getActiveState() << unit.getSubState() << unit.getFollowedBy()
             << unit.getUnitObjectPath() << unit.getJobId() << unit.getJobType()
             << unit.getJobObjectPath();
    argument.endStructure();
    return argument;
}

// Output UnitInfo object to data stream
QDataStream &operator<<(QDataStream &stream, const UnitInfo &unit)
{
    stream << unit.getName() << unit.getDescription() << unit.getLoadState()
           << unit.getActiveState() << unit.getSubState() << unit.getFollowedBy()
           << unit.getUnitObjectPath() << unit.getJobId() << unit.getJobType()
           << unit.getJobObjectPath();
    return stream;
}

// Read UnitInfo object from DBus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, UnitInfo &unit)
{
    QString name, description, loadState, activeState, subState, followedBy, unitObjectPath,
            jobType, jobObjectPath;
    quint32 jobId;
    argument.beginStructure();
    argument >> name >> description >> loadState >> activeState >> subState >> followedBy >>
             unitObjectPath >> jobId >> jobType >> jobObjectPath;
    unit.setName(name);
    unit.setDescription(description);
    unit.setLoadState(loadState);
    unit.setActiveState(activeState);
    unit.setSubState(subState);
    unit.setFollowedBy(followedBy);
    unit.setUnitObjectPath(unitObjectPath);
    unit.setJobId(jobId);
    unit.setJobType(jobType);
    unit.setJobObjectPath(jobObjectPath);
    argument.endStructure();
    return argument;
}

// Read UnitInfo object from data stream
const QDataStream &operator>>(QDataStream &stream, UnitInfo &unit)
{
    QString name, description, loadState, activeState, subState, followedBy, unitObjectPath,
            jobType, jobObjectPath;
    quint32 jobId;
    stream >> name >> description >> loadState >> activeState >> subState >> followedBy >>
           unitObjectPath >> jobId >> jobType >> jobObjectPath;
    unit.setName(name);
    unit.setDescription(description);
    unit.setLoadState(loadState);
    unit.setActiveState(activeState);
    unit.setSubState(subState);
    unit.setFollowedBy(followedBy);
    unit.setUnitObjectPath(unitObjectPath);
    unit.setJobId(jobId);
    unit.setJobType(jobType);
    unit.setJobObjectPath(jobObjectPath);
    return stream;
}

// Output list of UnitInfo objects to DBus argument
QDBusArgument &operator<<(QDBusArgument &argument, const UnitInfoList &list)
{
    argument.beginArray(qMetaTypeId<UnitInfo>());
    foreach (const UnitInfo &unit, list) {
        argument << unit;
    }
    argument.endArray();
    return argument;
}

// Read list of UnitInfo objects from DBus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, UnitInfoList &list)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        UnitInfo unit;
        argument >> unit;
        list << unit;
    }
    argument.endArray();
    return argument;
}
