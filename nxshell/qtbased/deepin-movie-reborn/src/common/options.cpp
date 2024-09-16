// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "options.h"

namespace dmr {

static CommandLineManager* _instance = nullptr;

CommandLineManager& CommandLineManager::get()
{
    if (!_instance) {
        _instance = new CommandLineManager();
    }

    return *_instance;
}

CommandLineManager::CommandLineManager()
{
    addHelpOption();
    addVersionOption();

    addPositionalArgument("path", ("Movie file path or directory"));
    addOptions({
        {{"V", "verbose"}, ("show detail log message")},
        {"VV", ("dump all debug message")},
        {{"c", "gpu"}, ("use gpu interface [on/off/auto]"), "bool", "auto"},
        {{"o", "override-config"}, ("override config for libmpv"), "file", ""},
        {"dvd-device", ("specify dvd playing device or file"), "device", "/dev/sr0"},
    });
}

bool CommandLineManager::verbose() const
{
    return this->isSet("verbose");
}

bool CommandLineManager::debug() const
{
    return this->isSet("VV");
}

QString CommandLineManager::openglMode() const
{
    return this->value("c");
//    return "on";
}

QString CommandLineManager::overrideConfig() const
{
    return this->value("o");
}

QString CommandLineManager::dvdDevice() const
{
    if (this->isSet("dvd-device")) {
        return this->value("dvd-device").trimmed();
    }

    return "";
}

}
