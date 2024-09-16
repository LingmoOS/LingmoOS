// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"sync"

	"github.com/godbus/dbus/v5"
	sessionwatcher "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.sessionwatcher1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/keyfile"
	dutils "github.com/linuxdeepin/go-lib/utils"
	"google.golang.org/protobuf/proto"
)

type BaseCollector interface {
	Init(service *dbusutil.Service, fn writeEventLogFunc) error
	Collect() error
	Stop() error
}

const (
	dbusServiceName = "org.deepin.dde.EventLog1"
	dbusPath        = "/org/deepin/dde/EventLog1"
	dbusInterface   = dbusServiceName
)

var (
	userExpPath    = "/var/public/deepin-user-experience/user"
	defaultExpPath = "/etc/deepin/deepin-user-experience"
	varTmpExpPath  = "/var/tmp/deepin/deepin-user-experience/state/"
	expPathV0      = "/var/tmp/exp-state-v0"
)

const (
	enableUserExp  = "switch=on"
	disableUserExp = "switch=off"
)

type EventLog struct {
	service *dbusutil.Service

	writeEventLogFn          writeEventLogFunc
	Enabled                  bool
	propMu                   sync.Mutex
	fileMu                   sync.Mutex
	currentUserVarTmpExpPath string
	sessionSigLoop           *dbusutil.SignalLoop
	sessionWatcher           sessionwatcher.SessionWatcher
}

func newEventLog(service *dbusutil.Service, fn writeEventLogFunc) *EventLog {
	m := &EventLog{
		service:                  service,
		writeEventLogFn:          fn,
		currentUserVarTmpExpPath: path.Join(varTmpExpPath, fmt.Sprintf("%v/info", os.Getuid())),
	}
	m.sessionWatcher = sessionwatcher.NewSessionWatcher(service.Conn())

	m.sessionSigLoop = dbusutil.NewSignalLoop(service.Conn(), 10)
	m.sessionSigLoop.Start()
	m.sessionWatcher.InitSignalExt(m.sessionSigLoop, true)
	_ = m.sessionWatcher.IsActive().ConnectChanged(func(hasValue, active bool) {
		if !hasValue {
			return
		}
		if active {
			logger.Info("start sync exp state")
			m.syncUserExpState()
		}
	})
	return m
}

func (e *EventLog) start() error {
	e.syncUserExpState()
	return nil
}

func (e *EventLog) GetInterfaceName() string {
	return dbusInterface
}

func (e *EventLog) Enable(enable bool) *dbus.Error {
	e.propMu.Lock()
	defer e.propMu.Unlock()
	err := e.setUserExpFileState(enable)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	e.setPropEnabled(enable)
	return nil
}

func (e *EventLog) ReportLog(log string) *dbus.Error {
	if e.writeEventLogFn != nil {
		e.writeEventLogFn(log)
		return nil
	} else {
		return dbusutil.ToError(errors.New("record log to local failed"))
	}
}

func (e *EventLog) syncUserExpState() {
	state := false
	if dutils.IsFileExist(expPathV0) {
		state = e.getV0ExpState()
	} else if dutils.IsFileExist(e.currentUserVarTmpExpPath) {
		// 从1054标准路径获取
		state = e.getUserExpStateFromVarTmpExpPath()
	} else if dutils.IsFileExist("/var/tmp/deepin/deepin-user-experience/state") { // 非当前用户创建的state/uid/info 时
		infos, err := ioutil.ReadDir("/var/tmp/deepin/deepin-user-experience/state")
		if err != nil {
			logger.Warning(err)
		} else {
			for _, info := range infos {
				// 只会有一个文件
				content, err := ioutil.ReadFile(filepath.Join("/var/tmp/deepin/deepin-user-experience/state", info.Name(), "info"))
				if err != nil {
					logger.Warning(err)
				} else {
					state = string(content) == enableUserExp
				}
				break
			}
		}
	} else if dutils.IsFileExist(userExpPath) {
		// 从历史版本获取
		state = e.getUserExpStateFromUserExpPath()
	} else {
		// 从安装器文件获取
		state = e.getUserExpStateFromDefaultPath()
	}
	err := e.setUserExpFileState(state)
	if err != nil {
		logger.Warning(err)
	}
	e.setPropEnabled(state)
}

func (e *EventLog) getUserExpStateFromUserExpPath() bool {
	e.fileMu.Lock()
	defer e.fileMu.Unlock()
	content, err := ioutil.ReadFile(userExpPath)
	if err != nil {
		logger.Warning(err)
		return false
	}
	if len(content) == 0 {
		// 如果user的内容是空的，则默认为false，并将状态写回user
		return false
	}
	cfg := &SysCfg{}
	err = proto.Unmarshal(content, cfg)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return cfg.UserExp
}

func (e *EventLog) getUserExpStateFromDefaultPath() bool {
	const (
		expSection = "ExperiencePlan"
		expKey     = "ExperienceState"
	)
	kf := keyfile.NewKeyFile()
	err := kf.LoadFromFile(defaultExpPath)
	if err != nil {
		logger.Warning(err)
		return false
	} else {
		state, err := kf.GetBool(expSection, expKey)
		if err != nil {
			logger.Warning(err)
			return false
		}
		return state
	}
}

func (e *EventLog) setUserExpFileState(state bool) error {
	e.fileMu.Lock()
	defer e.fileMu.Unlock()
	if dutils.IsFileExist(expPathV0) {
		oldState := e.getV0ExpState()
		if oldState == state {
			return nil
		}
	}
	var content []byte
	if state {
		content = []byte(enableUserExp)
	} else {
		content = []byte(disableUserExp)
	}
	err := ioutil.WriteFile(expPathV0, content, 0666)
	if err != nil {
		return err
	}
	_ = os.Chmod(expPathV0, 0666)
	return nil
}

func (e *EventLog) getV0ExpState() bool {
	content, err := ioutil.ReadFile(expPathV0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return string(content) == enableUserExp
}

func (e *EventLog) getUserExpStateFromVarTmpExpPath() bool {
	e.fileMu.Lock()
	defer e.fileMu.Unlock()
	content, err := ioutil.ReadFile(e.currentUserVarTmpExpPath)
	if err != nil {
		logger.Warning(err)
		return false
	}
	if len(content) == 0 {
		return false
	}

	return string(content) == enableUserExp
}
