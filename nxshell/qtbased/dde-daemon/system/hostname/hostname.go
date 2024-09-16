// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package hostname

import (
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strings"

	"github.com/linuxdeepin/dde-daemon/loader"
	hostname1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.hostname1"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

type Module struct {
	*loader.ModuleBase
	hostname *HostName
}

func (m *Module) GetDependencies() []string {
	return nil
}

func (m *Module) Start() error {
	if m.hostname != nil {
		return nil
	}
	logger.Debug("start hostname service")
	m.hostname = newHostName()

	service := loader.GetService()
	m.hostname.service = service

	m.hostname.init()

	return nil
}

func (m *Module) Stop() error {
	return nil
}

var logger = log.NewLogger("daemon/system/hostname")

func newModule(logger *log.Logger) *Module {
	m := new(Module)
	m.ModuleBase = loader.NewModuleBase("hostname", m, logger)
	return m
}

func init() {
	loader.Register(newModule(logger))
}

type HostName struct {
	service *dbusutil.Service
	sigLoop *dbusutil.SignalLoop
}

func newHostName() *HostName {
	return &HostName{}
}

func (h *HostName) init() {
	h.sigLoop = dbusutil.NewSignalLoop(h.service.Conn(), 10)
	h.sigLoop.Start()

	err := h.listenHostNameChangeSignals()
	if err != nil {
		logger.Warning("hostname listen error:", err)
	}
}

func (h *HostName) listenHostNameChangeSignals() error {
	hostnameObj := hostname1.NewHostname(h.service.Conn())
	hostnameObj.InitSignalExt(h.sigLoop, true)
	err := hostnameObj.Hostname().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		login := login1.NewManager(h.service.Conn())
		sessions, err := login.ListSessions(0)
		if err != nil {
			logger.Warning("call login.ListSessions error:", err)
			return
		}
		xorgCmdContent := ""
		for _, session := range sessions {
			logger.Debug("session:", session.Path)
			if session.UserName == "lightdm" {
				sessionObj, err := login1.NewSession(h.service.Conn(), session.Path)
				if err != nil {
					logger.Warning("call login1.NewSession error:", err)
					continue
				}
				display, err := sessionObj.Display().Get(0)
				if err != nil {
					logger.Warning("call sessionObj.Display().Get(0) error:", err)
					continue
				}
				if display != "" {
					xorgCmdContent = "/var/run/lightdm/root/" + display
					break
				}
			}
		}
		logger.Debug("xorgCmdContent:", xorgCmdContent)
		if xorgCmdContent != "" {
			dirList, err := ioutil.ReadDir("/proc/")
			if err != nil {
				logger.Debug("read dir /proc error:", err)
				return
			}
			for _, dir := range dirList {
				if !dir.IsDir() {
					continue
				}
				procPath := filepath.Join("/proc/", dir.Name())
				linkDst, err := os.Readlink(filepath.Join(procPath, "exe"))
				if err != nil {
					logger.Debug("Readlink error:", err)
					continue
				}
				dstFileName := path.Base(linkDst)
				if dstFileName != "Xorg" {
					continue
				}
				cmdline := filepath.Join(procPath, "cmdline")
				content, err := ioutil.ReadFile(cmdline) // #nosec G304
				if err != nil {
					logger.Debug("ReadFile error:", err)
					continue
				}
				if !strings.Contains(string(content), xorgCmdContent) {
					logger.Debug("cmdline not contains xorgCmdContent:", xorgCmdContent)
					continue
				}
				pid := dir.Name()
				err = exec.Command("kill", "-9", pid).Run() // #nosec G204
				if err != nil {
					logger.Warning("to kill process err:", err)
				}
				logger.Debug("to kill xorg process, pid:", pid)
				break
			}
		}
	})

	return err
}
