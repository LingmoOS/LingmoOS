// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "systeminformation.h"

/**
 * @brief QML exposure class for DrKonqi.
 * Since DrKonqi is not meant for construction but also not a proper singleton it sports
 * static functions that we'll want to pull into a context object that actually can serve
 * as singleton. This is meant to be a temporary crutch, ideally DrKonqi gets
 * refactored to a proper singleton.
 */
class Doctore : public QObject
{
    Q_OBJECT
    // TODO redo DrKonqi so it can work with QML and convert everything to properties.
public:
    using QObject::QObject;

    static Q_INVOKABLE void saveReport(const QString &text);
    static Q_INVOKABLE void copyToClipboard(const QString &text);
    static Q_INVOKABLE QString appName();
    static Q_INVOKABLE QString kdeBugzillaURL();
    static Q_INVOKABLE QString kdeBugzillaDomain();
    static Q_INVOKABLE bool isSafer();
    static Q_INVOKABLE bool ignoreQuality();

private:
    Q_PROPERTY(SystemInformation *systemInformation READ systemInformation CONSTANT)
    static SystemInformation *systemInformation();
};
