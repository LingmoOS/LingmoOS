// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package logined

import (
	"encoding/json"
	"fmt"
	"sync"

	"github.com/godbus/dbus/v5"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
	"github.com/linuxdeepin/go-lib/log"
)

//go:generate dbusutil-gen em -type Manager

// Manager manager logined user list
type Manager struct {
	service    *dbusutil.Service
	sysSigLoop *dbusutil.SignalLoop
	core       login1.Manager
	logger     *log.Logger

	userSessions map[uint32]SessionInfos
	locker       sync.Mutex

	UserList       string
	LastLogoutUser uint32
}

const (
	DBusPath = "/org/deepin/dde/Logined"
)

// Register register and install loginedManager on dbus
func Register(logger *log.Logger, service *dbusutil.Service) (*Manager, error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	core := login1.NewManager(systemBus)
	sysSigLoop := dbusutil.NewSignalLoop(systemBus, 10)
	sysSigLoop.Start()
	var m = &Manager{
		service:      service,
		core:         core,
		logger:       logger,
		userSessions: make(map[uint32]SessionInfos),
		sysSigLoop:   sysSigLoop,
	}

	go m.init()
	m.handleChanged()
	return m, nil
}

// Unregister destroy and free Manager object
func Unregister(m *Manager) {
	if m == nil {
		return
	}

	m.core.RemoveHandler(proxy.RemoveAllHandlers)
	m.sysSigLoop.Stop()

	if m.userSessions != nil {
		m.userSessions = nil
	}

	m = nil
}

func (m *Manager) init() {
	// the result struct: {id, uid, username, seat, path}
	sessions, err := m.core.ListSessions(0)
	if err != nil {
		m.logger.Warning("Failed to list sessions:", err)
		return
	}

	for _, session := range sessions {
		m.addSession(session.Path)
	}
	m.setPropUserList()
}

func (m *Manager) handleChanged() {
	m.core.InitSignalExt(m.sysSigLoop, true)
	_, _ = m.core.ConnectSessionNew(func(id string, sessionPath dbus.ObjectPath) {
		m.logger.Debug("[Event] session new:", id, sessionPath)
		added := m.addSession(sessionPath)
		if added {
			m.setPropUserList()
		}
	})
	_, _ = m.core.ConnectSessionRemoved(func(id string, sessionPath dbus.ObjectPath) {
		m.logger.Debug("[Event] session remove:", id, sessionPath)
		deleted := m.deleteSession(sessionPath)
		if deleted {
			m.setPropUserList()
		}
	})
}

func (m *Manager) addSession(sessionPath dbus.ObjectPath) bool {
	m.logger.Debug("Create user session for:", sessionPath)
	info, err := newSessionInfo(sessionPath)
	if err != nil {
		m.logger.Warning("Failed to add session:", sessionPath, err)
		return false
	}

	m.locker.Lock()
	defer m.locker.Unlock()
	infos, ok := m.userSessions[info.Uid]
	if !ok {
		m.userSessions[info.Uid] = SessionInfos{info}
		return true
	}

	infos, added := infos.Add(info)
	m.userSessions[info.Uid] = infos
	return added
}

func (m *Manager) deleteSession(sessionPath dbus.ObjectPath) bool {
	m.logger.Debug("Delete user session for:", sessionPath)
	m.locker.Lock()
	defer m.locker.Unlock()
	var deleted = false
	for uid, infos := range m.userSessions {
		idx := infos.Index(sessionPath)
		if idx == -1 {
			continue
		}
		sessionInfo := infos[idx]
		if sessionInfo.Display != "" && sessionInfo.Desktop != "" {
			m.setPropLastLogoutUser(sessionInfo.Uid)
		}

		tmp, ok := infos.Delete(sessionPath)
		if !ok {
			continue
		}
		deleted = true
		if len(tmp) == 0 {
			delete(m.userSessions, uid)
		} else {
			m.userSessions[uid] = tmp
		}
		break
	}
	return deleted
}

func (m *Manager) setPropUserList() {
	m.locker.Lock()
	defer m.locker.Unlock()

	if len(m.userSessions) == 0 {
		return
	}

	data := m.marshalUserSessions()
	if m.UserList == string(data) {
		return
	}
	m.UserList = string(data)
	_ = m.service.EmitPropertyChanged(m, "UserList", m.UserList)
}

func (m *Manager) setPropLastLogoutUser(uid uint32) {
	if m.LastLogoutUser != uid {
		m.LastLogoutUser = uid
		_ = m.service.EmitPropertyChanged(m, "LastLogoutUser", uid)
	}
}

func (m *Manager) marshalUserSessions() string {
	if len(m.userSessions) == 0 {
		return ""
	}

	var ret = "{"
	for k, v := range m.userSessions {
		data, err := json.Marshal(v)
		if err != nil {
			m.logger.Warning("Failed to marshal:", v, err)
			continue
		}
		ret += fmt.Sprintf("\"%v\":%s,", k, string(data))
	}

	v := []byte(ret)
	v[len(v)-1] = '}'
	return string(v)
}

func (*Manager) GetInterfaceName() string {
	return "org.deepin.dde.Logined"
}
