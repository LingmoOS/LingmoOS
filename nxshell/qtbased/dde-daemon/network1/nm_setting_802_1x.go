// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"fmt"

	"github.com/linuxdeepin/dde-daemon/network1/nm"
)

// Logic setter
func logicSetSetting8021xEap(data connectionData, value []string) (err error) {
	if len(value) == 0 {
		logger.Error("eap value is empty")
		err = fmt.Errorf(nmKeyErrorInvalidValue)
		return
	}
	eap := value[0]
	switch eap {
	case "tls":
		removeSettingKeyBut(data, nm.NM_SETTING_802_1X_SETTING_NAME,
			nm.NM_SETTING_802_1X_EAP,
			nm.NM_SETTING_802_1X_IDENTITY,
			nm.NM_SETTING_802_1X_CLIENT_CERT,
			nm.NM_SETTING_802_1X_CA_CERT,
			nm.NM_SETTING_802_1X_PRIVATE_KEY,
			nm.NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD,
			nm.NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS)
		setSetting8021xPrivateKeyPasswordFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "md5":
		removeSettingKeyBut(data, nm.NM_SETTING_802_1X_SETTING_NAME,
			nm.NM_SETTING_802_1X_EAP,
			nm.NM_SETTING_802_1X_IDENTITY,
			nm.NM_SETTING_802_1X_PASSWORD,
			nm.NM_SETTING_802_1X_PASSWORD_FLAGS)
		setSetting8021xPasswordFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "leap":
		removeSettingKeyBut(data, nm.NM_SETTING_802_1X_SETTING_NAME,
			nm.NM_SETTING_802_1X_EAP,
			nm.NM_SETTING_802_1X_IDENTITY,
			nm.NM_SETTING_802_1X_PASSWORD,
			nm.NM_SETTING_802_1X_PASSWORD_FLAGS)
		setSetting8021xPasswordFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "fast":
		removeSettingKeyBut(data, nm.NM_SETTING_802_1X_SETTING_NAME,
			nm.NM_SETTING_802_1X_EAP,
			nm.NM_SETTING_802_1X_IDENTITY,
			nm.NM_SETTING_802_1X_ANONYMOUS_IDENTITY,
			nm.NM_SETTING_802_1X_PAC_FILE,
			nm.NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING,
			nm.NM_SETTING_802_1X_PHASE2_AUTH,
			nm.NM_SETTING_802_1X_PASSWORD,
			nm.NM_SETTING_802_1X_PASSWORD_FLAGS)
		setSetting8021xPhase1FastProvisioning(data, "1")
		setSetting8021xPhase2Auth(data, "gtc")
		setSetting8021xPasswordFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "ttls":
		removeSettingKeyBut(data, nm.NM_SETTING_802_1X_SETTING_NAME,
			nm.NM_SETTING_802_1X_EAP,
			nm.NM_SETTING_802_1X_IDENTITY,
			nm.NM_SETTING_802_1X_ANONYMOUS_IDENTITY,
			nm.NM_SETTING_802_1X_CA_CERT,
			nm.NM_SETTING_802_1X_PHASE2_AUTH,
			nm.NM_SETTING_802_1X_PASSWORD,
			nm.NM_SETTING_802_1X_PASSWORD_FLAGS)
		setSetting8021xPhase2Auth(data, "pap")
		setSetting8021xPasswordFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	case "peap":
		removeSettingKeyBut(data, nm.NM_SETTING_802_1X_SETTING_NAME,
			nm.NM_SETTING_802_1X_EAP,
			nm.NM_SETTING_802_1X_IDENTITY,
			nm.NM_SETTING_802_1X_ANONYMOUS_IDENTITY,
			nm.NM_SETTING_802_1X_CA_CERT,
			nm.NM_SETTING_802_1X_PHASE1_PEAPVER,
			nm.NM_SETTING_802_1X_PHASE2_AUTH,
			nm.NM_SETTING_802_1X_PASSWORD,
			nm.NM_SETTING_802_1X_PASSWORD_FLAGS)
		removeSetting8021xPhase1Peapver(data)
		setSetting8021xPhase2Auth(data, "mschapv2")
		setSetting8021xPasswordFlags(data, nm.NM_SETTING_SECRET_FLAG_NONE)
	}
	setSetting8021xEap(data, value)
	return
}

// Virtual key getter
func getSettingVk8021xEnable(data connectionData) (value bool) {
	return isSettingExists(data, nm.NM_SETTING_802_1X_SETTING_NAME)
}

func getSettingVk8021xEap(data connectionData) (eap string) {
	eaps := getSetting8021xEap(data)
	if len(eaps) == 0 {
		logger.Error("eap value is empty")
		return
	}
	eap = eaps[0]
	return
}
