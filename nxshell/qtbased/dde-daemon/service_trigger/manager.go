// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package service_trigger

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/common/sessionmsg"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.notifications"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

type Manager struct {
	service    *dbusutil.Service
	serviceMap map[string]*Service

	notifications notifications.Notifications
	sysDBusDaemon ofdbus.DBus

	systemSigMonitor  *DBusSignalMonitor
	sessionSigMonitor *DBusSignalMonitor
	sysSigLoop        *dbusutil.SignalLoop
	agents            map[string]*agent
}

func newManager(service *dbusutil.Service) *Manager {
	m := &Manager{
		service:           service,
		systemSigMonitor:  newDBusSignalMonitor(busTypeSystem),
		sessionSigMonitor: newDBusSignalMonitor(busTypeSession),
		agents:            make(map[string]*agent),
	}
	return m
}

func (m *Manager) handleSysBusNameOwnerChanged(name, oldOwner, newOwner string) {
	if name != "" && oldOwner == "" && newOwner != "" && !strings.HasPrefix(name, ":") {
		// 新服务注册了, 需要重新注册 agent
		for _, agent := range m.agents {
			cfg := agent.cfg
			if cfg.Dest == name {
				time.AfterFunc(agent.getRegisterDelay(), func() {
					err := agent.register()
					if err != nil {
						logger.Warningf("agent %v register failed: %v", agent.name, err)
					}
				})
			}
		}
	}
}

func (m *Manager) start() error {
	m.loadServices()
	m.initAgents()

	sessionBus := m.service.Conn()
	m.notifications = notifications.NewNotifications(sessionBus)
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}

	m.sysDBusDaemon = ofdbus.NewDBus(sysBus)
	m.sysSigLoop = dbusutil.NewSignalLoop(sysBus, 10)
	m.sysSigLoop.Start()

	m.sysDBusDaemon.InitSignalExt(m.sysSigLoop, true)
	_, err = m.sysDBusDaemon.ConnectNameOwnerChanged(m.handleSysBusNameOwnerChanged)
	if err != nil {
		logger.Warning(err)
	}

	m.sessionSigMonitor.init()
	go m.sessionSigMonitor.signalLoop(m)

	m.systemSigMonitor.init()
	go m.systemSigMonitor.signalLoop(m)
	return nil
}

func (m *Manager) stop() error {
	m.sysSigLoop.Stop()

	err := m.sessionSigMonitor.stop()
	if err != nil {
		return err
	}

	return m.systemSigMonitor.stop()
}

// 处理从 dde-system-daemon 在 system bus 传递给 session 的消息
func (m *Manager) handleSysMessage(msgStr string) *dbus.Error {
	err := m.handleSysMessageAux(msgStr)
	if err != nil {
		logger.Warning(err)
	}
	return dbusutil.ToError(err)
}

func (m *Manager) handleSysMessageAux(msgStr string) error {
	var msg sessionmsg.Message
	err := json.Unmarshal([]byte(msgStr), &msg)
	if err != nil {
		return err
	}

	if msg.Type == sessionmsg.MessageTypeNotify {
		body, ok := msg.Body.(*sessionmsg.BodyNotify)
		if !ok {
			return fmt.Errorf("invalid message: type of msg.Body is %T", msg.Body)
		}
		m.notify(body)
	}
	return nil
}

func (m *Manager) notify(bodyNotify *sessionmsg.BodyNotify) {
	summary := bodyNotify.Summary.String()
	body := bodyNotify.Body.String()
	logger.Debugf("notify %q %q %q", bodyNotify.Icon, summary, body)

	if m.notifications == nil {
		logger.Warning("notifications is nil")
		return
	}

	appName := bodyNotify.AppName
	if appName == "" {
		appName = "dde-control-center"
	}

	_, err := m.notifications.Notify(0, appName, 0, bodyNotify.Icon,
		summary, body, bodyNotify.Actions, bodyNotify.Hints, int32(bodyNotify.ExpireTimeout))
	if err != nil {
		logger.Warning(err)
		return
	}
}

func (m *Manager) initAgents() {
	agentCfgs := []*DBusAgentConfig{
		{
			Name:           "main",
			AgentInterface: sessionmsg.AgentIfc,
			AgentPath:      sessionmsg.AgentPath,
			AgentMethods: []DBusAgentMethod{
				{
					Name: sessionmsg.MethodSendMessage,
					fn:   m.handleSysMessage,
				},
			},
		},
	}
	for _, agentCfg := range agentCfgs {
		agent, err := newAgent(agentCfg)
		if err != nil {
			logger.Warningf("new agent %v failed: %v", agentCfg.Name, err)
			continue
		}
		m.agents[agent.name] = agent
		err = agent.register()
		if err != nil {
			logger.Warningf("agent %v register failed: %v", agent.name, err)
		}
	}
}

func (m *Manager) loadServices() {
	m.serviceMap = make(map[string]*Service)
	m.loadServicesFromDir("/usr/lib/deepin-daemon/" + moduleName)
	m.loadServicesFromDir("/etc/deepin-daemon/" + moduleName)

	for _, service := range m.serviceMap {
		switch service.Monitor.Type {
		case typeDBus:
			dbusField := service.Monitor.DBus
			if dbusField == nil {
				continue
			}
			if dbusField.BusType == busTypeSystemStr {
				m.systemSigMonitor.appendService(service)
			} else if dbusField.BusType == busTypeSessionStr {
				m.sessionSigMonitor.appendService(service)
			}
		}
	}
}

const (
	serviceFileExt    = ".service.json"
	typeDBus          = "DBus"
	busTypeSystemStr  = "System"
	busTypeSessionStr = "Session"
)

func (m *Manager) loadServicesFromDir(dirname string) {
	fileInfoList, _ := ioutil.ReadDir(dirname)
	for _, fileInfo := range fileInfoList {
		if fileInfo.IsDir() {
			continue
		}

		name := fileInfo.Name()
		if !strings.HasSuffix(name, serviceFileExt) {
			continue
		}

		filename := filepath.Join(dirname, name)
		service, err := loadService(filename)
		if err != nil {
			logger.Warningf("failed to load %q: %v", filename, err)
			continue
		} else {
			logger.Debugf("load %q ok", filename)
		}

		_, ok := m.serviceMap[name]
		if ok {
			logger.Debugf("file %q overwrites the old", filename)
		}
		m.serviceMap[name] = service
	}
}

func getNameOwner(conn *dbus.Conn, name string) (string, error) {
	var owner string
	err := conn.BusObject().Call("org.freedesktop.DBus.GetNameOwner",
		0, name).Store(&owner)
	if err != nil {
		return "", err
	}
	return owner, err
}

func newReplacer(signal *dbus.Signal) *strings.Replacer {
	var oldNewSlice []string
	for idx, item := range signal.Body {
		oldStr := fmt.Sprintf("%%{arg%d}", idx)
		newStr := fmt.Sprintf("%v", item)
		logger.Debugf("old %q => new %q", oldStr, newStr)
		oldNewSlice = append(oldNewSlice, oldStr, newStr)
	}
	return strings.NewReplacer(oldNewSlice...)
}

func (m *Manager) execService(service *Service,
	signal *dbus.Signal) {
	if service.execFn != nil {
		service.execFn(signal)
		return
	}

	if len(service.Exec) == 0 {
		logger.Warning("service Exec empty")
		return
	}

	var args []string
	execArgs := service.Exec[1:]
	if logger.GetLogLevel() == log.LevelDebug {
		if service.Exec[0] == "sh" {
			// add -x option for debug shell
			execArgs = append([]string{"-x"}, execArgs...)
		}
	}

	replacer := newReplacer(signal)
	for _, arg := range execArgs {
		args = append(args, replacer.Replace(arg))
	}

	logger.Debugf("run cmd %q %#v", service.Exec[0], args)
	cmd := exec.Command(service.Exec[0], args...)
	out, err := cmd.CombinedOutput()
	logger.Debugf("cmd combined output: %s", out)
	if err != nil {
		logger.Warning(err)
	}
}
