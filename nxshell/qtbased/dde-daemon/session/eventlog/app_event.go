// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"encoding/json"
	"errors"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	dock "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.dde.daemon.dock"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

type TidTyp int

const (
	AppOpenTid  TidTyp = 1000000000
	AppCloseTid TidTyp = 1000000001
)

type AppEntry struct {
	Tid     TidTyp `json:"tid"`
	Time    int64  `json:"time"`
	Id      string `json:"id"`
	PkgName string `json:"pkgName"`
	AppName string `json:"appName"`
}

type appEventCollector struct {
	service         *dbusutil.Service
	dockObj         dock.Dock
	sessionSigLoop  *dbusutil.SignalLoop
	lock            sync.Mutex
	items           map[string]AppEntry
	writeEventLogFn writeEventLogFunc
}

func init() {
	register("app", newAppEventCollector())
}

func newAppEventCollector() *appEventCollector {
	return &appEventCollector{
		items: make(map[string]AppEntry),
	}
}

func (c *appEventCollector) Init(service *dbusutil.Service, fn writeEventLogFunc) error {
	logger.Info("app event collector init")
	if service == nil || fn == nil {
		return errors.New("failed to init appEventCollector: error args")
	}
	c.service = service
	c.writeEventLogFn = fn
	c.dockObj = dock.NewDock(service.Conn())
	c.sessionSigLoop = dbusutil.NewSignalLoop(service.Conn(), 10)
	c.dockObj.InitSignalExt(c.sessionSigLoop, true)
	c.sessionSigLoop.Start()
	return nil
}

func (c *appEventCollector) Collect() error {
	if c.dockObj == nil {
		return errors.New("dock init failed")
	}
	// 获取所有驻留的Entry，对每个Entry进行监听
	entryObjMap := make(map[dbus.ObjectPath]dock.Entry)
	var entryObjMapMu sync.Mutex
	insertEntryObjMap := func(entryPath dbus.ObjectPath, entryObj dock.Entry) {
		entryObjMapMu.Lock()
		entryObjMap[entryPath] = entryObj
		entryObjMapMu.Unlock()
	}
	removeEntryObjMap := func(entryPath dbus.ObjectPath) {
		entryObjMapMu.Lock()
		delete(entryObjMap, entryPath)
		entryObjMapMu.Unlock()
	}
	getEntryMapObj := func(entryPath dbus.ObjectPath) dock.Entry {
		entryObjMapMu.Lock()
		defer entryObjMapMu.Unlock()
		return entryObjMap[entryPath]
	}
	entries, err := c.dockObj.Entries().Get(0)
	if err != nil {
		logger.Warningf("failed to get entry, err: %v", err)
	} else {
		logger.Debug("app data get dock entries success, start monitor exist entry")
		// monitor all entry
		for _, entryPath := range entries {
			entryObj, err := dock.NewEntry(c.service.Conn(), entryPath)
			if err != nil {
				logger.Warningf("new entry failed, err: %v", err)
				continue
			}
			insertEntryObjMap(entryPath, entryObj)
			go c.monitor(entryObj)
		}
	}
	_, err = c.dockObj.ConnectEntryAdded(func(path dbus.ObjectPath, index int32) {
		// Entry 新增时，需要判断 Entry 内容，再进行处理，如果存在窗口，则会addEntry并发送打开
		entryObj, err := dock.NewEntry(c.service.Conn(), path)
		if err != nil {
			logger.Warningf("new entry failed, err: %v", err)
			return
		}
		insertEntryObjMap(path, entryObj)
		go c.monitor(entryObj)
	})
	if err != nil {
		logger.Debugf("monitor signal: entry add failed, err: %v", err)
	}
	_, err = c.dockObj.ConnectEntryRemoved(func(entryId string) {
		// Entry 移除时，需要判断是否addEntry过，如果该 Entry有记录，证明是打开的应用，可以发送关闭
		entry := c.getEntry(entryId)
		if entry.AppName == "" {
			return
		}

		if !c.removeEntry(entryId) {
			return
		}
		entryPath := dbus.ObjectPath(filepath.Join("/com/deepin/dde/daemon/Dock/entries/", entryId))
		entryObj := getEntryMapObj(entryPath)
		if entryObj != nil {
			entryObj.RemoveAllHandlers()
			removeEntryObjMap(entryPath)
		}
		entry.Tid = AppCloseTid
		_ = c.writeAppEventLog(entry)
	})
	if err != nil {
		logger.Warningf("monitor entry remove failed, err: %v", err)
	}
	return nil
}

func (c *appEventCollector) Stop() error {
	if c.sessionSigLoop != nil {
		c.sessionSigLoop.Stop()
	}
	if c.dockObj != nil {
		c.dockObj.RemoveAllHandlers()
	}
	return nil
}

func (c *appEventCollector) getEntry(entryId string) AppEntry {
	// lock
	c.lock.Lock()
	defer c.lock.Unlock()
	return c.items[entryId]
}

// 对Entry进行处理和监听
func (c *appEventCollector) monitor(entryObj dock.Entry) {
	logger.Debug("monitor:", entryObj.Path_())
	var entry AppEntry
	var getPackageNameOnce sync.Once
	// get app name from entry obj
	name, err := entryObj.Name().Get(0)
	if err != nil {
		logger.Warningf("get app name failed, err: %v", err)
	}
	entry.AppName = name
	logger.Debugf("app name is %v", name)
	var packageName string
	getPackageName := func() string {
		getPackageNameOnce.Do(func() {
			// get desktop file from entry obj,
			// desktop file wont be changed, so do not care about the file
			desktop, err := entryObj.DesktopFile().Get(0)
			if err != nil {
				logger.Warningf("get desktop file failed, err: %v", err)
				return
			}
			logger.Debugf("app desktop file is %v", desktop)
			_, err = os.Stat(desktop)
			if err != nil {
				logger.Warningf("desktop file cant found, desktop: %v", desktop)
				return
			}
			if isSymlink(desktop) {
				desktop, err = os.Readlink(desktop)
				if err != nil {
					logger.Warning(err)
				}
				logger.Debugf("desktop file is link file, real path is %v", desktop)
			}
			// run dpkg to get package name
			dpkg := []string{"dpkg", "-S", desktop}
			cmd := exec.Command("/bin/bash", "-c", strings.Join(dpkg, " "))
			logger.Debugf("dpkg command is %v", cmd)
			// run command to get package
			buf, err := cmd.Output()
			if err != nil {
				// it is ok if has no package
				logger.Debugf("app has no package name, app: %v reason: %v", name, err)
			} else {
				// parse return
				cmdRet := strings.Trim(string(buf), " ")
				infoSlice := strings.Split(cmdRet, ":")
				if len(infoSlice) < 2 {
					logger.Warningf("open app pkg invalid, pkg: %v", infoSlice)
					return
				}
				// save package
				packageName = infoSlice[0]
				return
			}
			return
		})
		return packageName
	}
	// get id from dbus path,
	//such as /com/deepin/dde/daemon/Dock/entries/e0T61978f3b
	entryId := filepath.Base(string(entryObj.Path_()))
	entry.Id = entryId
	entryObj.InitSignalExt(c.sessionSigLoop, true)
	// monitor entry active state change
	err = entryObj.WindowInfos().ConnectChanged(func(hasValue bool, value map[uint32]dock.WindowInfo) {
		// check if value is valid
		if !hasValue {
			return
		}
		entry.PkgName = getPackageName()
		// TODO: this code can use state machine to optimize
		// if now entry is active, should add this to map
		if len(value) != 0 {
			// 是否重复添加，第一次添加时才是应用打开
			if !c.addEntry(entryId, entry) {
				return
			}
			entry.Tid = AppOpenTid
		} else {
			// check if need post close app data
			if !c.removeEntry(entryId) {
				return
			}
			entry.Tid = AppCloseTid
		}
		_ = c.writeAppEventLog(entry)
	})
	if err != nil {
		return
	}
	// get now active state
	window, err := entryObj.WindowInfos().Get(0)
	if err != nil {
		return
	}
	if len(window) == 0 {
		logger.Debugf("app %v is docked, but not active", entry.AppName)
		return
	} else {
		entry.PkgName = getPackageName()
	}
	if !c.addEntry(entryId, entry) {
		return
	}
	// post open app data
	entry.Tid = AppOpenTid
	_ = c.writeAppEventLog(entry)
}

func (c *appEventCollector) addEntry(entryId string, entry AppEntry) bool {
	c.lock.Lock()
	defer c.lock.Unlock()
	// entry has already app, and entry is the same,
	// so entry has already exist, do not need to post data
	if item, ok := c.items[entryId]; ok && item.Id == entry.Id {
		return false
	}
	logger.Debugf("open app name %v", entry.AppName)
	// if entryId not exist, or entry is new obj, need post data
	c.items[entryId] = entry
	return true
}

func (c *appEventCollector) removeEntry(entryId string) bool {
	c.lock.Lock()
	defer c.lock.Unlock()
	// get entry if exist
	item, ok := c.items[entryId]
	if !ok {
		return false
	}
	logger.Debugf("close app %v", item.AppName)
	// delete entry from items
	delete(c.items, entryId)
	return true
}

// 判断文件 filename 是否是符号链接
func isSymlink(filename string) bool {
	fileInfo, err := os.Lstat(filename)
	if err != nil {
		return false
	}
	if fileInfo.Mode()&os.ModeSymlink != 0 {
		return true
	}
	return false
}

func (c *appEventCollector) writeAppEventLog(entry AppEntry) error {
	entry.Time = time.Now().Unix()
	content, err := json.Marshal(entry)
	if err != nil {
		return err
	}
	if c.writeEventLogFn != nil {
		c.writeEventLogFn(string(content))
	}

	return nil
}
