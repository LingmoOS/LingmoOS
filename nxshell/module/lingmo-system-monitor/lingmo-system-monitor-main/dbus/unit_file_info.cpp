// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unit_file_info.h"

#include <QString>
#include <QDebug>

/**
 * @brief The UnitFileInfoData class
 */
class UnitFileInfoData : public QSharedData
{
public:
    // Constructor
    UnitFileInfoData() {}
    // Copy constructor
    UnitFileInfoData(const UnitFileInfoData &rhs)
        : QSharedData(rhs)
        , __alignment(rhs.__alignment)
        , m_name(rhs.m_name)
        , m_status(rhs.m_status)
    {
    }
    // Copy assignment
    UnitFileInfoData &operator=(const UnitFileInfoData &rhs)
    {
        Q_UNUSED(__alignment);

        if (this != &rhs) {
            m_name = rhs.m_name;
            m_status = rhs.m_status;
        }
        return *this;
    }
    ~UnitFileInfoData() {}

    friend class UnitFileInfo;

private:
    int __alignment {};  // simply used for suppressing compiler warnings
    // unit file name
    QString m_name {};
    // unit file status
    QString m_status {};
};

// Constructor
UnitFileInfo::UnitFileInfo()
    : data(new UnitFileInfoData)
{
}

// Overloaded constructor
UnitFileInfo::UnitFileInfo(const QString &name, const QString &status)
    : data(new UnitFileInfoData())
{
    data->m_name = name;
    data->m_status = status;
}

// Copy constructor
UnitFileInfo::UnitFileInfo(const UnitFileInfo &rhs)
    : data(rhs.data)
{
}

// Copy assignment
UnitFileInfo &UnitFileInfo::operator=(const UnitFileInfo &rhs)
{
    if (this != &rhs)
        data.operator = (rhs.data);
    return *this;
}

UnitFileInfo::~UnitFileInfo() {}

// Compare this UnitFileInfo object with others
bool UnitFileInfo::operator==(const UnitFileInfo &other) const
{
    return data->m_name == other.getName() && data->m_status == other.getStatus();
}

// Get unit file name
QString UnitFileInfo::getName() const
{
    return data->m_name;
}

// Set unit file name
void UnitFileInfo::setName(const QString &name)
{
    data->m_name = name;
}

// Get unit file status
QString UnitFileInfo::getStatus() const
{
    return data->m_status;
}

// Set unit file status
void UnitFileInfo::setStatus(const QString &status)
{
    data->m_status = status;
}

// Register UnitFileInfo meta type
void UnitFileInfo::registerMetaType()
{
    qRegisterMetaType<UnitFileInfo>("UnitFileInfo");
    qDBusRegisterMetaType<UnitFileInfo>();
    qRegisterMetaType<UnitFileInfoList>("UnitFileInfoList");
    qDBusRegisterMetaType<UnitFileInfoList>();
}

// Print UnitFileInfo object to debug stream
QDebug &operator<<(QDebug &debug, const UnitFileInfo &unit)
{
    debug << unit.getName() << unit.getStatus();
    return debug;
}

// Output UnitFileInfo object to DBus argument
QDBusArgument &operator<<(QDBusArgument &argument, const UnitFileInfo &unit)
{
    argument.beginStructure();
    argument << unit.getName() << unit.getStatus();
    argument.endStructure();
    return argument;
}

// Output UnitFileInfo object to data stream
QDataStream &operator<<(QDataStream &stream, const UnitFileInfo &unit)
{
    stream << unit.getName() << unit.getStatus();
    return stream;
}

// Read UnitFileInfo info from DBus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, UnitFileInfo &unit)
{
    QString name, status;
    argument.beginStructure();
    argument >> name >> status;
    unit.setName(name);
    unit.setStatus(status);
    argument.endStructure();
    return argument;
}

// Read UnitFileInfo object from data stream
const QDataStream &operator>>(QDataStream &stream, UnitFileInfo &unit)
{
    QString name, status;
    stream >> name >> status;
    unit.setName(name);
    unit.setStatus(status);
    return stream;
}

// Output list of UnitFileInfo objects to DBus argument
QDBusArgument &operator<<(QDBusArgument &argument, const UnitFileInfoList &list)
{
    argument.beginArray(qMetaTypeId<UnitFileInfoList>());
    foreach (const UnitFileInfo &unit, list) {
        argument << unit;
    }
    argument.endArray();
    return argument;
}

// Read list of UnitFileInfo objects from DBus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, UnitFileInfoList &list)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        UnitFileInfo unit;
        argument >> unit;
        list << unit;
    }
    argument.endArray();
    return argument;
}
