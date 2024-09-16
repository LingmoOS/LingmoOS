// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartrecordplugin.h"

#include <DApplication>
#include <QDesktopWidget>
#include <QDBusInterface>

#define RecordShartPlugin "shot-start-record-plugin"
#define RecordShartApp "deepin-screen-recorder"  // 使用截图录屏的翻译
const QString QUICK_ITEM_KEY = QStringLiteral("quick_item_key");
const int DETECT_SERV_INTERVAL = 2000;  // 检测服务存在的定时器间隔

Q_LOGGING_CATEGORY(RECORD_LOG, "shot-start-record-plugin");

ShotStartRecordPlugin::ShotStartRecordPlugin(QObject *parent)
    : QObject(parent)
    , m_iconWidget(nullptr)
    , m_quickPanelWidget(nullptr)
    , m_tipsWidget(nullptr)

{
    m_isRecording = false;
    m_checkTimer = nullptr;
    m_bDockQuickPanel = false;
}

/**
 * @return pluginName:返回插件名称
 */
const QString ShotStartRecordPlugin::pluginName() const
{
    return RecordShartPlugin;
}

/**
 * @return pluginDisplayName:返回插件名称，用于界面显示
 */
const QString ShotStartRecordPlugin::pluginDisplayName() const
{
    return tr("Record");
}

/**
 * @brief init:插件初始化入口函数
 * @param proxyInter:主程序进程，上下文指针
 */
void ShotStartRecordPlugin::init(PluginProxyInterface *proxyInter)
{
#ifndef UNIT_TEST
    bool ret;
    bool getVersion = qApp->property("dock_api_version").toInt(&ret);
    qCInfo(RECORD_LOG) << "当前dock版本：" << getVersion << ret;
    if (ret && qApp->property("dock_api_version") >= ((2 << 16) | (0 << 8) | (0))) {
        m_bDockQuickPanel = true;
        qCWarning(RECORD_LOG) << "The current dock version does not support quick panels!!";
    }
    qCInfo(RECORD_LOG) << "正在加载翻译...";
    // 加载翻译
    QString appName = qApp->applicationName();
    qCDebug(RECORD_LOG) << "1 >>qApp->applicationName(): " << qApp->applicationName();
    qApp->setApplicationName(RecordShartApp);
    qCDebug(RECORD_LOG) << "2 >>qApp->applicationName(): " << qApp->applicationName();
    bool isLoad = qApp->loadTranslator();
    qApp->setApplicationName(appName);
    qCDebug(RECORD_LOG) << "3 >>qApp->applicationName(): " << qApp->applicationName();
    qCInfo(RECORD_LOG) << "翻译加载" << (isLoad ? "成功" : "失败");
#endif

    m_proxyInter = proxyInter;

    if (m_iconWidget.isNull())
        m_iconWidget.reset(new RecordIconWidget);
    if (m_quickPanelWidget.isNull()) {
        m_quickPanelWidget.reset(new QuickPanelWidget);
        m_quickPanelWidget->changeType(QuickPanelWidget::RECORD);
    }
    if (m_tipsWidget.isNull())
        m_tipsWidget.reset(new TipsWidget);

    if (m_bDockQuickPanel || !pluginIsDisable()) {
        qCInfo(RECORD_LOG) << "the current plugin has been added to the dock";
        m_proxyInter->itemAdded(this, pluginName());
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ShotRecorder.Recorder.PanelStatus") &&
        sessionBus.registerObject(
            "/com/deepin/ShotRecorder/Recorder/PanelStatus", this, QDBusConnection::ExportScriptableSlots)) {
        qCWarning(RECORD_LOG) << "dbus service registration failed!";
    }

    connect(m_quickPanelWidget.data(), &QuickPanelWidget::clicked, this, &ShotStartRecordPlugin::onClickQuickPanel);
}

/**
   @return 返回当前插件是否被禁用
 */
bool ShotStartRecordPlugin::pluginIsDisable()
{
    if (m_bDockQuickPanel) {
        qCWarning(RECORD_LOG) << "The current dock version does not support quick panels!!";
        return false;
    }
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

/**
   @brief 切换插件禁用状态
 */
void ShotStartRecordPlugin::pluginStateSwitched()
{
    const bool disabledNew = !pluginIsDisable();
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

/**
 * @brief itemWidget:返回插件主控件，用于dde-dock面板上显示
 * @param itemKey:控件名称
 */
QWidget *ShotStartRecordPlugin::itemWidget(const QString &itemKey)
{
    qCDebug(RECORD_LOG) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey == QUICK_ITEM_KEY) {
        qCDebug(RECORD_LOG) << "Input Quick Panel Widget!";
        return m_quickPanelWidget.data();
    } else if (itemKey == RecordShartPlugin) {
        qCDebug(RECORD_LOG) << "Input Common Plugin Widget!";
        return m_iconWidget.data();
    }
    return nullptr;
}

/**
   @return 返回鼠标Hover时的Tips信息，目前为快捷键信息
 */
QWidget *ShotStartRecordPlugin::itemTipsWidget(const QString &itemKey)
{
    qCDebug(RECORD_LOG) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey != RecordShartPlugin)
        return nullptr;
    m_tipsWidget->setText(tr("Record") + m_iconWidget->getSysShortcuts("deepin-screen-recorder"));
    return m_tipsWidget.data();
}

/**
   @return 返回当前托盘图标的顺序
 */
int ShotStartRecordPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 1).toInt();
}

/**
   @brief 记录当前托盘图标的顺序
 */
void ShotStartRecordPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

/**
   @brief 鼠标单击托盘图标执行指令，唤起截图录屏
 */
const QString ShotStartRecordPlugin::itemCommand(const QString &itemKey)
{
    qCDebug(RECORD_LOG) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey == RecordShartPlugin) {
        qCDebug(RECORD_LOG) << "(itemCommand) Input Common Plugin Widget!";
        return "dbus-send --print-reply --dest=com.deepin.ScreenRecorder /com/deepin/ScreenRecorder "
               "com.deepin.ScreenRecorder.stopRecord";
    } else {
        qCWarning(RECORD_LOG) << "(itemCommand) Input unknow widget!";
    }
    return QString();
}

/**
   @brief 请求右键菜单内容，目前不再提供右键菜单
 */
const QString ShotStartRecordPlugin::itemContextMenu(const QString &)
{
    // 不再提供右键菜单
    return QString();
}

/**
   @brief 右键菜单触发，目前不再提供右键菜单
 */
void ShotStartRecordPlugin::invokedMenuItem(const QString &, const QString &, const bool)
{
    // 不再提供右键菜单
    return;
}

/**
   @brief 录屏开始，隐藏任务栏托盘图标，快捷面板切换录制中
        任务栏"录制中"图标由recordtime插件提供
   @return 是否启动成功
 */
bool ShotStartRecordPlugin::onStart()
{
    m_bPreviousIsVisable = getTrayIconVisible();
    if (m_bPreviousIsVisable) {
        // 仅隐藏任务栏图标
        setTrayIconVisible(false);
    }

    qCDebug(RECORD_LOG) << "Start The Clock!";
    m_isRecording = true;
    m_quickPanelWidget->start();
    qCDebug(RECORD_LOG) << "(onStart) Is Recording? " << m_isRecording;
    m_quickPanelWidget->changeType(QuickPanelWidget::RECORDING);
    return true;
}

/**
   @brief 录屏结束，复位托盘图标和快捷面板图标
 */
void ShotStartRecordPlugin::onStop()
{
    if (m_bPreviousIsVisable) {
        // 恢复显示任务栏图标
        setTrayIconVisible(true);
    }

    m_isRecording = false;
    m_quickPanelWidget->stop();
    qCDebug(RECORD_LOG) << "(onStop) Is Recording? " << m_isRecording;
    m_quickPanelWidget->changeType(QuickPanelWidget::RECORD);
    qCDebug(RECORD_LOG) << "End The Clock!";
}

/**
   @brief 接收"录制中"消息，防止由于其它原因导致截图录屏退出而托盘图标未复原
 */
void ShotStartRecordPlugin::onRecording()
{
    qCDebug(RECORD_LOG) << "(onRecording) Is Recording" << m_isRecording;
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
                qCWarning(RECORD_LOG) << qPrintable("Unsafe stop recoding!");
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

/**
   @brief 接收暂停信号并设置快捷面板图标状态
 */
void ShotStartRecordPlugin::onPause()
{
    qCDebug(RECORD_LOG) << "(onPause) Is Recording? " << m_isRecording;
    m_quickPanelWidget->pause();
    qCDebug(RECORD_LOG) << "Pause The Clock!";
}

/**
   @brief 快捷面板点击时弹出截图录屏录制
 */
void ShotStartRecordPlugin::onClickQuickPanel()
{
    qCDebug(RECORD_LOG) << "(onClickQuickPanel) 点击快捷面板";

    qCDebug(RECORD_LOG) << "Get Record DBus Interface";
    QDBusInterface recordDBusInterface(
        "com.deepin.ScreenRecorder", "/com/deepin/ScreenRecorder", "com.deepin.ScreenRecorder", QDBusConnection::sessionBus());
    recordDBusInterface.asyncCall("stopRecord");
    qCDebug(RECORD_LOG) << "Recorder plugin stop run!";
}

/**
   @brief 录制过程中给设置任务栏图标隐藏，但快捷面板图标仍显示。
 */
void ShotStartRecordPlugin::setTrayIconVisible(bool visible)
{
    // 使用DBus接口仅隐藏任务栏图标，快捷面板图标仍显示，参数是插件的 displayName（历史原因）
    QDBusInterface interface("com.deepin.dde.Dock", "/com/deepin/dde/Dock", "com.deepin.dde.Dock", QDBusConnection::sessionBus());
    interface.call("setPluginVisible", pluginDisplayName(), visible);
}

/**
   @return 当前图标在任务栏是否可见
 */
bool ShotStartRecordPlugin::getTrayIconVisible()
{
    QDBusInterface interface("com.deepin.dde.Dock", "/com/deepin/dde/Dock", "com.deepin.dde.Dock", QDBusConnection::sessionBus());
    auto msg = interface.call("getPluginVisible", pluginDisplayName());
    if (QDBusMessage::ReplyMessage == msg.type()) {
        return msg.arguments().takeFirst().toBool();
    }
    return false;
}

ShotStartRecordPlugin::~ShotStartRecordPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();

    if (nullptr != m_quickPanelWidget)
        m_quickPanelWidget->deleteLater();
}
