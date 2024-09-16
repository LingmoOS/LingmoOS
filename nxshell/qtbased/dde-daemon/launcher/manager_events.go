// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"os"
	"path/filepath"
	"sync/atomic"
	"time"

	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/gsettings"
)

const (
	desktopFilePattern = `[^.]*.desktop`
)

func isDesktopFile(path string) bool {
	basename := filepath.Base(path)
	matched, _ := filepath.Match(desktopFilePattern, basename)
	return matched
}

//当允许或者禁止搜索包名时,设置SearchTarget中的包名字段
func (m *Manager) handlePackageNameSearchChanged() {
	enabled := m.settings.GetBoolean(gsKeyPackageNameSearch)
	logger.Debug("itemSearchTarget update, search package name enable: ", enabled)
	if enabled != m.packageNameSearchEnabled {
		if enabled {
			for _, item := range m.items {
				item.addSearchTarget(idScore, item.ID)
			}
		} else {
			for _, item := range m.items {
				item.deleteSearchTarget(item.ID)
			}
		}
	}
	m.packageNameSearchEnabled = enabled

}

func (m *Manager) handleAppHiddenChanged() {
	m.appsHiddenMu.Lock()
	defer m.appsHiddenMu.Unlock()

	newVal := m.settings.GetStrv(gsKeyAppsHidden)
	logger.Debug(gsKeyAppsHidden+" changed", newVal)

	added, removed := diffAppsHidden(m.appsHidden, newVal)
	logger.Debugf(gsKeyAppsHidden+" added: %v, removed: %v", added, removed)
	for _, appID := range added {
		// apps need to be hidden
		item := m.getItemById(appID)
		if item == nil {
			continue
		}

		m.removeItem(appID)
		m.emitItemChanged(item, AppStatusDeleted)
	}

	for _, appID := range removed {
		// apps need to be displayed
		item := m.getItemById(appID)
		if item != nil {
			continue
		}

		appInfo := desktopappinfo.NewDesktopAppInfo(appID)
		if appInfo == nil {
			continue
		}

		item = NewItemWithDesktopAppInfo(appInfo)
		m.setItemID(item)
		shouldShow := appInfo.ShouldShow() &&
			!isDeepinCustomDesktopFile(appInfo.GetFileName())

		if !shouldShow {
			continue
		}

		m.addItemWithLock(item)
		m.emitItemChanged(item, AppStatusCreated)
	}
	m.appsHidden = newVal
}

func (m *Manager) listenSettingsChanged() {
	gsettings.ConnectChanged(gsSchemaLauncher, "*", func(key string) {
		switch key {
		case gsKeyAppsHidden:
			m.handleAppHiddenChanged()
		case gsKeyPackageNameSearch:
			m.handlePackageNameSearchChanged()
		}
	})
}

func diffAppsHidden(old, new []string) (added, removed []string) {
	if len(old) == 0 {
		return new, nil
	}
	if len(new) == 0 {
		return nil, old
	}

	oldMap := strSliceToMap(old)
	newMap := strSliceToMap(new)
	return diffMapKeys(oldMap, newMap)
}

func strSliceToMap(slice []string) map[string]struct{} {
	result := make(map[string]struct{}, len(slice))
	for _, value := range slice {
		result[value] = struct{}{}
	}
	return result
}

func diffMapKeys(oldMap, newMap map[string]struct{}) (added, removed []string) {
	for oldKey := range oldMap {
		if _, ok := newMap[oldKey]; !ok {
			// key found in oldMap, but not in newMap => removed
			removed = append(removed, oldKey)
		}
	}

	for newKey := range newMap {
		if _, ok := oldMap[newKey]; !ok {
			// key found in newMap, but not in oldMap => added
			added = append(added, newKey)
		}
	}

	return
}

func (m *Manager) handleFsWatcherEvents() {
	watcher := m.fsWatcher
	for {
		select {
		case ev, ok := <-watcher.Events:
			if !ok {
				logger.Error("Invalid watcher event:", ev)
				return
			}

			logger.Debugf("fsWatcher event: %v", ev)
			m.delayHandleFileEvent(ev.Name)
		case err := <-watcher.Errors:
			logger.Warning("fsWatcher error", err)
			return
		}
	}
}

func (m *Manager) delayHandleFileEvent(name string) {
	m.fsEventTimersMutex.Lock()
	defer m.fsEventTimersMutex.Unlock()

	delay := 2000 * time.Millisecond
	timer, ok := m.fsEventTimers[name]
	if ok {
		timer.Stop()
		timer.Reset(delay)
	} else {
		m.fsEventTimers[name] = time.AfterFunc(delay, func() {
			switch {
			case name == desktopPkgMapFile:
				if err := m.loadDesktopPkgMap(); err != nil {
					logger.Warning(err)
					return
				}
				// retry queryPkgName for m.noPkgItemIDs
				logger.Debugf("m.noPkgItemIDs: %v", m.noPkgItemIDs)
				for id := range m.noPkgItemIDs {
					pkg, err := m.queryPkgName(id, "")
					if err != nil {
						logger.Warning(err)
					}
					logger.Debugf("item id %q pkg %q", id, pkg)

					if pkg != "" {
						item := m.getItemById(id)
						cid := m._queryCategoryID(item, pkg)

						if cid != item.CategoryID {
							// item.CategoryID changed
							item.CategoryID = cid
							m.emitItemChanged(item, AppStatusModified)
						}
						delete(m.noPkgItemIDs, id)
					}
				}

			case name == applicationsFile:
				if err := m.loadPkgCategoryMap(); err != nil {
					logger.Warning(err)
				}

			case isDesktopFile(name):
				m.checkDesktopFile(name)
			}
		})
	}
}

func (m *Manager) checkDesktopFile(file string) {
	logger.Debug("checkDesktopFile", file)
	appId := m.getAppIdByFilePath(file)
	logger.Debugf("app id %q", appId)
	if appId == "" {
		logger.Warningf("appId is empty, ignore file %q", file)
		return
	}

	item := m.getItemById(appId)

	appInfo, err := desktopappinfo.NewDesktopAppInfoFromFile(file)
	if err != nil {
		logger.Debug("call NewDesktopAppInfoFromFile failed", err)
	}
	if appInfo == nil {
		logger.Warningf("appId %q appInfo is nil", appId)
		if item != nil {
			m.removeItem(appId)
			m.emitItemChanged(item, AppStatusDeleted)

			// remove desktop file in user's desktop direcotry
			os.Remove(appInDesktop(appId))
		}
	} else {
		// appInfo is not nil
		newItem := NewItemWithDesktopAppInfo(appInfo)
		m.setItemID(newItem)
		shouldShow := appInfo.ShouldShow() &&
			!isDeepinCustomDesktopFile(appInfo.GetFileName()) &&
			!m.hiddenByGSettingsWithLock(newItem.ID)

		// add or update item
		if item != nil {

			if shouldShow {
				// update item
				m.addItemWithLock(newItem)
				m.emitItemChanged(newItem, AppStatusModified)
			} else {
				m.removeItem(appId)
				m.emitItemChanged(newItem, AppStatusDeleted)
			}
		} else {
			if shouldShow {
				if appInfo.IsExecutableOk() {
					m.addItemWithLock(newItem)
					m.emitItemChanged(newItem, AppStatusCreated)
				} else {
					go m.retryAddItem(appInfo, newItem)
				}
			}
		}
	}
}

func (m *Manager) retryAddItem(appInfo *desktopappinfo.DesktopAppInfo, item *Item) {
	for i := 0; i < 10; i++ {
		time.Sleep(time.Second)
		logger.Debug("retry add item", item.Path)
		if appInfo.IsExecutableOk() {
			m.addItemWithLock(item)
			m.emitItemChanged(item, AppStatusCreated)
			return
		}
	}
}

func (m *Manager) emitItemChanged(item *Item, status string) {
	atomic.StoreUint32(&m.itemsChangedHit, 1)
	itemInfo := item.newItemInfo()
	logger.Debugf("emit signal ItemChanged status: %v, itemInfo: %v", status, itemInfo)
	err := m.service.Emit(m, "ItemChanged", status, itemInfo, itemInfo.CategoryID)
	if err != nil {
		logger.Warning("emit emitItemChanged Failed", err)
	}
}
