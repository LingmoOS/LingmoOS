// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QStringList>

// Whether the program is a Chromium build (browser, Electron or CEF app),
// judged by the resources shipped beside its binary.
bool isChromiumProgram(const QString &program);

// The command with the switches Chromium-based programs are started with.
QStringList withChromiumSwitches(const QStringList &command);
