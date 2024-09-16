// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package proxychains

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sync"

	proxy "github.com/linuxdeepin/go-dbus-factory/system/com.deepin.system.proxy"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/xdg/basedir"
)

//go:generate dbusutil-gen em -type Manager

var logger *log.Logger

func SetLogger(l *log.Logger) {
	logger = l
}

type Manager struct {
	service  *dbusutil.Service
	PropsMu  sync.RWMutex
	appProxy proxy.App
	Enable   bool
	Type     string
	IP       string
	Port     uint32
	User     string
	Password string

	jsonFile string
	confFile string
}

func NewManager(service *dbusutil.Service) *Manager {
	cfgDir := basedir.GetUserConfigDir()
	jsonFile := filepath.Join(cfgDir, "deepin", "proxychains.json")
	confFile := filepath.Join(cfgDir, "deepin", "proxychains.conf")
	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warningf("get sys bus failed, err: %v", err)
	}
	m := &Manager{
		jsonFile: jsonFile,
		confFile: confFile,
		service:  service,
		appProxy: proxy.NewApp(sysBus),
	}
	go m.init()
	return m
}

const (
	DBusPath      = "/org/deepin/dde/Network/ProxyChains"
	dbusInterface = "org.deepin.dde.Network.ProxyChains"
)

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

const defaultType = "http"

func (m *Manager) init() {
	cfg, err := loadConfig(m.jsonFile)
	logger.Debug("load proxychains config file:", m.jsonFile)
	if err != nil {
		if os.IsNotExist(err) {
			logger.Debug("proxychans config file not found")
		} else {
			logger.Warning("load proxychains config failed:", err)
		}
		m.Type = defaultType

		return
	}

	m.Enable = cfg.Enable
	m.Type = cfg.Type
	m.IP = cfg.IP
	m.Port = cfg.Port
	m.User = cfg.User
	m.Password = cfg.Password

	changed := m.fixConfig()
	logger.Debug("fixConfig changed:", changed)
	if changed {
		if err := m.saveConfig(); err != nil {
			logger.Warning("save config failed", err)
		}
	}

	if !m.checkConfig() {
		// config is invalid
		logger.Warning("config is invalid")
		if err := m.removeConf(); err != nil {
			logger.Warning("remove conf file failed:", err)
		}
	}

	if m.Enable && m.IP != "" && m.Port != 0 {
		settings := proxy.Proxy{
			ProtoType: m.Type,
			Name:      "default",
			Server:    m.IP,
			Port:      int(m.Port),
			UserName:  m.User,
			Password:  m.Password,
		}
		buf, err := json.Marshal(settings)
		if err != nil {
			return
		}
		err = m.appProxy.AddProxy(0, m.Type, "default", buf)
		if err != nil {
			logger.Warningf("add proxy failed, err: %v", err)
			return
		}
		err = m.appProxy.StartProxy(0, m.Type, "default", false)
		if err != nil {
			logger.Warningf("start proxy failed, err: %v", err)
			return
		}
	}
}

func (m *Manager) saveConfig() error {
	cfg := &Config{
		Enable:   m.Enable,
		Type:     m.Type,
		IP:       m.IP,
		Port:     m.Port,
		User:     m.User,
		Password: m.Password,
	}
	return cfg.save(m.jsonFile)
}

// nolint
func (m *Manager) notifyChange(prop string, v interface{}) {
	err := m.service.EmitPropertyChanged(m, prop, v)
	if err != nil {
		logger.Warning("failed to emit signal", err)
	}
}

func (m *Manager) fixConfig() bool {
	var changed bool
	if !validType(m.Type) {
		m.Type = defaultType
		changed = true
	}

	if m.IP != "" && !validIPv4(m.IP) {
		m.IP = ""
		changed = true
	}

	if !validUser(m.User) {
		m.User = ""
		changed = true
	}

	if !validPassword(m.Password) {
		m.Password = ""
		changed = true
	}

	return changed
}

func (m *Manager) checkConfig() bool {
	if !validType(m.Type) {
		return false
	}

	if !validIPv4(m.IP) {
		return false
	}

	if !validUser(m.User) {
		return false
	}

	if !validPassword(m.Password) {
		return false
	}
	return true
}

type InvalidParamError struct {
	Param string
}

// nolint
func (err InvalidParamError) Error() string {
	return fmt.Sprintf("invalid param %s", err.Param)
}

func (m *Manager) SetEnable(enable bool) *dbus.Error {
	m.Enable = enable
	m.notifyChange("Enable", enable)
	err := m.set(m.Type, m.IP, m.Port, m.User, m.Password)
	return dbusutil.ToError(err)
}

func (m *Manager) Set(type0, ip string, port uint32, user, password string) *dbus.Error {
	err := m.set(type0, ip, port, user, password)
	return dbusutil.ToError(err)
}

func (m *Manager) set(type0, ip string, port uint32, user, password string) error {
	// allow type0 is empty
	if type0 == "" {
		type0 = defaultType
	}
	if !validType(type0) {
		return InvalidParamError{"Type"}
	}

	var disable bool
	if ip == "" && port == 0 {
		disable = true
	}

	if !m.Enable {
		disable = true
	}

	if !disable && !validIPv4(ip) {
		notifyAppProxyEnableFailed()
		return InvalidParamError{"IP"}
	}

	if m.Enable && !validUser(user) {
		notifyAppProxyEnableFailed()
		return InvalidParamError{"User"}
	}

	if m.Enable && !validPassword(password) {
		notifyAppProxyEnableFailed()
		return InvalidParamError{"Password"}
	}

	if m.Enable && ((user == "" && password != "") || (user != "" && password == "")) {
		notifyAppProxyEnableFailed()
		return errors.New("user and password are not provided at the same time")
	}

	// all params are ok
	m.PropsMu.Lock()
	defer m.PropsMu.Unlock()

	if m.Type != type0 {
		m.Type = type0
		m.notifyChange("Type", type0)
	}

	if m.IP != ip {
		m.IP = ip
		m.notifyChange("IP", ip)
	}

	if m.Port != port {
		m.Port = port
		m.notifyChange("Port", port)
	}

	if m.User != user {
		m.User = user
		m.notifyChange("User", user)
	}

	if m.Password != password {
		m.Password = password
		m.notifyChange("Password", password)
	}

	err := m.saveConfig()
	if err != nil {
		notifyAppProxyEnableFailed()
		return err
	}

	if disable {
		err = m.appProxy.StopProxy(0)
		if err != nil {
			logger.Warningf("stop proxy failed, err: %v", err)
			return err
		}
		err = m.appProxy.ClearProxy(0)
		if err != nil {
			logger.Warningf("clear proxy failed, err: %v", err)
			return err
		}
		return m.removeConf()
	}

	// enable
	err = m.writeConf()
	if err != nil {
		notifyAppProxyEnableFailed()
	} else {
		notifyAppProxyEnabled()
	}
	settings := proxy.Proxy{
		ProtoType: type0,
		Name:      "default",
		Server:    ip,
		Port:      int(port),
		UserName:  user,
		Password:  password,
	}
	buf, err := json.Marshal(settings)
	if err != nil {
		return err
	}
	err = m.appProxy.AddProxy(0, type0, "default", buf)
	if err != nil {
		logger.Warningf("add proxy failed, err: %v", err)
		return err
	}
	err = m.appProxy.StartProxy(0, type0, "default", false)
	if err != nil {
		logger.Warningf("start proxy failed, err: %v", err)
		return err
	}
	return err
}

func (m *Manager) writeConf() error {
	const head = `# Written by ` + dbusInterface + `
strict_chain
quiet_mode
proxy_dns
remote_dns_subnet 224
tcp_read_time_out 15000
tcp_connect_time_out 8000
localnet 127.0.0.0/255.0.0.0

[ProxyList]
`
	fh, err := os.Create(m.confFile)
	if err != nil {
		return err
	}
	_, err = fh.WriteString(head)
	if err != nil {
		return err
	}

	proxy := fmt.Sprintf("%s\t%s\t%v", m.Type, m.IP, m.Port)
	if m.User != "" && m.Password != "" {
		proxy += fmt.Sprintf("\t%s\t%s", m.User, m.Password)
	}
	_, err = fh.WriteString(proxy + "\n")
	if err != nil {
		return err
	}

	err = fh.Sync()
	if err != nil {
		return err
	}

	return fh.Close()
}

func (m *Manager) removeConf() error {
	err := os.Remove(m.confFile)
	if os.IsNotExist(err) {
		// ignore file not exist error
		return nil
	}
	return err
}
