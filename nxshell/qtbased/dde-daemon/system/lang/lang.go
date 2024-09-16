// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lang

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/loader"
	accounts "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.accounts1"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/strv"
	"os"
	"path/filepath"
	"sync"
)

const (
	langService = "org.deepin.dde.Lang1"

	userLocaleConfigFile    = ".config/locale.conf"
	userLocaleConfigFileTmp = ".config/.locale.conf"
)

var _desktopType = strv.Strv{"x11", "wayland"}

type Module struct {
	*loader.ModuleBase
	lang *Lang
}

func (m *Module) GetDependencies() []string {
	return nil
}

func (m *Module) Start() error {
	if m.lang != nil {
		return nil
	}
	logger.Debug("start language service")
	m.lang = newLang()

	service := loader.GetService()
	m.lang.service = service

	m.lang.init()

	return nil
}

func (m *Module) Stop() error {
	// TODO:
	return nil
}

var logger = log.NewLogger("daemon/system/lang")

func newModule(logger *log.Logger) *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("lang", m, logger)
	return m
}

func init() {
	loader.Register(newModule(logger))
}

//go:generate dbusutil-gen -type Lang lang.go
//go:generate dbusutil-gen em -type Lang

type Lang struct {
	sessionPathHomeMapMu sync.Mutex
	service              *dbusutil.Service
	sigLoop              *dbusutil.SignalLoop
	sessionPathHomeMap   map[dbus.ObjectPath]string // 存放每个图形session以及其对应的家目录路径
}

func newLang() *Lang {
	return &Lang{
		sessionPathHomeMap: make(map[dbus.ObjectPath]string),
	}
}

func (l *Lang) init() {
	l.sigLoop = dbusutil.NewSignalLoop(l.service.Conn(), 10)
	l.sigLoop.Start()

	l.loadSessionList()
	l.listenSystemSignals()
	l.updateAllUserLocale()
}

func (l *Lang) updateLocaleBySessionPath(_ string, sessionPath dbus.ObjectPath) {
	var homeDir string
	var ok bool

	l.sessionPathHomeMapMu.Lock()
	if homeDir, ok = l.sessionPathHomeMap[sessionPath]; !ok { // 当sessionPath未保存到map中,则无需处理
		logger.Debugf("can not find sessionPath %s, return", sessionPath)
		l.sessionPathHomeMapMu.Unlock()
		return
	}
	l.sessionPathHomeMapMu.Unlock()

	l.updateLocaleByHomeDir(homeDir)
	l.deleteSessionPathHomeMap(sessionPath)
}

func (l *Lang) updateLocaleByHomeDir(homeDir string) {
	localeConfigFile := filepath.Join(homeDir, userLocaleConfigFile)
	localeConfigFileTmp := filepath.Join(homeDir, userLocaleConfigFileTmp)
	l.updateLocaleFile(localeConfigFileTmp, localeConfigFile)
}

func (l *Lang) updateLocaleFile(tempLocaleFilePath, localeFilePath string) {
	_, err := os.Stat(tempLocaleFilePath)
	if err != nil {
		return
	}
	err = os.Rename(tempLocaleFilePath, localeFilePath)
	if err != nil {
		logger.Warning(err)
	}
}

func (l *Lang) updateAllUserLocale() {
	l.sessionPathHomeMapMu.Lock()
	defer l.sessionPathHomeMapMu.Unlock()
	for _, homeDir := range l.sessionPathHomeMap {
		logger.Debug("update local in : ", homeDir)
		l.updateLocaleByHomeDir(homeDir)
	}
}

func (l *Lang) deleteSessionPathHomeMap(sessionPath dbus.ObjectPath) {
	l.sessionPathHomeMapMu.Lock()
	defer l.sessionPathHomeMapMu.Unlock()
	delete(l.sessionPathHomeMap, sessionPath)
}

func (l *Lang) addSessionMap(_ string, _ dbus.ObjectPath) {
	l.loadSessionList()
	l.updateAllUserLocale()
}

func (l *Lang) listenSystemSignals() {
	manager := login1.NewManager(l.service.Conn())
	manager.InitSignalExt(l.sigLoop, true)

	// 登录时,监听是哪个 session 登录,获取这个 session 的用户信息保存到 map 中
	_, err := manager.ConnectSessionNew(l.addSessionMap)
	if err != nil {
		logger.Warning(err)
	}

	// 注销时,监听是哪个 session 注销,从 map 中获取这个 session 的用户信息, 对这个用户的家目录下语言配置进行更新
	_, err = manager.ConnectSessionRemoved(l.updateLocaleBySessionPath)
	if err != nil {
		logger.Warning(err)
	}
}

func (l *Lang) loadSessionList() {
	manager := login1.NewManager(l.service.Conn())
	sessions, err := manager.ListSessions(0)
	if err != nil {
		logger.Warning(err)
		return
	}

	l.sessionPathHomeMapMu.Lock()
	defer l.sessionPathHomeMapMu.Unlock()

	for _, sessionDetail := range sessions {
		session, err := login1.NewSession(l.service.Conn(), sessionDetail.Path)
		if err != nil {
			continue
		}
		type0, err := session.Type().Get(0) // 远程登录或者tty登录时创建的session无需保存到map中
		if err != nil {
			continue
		}
		if !_desktopType.Contains(type0) {
			continue
		}
		l.sessionPathHomeMap[sessionDetail.Path] = l.getHomeDirBySessionDetail(sessionDetail)
	}
}

func (l *Lang) getHomeDirBySessionDetail(sessionDetail login1.SessionDetail) string {
	account := accounts.NewAccounts(l.service.Conn())

	userPath, err := account.FindUserByName(0, sessionDetail.UserName)
	if err != nil {
		logger.Warning(err)
		return ""
	}
	user, err := accounts.NewUser(l.service.Conn(), dbus.ObjectPath(userPath))
	if err != nil {
		logger.Warning(err)
		return ""
	}

	homeDir, err := user.HomeDir().Get(0)
	if err != nil {
		logger.Warning(err)
		return ""
	}
	return homeDir
}
