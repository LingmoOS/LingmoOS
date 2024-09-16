// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QStringList>

namespace utils
{

QString authCodeURL(const QString &clientID,
                    const QStringList &scopes,
                    const QString &callback,
                    const QString &state);

QString authCodeURL(const QString &path,
                    const QString &clientID,
                    const QStringList &scopes,
                    const QString &callback,
                    const QString &state);

QString windowSizeURL();

QString getThemeName();

QString getActiveColor();

QString getStandardFont();

QString getDeviceKernel();

QString getOsVersion();

QString getDeviceCode();

void sendDBusNotify(const QString &message);

QString getLang(const QString &region);

bool isTablet();

QString getDeviceType();

QStringList getDeviceInfo();

}
