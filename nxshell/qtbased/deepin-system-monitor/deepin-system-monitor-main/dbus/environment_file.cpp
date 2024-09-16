// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environment_file.h"

// Default constructor
EnvironmentFile::EnvironmentFile()
    : envFile()
    , flag(false)
{
}

// Copy constructor
EnvironmentFile::EnvironmentFile(const EnvironmentFile &rhs) :
    envFile(rhs.envFile),
    flag(rhs.flag)
{
}

// Copy assignment
EnvironmentFile &EnvironmentFile::operator=(const EnvironmentFile &rhs)
{
    if (this != &rhs) {
        envFile = rhs.envFile;
        flag = rhs.flag;
    }
    return *this;
}

// Destructor
EnvironmentFile::~EnvironmentFile() {}

// Register meta type
void EnvironmentFile::registerMetaType()
{
    qRegisterMetaType<EnvironmentFile>("EnvironmentFile");
    qDBusRegisterMetaType<EnvironmentFile>();
    qRegisterMetaType<EnvironmentFileList>("EnvironmentFileList");
    qDBusRegisterMetaType<EnvironmentFileList>();
}

// Print EnvironmentFile object to debug stream
QDebug &operator<<(QDebug &debug, const EnvironmentFile &file)
{
    debug << file.envFile << " flag: " << file.flag;
    return debug;
}

// Print EnvironmentFile object to DBus argument object
QDBusArgument &operator<<(QDBusArgument &argument, const EnvironmentFile &file)
{
    argument.beginStructure();
    argument << file.envFile << file.flag;
    argument.endStructure();
    return argument;
}

// Print EnvironmentFile object to data stream
QDataStream &operator<<(QDataStream &stream, const EnvironmentFile &file)
{
    stream << file.envFile << file.flag;
    return stream;
}

// Read EnvironmentFile object from DBus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, EnvironmentFile &file)
{
    argument.beginStructure();
    argument >> file.envFile >> file.flag;
    argument.endStructure();
    return argument;
}

// Read EnvironmentFile object from data stream
const QDataStream &operator>>(QDataStream &stream, EnvironmentFile &file)
{
    stream >> file.envFile >> file.flag;
    return stream;
}
