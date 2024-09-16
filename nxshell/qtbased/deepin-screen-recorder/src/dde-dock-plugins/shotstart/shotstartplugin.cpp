// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartplugin.h"
#include <DApplication>
#include <QDesktopWidget>
#include <QDBusInterface>

#define ShotShartPlugin "shot-start-plugin"
#define ShotShartApp "deepin-screen-recorder"  // 使用截图录屏的翻译
const QString QUICK_ITEM_KEY = QStringLiteral("quick_item_key");
const int DETECT_SERV_INTERVAL = 2000;  // 检测服务存在的定时器间隔

Q_LOGGING_CATEGORY(SHOT_LOG, "shot-start-plugin");

ShotStartPlugin::ShotStartPlugin(QObject *parent)
    : QObject(parent)
    , m_iconWidget(nullptr)
    , m_quickPanelWidget(nullptr)
    , m_tipsWidget(nullptr)

{
    m_isRecording = false;
    m_checkTimer = nullptr;
    m_bDockQuickPanel = false;
}

const QString ShotStartPlugin::pluginName() const
{
    return ShotShartPlugin;
}

const QString ShotStartPlugin::pluginDisplayName() const
{
    return tr("Screenshot");
}

void ShotStartPlugin::init(PluginProxyInterface *proxyInter)
{
#ifndef UNIT_TEST
    bool ret;
    bool getVersion = qApp->property("dock_api_version").toInt(&ret);
    qCInfo(SHOT_LOG) << "当前dock版本：" << getVersion << ret;
    if (ret && qApp->property("dock_api_version") >= ((2 << 16) | (0 << 8) | (0))) {
        m_bDockQuickPanel = true;
        qCWarning(SHOT_LOG) << "The current dock version does not support quick panels!!";
    }
    qCInfo(SHOT_LOG) << "正在加载翻译...";
    // 加载翻译
    QString appName = qApp->applicationName();
    qCDebug(SHOT_LOG) << "1 >>qApp->applicationName(): " << qApp->applicationName();
    qApp->setApplicationName(ShotShartApp);
    qCDebug(SHOT_LOG) << "2 >>qApp->applicationName(): " << qApp->applicationName();
    bool isLoad = qApp->loadTranslator();
    qApp->setApplicationName(appName);
    qCDebug(SHOT_LOG) << "3 >>qApp->applicationName(): " << qApp->applicationName();
    qCInfo(SHOT_LOG) << "翻译加载" << (isLoad ? "成功" : "失败");
#endif

    m_proxyInter = proxyInter;

    if (m_iconWidget.isNull())
        m_iconWidget.reset(new IconWidget);
    if (m_quickPanelWidget.isNull()) {
        m_quickPanelWidget.reset(new QuickPanelWidget);
        // "截图"快捷面板不再响应录制中动画效果，固定为截图图标
        m_quickPanelWidget->changeType(QuickPanelWidget::SHOT);
    }
    if (m_tipsWidget.isNull())
        m_tipsWidget.reset(new TipsWidget);

    if (m_bDockQuickPanel || !pluginIsDisable()) {
        qCInfo(SHOT_LOG) << "the current plugin has been added to the dock";
        m_proxyInter->itemAdded(this, pluginName());
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ShotRecorder.PanelStatus") &&
        sessionBus.registerObject("/com/deepin/ShotRecorder/PanelStatus", this, QDBusConnection::ExportScriptableSlots)) {
        qCWarning(SHOT_LOG) << "dbus service registration failed!";
    }

    connect(m_quickPanelWidget.data(), &QuickPanelWidget::clicked, this, &ShotStartPlugin::onClickQuickPanel);
}

bool ShotStartPlugin::pluginIsDisable()
{
    if (m_bDockQuickPanel) {
        qCWarning(SHOT_LOG) << "The current dock version does not support quick panels!!";
        return false;
    }
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

void ShotStartPlugin::pluginStateSwitched()
{
    const bool disabledNew = !pluginIsDisable();
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

QWidget *ShotStartPlugin::itemWidget(const QString &itemKey)
{
    qCDebug(SHOT_LOG) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey == QUICK_ITEM_KEY) {
        qCDebug(SHOT_LOG) << "Input Quick Panel Widget!";
        return m_quickPanelWidget.data();
    } else if (itemKey == ShotShartPlugin) {
        qCDebug(SHOT_LOG) << "Input Common Plugin Widget!";
        return m_iconWidget.data();
    }
    return nullptr;
}

QWidget *ShotStartPlugin::itemTipsWidget(const QString &itemKey)
{
    qCDebug(SHOT_LOG) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey != ShotShartPlugin)
        return nullptr;
    m_tipsWidget->setText(tr("Screenshot") + m_iconWidget->getSysShortcuts("screenshot"));
    return m_tipsWidget.data();
}

int ShotStartPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 1).toInt();
}

void ShotStartPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

const QString ShotStartPlugin::itemCommand(const QString &itemKey)
{
    qCDebug(SHOT_LOG) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey == ShotShartPlugin) {
        qCDebug(SHOT_LOG) << "(itemCommand) Input Common Plugin Widget!";
        // 录屏过程不响应点击
        if (!m_isRecording) {
            qCDebug(SHOT_LOG) << "Get DBus Interface";
            return "dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot "
                   "com.deepin.Screenshot.StartScreenshot";
        }
    } else {
        qCWarning(SHOT_LOG) << "(itemCommand) Input unknow widget!";
    }
    return QString();
}

const QString ShotStartPlugin::itemContextMenu(const QString &)
{
    // 拆分截图和录屏托盘图标，不再提供右键菜单
    return QString();
}

void ShotStartPlugin::invokedMenuItem(const QString &, const QString &, const bool)
{
    // 拆分截图和录屏托盘图标，不再提供右键菜单
    return;
}

bool ShotStartPlugin::onStart()
{
    qCDebug(SHOT_LOG) << "Disable screenshot tray icon";
    m_isRecording = true;
    m_iconWidget->setEnabled(false);
    m_iconWidget->update();

    m_quickPanelWidget->setEnabled(false);
    qCDebug(SHOT_LOG) << "(onStart) Is Recording? " << m_isRecording;
    return true;
}

void ShotStartPlugin::onStop()
{
    qCDebug(SHOT_LOG) << "(onStop) Is Recording? " << m_isRecording;
    m_isRecording = false;
    m_iconWidget->setEnabled(true);
    m_iconWidget->update();

    m_quickPanelWidget->setEnabled(true);
    qCDebug(SHOT_LOG) << "Enable screenshot tray icon";
}

void ShotStartPlugin::onRecording()
{
    qCDebug(SHOT_LOG) << "(onRecording) Is Recording" << m_isRecording;
    m_nextCount++;
    if (1 == m_nextCount) {
        if (!m_checkTimer) {
            m_checkTimer = new QTimer(this);
        }
        connect(m_checkTimer, &QTimer::timeout, this, [=] {
            //说明录屏还在进行中
            if (m_count < m_nextCount) {
                m_count = m_nextCount;
            }
            //说明录屏已经停止了
            else {
                qCWarning(SHOT_LOG) << qPrintable("Unsafe stop recoding!");
                onStop();
                m_checkTimer->stop();
            }
        });
        m_checkTimer->start(DETECT_SERV_INTERVAL);
    }

    if (m_checkTimer && !m_checkTimer->isActive()) {
        m_checkTimer->start(DETECT_SERV_INTERVAL);
    }
}

void ShotStartPlugin::onPause()
{
    // DoNothing
}

void ShotStartPlugin::onClickQuickPanel()
{
    // 录制中不再响应快捷面板
    qCDebug(SHOT_LOG) << "(onClickQuickPanel) 点击快捷面板";
    if (!m_isRecording) {
        qCDebug(SHOT_LOG) << "Get Shot DBus Interface";
        QDBusInterface shotDBusInterface(
            "com.deepin.Screenshot", "/com/deepin/Screenshot", "com.deepin.Screenshot", QDBusConnection::sessionBus());
        shotDBusInterface.asyncCall("StartScreenshot");
        qCDebug(SHOT_LOG) << "Shot and Recorder plugin start run!";
    }
}

ShotStartPlugin::~ShotStartPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();

    if (nullptr != m_quickPanelWidget)
        m_quickPanelWidget->deleteLater();
}
