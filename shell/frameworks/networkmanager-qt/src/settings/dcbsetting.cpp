/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dcbsetting.h"
#include "dcbsetting_p.h"

#include <QDebug>

NetworkManager::DcbSettingPrivate::DcbSettingPrivate()
    : name(NM_SETTING_DCB_SETTING_NAME)
    , appFcoeMode(NM_SETTING_DCB_FCOE_MODE_FABRIC)
    , appFcoePriority(-1)
    , appFipPriority(-1)
    , appIscsiPriority(-1)
    , priorityFlowControl({0, 0, 0, 0, 0, 0, 0, 0})
    , priorityBandwidth({0, 0, 0, 0, 0, 0, 0, 0})
    , priorityGroupBandwidth({0, 0, 0, 0, 0, 0, 0, 0})
    , priorityGroupId({0, 0, 0, 0, 0, 0, 0, 0})
    , priorityStrictBandwidth({0, 0, 0, 0, 0, 0, 0, 0})
    , priorityTrafficClass({0, 0, 0, 0, 0, 0, 0, 0})
{
}

NetworkManager::DcbSetting::DcbSetting()
    : Setting(Setting::Dcb)
    , d_ptr(new DcbSettingPrivate())
{
}

NetworkManager::DcbSetting::DcbSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new DcbSettingPrivate())
{
    setAppFcoeMode(other->appFcoeMode());
    setAppFcoePriority(other->appFcoePriority());
    setAppFipPriority(other->appFipPriority());
    setAppIscsiPriority(other->appIscsiPriority());
    setAppFcoeFlags(other->appFcoeFlags());
    setAppFipFlags(other->appFipFlags());
    setAppIscsiFlags(other->appIscsiFlags());
    setPriorityGroupFlags(other->priorityGroupFlags());
    setPriorityFlowControlFlags(other->priorityFlowControlFlags());
    setPriorityFlowControl(other->priorityFlowControl());
    setPriorityBandwidth(other->priorityBandwidth());
    setPriorityGroupBandwidth(other->priorityGroupBandwidth());
    setPriorityGroupId(other->priorityGroupId());
    setPriorityStrictBandwidth(other->priorityStrictBandwidth());
    setPriorityTrafficClass(other->priorityTrafficClass());
}

NetworkManager::DcbSetting::~DcbSetting()
{
    delete d_ptr;
}

QString NetworkManager::DcbSetting::name() const
{
    Q_D(const DcbSetting);

    return d->name;
}

void NetworkManager::DcbSetting::setAppFcoeMode(const QString &mode)
{
    Q_D(DcbSetting);

    d->appFcoeMode = mode;
}

QString NetworkManager::DcbSetting::appFcoeMode() const
{
    Q_D(const DcbSetting);

    return d->appFcoeMode;
}

void NetworkManager::DcbSetting::setAppFcoePriority(qint32 priority)
{
    Q_D(DcbSetting);

    d->appFcoePriority = priority;
}

qint32 NetworkManager::DcbSetting::appFcoePriority() const
{
    Q_D(const DcbSetting);

    return d->appFcoePriority;
}

void NetworkManager::DcbSetting::setAppFipPriority(qint32 priority)
{
    Q_D(DcbSetting);

    d->appFipPriority = priority;
}

qint32 NetworkManager::DcbSetting::appFipPriority() const
{
    Q_D(const DcbSetting);

    return d->appFipPriority;
}

void NetworkManager::DcbSetting::setAppIscsiPriority(qint32 priority)
{
    Q_D(DcbSetting);

    d->appIscsiPriority = priority;
}

qint32 NetworkManager::DcbSetting::appIscsiPriority() const
{
    Q_D(const DcbSetting);

    return d->appIscsiPriority;
}

void NetworkManager::DcbSetting::setAppFcoeFlags(NetworkManager::DcbSetting::DcbFlags flags)
{
    Q_D(DcbSetting);

    d->appFcoeFlags = flags;
}

NetworkManager::DcbSetting::DcbFlags NetworkManager::DcbSetting::appFcoeFlags() const
{
    Q_D(const DcbSetting);

    return d->appFcoeFlags;
}

void NetworkManager::DcbSetting::setAppFipFlags(NetworkManager::DcbSetting::DcbFlags flags)
{
    Q_D(DcbSetting);

    d->appFipFlags = flags;
}

NetworkManager::DcbSetting::DcbFlags NetworkManager::DcbSetting::appFipFlags() const
{
    Q_D(const DcbSetting);

    return d->appFipFlags;
}

void NetworkManager::DcbSetting::setAppIscsiFlags(NetworkManager::DcbSetting::DcbFlags flags)
{
    Q_D(DcbSetting);

    d->appIscsiFlags = flags;
}

NetworkManager::DcbSetting::DcbFlags NetworkManager::DcbSetting::appIscsiFlags() const
{
    Q_D(const DcbSetting);

    return d->appIscsiFlags;
}

void NetworkManager::DcbSetting::setPriorityGroupFlags(NetworkManager::DcbSetting::DcbFlags flags)
{
    Q_D(DcbSetting);

    d->priorityGroupFlags = flags;
}

NetworkManager::DcbSetting::DcbFlags NetworkManager::DcbSetting::priorityGroupFlags() const
{
    Q_D(const DcbSetting);

    return d->priorityGroupFlags;
}

void NetworkManager::DcbSetting::setPriorityFlowControlFlags(NetworkManager::DcbSetting::DcbFlags flags)
{
    Q_D(DcbSetting);

    d->priorityFlowControlFlags = flags;
}

NetworkManager::DcbSetting::DcbFlags NetworkManager::DcbSetting::priorityFlowControlFlags() const
{
    Q_D(const DcbSetting);

    return d->priorityFlowControlFlags;
}

void NetworkManager::DcbSetting::setPriorityFlowControl(quint32 userPriority, bool enabled)
{
    Q_D(DcbSetting);

    if (userPriority < 8) {
        d->priorityFlowControl[userPriority] = enabled;
    }
}

bool NetworkManager::DcbSetting::priorityFlowControl(quint32 userPriority) const
{
    Q_D(const DcbSetting);

    if (userPriority < 8) {
        return d->priorityFlowControl.value(userPriority);
    }
    return false;
}

void NetworkManager::DcbSetting::setPriorityFlowControl(const UIntList &list)
{
    Q_D(DcbSetting);

    d->priorityFlowControl = list;
}

UIntList NetworkManager::DcbSetting::priorityFlowControl() const
{
    Q_D(const DcbSetting);

    return d->priorityFlowControl;
}

void NetworkManager::DcbSetting::setPriorityBandwidth(quint32 userPriority, quint32 bandwidthPercent)
{
    Q_D(DcbSetting);

    if (userPriority < 8) {
        d->priorityBandwidth[userPriority] = bandwidthPercent;
    }
}

quint32 NetworkManager::DcbSetting::priorityBandwidth(quint32 userPriority) const
{
    Q_D(const DcbSetting);

    if (userPriority < 8) {
        return d->priorityBandwidth.value(userPriority);
    }
    return 0;
}

void NetworkManager::DcbSetting::setPriorityBandwidth(const UIntList &list)
{
    Q_D(DcbSetting);

    d->priorityBandwidth = list;
}

UIntList NetworkManager::DcbSetting::priorityBandwidth() const
{
    Q_D(const DcbSetting);

    return d->priorityBandwidth;
}

void NetworkManager::DcbSetting::setPriorityGroupBandwidth(quint32 groupId, quint32 bandwidthPercent)
{
    Q_D(DcbSetting);

    if (groupId < 8) {
        d->priorityGroupBandwidth[groupId] = bandwidthPercent;
    }
}

quint32 NetworkManager::DcbSetting::priorityGroupBandwidth(quint32 groupId) const
{
    Q_D(const DcbSetting);

    if (groupId < 8) {
        return d->priorityGroupBandwidth.value(groupId);
    }
    return 0;
}

void NetworkManager::DcbSetting::setPriorityGroupBandwidth(const UIntList &list)
{
    Q_D(DcbSetting);

    d->priorityGroupBandwidth = list;
}

UIntList NetworkManager::DcbSetting::priorityGroupBandwidth() const
{
    Q_D(const DcbSetting);

    return d->priorityGroupBandwidth;
}

void NetworkManager::DcbSetting::setPriorityGroupId(quint32 userPriority, quint32 groupId)
{
    Q_D(DcbSetting);

    if (userPriority < 8) {
        d->priorityGroupId[userPriority] = groupId;
    }
}

quint32 NetworkManager::DcbSetting::priorityGroupId(quint32 userPriority) const
{
    Q_D(const DcbSetting);

    if (userPriority < 8) {
        return d->priorityGroupId.value(userPriority);
    }
    return 0;
}

void NetworkManager::DcbSetting::setPriorityGroupId(const UIntList &list)
{
    Q_D(DcbSetting);

    d->priorityGroupId = list;
}

UIntList NetworkManager::DcbSetting::priorityGroupId() const
{
    Q_D(const DcbSetting);

    return d->priorityGroupId;
}

void NetworkManager::DcbSetting::setPriorityStrictBandwidth(quint32 userPriority, bool strict)
{
    Q_D(DcbSetting);

    if (userPriority < 8) {
        d->priorityStrictBandwidth[userPriority] = strict;
    }
}

bool NetworkManager::DcbSetting::priorityStrictBandwidth(quint32 userPriority) const
{
    Q_D(const DcbSetting);

    if (userPriority < 8) {
        return d->priorityStrictBandwidth.value(userPriority);
    }
    return false;
}

void NetworkManager::DcbSetting::setPriorityStrictBandwidth(const UIntList &list)
{
    Q_D(DcbSetting);

    d->priorityStrictBandwidth = list;
}

UIntList NetworkManager::DcbSetting::priorityStrictBandwidth() const
{
    Q_D(const DcbSetting);

    return d->priorityStrictBandwidth;
}

void NetworkManager::DcbSetting::setPriorityTrafficClass(quint32 userPriority, quint32 trafficClass)
{
    Q_D(DcbSetting);

    if (userPriority < 8) {
        d->priorityTrafficClass[userPriority] = trafficClass;
    }
}

quint32 NetworkManager::DcbSetting::priorityTrafficClass(quint32 userPriority) const
{
    Q_D(const DcbSetting);

    if (userPriority < 8) {
        return d->priorityTrafficClass.value(userPriority);
    }
    return 0;
}

void NetworkManager::DcbSetting::setPriorityTrafficClass(const UIntList &list)
{
    Q_D(DcbSetting);

    d->priorityTrafficClass = list;
}

UIntList NetworkManager::DcbSetting::priorityTrafficClass() const
{
    Q_D(const DcbSetting);

    return d->priorityTrafficClass;
}

void NetworkManager::DcbSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_FCOE_MODE))) {
        setAppFcoeMode(setting.value(QLatin1String(NM_SETTING_DCB_APP_FCOE_MODE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_FCOE_PRIORITY))) {
        setAppFcoePriority(setting.value(QLatin1String(NM_SETTING_DCB_APP_FCOE_PRIORITY)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_FIP_PRIORITY))) {
        setAppFipPriority(setting.value(QLatin1String(NM_SETTING_DCB_APP_FIP_PRIORITY)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_ISCSI_PRIORITY))) {
        setAppIscsiPriority(setting.value(QLatin1String(NM_SETTING_DCB_APP_ISCSI_PRIORITY)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_FCOE_FLAGS))) {
        setAppFcoeFlags(static_cast<DcbFlags>(setting.value(QLatin1String(NM_SETTING_DCB_APP_FCOE_FLAGS)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_FIP_FLAGS))) {
        setAppFipFlags(static_cast<DcbFlags>(setting.value(QLatin1String(NM_SETTING_DCB_APP_FIP_FLAGS)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_APP_ISCSI_FLAGS))) {
        setAppIscsiFlags(static_cast<DcbFlags>(setting.value(QLatin1String(NM_SETTING_DCB_APP_ISCSI_FLAGS)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_FLAGS))) {
        setPriorityGroupFlags(static_cast<DcbFlags>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_FLAGS)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_FLOW_CONTROL_FLAGS))) {
        setPriorityFlowControlFlags(static_cast<DcbFlags>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_FLOW_CONTROL_FLAGS)).toUInt()));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_FLOW_CONTROL))) {
        setPriorityFlowControl(qdbus_cast<UIntList>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_FLOW_CONTROL))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_BANDWIDTH))) {
        setPriorityBandwidth(qdbus_cast<UIntList>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_BANDWIDTH))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_BANDWIDTH))) {
        setPriorityGroupBandwidth(qdbus_cast<UIntList>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_BANDWIDTH))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_ID))) {
        setPriorityGroupId(qdbus_cast<UIntList>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_ID))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_STRICT_BANDWIDTH))) {
        setPriorityStrictBandwidth(qdbus_cast<UIntList>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_STRICT_BANDWIDTH))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_DCB_PRIORITY_TRAFFIC_CLASS))) {
        setPriorityTrafficClass(qdbus_cast<UIntList>(setting.value(QLatin1String(NM_SETTING_DCB_PRIORITY_TRAFFIC_CLASS))));
    }
}

QVariantMap NetworkManager::DcbSetting::toMap() const
{
    QVariantMap setting;

    if (!appFcoeMode().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_DCB_APP_FCOE_MODE), appFcoeMode());
    }

    if (appFcoePriority() >= 0) {
        setting.insert(QLatin1String(NM_SETTING_DCB_APP_FCOE_PRIORITY), appFcoePriority());
    }

    if (appFipPriority() >= 0) {
        setting.insert(QLatin1String(NM_SETTING_DCB_APP_FIP_PRIORITY), appFipPriority());
    }

    if (appIscsiPriority() >= 0) {
        setting.insert(QLatin1String(NM_SETTING_DCB_APP_ISCSI_PRIORITY), appIscsiPriority());
    }

    setting.insert(QLatin1String(NM_SETTING_DCB_APP_FCOE_FLAGS), (int)appFcoeFlags());
    setting.insert(QLatin1String(NM_SETTING_DCB_APP_FIP_FLAGS), (int)appFipFlags());
    setting.insert(QLatin1String(NM_SETTING_DCB_APP_ISCSI_FLAGS), (int)appIscsiFlags());
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_FLAGS), (int)priorityGroupFlags());
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_FLOW_CONTROL_FLAGS), (int)priorityFlowControlFlags());

    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_FLOW_CONTROL), QVariant::fromValue(priorityFlowControl()));
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_BANDWIDTH), QVariant::fromValue(priorityBandwidth()));
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_BANDWIDTH), QVariant::fromValue(priorityGroupBandwidth()));
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_GROUP_ID), QVariant::fromValue(priorityGroupId()));
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_STRICT_BANDWIDTH), QVariant::fromValue(priorityStrictBandwidth()));
    setting.insert(QLatin1String(NM_SETTING_DCB_PRIORITY_TRAFFIC_CLASS), QVariant::fromValue(priorityTrafficClass()));

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::DcbSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_DCB_APP_FCOE_MODE << ": " << setting.appFcoeMode() << '\n';
    dbg.nospace() << NM_SETTING_DCB_APP_FCOE_PRIORITY << ": " << setting.appFcoePriority() << '\n';
    dbg.nospace() << NM_SETTING_DCB_APP_FIP_PRIORITY << ": " << setting.appFipPriority() << '\n';
    dbg.nospace() << NM_SETTING_DCB_APP_ISCSI_PRIORITY << ": " << setting.appIscsiPriority() << '\n';
    dbg.nospace() << NM_SETTING_DCB_APP_FCOE_FLAGS << ": " << setting.appFcoeFlags() << '\n';
    dbg.nospace() << NM_SETTING_DCB_APP_FIP_FLAGS << ": " << setting.appFipFlags() << '\n';
    dbg.nospace() << NM_SETTING_DCB_APP_ISCSI_FLAGS << ": " << setting.appIscsiFlags() << '\n';
    dbg.nospace() << NM_SETTING_DCB_PRIORITY_GROUP_FLAGS << ": " << setting.priorityGroupFlags() << '\n';
    dbg.nospace() << NM_SETTING_DCB_PRIORITY_FLOW_CONTROL_FLAGS << ": " << setting.priorityFlowControlFlags() << '\n';

    dbg.nospace() << NM_SETTING_DCB_PRIORITY_FLOW_CONTROL << ": ";
    const auto flowControlList = setting.priorityFlowControl();
    for (const quint32 enabled : flowControlList) {
        dbg.nospace() << enabled << ' ';
    }
    dbg.nospace() << '\n';

    dbg.nospace() << NM_SETTING_DCB_PRIORITY_BANDWIDTH << ": ";
    const auto bwList = setting.priorityBandwidth();
    for (const quint32 bandwidthPercent : bwList) {
        dbg.nospace() << bandwidthPercent << ' ';
    }
    dbg.nospace() << '\n';

    dbg.nospace() << NM_SETTING_DCB_PRIORITY_GROUP_BANDWIDTH << ": ";
    const auto gbwList = setting.priorityGroupBandwidth();
    for (const quint32 bandwidthPercent : gbwList) {
        dbg.nospace() << bandwidthPercent << ' ';
    }
    dbg.nospace() << '\n';

    dbg.nospace() << NM_SETTING_DCB_PRIORITY_GROUP_ID << ": ";
    const auto groupIds = setting.priorityGroupId();
    for (const quint32 groupId : groupIds) {
        dbg.nospace() << groupId << ' ';
    }
    dbg.nospace() << '\n';

    dbg.nospace() << NM_SETTING_DCB_PRIORITY_STRICT_BANDWIDTH << ": ";
    const auto strictList = setting.priorityStrictBandwidth();
    for (const quint32 strict : strictList) {
        dbg.nospace() << strict << ' ';
    }
    dbg.nospace() << '\n';

    dbg.nospace() << NM_SETTING_DCB_PRIORITY_TRAFFIC_CLASS << ": ";
    const auto trafficList = setting.priorityTrafficClass();
    for (const quint32 trafficClass : trafficList) {
        dbg.nospace() << trafficClass << ' ';
    }
    dbg.nospace() << '\n';

    return dbg.maybeSpace();
}
