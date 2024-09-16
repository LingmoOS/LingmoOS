// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"errors"
	"fmt"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	btcommon "github.com/linuxdeepin/dde-daemon/common/bluetooth"
	bluez "github.com/linuxdeepin/go-dbus-factory/system/org.bluez"
	sysbtagent "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.bluetooth1.agent"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	agentDBusPath      = dbusPath + "/Agent"
	agentDBusInterface = "org.bluez.Agent1"
)

type userAgentMap struct {
	mu        sync.Mutex
	m         map[string]*sessionAgentMapItem // key 是 uid
	activeUid string
}

type sessionAgentMapItem struct {
	sessions map[dbus.ObjectPath]login1.Session   // key 是 session 的路径
	agents   map[dbus.ObjectPath]sysbtagent.Agent // key 是 agent 的路径
}

func (m *userAgentMap) setActiveUid(uid string) {
	m.mu.Lock()
	m.activeUid = uid
	m.mu.Unlock()
}

func newUserAgentMap() *userAgentMap {
	return &userAgentMap{
		m: make(map[string]*sessionAgentMapItem, 1),
	}
}

func (m *userAgentMap) addAgent(uid string, agent sysbtagent.Agent) {
	m.mu.Lock()
	defer m.mu.Unlock()

	item, ok := m.m[uid]
	if ok {
		if item.agents == nil {
			item.agents = make(map[dbus.ObjectPath]sysbtagent.Agent, 1)
		}
		if len(item.agents) > 10 {
			// 限制数量
			return
		}
		item.agents[agent.Path_()] = agent
	} else {
		m.m[uid] = &sessionAgentMapItem{
			agents: map[dbus.ObjectPath]sysbtagent.Agent{
				agent.Path_(): agent,
			},
		}
	}
}

func (m *userAgentMap) handleNameLost(name string) {
	m.mu.Lock()
	defer m.mu.Unlock()

	for _, item := range m.m {
		for path, agent := range item.agents {
			if agent.ServiceName_() == name {
				logger.Debug("remove agent", name, path)
				delete(item.agents, path)
			}
		}
	}
}

func (m *userAgentMap) removeAgent(uid string, agentPath dbus.ObjectPath) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	item, ok := m.m[uid]
	if !ok {
		return errors.New("invalid uid")
	}

	if _, ok := item.agents[agentPath]; !ok {
		return errors.New("invalid agent path")
	}
	delete(item.agents, agentPath)
	return nil
}

func (m *userAgentMap) addUser(uid string) {
	m.mu.Lock()
	defer m.mu.Unlock()

	_, ok := m.m[uid]
	if !ok {
		m.m[uid] = &sessionAgentMapItem{
			sessions: make(map[dbus.ObjectPath]login1.Session),
			agents:   make(map[dbus.ObjectPath]sysbtagent.Agent),
		}
	}
}

func (m *userAgentMap) removeUser(uid string) {
	m.mu.Lock()
	defer m.mu.Unlock()

	item, ok := m.m[uid]
	if !ok {
		return
	}

	for sessionPath, session := range item.sessions {
		session.RemoveAllHandlers()
		delete(item.sessions, sessionPath)
	}
	delete(m.m, uid)
}

func (m *userAgentMap) hasUser(uid string) bool {
	m.mu.Lock()
	defer m.mu.Unlock()
	_, ok := m.m[uid]
	return ok
}

func (m *userAgentMap) addSession(uid string, session login1.Session) bool {
	m.mu.Lock()
	defer m.mu.Unlock()

	item, ok := m.m[uid]
	if !ok {
		return false
	}

	_, ok = item.sessions[session.Path_()]
	if ok {
		return false
	}
	item.sessions[session.Path_()] = session
	return true
}

func (m *userAgentMap) removeSession(sessionPath dbus.ObjectPath) {
	m.mu.Lock()
	defer m.mu.Unlock()

	for _, item := range m.m {
		for sPath, session := range item.sessions {
			if sPath == sessionPath {
				session.RemoveAllHandlers()
				delete(item.sessions, sPath)
			}
		}
	}
}

func (m *userAgentMap) getActiveAgent() sysbtagent.Agent {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.activeUid == "" {
		return nil
	}

	item := m.m[m.activeUid]
	if item == nil {
		return nil
	}
	// 目前只使用标准的
	return item.agents[btcommon.SessionAgentPath]
}

func (m *userAgentMap) GetActiveAgentName() string {
	agent := m.getActiveAgent()
	if agent != nil {
		return agent.ServiceName_()
	}
	return ""
}

func (m *userAgentMap) GetAllAgentNames() (result []string) {
	m.mu.Lock()
	defer m.mu.Unlock()

	for _, item := range m.m {
		// 目前只使用标准的
		agent := item.agents[btcommon.SessionAgentPath]
		if agent != nil {
			result = append(result, agent.ServiceName_())
		}
	}
	return result
}

type authorize struct {
	path   dbus.ObjectPath
	key    string
	accept bool
}

type agent struct {
	service      *dbusutil.Service
	bluezManager bluez.Manager

	b       *SysBluetooth
	rspChan chan authorize

	mu            sync.Mutex
	requestDevice dbus.ObjectPath
}

func (*agent) GetInterfaceName() string {
	return agentDBusInterface
}

func toErrFromAgent(err error) *dbus.Error {
	if err == nil {
		return nil
	}
	busErr, ok := err.(dbus.Error)
	if !ok {
		// 强制转换为 Rejected
		return &dbus.Error{
			Name: btcommon.ErrNameRejected,
			Body: []interface{}{err.Error()},
		}
	}

	if busErr.Name == btcommon.ErrNameRejected || busErr.Name == btcommon.ErrNameCanceled {
		return &busErr
	}

	// 强制转换为 Rejected
	return &dbus.Error{
		Name: btcommon.ErrNameRejected,
		Body: []interface{}{fmt.Sprintf("[%s] %s", busErr.Name, busErr.Error())},
	}
}

/*****************************************************************************/

// Release method gets called when the service daemon unregisters the agent.
// An agent can use it to do cleanup tasks. There is no need to unregister the
// agent, because when this method gets called it has already been unregistered.
func (a *agent) Release() *dbus.Error {
	logger.Info("Release()")
	return nil
}

// RequestPinCode method gets called when the service daemon needs to get the passkey for an authentication.
// The return value should be a string of 1-16 characters length. The string can be alphanumeric.
// Possible errors: org.bluez.Error.Rejected
//
//	org.bluez.Error.Canceled
func (a *agent) RequestPinCode(device dbus.ObjectPath) (pinCode string, busErr *dbus.Error) {
	logger.Info("RequestPinCode()")

	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return "", btcommon.ErrRejected
	}

	d, err := a.b.getDevice(device)
	if err != nil {
		logger.Warning(err)
		return "", btcommon.ErrRejected
	}
	d.agentWorkStart()
	defer d.agentWorkEnd()

	pinCode, err = ua.RequestPinCode(0, device)
	return pinCode, toErrFromAgent(err)
}

// DisplayPinCode method gets called when the service daemon needs to display a pincode for an authentication.
// An empty reply should be returned. When the pincode needs no longer to be displayed, the Cancel method
// of the agent will be called. This is used during the pairing process of keyboards that don't support
// Bluetooth 2.1 Secure Simple Pairing, in contrast to DisplayPasskey which is used for those that do.
// This method will only ever be called once since older keyboards do not support typing notification.
// Note that the PIN will always be a 6-digit number, zero-padded to 6 digits. This is for harmony with
// the later specification.
// Possible errors: org.bluez.Error.Rejected
//
//	org.bluez.Error.Canceled
func (a *agent) DisplayPinCode(device dbus.ObjectPath, pinCode string) (busErr *dbus.Error) {
	logger.Info("DisplayPinCode()", pinCode)
	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return btcommon.ErrRejected
	}
	err := ua.DisplayPinCode(0, device, pinCode)
	return toErrFromAgent(err)
}

// RequestPasskey method gets called when the service daemon needs to get the passkey for an authentication.
// The return value should be a numeric value between 0-999999.
// Possible errors: org.bluez.Error.Rejected
//
//	org.bluez.Error.Canceled
func (a *agent) RequestPasskey(device dbus.ObjectPath) (passkey uint32, busErr *dbus.Error) {
	logger.Info("RequestPasskey()")
	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return 0, btcommon.ErrRejected
	}

	d, err := a.b.getDevice(device)
	if err != nil {
		logger.Warning(err)
		return 0, btcommon.ErrRejected
	}
	d.agentWorkStart()
	defer d.agentWorkEnd()

	passkey, err = ua.RequestPasskey(0, device)
	return passkey, toErrFromAgent(err)
}

// DisplayPasskey method gets called when the service daemon needs to display a passkey for an authentication.
// The entered parameter indicates the number of already typed keys on the remote side.
// An empty reply should be returned. When the passkey needs no longer to be displayed, the Cancel method
// of the agent will be called.
// During the pairing process this method might be called multiple times to update the entered value.
// Note that the passkey will always be a 6-digit number, so the display should be zero-padded at the start if
// the value contains less than 6 digits.
func (a *agent) DisplayPasskey(device dbus.ObjectPath, passkey uint32, entered uint16) *dbus.Error {
	logger.Info("DisplayPasskey()", passkey, entered)
	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return btcommon.ErrRejected
	}
	err := ua.DisplayPasskey(0, device, passkey, entered)
	return toErrFromAgent(err)
}

// RequestConfirmation This method gets called when the service daemon needs to confirm a passkey for an authentication.
// To confirm the value it should return an empty reply or an error in case the passkey is invalid.
// Note that the passkey will always be a 6-digit number, so the display should be zero-padded at the start if
// the value contains less than 6 digits.
// Possible errors: org.bluez.Error.Rejected
//
//	org.bluez.Error.Canceled
func (a *agent) RequestConfirmation(device dbus.ObjectPath, passkey uint32) *dbus.Error {
	logger.Info("RequestConfirmation", device, passkey)
	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return btcommon.ErrRejected
	}

	d, err := a.b.getDevice(device)
	if err != nil {
		logger.Warning(err)
		return btcommon.ErrRejected
	}
	d.agentWorkStart()
	defer d.agentWorkEnd()

	if d.Icon == "audio-card" && strings.Contains(strings.ToLower(d.Name), "huawei") {
		logger.Warning("audio-card device don't need confirm")
		return nil
	}

	err = ua.RequestConfirmation(0, device, passkey)
	return toErrFromAgent(err)
}

// RequestAuthorization method gets called to request the user to authorize an incoming pairing attempt which
// would in other circumstances trigger the just-works model.
// Possible errors: org.bluez.Error.Rejected
//
//	org.bluez.Error.Canceled
func (a *agent) RequestAuthorization(device dbus.ObjectPath) *dbus.Error {
	logger.Info("RequestAuthorization()")
	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return btcommon.ErrRejected
	}

	d, err := a.b.getDevice(device)
	if err != nil {
		logger.Warning(err)
		return btcommon.ErrRejected
	}
	d.agentWorkStart()
	defer d.agentWorkEnd()

	err = ua.RequestAuthorization(0, device)
	return toErrFromAgent(err)
}

// AuthorizeService method gets called when the service daemon needs to authorize a connection/service request.
// Possible errors: org.bluez.Error.Rejected
//
//	org.bluez.Error.Canceled
func (a *agent) AuthorizeService(device dbus.ObjectPath, uuid string) *dbus.Error {
	logger.Infof("AuthorizeService %q %q", device, uuid)
	_, err := a.b.getDevice(device)
	if err != nil {
		logger.Warning(err)
		return btcommon.ErrRejected
	}

	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return btcommon.ErrRejected
	}
	err = ua.AuthorizeService(0, device, uuid)
	return toErrFromAgent(err)
}

// Cancel method gets called to indicate that the agent request failed before a reply was returned.
func (a *agent) Cancel() *dbus.Error {
	logger.Info("Cancel()")
	ua := a.b.getActiveUserAgent()
	if ua == nil {
		logger.Warning("ua is nil")
		return nil
	}
	err := ua.Cancel(0)
	return toErrFromAgent(err)
}

/*****************************************************************************/

func newAgent(service *dbusutil.Service) (a *agent) {
	a = &agent{
		service: service,
		rspChan: make(chan authorize),
	}
	return
}

func (a *agent) init() {
	sysBus := a.service.Conn()
	a.bluezManager = bluez.NewManager(sysBus)
	a.registerDefaultAgent()
}

func (a *agent) registerDefaultAgent() {
	// register agent
	err := a.bluezManager.AgentManager().RegisterAgent(0, agentDBusPath, "DisplayYesNo")
	if err != nil {
		logger.Warning("failed to register agent:", err)
		return
	}

	// request default agent
	err = a.bluezManager.AgentManager().RequestDefaultAgent(0, agentDBusPath)
	if err != nil {
		logger.Warning("failed to become the default agent:", err)
		err = a.bluezManager.AgentManager().UnregisterAgent(0, agentDBusPath)
		if err != nil {
			logger.Warning(err)
		}
		return
	}
}

func (a *agent) destroy() {
	err := a.bluezManager.AgentManager().UnregisterAgent(0, agentDBusPath)
	if err != nil {
		logger.Warning(err)
	}

	err = a.service.StopExport(a)
	if err != nil {
		logger.Warning(err)
	}
}
