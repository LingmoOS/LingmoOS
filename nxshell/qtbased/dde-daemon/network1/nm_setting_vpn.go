// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"os"
	"path/filepath"

	"github.com/linuxdeepin/go-lib/keyfile"
)

const (
	nmVpnL2tpNameFile        = "nm-l2tp-service.name"
	nmVpnOpenconnectNameFile = "nm-openconnect-service.name"
	nmVpnOpenvpnNameFile     = "nm-openvpn-service.name"
	nmVpnPptpNameFile        = "nm-pptp-service.name"
	nmVpnStrongswanNameFile  = "nm-strongswan-service.name"
	nmVpnVpncNameFile        = "nm-vpnc-service.name"
)

const (
	nmOpenConnectServiceType = "org.freedesktop.NetworkManager.openconnect"
)

func getVpnAuthDialogBin(data connectionData) (authdialog string) {
	vpnType := getCustomConnectionType(data)
	return doGetVpnAuthDialogBin(vpnType)
}

func doGetVpnAuthDialogBin(vpnType string) (authdialog string) {
	k := keyfile.NewKeyFile()
	err := k.LoadFromFile(getVpnNameFile(vpnType))
	if err != nil {
		logger.Warning("failed to load from file:", err)
	}
	authdialog, _ = k.GetString("GNOME", "auth-dialog")

	return
}

func getVpnNameFile(vpnType string) (nameFile string) {
	var baseName string
	switch vpnType {
	case connectionVpnL2tp:
		baseName = nmVpnL2tpNameFile
	case connectionVpnOpenconnect:
		baseName = nmVpnOpenconnectNameFile
	case connectionVpnOpenvpn:
		baseName = nmVpnOpenvpnNameFile
	case connectionVpnPptp:
		baseName = nmVpnPptpNameFile
	case connectionVpnStrongswan:
		baseName = nmVpnStrongswanNameFile
	case connectionVpnVpnc:
		baseName = nmVpnVpncNameFile
	default:
		return ""
	}

	for _, dir := range []string{"/etc/NetworkManager/VPN", "/usr/lib/NetworkManager/VPN"} {
		nameFile = filepath.Join(dir, baseName)
		_, err := os.Stat(nameFile)
		if err == nil {
			return nameFile
		}
	}

	return ""
}
