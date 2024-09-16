// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package sessionwatcher

import (
	"sync"

	"github.com/godbus/dbus/v5"
	libdisplay "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.display1"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

//go:generate dbusutil-gen em -type Manager

const (
	dbusServiceName = "org.deepin.dde.SessionWatcher1"
	dbusPath        = "/org/deepin/dde/SessionWatcher1"
	dbusInterface   = dbusServiceName
)

type Manager struct {
	service           *dbusutil.Service
	display           libdisplay.Display
	loginManager      login1.Manager
	systemSigLoop     *dbusutil.SignalLoop
	mu                sync.Mutex
	sessions          map[string]login1.Session
	activeSessionType string

	PropsMu  sync.RWMutex
	IsActive bool
}

var (
	_validSessionList = []string{
		"x11",
		"wayland",
	}
)

func newManager(service *dbusutil.Service) (*Manager, error) {
	manager := &Manager{
		service:  service,
		sessions: make(map[string]login1.Session),
	}
	systemConn, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	sessionConn := service.Conn()
	manager.loginManager = login1.NewManager(systemConn)
	manager.display = libdisplay.NewDisplay(sessionConn)

	manager.systemSigLoop = dbusutil.NewSignalLoop(systemConn, 10)
	manager.systemSigLoop.Start()
	manager.loginManager.InitSignalExt(manager.systemSigLoop, true)

	// default as active
	manager.IsActive = true
	return manager, nil
}

func (m *Manager) destroy() {
	m.mu.Lock()
	for _, session := range m.sessions {
		session.RemoveHandler(proxy.RemoveAllHandlers)
	}
	m.mu.Unlock()

	m.loginManager.RemoveHandler(proxy.RemoveAllHandlers)
	m.systemSigLoop.Stop()
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func (m *Manager) initUserSessions() {
	sessions, err := m.loginManager.ListSessions(0)
	if err != nil {
		logger.Warning("List sessions failed:", err)
		return
	}

	for _, session := range sessions {
		m.addSession(session.SessionId, session.Path)
	}
	m.handleSessionChanged()
	_, err = m.loginManager.ConnectSessionNew(func(id string, path dbus.ObjectPath) {
		logger.Debug("Session added:", id, path)
		m.addSession(id, path)
		m.handleSessionChanged()
	})
	if err != nil {
		logger.Warning("ConnectSessionNew error:", err)
	}

	_, err = m.loginManager.ConnectSessionRemoved(func(id string, path dbus.ObjectPath) {
		logger.Debug("Session removed:", id, path)
		m.deleteSession(id, path)
		m.handleSessionChanged()
	})
	if err != nil {
		logger.Warning("ConnectSessionRemoved error:", err)
	}
}

func (m *Manager) addSession(id string, path dbus.ObjectPath) {
	systemConn := m.systemSigLoop.Conn()
	session, err := login1.NewSession(systemConn, path)
	if err != nil {
		logger.Warning(err)
		return
	}

	userInfo, err := session.User().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	uid := userInfo.UID
	logger.Debug("Add session:", id, path, uid)
	if !isCurrentUser(uid) {
		logger.Debug("Not the current user session:", id, path, uid)
		return
	}
	remote, err := session.Remote().Get(0)
	if err != nil {
		logger.Warning(err)
		return
	}
	if remote {
		logger.Debugf("session %v is remote", id)
		return
	}

	m.mu.Lock()
	m.sessions[id] = session
	m.mu.Unlock()

	session.InitSignalExt(m.systemSigLoop, true)
	err = session.Active().ConnectChanged(func(hasValue bool, value bool) {
		m.handleSessionChanged()
	})
	if err != nil {
		logger.Warning("ConnectChanged error:", err)
	}
}

func (m *Manager) deleteSession(id string, path dbus.ObjectPath) {
	m.mu.Lock()
	session, ok := m.sessions[id]
	if !ok {
		m.mu.Unlock()
		return
	}

	session.RemoveHandler(proxy.RemoveAllHandlers)
	logger.Debug("Delete session:", id, path)
	delete(m.sessions, id)
	m.mu.Unlock()
}

func (m *Manager) handleSessionChanged() {
	m.mu.Lock()
	defer m.mu.Unlock()
	if len(m.sessions) == 0 {
		return
	}

	session := m.getActiveSession()
	var isActive bool
	var sessionType string
	if session != nil {
		isActive = true
		var err error
		sessionType, err = session.Type().Get(0)
		if err != nil {
			logger.Warning(err)
		}
	}

	m.activeSessionType = sessionType
	m.PropsMu.Lock()
	changed := m.setIsActive(isActive)
	m.PropsMu.Unlock()
	if !changed {
		return
	}

	if isActive {
		logger.Debug("[handleSessionChanged] Resume pulse")
		// fixed block when unused pulse-audio
		go suspendPulseSinks(0)
		go suspendPulseSources(0)

		logger.Debug("[handleSessionChanged] Refresh Brightness")
		go func() {
			_ = m.display.RefreshBrightness(0)
		}()
	} else {
		logger.Debug("[handleSessionChanged] Suspend pulse")
		go suspendPulseSinks(1)
		go suspendPulseSources(1)
	}
}

// return is changed?
func (m *Manager) setIsActive(val bool) bool {
	if m.IsActive != val {
		m.IsActive = val
		logger.Debug("[setIsActive] IsActive changed:", val)
		err := m.service.EmitPropertyChanged(m, "IsActive", val)
		if err != nil {
			logger.Warning("EmitPropertyChanged error:", err)
		}
		return true
	}
	return false
}

func (m *Manager) getActiveSession() login1.Session {
	for _, session := range m.sessions {
		seatInfo, err := session.Seat().Get(0)
		if err != nil {
			logger.Warning(err)
			continue
		}

		if seatInfo.Id != "" && seatInfo.Path != "/" {
			active, err := session.Active().Get(0)
			if err != nil {
				logger.Warning(err)
				continue
			}
			if active {
				return session
			}
		}
	}
	return nil
}

func (m *Manager) IsX11SessionActive(sender dbus.Sender) (active bool, busErr *dbus.Error) {
	// 从login1获取当前登录的用户是否激活
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	login1Manager := login1.NewManager(service.Conn())
	userObjectPath, err := login1Manager.GetUserByPID(0, pid)
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	userDbus, err := login1.NewUser(service.Conn(), userObjectPath)
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	sessionInfo, err := userDbus.Display().Get(0)
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	session, err := login1.NewSession(service.Conn(), sessionInfo.Path)
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	isActive, err := session.Active().Get(0)
	if err != nil {
		logger.Warning(err)
		return false, dbusutil.ToError(err)
	}

	if isActive {
		return true, nil
	}

	return false, nil
}

func (m *Manager) GetSessions() (sessions []dbus.ObjectPath, err *dbus.Error) {
	m.mu.Lock()
	sessions = make([]dbus.ObjectPath, len(m.sessions))
	i := 0
	for _, session := range m.sessions {
		sessions[i] = session.Path_()
		i++
	}
	m.mu.Unlock()
	return
}
