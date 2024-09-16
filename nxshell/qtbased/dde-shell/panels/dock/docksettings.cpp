// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsglobal.h"
#include "constants.h"
#include "docksettings.h"

#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(dockSettingsLog, "dde.shell.dock.docksettings")

const static QString keyPosition                = "Position";
const static QString keyHideMode                = "Hide_Mode";
const static QString keyDockSize                = "Dock_Size";
const static QString keyItemAlignment           = "Item_Alignment";
const static QString keyIndicatorStyle          = "Indicator_Style";
const static QString keyPluginsVisible           = "Plugins_Visible";
const static QString keyShowInPrimary           = "Show_In_Primary";

namespace dock {

static QString hideMode2String(HideMode mode)
{
    switch (mode) {
    case HideMode::KeepShowing:
        return "keep-showing";
    case HideMode::KeepHidden:
        return "keep-hidden";
    case HideMode::SmartHide:
        return "smart-hide";
    default:
        return "keep-showing";
    }
}

static HideMode string2HideMode(const QString& modeStr)
{
    if ("keep-hidden" == modeStr)
        return HideMode::KeepHidden;
    if ("smart-hide" == modeStr)
        return HideMode::SmartHide;
    return HideMode::KeepShowing;
}

static QString position2String(Position position)
{
    switch (position) {
        case Position::Top:
            return "top";
        case Position::Right:
            return "right";
        case Position::Left:
            return "left";
        case Position::Bottom:
            return "bottom";
        default:
            return "bottom";
    }
}

static Position string2Position(const QString& modeStr)
{
    if (modeStr == "left") return Position::Left;
    else if (modeStr == "right") return Position::Right;
    else if (modeStr == "top") return Position::Top;
    else if (modeStr == "bottom") return Position::Bottom;
    return Position::Bottom;
}

static QString itemAlignment2String(const ItemAlignment& alignment)
{
    switch (alignment) {
        case ItemAlignment::LeftAlignment:
            return "left";
        case ItemAlignment::CenterAlignment:
            return "center";
    }

    return "center";
}

static ItemAlignment string2ItenAlignment(const QString& alignmentStr)
{
    if (alignmentStr == "left")
        return ItemAlignment::LeftAlignment;
    else if (alignmentStr == "center")
        return ItemAlignment::CenterAlignment;

    return ItemAlignment::CenterAlignment;
}

static QString indicatorStyle2String(IndicatorStyle mode)
{
    switch (mode) {
    case IndicatorStyle::Fashion:
        return "fashion";
    case IndicatorStyle::Efficient:
        return "efficient";
    default:
        return "fashion";
    }
}

static IndicatorStyle string2IndicatorStyle(const QString& modeStr)
{
    if ("fashion" == modeStr)
        return IndicatorStyle::Fashion;
    else if ("efficient" == modeStr)
        return IndicatorStyle::Efficient;
    return IndicatorStyle::Fashion;
}

DockSettings* DockSettings::instance()
{
    static DockSettings* dockSettings = nullptr;
    if (!dockSettings) {
        dockSettings = new DockSettings();
    }

    return dockSettings;
}

DockSettings::DockSettings(QObject* parent)
    : QObject(parent)
    , m_dockConfig(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.dock", QString(), this))
    , m_writeTimer(new QTimer(this))
    , m_dockSize(dock::DEFAULT_DOCK_SIZE)
    , m_hideMode(dock::KeepShowing)
    , m_dockPosition(dock::Bottom)
    , m_alignment(dock::CenterAlignment)
    , m_style(dock::Fashion)
{
    m_writeTimer->setSingleShot(true);
    m_writeTimer->setInterval(1000);
    init();
}

void DockSettings::init()
{
    if (m_dockConfig && m_dockConfig->isValid()) {
        m_dockSize = m_dockConfig->value(keyDockSize).toUInt();
        m_hideMode = string2HideMode(m_dockConfig->value(keyHideMode).toString());
        m_dockPosition = string2Position(m_dockConfig->value(keyPosition).toString());
        m_alignment = string2ItenAlignment(m_dockConfig->value(keyItemAlignment).toString());
        m_style = string2IndicatorStyle(m_dockConfig->value(keyIndicatorStyle).toString());
        m_pluginsVisible = m_dockConfig->value(keyPluginsVisible).toMap();
        m_showInPrimary = m_dockConfig->value(keyShowInPrimary).toBool();

        connect(m_dockConfig.data(), &DConfig::valueChanged, this, [this](const QString& key){
            if (keyDockSize == key) {
                auto size = m_dockConfig->value(keyDockSize).toUInt();
                if (size == m_dockSize || size > dock::MAX_DOCK_SIZE || size < dock::MIN_DOCK_SIZE) return;
                m_dockSize = size;
                Q_EMIT dockSizeChanged(m_dockSize);
            } else if (keyHideMode == key) {
                auto hidemode = string2HideMode(m_dockConfig->value(keyHideMode).toString());
                if (hidemode == m_hideMode) return;
                m_hideMode = hidemode;
                Q_EMIT hideModeChanged(m_hideMode);
            } else if (keyPosition == key) {
                auto position = string2Position(m_dockConfig->value(keyPosition).toString());
                if (position == m_dockPosition) return;
                m_dockPosition = position;
                Q_EMIT positionChanged(m_dockPosition);
            } else if (keyItemAlignment == key) {
                auto alignment = string2ItenAlignment(m_dockConfig->value(keyItemAlignment).toString());
                if (alignment == m_alignment) return;
                m_alignment = alignment;
                Q_EMIT itemAlignmentChanged(m_alignment);
            } else if (keyIndicatorStyle == key) {
                auto style = string2IndicatorStyle(m_dockConfig->value(keyIndicatorStyle).toString());
                if (style == m_style) return;
                m_style = style;
                Q_EMIT indicatorStyleChanged(m_style);
            } else if (keyPluginsVisible == key) {
                auto pluginsVisible = m_dockConfig->value(keyPluginsVisible).toMap();
                setPluginsVisible(pluginsVisible);
            } else if (keyShowInPrimary == key) {
                auto showInPrimary = m_dockConfig->value(keyShowInPrimary).toBool();
                if (showInPrimary == m_showInPrimary) return;
                m_showInPrimary = showInPrimary;
                Q_EMIT showInPrimaryChanged(m_showInPrimary);
            }
        });
    } else {
        qCCritical(dockSettingsLog()) << "unable to create config for org.deepin.dde.dock";
    }
}

uint DockSettings::dockSize()
{
    return m_dockSize;
}

void DockSettings::setDockSize(const uint& size)
{
    if (size == m_dockSize) return;

    m_dockSize = size;
    Q_EMIT dockSizeChanged(m_dockSize);
    addWriteJob(dockSizeJob);
}

HideMode DockSettings::hideMode()
{
    return m_hideMode;
}

void DockSettings::setHideMode(const HideMode& mode)
{
    if (mode == m_hideMode) return;

    m_hideMode = mode;
    Q_EMIT hideModeChanged(m_hideMode);
    addWriteJob(hideModeJob);
}

Position DockSettings::position()
{
    return m_dockPosition;
}

void DockSettings::setPosition(const Position& position)
{
    if (position == m_dockPosition) return;

    m_dockPosition = position;
    Q_EMIT positionChanged(m_dockPosition);
    addWriteJob(positionJob);
}

ItemAlignment DockSettings::itemAlignment()
{
    return m_alignment;
}

void DockSettings::setItemAlignment(const ItemAlignment& alignment)
{
    if (alignment == m_alignment) return;

    m_alignment = alignment;
    Q_EMIT itemAlignmentChanged(m_alignment);
    addWriteJob(itemAlignmentJob);
}

IndicatorStyle DockSettings::indicatorStyle()
{
    return m_style;
}

void DockSettings::setIndicatorStyle(const IndicatorStyle& style)
{
    if (style == m_style) return;

    m_style = style;
    Q_EMIT indicatorStyleChanged(style);
    addWriteJob(indicatorStyleJob);
}

QVariantMap DockSettings::pluginsVisible()
{
    return m_pluginsVisible;
}

void DockSettings::setPluginsVisible(const QVariantMap & pluginsVisible)
{
    if (m_pluginsVisible == pluginsVisible) {
        return;
    }
    m_pluginsVisible = pluginsVisible;
    m_dockConfig->setValue(keyPluginsVisible, QVariant::fromValue(m_pluginsVisible));
    Q_EMIT pluginsVisibleChanged(m_pluginsVisible);
}

void DockSettings::setShowInPrimary(bool newShowInPrimary)
{
    if (m_showInPrimary == newShowInPrimary) {
        return;
    }
    m_showInPrimary = newShowInPrimary;
    m_dockConfig->setValue(keyShowInPrimary, m_showInPrimary);
    Q_EMIT showInPrimaryChanged(m_showInPrimary);
}

bool DockSettings::showInPrimary() const
{
    return m_showInPrimary;
}

void DockSettings::addWriteJob(WriteJob job)
{
    if (m_writeJob.contains(job)) return;

    m_writeJob.push_back(job);
    checkWriteJob();
}

void DockSettings::checkWriteJob()
{
    m_writeTimer->disconnect(this);
    if (m_writeJob.isEmpty()) return;

    auto job = m_writeJob.front();
    m_writeJob.pop_front();

    switch (job) {
    case dockSizeJob: {
        connect(m_writeTimer, &QTimer::timeout, this, [this](){
            m_dockConfig->setValue(keyDockSize, m_dockSize);
            checkWriteJob();
        });
        break;
    }
    case hideModeJob: {
        connect(m_writeTimer, &QTimer::timeout, this, [this](){
            m_dockConfig->setValue(keyHideMode, hideMode2String(m_hideMode));
            checkWriteJob();
        });
        break;
    }
    case positionJob: {
        connect(m_writeTimer, &QTimer::timeout, this, [this](){
            m_dockConfig->setValue(keyPosition, position2String(m_dockPosition));
            checkWriteJob();
        });
        break;
    }
    case itemAlignmentJob: {
        connect(m_writeTimer, &QTimer::timeout, this, [this](){
            m_dockConfig->setValue(keyItemAlignment, itemAlignment2String(m_alignment));
            checkWriteJob();
        });
        break;
    }
    case indicatorStyleJob: {
        connect(m_writeTimer, &QTimer::timeout, this, [this](){
            m_dockConfig->setValue(keyIndicatorStyle, indicatorStyle2String(m_style));
            checkWriteJob();
        });
        break;
    }
    }

    m_writeTimer->start();
}


}
