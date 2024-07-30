/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pluginbase.h"

using namespace Purpose;

PluginBase::PluginBase(QObject *parent)
    : QObject(parent)
{
}

PluginBase::~PluginBase()
{
}

#include "moc_pluginbase.cpp"
