// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ProxyAddon.h"

#include "Dim.h"

#include <QStandardPaths>

using namespace org::deepin::dim;

ProxyAddon::ProxyAddon(Dim *dim, const std::string &key, const QString &iconName)
    : InputMethodAddon(dim, key, iconName)
{
}

ProxyAddon::~ProxyAddon() { }

void ProxyAddon::updateActiveInputMethods(const std::vector<std::string> &value)
{
    activeInputMethods_ = value;

    ProxyEvent event(EventType::ProxyActiveInputMethodsChanged, this);
    dim()->postEvent(event);
}

bool ProxyAddon::isExecutableExisted(const QString &name)
{
    return !QStandardPaths::findExecutable(name, QStringList()).isEmpty();
}
