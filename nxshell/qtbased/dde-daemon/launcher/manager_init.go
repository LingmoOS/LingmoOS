// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"path/filepath"
	"unicode/utf8"

	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

func (m *Manager) initItems() {
	// load items
	m.items = make(map[string]*Item)

	skipDirs := make(map[string][]string)
	skipDirs["/usr/share/applications"] = []string{"screensavers"}
	userAppsDir := getUserAppDir()
	skipDirs[userAppsDir] = []string{"menu-xdg"}
	allApps := desktopappinfo.GetAll(skipDirs)
	for _, ai := range allApps {
		if !ai.IsExecutableOk() ||
			!utf8.ValidString(ai.GetId()) ||
			isDeepinCustomDesktopFile(ai.GetFileName()) {
			continue
		}
		item := NewItemWithDesktopAppInfo(ai)
		m.setItemID(item)

		if m.hiddenByGSettings(item.ID) {
			continue
		}
		m.addItem(item)
	}
	logger.Debug("load items count:", len(m.items))
}

func shouldCheckDesktopFile(filename string) bool {
	dir, basename := filepath.Split(filename)
	dir = filepath.Clean(dir)
	matched, _ := filepath.Match(desktopFilePattern, basename)
	if !matched {
		return false
	}

	// ignore $HOME/.local/share/applications/menu-xdg/
	skipDir := filepath.Join(getUserAppDir(), "menu-xdg")
	return dir != skipDir
}

type popPushOp struct {
	popCount  int
	runesPush []rune
}

func (m *Manager) handlePopPushOps() {
	stack := m.searchTaskStack
	for op := range m.popPushOpChan {
		logger.Debug("op:", op)

		for i := 0; i < op.popCount; i++ {
			stack.Pop()
		}
		if len(op.runesPush) == 0 {
			// emit top result
			top := stack.topTask()
			if top == nil {
				logger.Debug("emit SearchDone []")
				err := m.service.Emit(m, "SearchDone", []string{})
				if err != nil {
					logger.Warning("emit SearchDone Failed:", err)
				}
			} else {
				top.emitResult()
			}
		}
		for _, v := range op.runesPush {
			stack.Push(v)
		}
	}
}

func (m *Manager) destroy() {
	m.appsObj.RemoveHandler(proxy.RemoveAllHandlers)
	m.syncConfig.Destroy()
	m.sysSigLoop.Stop()
	m.sessionSigLoop.Stop()
}
