// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package apps1

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/fsnotify/fsnotify"
	dbus "github.com/godbus/dbus/v5"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	dbusServiceName         = "org.deepin.dde.Apps1"
	dbusPath                = "/org/deepin/dde/Apps1"
	alRecorderDBusInterface = dbusServiceName + ".LaunchedRecorder"
	minUid                  = 1000
)

// app launched recorder
type ALRecorder struct {
	watcher *DFWatcher
	// key is SubRecorder.root
	subRecorders      map[string]*SubRecorder
	subRecordersMutex sync.RWMutex
	loginManager      login1.Manager

	// nolint
	signals *struct {
		Launched struct {
			file string
		}

		StatusSaved struct {
			root string
			file string
			ok   bool
		}

		ServiceRestarted struct{}
	}
}

func newALRecorder(watcher *DFWatcher) (*ALRecorder, error) {
	r := &ALRecorder{
		watcher:      watcher,
		subRecorders: make(map[string]*SubRecorder),
	}
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	r.loginManager = login1.NewManager(systemBus)

	go r.listenEvents()

	sysDataDirs := getSystemDataDirs()
	for _, dataDir := range sysDataDirs {
		r.watchAppsDir(0, "", filepath.Join(dataDir, "applications"))
	}

	sysSigLoop := dbusutil.NewSignalLoop(systemBus, 10)
	sysSigLoop.Start()
	r.loginManager.InitSignalExt(sysSigLoop, true)
	_, err = r.loginManager.ConnectUserRemoved(func(uid uint32, userPath dbus.ObjectPath) {
		r.handleUserRemoved(int(uid))
	})
	if err != nil {
		logger.Warning(err)
	}

	return r, nil
}

func (*ALRecorder) GetInterfaceName() string {
	return alRecorderDBusInterface
}

func (r *ALRecorder) Service() *dbusutil.Service {
	return r.watcher.service
}

func (r *ALRecorder) emitLaunched(file string) {
	err := r.Service().Emit(r, "Launched", file)
	if err != nil {
		logger.Warning(err)
	}
}

func (r *ALRecorder) emitStatusSaved(root, file string, ok bool) {
	err := r.Service().Emit(r, "StatusSaved", root, file, ok)
	if err != nil {
		logger.Warning(err)
	}
}

func (r *ALRecorder) emitServiceRestarted() {
	err := r.Service().Emit(r, "ServiceRestarted")
	if err != nil {
		logger.Warning(err)
	}
}

func (r *ALRecorder) listenEvents() {
	eventChan := r.watcher.eventChan
	for {
		ev := <-eventChan
		logger.Debugf("ALRecorder ev: %#v", ev)
		name := ev.Name

		if isDesktopFile(name) {
			if ev.IsFound || (ev.Op&fsnotify.Create != 0) || (ev.Op&fsnotify.Write != 0) {
				// added
				r.handleAdded(name)
			} else if ev.NotExist {
				// removed
				r.handleRemoved(name)
			}
		} else if ev.NotExist && (ev.Op&fsnotify.Rename != 0) {
			// may be dir removed
			r.handleDirRemoved(name)
		}
	}
}

func (r *ALRecorder) UninstallHints(desktopFiles []string) *dbus.Error {
	logger.Infof("dbus call UninstallHints desktopFiles %v", desktopFiles)

	for _, filename := range desktopFiles {
		r.uninstallHint(filename)
	}
	return nil
}

func (r *ALRecorder) uninstallHint(file string) {
	r.subRecordersMutex.RLock()
	defer r.subRecordersMutex.RUnlock()

	for _, sr := range r.subRecorders {
		if strings.HasPrefix(file, sr.root) {
			rel, _ := filepath.Rel(sr.root, file)
			sr.uninstallHint(removeDesktopExt(rel))
			return
		}
	}
}

// handleAdded handle desktop file added
func (r *ALRecorder) handleAdded(file string) {
	r.subRecordersMutex.RLock()
	defer r.subRecordersMutex.RUnlock()

	for _, sr := range r.subRecorders {
		if strings.HasPrefix(file, sr.root) {
			rel, _ := filepath.Rel(sr.root, file)
			sr.handleAdded(removeDesktopExt(rel))
			return
		}
	}
}

// handleRemoved: handle desktop file removed
func (r *ALRecorder) handleRemoved(file string) {
	r.subRecordersMutex.RLock()
	defer r.subRecordersMutex.RUnlock()

	for _, sr := range r.subRecorders {
		if strings.HasPrefix(file, sr.root) {
			rel, _ := filepath.Rel(sr.root, file)
			sr.handleRemoved(removeDesktopExt(rel))
			return
		}
	}
}

func (r *ALRecorder) handleDirRemoved(file string) {
	r.subRecordersMutex.RLock()
	defer r.subRecordersMutex.RUnlock()

	for _, sr := range r.subRecorders {
		if strings.HasPrefix(file, sr.root) {
			rel, _ := filepath.Rel(sr.root, file)
			sr.handleDirRemoved(rel)
			return
		}
	}
}

func (r *ALRecorder) MarkLaunched(file string) *dbus.Error {
	logger.Debugf("dbus call MarkLaunched file: %q", file)
	r.subRecordersMutex.RLock()
	defer r.subRecordersMutex.RUnlock()

	for _, sr := range r.subRecorders {
		if strings.HasPrefix(file, sr.root) {
			rel, _ := filepath.Rel(sr.root, file)
			if sr.MarkLaunched(removeDesktopExt(rel)) {
				r.emitLaunched(file)
			}
			return nil
		}
	}
	logger.Debug("MarkLaunched failed")
	return nil
}

func (r *ALRecorder) GetNew(sender dbus.Sender) (newApps map[string][]string, busErr *dbus.Error) {
	logger.Infof("dbus call GetNew sender %v", sender)

	uid, err := r.Service().GetConnUID(string(sender))
	if err != nil {
		logger.Warning(err)
		return nil, dbusutil.ToError(err)
	}

	ret := make(map[string][]string)
	r.subRecordersMutex.RLock()

	for _, sr := range r.subRecorders {
		if intSliceContains(sr.uids, int(uid)) {
			newApps := sr.GetNew()
			if len(newApps) > 0 {
				ret[sr.root] = newApps
			}
		}
	}
	r.subRecordersMutex.RUnlock()

	return ret, nil
}

func (r *ALRecorder) watchAppsDir(uid int, home, appsDir string) {
	r.subRecordersMutex.Lock()
	defer r.subRecordersMutex.Unlock()

	sr := r.subRecorders[appsDir]
	if sr != nil {
		// subRecorder exists
		logger.Debugf("subRecorder for %q exists", appsDir)
		if !intSliceContains(sr.uids, uid) {
			sr.uids = append(sr.uids, uid)
			logger.Debug("append uid", uid)
		}
		return
	}

	sr = NewSubRecorder(uid, home, appsDir, r)
	r.subRecorders[appsDir] = sr
}

// 判断filename是否是符号链接
func isSymlink(filename string) bool {
	fileInfo, err := os.Lstat(filename)
	if err != nil {
		return false
	}
	return fileInfo.Mode()&os.ModeSymlink != 0
}

// 获取path所有子path
func getPathDirs(filename string) (ret []string) {
	for {
		ret = append(ret, filename)
		parentDir := filepath.Dir(filename)
		if parentDir == filename {
			break
		}
		filename = parentDir
	}
	return ret
}

func (r *ALRecorder) WatchDirs(sender dbus.Sender, dataDirs []string) *dbus.Error {
	logger.Infof("dbus call WatchDirs  sender %v, dataDirs %v", sender, dataDirs)

	uid, err := r.Service().GetConnUID(string(sender))
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	logger.Debugf("WatchDirs uid: %d, data dirs: %#v", uid, dataDirs)
	// check uid
	if uid < minUid {
		err = errors.New("invalid uid")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	// check dataDirs
	for _, dataDir := range dataDirs {
		if !filepath.IsAbs(dataDir) {
			err = fmt.Errorf("%q is not absolute path", dataDir)
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
		if isSymlink(dataDir) {
			err = fmt.Errorf("%q is a symbolic link", dataDir)
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}

	// get home dir
	home, err := getHomeByUid(int(uid))
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	for _, dataDir := range dataDirs {
		appsDir := filepath.Join(dataDir, "applications")
		r.watchAppsDir(int(uid), home, appsDir)
	}
	return nil
}

func (r *ALRecorder) handleUserRemoved(uid int) {
	logger.Debug("handleUserRemoved uid:", uid)
	r.subRecordersMutex.Lock()

	for _, sr := range r.subRecorders {
		logger.Debug(sr.root, sr.uids)
		sr.uids = intSliceRemove(sr.uids, uid)
		if len(sr.uids) == 0 {
			sr.Destroy()
			delete(r.subRecorders, sr.root)
		}
	}

	r.subRecordersMutex.Unlock()
	logger.Debug("r.subRecorders:", r.subRecorders)
}
