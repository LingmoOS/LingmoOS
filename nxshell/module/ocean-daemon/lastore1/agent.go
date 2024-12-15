// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lastore

import (
	"os"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	ControlCenter "github.com/lingmoos/go-dbus-factory/session/com.lingmo.ocean.ControlCenter"
	kwayland "github.com/lingmoos/go-dbus-factory/session/org.lingmo.ocean.kwayland1"
	lastore "github.com/lingmoos/go-dbus-factory/system/org.lingmo.ocean.lastore1"
	"github.com/lingmoos/go-lib/dbusutil"
)

const (
	// lastore agent的interface name
	sessionAgentInterface = "com.lingmo.lastore.Agent"
	sessionAgentPath      = "/com/lingmo/lastore/agent"
)

// 对应org.lingmo.ocean.Network1.GetProxy方法的key值
const (
	proxyTypeHttp  = "http"
	proxyTypeHttps = "https"
	proxyTypeFtp   = "ftp"
	proxyTypeSocks = "socks"
)

// 对应系统代理环境变量
const (
	envHttpProxy  = "http_proxy"
	envHttpsProxy = "https_proxy"
	envFtpProxy   = "ftp_proxy"
	envAllProxy   = "all_proxy"
)

// Agent 需要实现GetManualProxy、SendNotify、CloseNotification、ReportLog四个接口
type Agent struct {
	sysService       *dbusutil.Service
	sessionService   *dbusutil.Service
	lastoreObj       *Lastore
	sysLastore       lastore.Lastore
	waylandWM        kwayland.WindowManager
	controlCenter    ControlCenter.ControlCenter
	mu               sync.Mutex
	isWaylandSession bool
}

func newAgent(l *Lastore) (*Agent, error) {
	sysBus, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning(err)
		return nil, err
	}
	sessionBus, err := dbusutil.NewSessionService()
	if err != nil {
		logger.Warning(err)
		return nil, err
	}
	a := &Agent{
		sysService:     sysBus,
		sessionService: sessionBus,
	}
	a.lastoreObj = l
	a.sysLastore = lastore.NewLastore(a.sysService.Conn())
	if strings.Contains(os.Getenv("XDG_SESSION_TYPE"), "wayland") {
		a.isWaylandSession = true
		a.waylandWM = kwayland.NewWindowManager(a.sessionService.Conn())
	}
	a.controlCenter = ControlCenter.NewControlCenter(a.sessionService.Conn())
	return a, nil
}

func (a *Agent) init() error {
	err := a.sysService.Export(sessionAgentPath, a)
	if err != nil {
		logger.Warning(err)
		return err
	}
	err = a.sysLastore.Manager().RegisterAgent(0, sessionAgentPath)
	if err != nil {
		logger.Warning(err)
		return err
	}
	return nil
}

func (a *Agent) destroy() {
	err := a.sysLastore.Manager().UnRegisterAgent(0, sessionAgentPath)
	if err != nil {
		logger.Warning(err)
	}
}

func (a *Agent) checkCallerAuth(sender dbus.Sender) bool {
	const rootUid = 0
	uid, err := a.sysService.GetConnUID(string(sender))
	if err != nil {
		return false
	}
	if uid != rootUid {
		logger.Warningf("not allow %v call this method", sender)
		return false
	}
	return true
}
