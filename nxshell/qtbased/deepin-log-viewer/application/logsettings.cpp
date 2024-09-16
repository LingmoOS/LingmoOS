// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logsettings.h"
#include "utils.h"

#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#define MAINWINDOW_HEIGHT_NAME "logMainWindowHeightName"
#define MAINWINDOW_WIDTH_NAME "logMainWindowWidthName"

std::atomic<LogSettings *> LogSettings::m_instance;
std::mutex LogSettings::m_mutex;

// 审计类型归类规则配置文件
const QString AUDIT_CONFIG_PATH = "/usr/share/deepin-log-viewer/auditRule.conf";

/**
 * @brief LogSettings::LogSettings 构造函数从配置文件初始化配置
 * @param parent　父对象
 */
LogSettings::LogSettings(QObject *parent)
    : QObject(parent),
      m_winInfoConfig(nullptr),
      m_logDirConfig(nullptr),
      m_configPath(""),
      m_logDirPath("")
{
    QDir infoPath(Utils::getConfigPath());
    if (!infoPath.exists()) {
        infoPath.mkpath(Utils::getConfigPath());
    }

    m_configPath = infoPath.filePath("wininfo-config.conf");
    m_winInfoConfig = new QSettings(m_configPath, QSettings::IniFormat, this);

    m_logDirPath = infoPath.filePath("logdir-config.conf");
    m_logDirConfig = new QSettings(m_logDirPath, QSettings::IniFormat, this);
}

/**
 * @brief LogSettings::getConfigWinSize　通过配置文件获取之前保存的窗口大小
 * @return　之前保存的窗口大小
 */
QSize LogSettings::getConfigWinSize()
{
    QVariant tempHeight = m_winInfoConfig->value(MAINWINDOW_HEIGHT_NAME);
    QVariant tempWidth = m_winInfoConfig->value(MAINWINDOW_WIDTH_NAME);
    int winHeight = MAINWINDOW_HEIGHT;
    int winWidth = MAINWINDOW_WIDTH;
    if (tempHeight.isValid()) {

        winHeight = tempHeight.toInt();
        winHeight = winHeight > MAINWINDOW_HEIGHT ? winHeight : MAINWINDOW_HEIGHT;
    }
    if (tempHeight.isValid()) {

        winWidth = tempWidth.toInt();
        winWidth = winWidth > MAINWINDOW_WIDTH ? winWidth : MAINWINDOW_WIDTH;
    }

    return  QSize(winWidth, winHeight);
}

/**
 * @brief LogSettings::saveConfigWinSize 保存当前窗口尺寸到配置文件
 * @param w　当前窗口宽度
 * @param h　当前窗口高度
 */
void LogSettings::saveConfigWinSize(int w, int h)
{
    int winWidth = w > MAINWINDOW_WIDTH ? w : MAINWINDOW_WIDTH;
    int winHeight = h > MAINWINDOW_HEIGHT ? h : MAINWINDOW_HEIGHT;
    m_winInfoConfig->setValue(MAINWINDOW_HEIGHT_NAME, winHeight);
    m_winInfoConfig->setValue(MAINWINDOW_WIDTH_NAME, winWidth);
    m_winInfoConfig->sync();
}

QMap<QString, QStringList> LogSettings::loadAuditMap()
{
    QMap<QString, QStringList> auditType2EventType;
    QSettings auditConfig(AUDIT_CONFIG_PATH, QSettings::IniFormat);
    QStringList auditTypes = auditConfig.childKeys();
    for (auto auditType : auditTypes) {
        QString value = auditConfig.value(auditType).toString();
        QStringList strList = value.split('@', QString::SkipEmptyParts);
        auditType2EventType.insert(auditType, strList);
    }

    return auditType2EventType;
}
