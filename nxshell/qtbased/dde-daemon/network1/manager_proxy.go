// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"golang.org/x/xerrors"
)

const (
	proxyTypeHttp  = "http"
	proxyTypeHttps = "https"
	proxyTypeFtp   = "ftp"
	proxyTypeSocks = "socks"

	// The Deepin proxy gsettings schemas use the same path with
	// org.gnome.system.proxy which is /system/proxy. So in fact they
	// control the same values, and we don't need to synchronize them
	// at all.
	gsettingsIdProxy = "com.deepin.wrap.gnome.system.proxy"

	gkeyProxyMode   = "mode"
	proxyModeNone   = "none"
	proxyModeManual = "manual"
	proxyModeAuto   = "auto"

	gkeyProxyAuto                   = "autoconfig-url"
	gkeyProxyIgnoreHosts            = "ignore-hosts"
	gkeyProxyHost                   = "host"
	gkeyProxyPort                   = "port"
	gkeyProxyUseAuthentication      = "use-authentication"
	gkeyProxyAuthenticationUser     = "authentication-user"
	gkeyProxyAuthenticationPassword = "authentication-password"

	gchildProxyHttp  = "http"
	gchildProxyHttps = "https"
	gchildProxyFtp   = "ftp"
	gchildProxySocks = "socks"
)

var (
	proxySettings           *gio.Settings
	proxyChildSettingsHttp  *gio.Settings
	proxyChildSettingsHttps *gio.Settings
	proxyChildSettingsFtp   *gio.Settings
	proxyChildSettingsSocks *gio.Settings
)

func initProxyGsettings() {
	proxySettings = gio.NewSettings(gsettingsIdProxy)
	proxyChildSettingsHttp = proxySettings.GetChild(gchildProxyHttp)
	proxyChildSettingsHttps = proxySettings.GetChild(gchildProxyHttps)
	proxyChildSettingsFtp = proxySettings.GetChild(gchildProxyFtp)
	proxyChildSettingsSocks = proxySettings.GetChild(gchildProxySocks)

	// 如果ip全为空，则自动设置代理为None
	proxyAuto := proxySettings.GetString(gkeyProxyAuto)
	http := proxyChildSettingsHttp.GetString(gkeyProxyHost)
	https := proxyChildSettingsHttps.GetString(gkeyProxyHost)
	ftp := proxyChildSettingsFtp.GetString(gkeyProxyHost)
	socks := proxyChildSettingsSocks.GetString(gkeyProxyHost)
	if proxyAuto == "" && http == "" && https == "" && ftp == "" && socks == "" {
		proxySettings.SetString(gkeyProxyMode, proxyModeNone)
	}
}

func getProxyChildSettings(proxyType string) (childSettings *gio.Settings, err error) {
	switch proxyType {
	case proxyTypeHttp:
		childSettings = proxyChildSettingsHttp
	case proxyTypeHttps:
		childSettings = proxyChildSettingsHttps
	case proxyTypeFtp:
		childSettings = proxyChildSettingsFtp
	case proxyTypeSocks:
		childSettings = proxyChildSettingsSocks
	default:
		err = fmt.Errorf("not a valid proxy type: %s", proxyType)
		logger.Error(err)
	}
	return
}

// GetProxyMethod get current proxy method, it would be "none",
// "manual" or "auto".
func (m *Manager) GetProxyMethod() (proxyMode string, err *dbus.Error) {
	proxyMode = proxySettings.GetString(gkeyProxyMode)
	logger.Info("GetProxyMethod", proxyMode)
	return
}

func (m *Manager) SetProxyMethod(proxyMode string) *dbus.Error {
	err := m.setProxyMethod(proxyMode)
	return dbusutil.ToError(err)
}

func (m *Manager) setProxyMethod(proxyMode string) (err error) {
	logger.Info("SetProxyMethod", proxyMode)
	err = checkProxyMethod(proxyMode)
	if err != nil {
		return
	}

	// ignore if proxyModeNone already set
	currentMethod, _ := m.GetProxyMethod()
	if proxyMode == proxyModeNone && currentMethod == proxyModeNone {
		return
	}

	ok := proxySettings.SetString(gkeyProxyMode, proxyMode)
	if !ok {
		err = fmt.Errorf("set proxy method through gsettings failed")
		return
	}
	m.service.Emit(m, "ProxyMethodChanged", proxyMode)
	switch proxyMode {
	case proxyModeNone:
		notifyProxyDisabled()
	default:
		notifyProxyEnabled()
	}
	return
}
func checkProxyMethod(proxyMode string) (err error) {
	switch proxyMode {
	case proxyModeNone, proxyModeManual, proxyModeAuto:
	default:
		err = fmt.Errorf("invalid proxy method %s", proxyMode)
		logger.Error(err)
	}
	return
}

// GetAutoProxy get proxy PAC file URL for "auto" proxy mode, the
// value will keep there even the proxy mode is not "auto".
func (m *Manager) GetAutoProxy() (proxyAuto string, err *dbus.Error) {
	proxyAuto = proxySettings.GetString(gkeyProxyAuto)
	return
}

func (m *Manager) SetAutoProxy(proxyAuto string) (busErr *dbus.Error) {
	logger.Debug("set autoconfig-url for proxy", proxyAuto)
	ok := proxySettings.SetString(gkeyProxyAuto, proxyAuto)
	if !ok {
		err := fmt.Errorf("set autoconfig-url proxy through gsettings failed %s", proxyAuto)
		logger.Error(err)
		busErr = dbusutil.ToError(err)
	}
	return
}

// GetProxyIgnoreHosts get the ignored hosts for proxy network which
// is a string separated by ",".
func (m *Manager) GetProxyIgnoreHosts() (ignoreHosts string, err *dbus.Error) {
	array := proxySettings.GetStrv(gkeyProxyIgnoreHosts)
	ignoreHosts = strings.Join(array, ", ")
	return
}
func (m *Manager) SetProxyIgnoreHosts(ignoreHosts string) (busErr *dbus.Error) {
	logger.Debug("set ignore-hosts for proxy", ignoreHosts)
	ignoreHostsFixed := strings.Replace(ignoreHosts, " ", "", -1)
	array := strings.Split(ignoreHostsFixed, ",")
	ok := proxySettings.SetStrv(gkeyProxyIgnoreHosts, array)
	if !ok {
		err := fmt.Errorf("set automatic proxy through gsettings failed %s", ignoreHosts)
		logger.Error(err)
		busErr = dbusutil.ToError(err)
	}
	return
}

// GetProxy get the host and port for target proxy type.
func (m *Manager) GetProxy(proxyType string) (host, port string, busErr *dbus.Error) {
	childSettings, err := getProxyChildSettings(proxyType)
	if err != nil {
		busErr = dbusutil.ToError(err)
		return
	}
	host = childSettings.GetString(gkeyProxyHost)
	port = strconv.Itoa(int(childSettings.GetInt(gkeyProxyPort)))
	return
}

// SetProxy set host and port for target proxy type.
func (m *Manager) SetProxy(proxyType, host, port string) *dbus.Error {
	err := m.setProxy(proxyType, host, port)
	return dbusutil.ToError(err)
}

func (m *Manager) setProxy(proxyType, host, port string) (err error) {
	logger.Debugf("Manager.SetProxy proxyType: %q, host: %q, port: %q", proxyType, host, port)

	if port == "" {
		port = "0"
	}
	portInt, err := strconv.ParseInt(port, 10, 32)
	if err != nil {
		logger.Error(err)
		return
	}
	if portInt < 0 || portInt > 65535 {
		return xerrors.New("port number must be an integer between 0 and 65535")
	}

	childSettings, err := getProxyChildSettings(proxyType)
	if err != nil {
		return
	}

	var ok bool
	if ok = childSettings.SetString(gkeyProxyHost, host); ok {
		ok = childSettings.SetInt(gkeyProxyPort, int32(portInt))
	}
	if !ok {
		err = fmt.Errorf("set proxy value to gsettings failed: %s, %s:%s", proxyType, host, port)
		logger.Error(err)
	}

	return
}

func (m *Manager) GetProxyAuthentication(proxyType string) (user, password string, enable bool, busErr *dbus.Error) {
	childSettings, err := getProxyChildSettings(proxyType)
	if err != nil {
		busErr = dbusutil.ToError(busErr)
		return
	}

	if !childSettings.GetSchema().HasKey(gkeyProxyUseAuthentication) {
		busErr = dbusutil.ToError(fmt.Errorf("%s is not support authentication", proxyType))
		return
	}

	enable = childSettings.GetBoolean(gkeyProxyUseAuthentication)
	user = childSettings.GetString(gkeyProxyAuthenticationUser)
	password = childSettings.GetString(gkeyProxyAuthenticationPassword)

	return
}

func (m *Manager) SetProxyAuthentication(proxyType, user, password string, enable bool) *dbus.Error {
	logger.Debugf("Manager.SetProxyAuthentication proxyType: %s, host: %s, port: %s, use: %v", proxyType, user, password, enable)
	childSettings, err := getProxyChildSettings(proxyType)
	if err != nil {
		return dbusutil.ToError(err)
	}

	if !childSettings.GetSchema().HasKey(gkeyProxyUseAuthentication) {
		return dbusutil.ToError(fmt.Errorf("%s is not support authentication", proxyType))
	}

	if !childSettings.SetBoolean(gkeyProxyUseAuthentication, enable) ||
		!childSettings.SetString(gkeyProxyAuthenticationUser, user) ||
		!childSettings.SetString(gkeyProxyAuthenticationPassword, password) {
		dbusutil.ToError(fmt.Errorf("set proxy authentication value to gsettings failed: %s, %s:%s", proxyType, user, password))
	}

	return nil
}
