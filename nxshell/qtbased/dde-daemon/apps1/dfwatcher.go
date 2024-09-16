// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package apps1

import (
	"os"
	"path/filepath"
	"time"
	"unicode/utf8"

	"github.com/fsnotify/fsnotify"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

const (
	dfWatcherDBusInterface = dbusServiceName + ".DesktopFileWatcher"
)

// desktop file watcher
type DFWatcher struct {
	service   *dbusutil.Service
	fsWatcher *fsWatcher
	sem       chan int
	eventChan chan *FileEvent

	// nolint
	signals *struct {
		Event struct {
			name string
			op   uint32
		}
	}
}

func newDFWatcher(service *dbusutil.Service) (*DFWatcher, error) {
	w := new(DFWatcher)

	interval := 6 * time.Second
	if logger.GetLogLevel() == log.LevelDebug {
		interval = 3 * time.Second
	}
	fsWatcher, err := newFsWatcher(interval)
	if err != nil {
		return nil, err
	}
	fsWatcher.trySuccessCb = func(file string) {
		w.addRecursive(file, true)
	}
	w.fsWatcher = fsWatcher
	w.service = service
	w.sem = make(chan int, 4)
	w.eventChan = make(chan *FileEvent, 10)
	go w.listenEvents()
	return w, nil
}

func (*DFWatcher) GetInterfaceName() string {
	return dfWatcherDBusInterface
}

func (w *DFWatcher) listenEvents() {
	for {
		select {
		case ev, ok := <-w.fsWatcher.Events:
			if !ok {
				logger.Error("Invalid event:", ev)
				return
			}
			logger.Debug("event", ev)
			w.handleEvent(ev)

		case err := <-w.fsWatcher.Errors:
			logger.Warning("error", err)
			return
		}
	}
}

func (w *DFWatcher) handleEvent(event fsnotify.Event) {
	w.fsWatcher.handleEvent(event)
	ev := NewFileEvent(event)
	file := ev.Name

	if !utf8.ValidString(file) {
		logger.Warningf("ignore file %q event, invalid utf8 string", file)
		return
	}

	if (ev.Op&fsnotify.Create != 0 || ev.Op&fsnotify.Rename != 0) && ev.IsDir {
		// it exist and is dir
		w.addRecursive(file, true)
		return
	}
	w.notifyEvent(ev)
}

func (w *DFWatcher) notifyEvent(ev *FileEvent) {
	logger.Debugf("notifyEvent %q", ev.Name)
	err := w.service.Emit(w, "Event", ev.Name, uint32(0))
	if err != nil {
		logger.Warning(err)
	}
	w.eventChan <- ev
}

func (w *DFWatcher) add(path string) error {
	logger.Debug("DFWatcher.add", path)
	return w.fsWatcher.Watcher.Add(path)
}

func (w *DFWatcher) remove(path string) error {
	logger.Debug("DFWatcher.remove", path)
	return w.fsWatcher.Watcher.Remove(path)
}

func (w *DFWatcher) addRecursive(path string, loadExisted bool) {
	logger.Debug("DFWatcher.addRecursive", path, loadExisted)
	err := filepath.Walk(path, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			logger.Warning(err)
			return nil
		}
		if info.IsDir() {
			logger.Debug("DFWatcher.addRecursive watch", path)
			err := w.add(path)
			if err != nil {
				logger.Warning(err)
			}
		} else if loadExisted {
			if isDesktopFile(path) {
				w.notifyEvent(NewFileFoundEvent(path))
			}
		}
		return nil
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (w *DFWatcher) removeRecursive(path string) {
	logger.Debug("DFWatcher.removeRecursive", path)
	err := filepath.Walk(path,
		func(path string, info os.FileInfo, err error) error {
			if err != nil {
				// ignore file not exist error
				if !os.IsNotExist(err) {
					logger.Warning(err)
				}
				return nil
			}

			if info.IsDir() {
				err := w.remove(path)
				if err != nil {
					logger.Warning(err)
				}
			}
			return nil
		})
	if err != nil {
		logger.Warning(err)
	}
}
