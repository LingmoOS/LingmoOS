// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DimDBusType.h"

void registerDimQtDBusTypes()
{
    qRegisterMetaType<InputMethodData>("InputMethodData");
    qRegisterMetaType<InputMethodDataList>("InputMethodDataList");
    qRegisterMetaType<Config>("Config");
    qRegisterMetaType<ConfigOption>("ConfigOption");
    qRegisterMetaType<InputMethodConfigList>("InputMethodConfigList");
    qRegisterMetaType<InputMethodEntry>("InputMethodEntry");
    qRegisterMetaType<InputMethodEntryList>("InputMethodEntryList");

    qDBusRegisterMetaType<InputMethodData>();
    qDBusRegisterMetaType<InputMethodDataList>();
    qDBusRegisterMetaType<Config>();
    qDBusRegisterMetaType<ConfigOption>();
    qDBusRegisterMetaType<InputMethodConfigList>();
    qDBusRegisterMetaType<InputMethodEntry>();
    qDBusRegisterMetaType<InputMethodEntryList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const InputMethodData &data)
{
    argument.beginStructure();
    argument << data.addon << data.name;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, InputMethodData &data)
{
    argument.beginStructure();
    argument >> data.addon >> data.name;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigOption &data)
{
    argument.beginStructure();
    argument << data.arg1 << data.arg2 << data.arg3 << QDBusVariant(data.arg4) << data.arg5;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigOption &data)
{
    argument.beginStructure();
    argument >> data.arg1 >> data.arg2 >> data.arg3 >> data.arg4 >> data.arg5;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Config &data)
{
    argument.beginStructure();
    argument << data.name;
    argument.beginArray(qMetaTypeId<ConfigOption>());
    for (const auto &element : data.opt) {
        argument << element;
    }
    argument.endArray();
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Config &data)
{
    argument.beginStructure();
    argument >> data.name;
    argument.beginArray();
    while (!argument.atEnd()) {
        ConfigOption element;
        argument >> element;
        data.opt.append(element);
    }
    argument.endArray();
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const InputMethodEntry &data)
{
    argument.beginStructure();
    argument << data.addonKey << data.uniqueName << data.name << data.description
             << data.label << data.iconName;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, InputMethodEntry &data)
{
    argument.beginStructure();
    argument >> data.addonKey >> data.uniqueName >> data.name >> data.description
        >> data.label >> data.iconName;
    argument.endStructure();

    return argument;
}
