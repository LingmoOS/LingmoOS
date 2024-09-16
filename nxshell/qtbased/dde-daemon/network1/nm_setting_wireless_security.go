// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"errors"
	"fmt"

	"github.com/linuxdeepin/dde-daemon/network1/nm"
)

// Virtual key getter and setter
func getSettingVkWirelessSecurityKeyMgmt(data connectionData) (value string) {
	if !isSettingExists(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME) {
		value = "none"
		return
	}
	keyMgmt := getSettingWirelessSecurityKeyMgmt(data)
	switch keyMgmt {
	case "none":
		value = "wep"
	case "wpa-psk":
		value = "wpa-psk"
	case "sae":
		value = "sae"
	case "wpa-eap":
		value = "wpa-eap"
	}
	return
}

func getApSecTypeFromConnData(data connectionData) (apSecType, error) {
	if !isSettingExists(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME) {
		return apSecNone, nil
	}
	keyMgmt := getSettingWirelessSecurityKeyMgmt(data)
	authAlg := getSettingWirelessSecurityAuthAlg(data)
	switch keyMgmt {
	case "none":
		if authAlg == "open" || authAlg == "shared" {
			return apSecWep, nil
		}
	case "wpa-psk":
		return apSecPsk, nil
	case "sae":
		return apSecSae, nil
	case "wpa-eap":
		return apSecEap, nil
	}

	return apSecNone, errors.New("unknown apSecType")
}

func logicSetSettingVkWirelessSecurityKeyMgmt(data connectionData, value string) (err error) {
	switch value {
	default:
		logger.Error("invalid value", value)
		err = fmt.Errorf(nmKeyErrorInvalidValue)
	case "none":
		removeSetting(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
		removeSetting(data, nm.NM_SETTING_802_1X_SETTING_NAME)
	case "wep":
		addSetting(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
		removeSetting(data, nm.NM_SETTING_802_1X_SETTING_NAME)

		removeSettingKeyBut(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME,
			nm.NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
			nm.NM_SETTING_WIRELESS_SECURITY_AUTH_ALG,
			nm.NM_SETTING_WIRELESS_SECURITY_WEP_KEY0,
			nm.NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS,
			nm.NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
		)
		setSettingWirelessSecurityKeyMgmt(data, "none")
		setSettingWirelessSecurityWepKeyFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
		setSettingWirelessSecurityWepKeyType(data, nm.NM_WEP_KEY_TYPE_KEY)
	case "wpa-psk":
		addSetting(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
		removeSetting(data, nm.NM_SETTING_802_1X_SETTING_NAME)

		removeSettingKeyBut(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME,
			nm.NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
			nm.NM_SETTING_WIRELESS_SECURITY_PSK,
			nm.NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS,
		)
		setSettingWirelessSecurityKeyMgmt(data, "wpa-psk")
		setSettingWirelessSecurityAuthAlg(data, "open")
		setSettingWirelessSecurityPskFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "sae":
		addSetting(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
		removeSetting(data, nm.NM_SETTING_802_1X_SETTING_NAME)
		removeSettingKeyBut(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME,
			nm.NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
			nm.NM_SETTING_WIRELESS_SECURITY_PSK,
			nm.NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS,
		)
		setSettingWirelessSecurityKeyMgmt(data, "sae")
		setSettingWirelessSecurityPskFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "wpa-eap", "wpa-eap-suite-b-192":
		addSetting(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
		addSetting(data, nm.NM_SETTING_802_1X_SETTING_NAME)

		removeSettingKeyBut(data, nm.NM_SETTING_WIRELESS_SECURITY_SETTING_NAME,
			nm.NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
		)
		setSettingWirelessSecurityKeyMgmt(data, value)
		err = logicSetSetting8021xEap(data, []string{"tls"})
	}
	return
}
