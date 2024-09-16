// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QString>
#include <QPair>
#include <QLocale>
#include <QVersionNumber>

//#include <string>
//#include <vector>

#define MAIN_WINDOW_W 750
#define MAIN_WINDOW_H 550
//#define MAIN_WINDOW_W 700
//#define MAIN_WINDOW_H 500
#define SOFTWARE_TABLE_ROW_H 40

enum class Orientation {
    Horizontal,
    Vertical
};

/**
 * @brief Type of License
 */
enum class LicenseType {
    USER,       // End User License Agreement
    PRIVACY,    // Privacy Protection Guide
};

/**
 * @brief The result of upgrade check entries
 */
enum class CheckResultType {
    PASSED,
    WARNING,
    FAILED,
};

/**
 * @brief Stages of system upgrade
 *
 */
enum class UpgradeStage {
    INIT,              // Upgrade hasn't yet been started. Set it for displaying errors after upgrade.
    EVALUATE_SOFTWARE, // Software Evaluation
    PREPARATION,       // Upgrade Preparation
    BACKUP,            // System Backup
    RESTORATION,       // System Restoration
    MIGRATION,         // App Migration
    MIGRATIONLIST,     // App Migration list
};

// QDBus async call timeout. Unit: ms
const int kQDBusAsyncCallTimeout = 5 * 3600000;
// The size of download buffer. Unit: Byte
const int kReadBufferSize = 512 * 1024;
// Interval of retry download request. Unit：ms
const int kRetryInterval = 5000;
// Download retry timeout. Unit：s
const int kReconnectTimeout = 300;

// ISO Download location
const QString kDownloadPath(".cache/deepin-system-upgrade-tool");
// Upgrade state file path
const QString kUpgradeStatesPath("/etc/deepin-upgrade-manager/state.records");

const QVersionNumber kUosMinimalVersion = QVersionNumber::fromString("1060");
const QVersionNumber kDeepinMinimalVersion = QVersionNumber::fromString("20.9");
const QVersionNumber kDeepinTargetVersion = QVersionNumber::fromString("23");
const QString kCurrentLocale = QLocale::system().name();
