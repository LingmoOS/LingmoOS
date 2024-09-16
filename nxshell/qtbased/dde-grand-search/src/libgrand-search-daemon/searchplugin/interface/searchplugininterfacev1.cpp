// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchplugininterfacev1.h"

using namespace GrandSearch;

SearchPluginInterfaceV1::SearchPluginInterfaceV1(const QString &service, const QString &path, const char *interface,
                                                 const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, interface, connection, parent)
{
}

SearchPluginInterfaceV1::~SearchPluginInterfaceV1()
{
}

