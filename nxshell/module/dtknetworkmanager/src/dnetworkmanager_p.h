// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNETWORKMANAGER_P_H
#define DNETWORKMANAGER_P_H

#include "dbus/dnetworkmanagerinterface.h"
#include "settings/dnmsetting.h"
#include "dconnectionsetting.h"
#include "dnetworkmanager.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DNetworkManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DNetworkManagerPrivate(DNetworkManager *parent = nullptr);
    ~DNetworkManagerPrivate() override = default;

    static Core::DExpected<DNMSetting::SettingType> getSettingTypeFromConnId(const quint64 connId)
    {
        DConnectionSetting setting(connId);
        auto settings = setting.settings();

        if (!settings)
            return Core::DUnexpected{std::move(settings).error()};
        else
            return getSettingTypeFromMap(settings.value());
    }

    static Core::DExpected<DNMSetting::SettingType> getSettingTypeFromMap(const SettingDesc &map)
    {
        const auto &section = map.find("connection");
        if (section == map.cend())
            return Core::DUnexpected{Core::emplace_tag::USE_EMPLACE, -1, "can't find connection section in configuration"};
        const auto &typeStr = (*section).find("type");
        if (typeStr == (*section).cend())
            return Core::DUnexpected{Core::emplace_tag::USE_EMPLACE, -1, "can't find connection type in connection section"};
        auto connectionType = DNMSetting::stringToType((*typeStr).toString());
        return connectionType;
    }

    static QByteArray getObjPath(const DNMSetting::SettingType type)
    {
        switch (type) {
            case DNMSetting::SettingType::Wireless:
            case DNMSetting::SettingType::WirelessSecurity:
                return "/org/freedesktop/NetworkManager/AccessPoint/";
                [[fallthrough]];
            case DNMSetting::SettingType::Vpn:
                return "/org/freedesktop/NetworkManager/ActiveConnection/";
            default:
                return "/";
        }
    }

    DNetworkManager *q_ptr{nullptr};
    DNetworkManagerInterface *m_manager{nullptr};
    Q_DECLARE_PUBLIC(DNetworkManager)
};
DNETWORKMANAGER_END_NAMESPACE

#endif
