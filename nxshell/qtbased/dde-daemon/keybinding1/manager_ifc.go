// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	"github.com/linuxdeepin/dde-daemon/keybinding1/util"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	dbusServiceName = "org.deepin.dde.Keybinding1"
	dbusPath        = "/org/deepin/dde/Keybinding1"
	dbusInterface   = "org.deepin.dde.Keybinding1"
)

type ErrInvalidShortcutType struct {
	Type int32
}

func (err ErrInvalidShortcutType) Error() string {
	return fmt.Sprintf("shortcut type %v is invalid", err.Type)
}

type ErrShortcutNotFound struct {
	Id   string
	Type int32
}

func (err ErrShortcutNotFound) Error() string {
	return fmt.Sprintf("shortcut id %q type %v is not found", err.Id, err.Type)
}

var errTypeAssertionFail = errors.New("type assertion failed")
var errShortcutKeystrokesUnmodifiable = errors.New("keystrokes of this shortcut is unmodifiable")
var errKeystrokeUsed = errors.New("keystroke had been used")
var errNameUsed = errors.New("name had been used")

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

// true : ignore
func (m *Manager) isIgnoreRepeat(name string) bool {
	const minKeyEventInterval = 200 * time.Millisecond
	now := time.Now()
	duration := now.Sub(m.lastMethodCalledTime)
	logger.Debug("duration:", duration)
	if 0 < duration && duration < minKeyEventInterval {
		logger.Debug(">>Ignore ", name)
		return true
	}
	m.lastMethodCalledTime = now
	return false
}

func (m *Manager) setAccelForWayland(gsettings *gio.Settings, wmObj wm.Wm) {
	for _, id := range gsettings.ListKeys() {
		var accelJson string
		var err error
		if id == "screenshot-window" {
			accelJson = `{"Id":"screenshot-window","Accels":["SysReq"]}` //+ Alt+print对应kwin识别的键SysReq
		} else if id == "launcher" {
			accelJson = `{"Id":"launcher","Accels":["Super_L"]}` // wayland左右super对应的都是Super_L
		} else if id == "system_monitor" {
			accelJson = `{"Id":"system_monitor","Accels":["<Crtl><Alt>Escape"]}`
		} else {
			accelJson, err = util.MarshalJSON(util.KWinAccel{
				Id:         id,
				Keystrokes: gsettings.GetStrv(id),
			})
			if err != nil {
				logger.Warning("failed to get json:", err)
				continue
			}
		}

		ok, err := wmObj.SetAccel(0, accelJson)
		if !ok {
			logger.Warning("failed to set KWin accels:", id, gsettings.GetStrv(id), err)
		}
	}
}

// Reset reset all shortcut
func (m *Manager) Reset() *dbus.Error {
	if m.isIgnoreRepeat("Reset") {
		return nil
	}

	customShortcuts := m.customShortcutManager.List()
	m.shortcutManager.UngrabAll()

	m.enableListenGSettingsChanged(false)
	// reset all gsettings
	resetGSettings(m.gsSystem)
	resetGSettings(m.gsMediaKey)
	if m.gsGnomeWM != nil {
		resetGSettings(m.gsGnomeWM)
	}
	if _useWayland {
		m.setAccelForWayland(m.gsSystem, m.wm)
		m.setAccelForWayland(m.gsMediaKey, m.wm)
		if m.gsGnomeWM != nil {
			m.setAccelForWayland(m.gsGnomeWM, m.wm)
		}
	}

	// reset for KWin
	if shouldUseDDEKwin() {
		err := resetKWin(m.wm)
		if err != nil {
			logger.Warning("failed to reset for KWin:", err)
		}
		// 由于快捷键冲突原因，有必要重置两遍
		err = resetKWin(m.wm)
		if err != nil {
			logger.Warning("failed to reset for KWin:", err)
		}
	}

	changes := m.shortcutManager.ReloadAllShortcutsKeystrokes()
	m.enableListenGSettingsChanged(true)
	m.shortcutManager.GrabAll()

	for _, cs := range customShortcuts {
		keystrokes := cs.GetKeystrokes()
		var newKeystrokes []*shortcuts.Keystroke
		modifyFlag := false
		for _, keystroke := range keystrokes {
			conflictKeystroke, err := m.shortcutManager.FindConflictingKeystroke(keystroke)
			if err != nil {
				logger.Warning(err)
				modifyFlag = true
				continue
			}
			if conflictKeystroke != nil {
				logger.Debugf("keystroke %v has conflict", keystroke)
				modifyFlag = true
			} else {
				newKeystrokes = append(newKeystrokes, keystroke)
			}
		}

		cs0 := m.shortcutManager.GetByUid(cs.GetUid())
		if cs0 == nil {
			logger.Warning("cs0 is nil")
			continue
		}

		m.shortcutManager.ModifyShortcutKeystrokes(cs0, newKeystrokes)
		if modifyFlag {
			err := cs0.SaveKeystrokes()
			if err != nil {
				logger.Warning(err)
			}
			// 将修改的自定义快捷键补充到 changes 列表中
			changes = append(changes, cs0)
		}
	}

	for _, shortcut := range changes {
		m.emitShortcutSignal(shortcutSignalChanged, shortcut)
	}
	return nil
}

func (m *Manager) ListAllShortcuts() (shortcuts string, busErr *dbus.Error) {
	list := m.shortcutManager.List()
	ret, err := util.MarshalJSON(list)
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	return ret, nil
}

func (m *Manager) EnableSystemShortcut(shortcuts []string, enabled bool, isPersistent bool) *dbus.Error {
	for _, shortcut := range shortcuts {
		if enabled {
			m.DisabledSystemShortcutsList, _ = m.DisabledSystemShortcutsList.Delete(shortcut)
		} else {
			m.DisabledSystemShortcutsList, _ = m.DisabledSystemShortcutsList.Add(shortcut)
		}
	}
	logger.Debug("DisabledSystemShortcutsList:", m.DisabledSystemShortcutsList)
	return nil
}

func (m *Manager) ListShortcutsByType(type0 int32) (shortcuts string, busErr *dbus.Error) {
	list := m.shortcutManager.ListByType(type0)
	ret, err := util.MarshalJSON(list)
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	return ret, nil
}

func (m *Manager) AddCustomShortcut(name, action, keystroke string) (id string,
	type0 int32, busErr *dbus.Error) {

	logger.Debugf("Add custom key: %q %q %q", name, action, keystroke)
	ks, err := shortcuts.ParseKeystroke(keystroke)
	if err != nil {
		logger.Warning(err)
		busErr = dbusutil.ToError(err)
		return
	}

	exist := m.shortcutManager.GetByIdType(name, shortcuts.ShortcutTypeCustom)
	if exist != nil {
		err = errNameUsed
		logger.Warning(err)
		busErr = dbusutil.ToError(err)
		return
	}

	conflictKeystroke, err := m.shortcutManager.FindConflictingKeystroke(ks)
	if err != nil {
		logger.Warning(err)
		busErr = dbusutil.ToError(err)
		return
	}
	if conflictKeystroke != nil {
		err = errKeystrokeUsed
		logger.Warning(err)
		busErr = dbusutil.ToError(err)
		return
	}

	shortcut, err := m.customShortcutManager.Add(name, action, []*shortcuts.Keystroke{ks}, m.wm)
	if err != nil {
		logger.Warning(err)
		busErr = dbusutil.ToError(err)
		return
	}
	if _useWayland {
		err := m.processWaylandCustomShortcut(name, action, keystroke)
		if err != nil {
			logger.Warning(err)
			busErr = dbusutil.ToError(err)
			return
		}
	}
	m.shortcutManager.Add(shortcut)
	m.emitShortcutSignal(shortcutSignalAdded, shortcut)
	id = shortcut.GetId()
	type0 = shortcut.GetType()
	return
}

func (m *Manager) DeleteCustomShortcut(id string) *dbus.Error {
	logger.Debug("DeleteCustomShortcut", id)
	shortcut := m.shortcutManager.GetByIdType(id, shortcuts.ShortcutTypeCustom)
	if err := m.customShortcutManager.Delete(shortcut.GetId()); err != nil {
		return dbusutil.ToError(err)
	}
	m.shortcutManager.Delete(shortcut)
	if _useWayland {
		id += "-cs"
		logger.Debug("RemoveAccel id: ", id)
		err := m.wm.RemoveAccel(0, id)
		if err != nil {
			return dbusutil.ToError(errors.New("RemoveAccel failed, id: " + id))
		}
		delete(m.shortcutManager.WaylandCustomShortCutMap, id)
	}
	m.emitShortcutSignal(shortcutSignalDeleted, shortcut)
	return nil
}

func (m *Manager) ClearShortcutKeystrokes(id string, type0 int32) *dbus.Error {
	logger.Debug("ClearShortcutKeystrokes", id, type0)
	shortcut := m.shortcutManager.GetByIdType(id, type0)
	if shortcut == nil {
		return dbusutil.ToError(ErrShortcutNotFound{id, type0})
	}
	m.shortcutManager.ModifyShortcutKeystrokes(shortcut, nil)
	err := shortcut.SaveKeystrokes()
	if err != nil {
		return dbusutil.ToError(err)
	}
	if shortcut.ShouldEmitSignalChanged() {
		m.emitShortcutSignal(shortcutSignalChanged, shortcut)
	}
	return nil
}

func (m *Manager) LookupConflictingShortcut(keystroke string) (shortcut string, busErr *dbus.Error) {
	ks, err := shortcuts.ParseKeystroke(keystroke)
	if err != nil {
		// parse keystroke error
		return "", dbusutil.ToError(err)
	}

	conflictKeystroke, err := m.shortcutManager.FindConflictingKeystroke(ks)
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	if conflictKeystroke != nil {
		detail, err := util.MarshalJSON(conflictKeystroke.Shortcut)
		if err != nil {
			return "", dbusutil.ToError(err)
		}
		return detail, nil
	}
	return "", nil
}

func (m *Manager) processWaylandCustomShortcut(id, cmd, keystroke string) *dbus.Error {
	logger.Debugf("WaylandCustomShortcut id: %q, cmd: %q, keystroke: %q", id, cmd, keystroke)
	wlname := id + "-cs"
	keystrokeStrv := make([]string, 0)
	keystrokeStrv = append(keystrokeStrv, keystroke)
	accelJson, err := util.MarshalJSON(util.KWinAccel{
		Id:         wlname,
		Keystrokes: keystrokeStrv,
	})
	if err != nil {
		logger.Warning("accelJson failed: ", accelJson)
		return dbusutil.ToError(err)
	}
	logger.Debug("SetAccel: ", wlname, keystrokeStrv)
	ok, err := m.wm.SetAccel(0, accelJson)
	if !ok {
		logger.Warning("SetAccel failed, accelJson: ", accelJson)
		return dbusutil.ToError(err)
	}
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning("sessionBus create failed", err)
		return dbusutil.ToError(err)
	}
	obj := sessionBus.Object("org.kde.kglobalaccel", "/kglobalaccel")
	err = obj.Call("org.kde.KGlobalAccel.setActiveByUniqueName", 0, wlname, true).Err
	logger.Debug("setActiveByUniqueName: ", wlname)
	if err != nil {
		logger.Warning("setActiveByUniqueName failed")
		return dbusutil.ToError(err)
	}
	logger.Debug("WaylandCustomShortCutMap set", wlname)
	m.shortcutManager.WaylandCustomShortCutMap[wlname] = cmd
	return nil
}

// ModifyCustomShortcut modify custom shortcut
//
// id: shortcut id
// name: new name
// cmd: new commandline
// keystroke: new keystroke
func (m *Manager) ModifyCustomShortcut(id, name, cmd, keystroke string) *dbus.Error {
	logger.Debugf("ModifyCustomShortcut id: %q, name: %q, cmd: %q, keystroke: %q", id, name, cmd, keystroke)
	const ty = shortcuts.ShortcutTypeCustom
	// get the shortcut
	shortcut := m.shortcutManager.GetByIdType(id, ty)
	if shortcut == nil {
		return dbusutil.ToError(ErrShortcutNotFound{id, ty})
	}
	customShortcut, ok := shortcut.(*shortcuts.CustomShortcut)
	if !ok {
		return dbusutil.ToError(errTypeAssertionFail)
	}

	var keystrokes []*shortcuts.Keystroke
	if keystroke != "" {
		ks, err := shortcuts.ParseKeystroke(keystroke)
		if err != nil {
			return dbusutil.ToError(err)
		}
		// check conflicting
		conflictKeystroke, err := m.shortcutManager.FindConflictingKeystroke(ks)
		if err != nil {
			return dbusutil.ToError(err)
		}
		if conflictKeystroke != nil && conflictKeystroke.Shortcut != shortcut {
			return dbusutil.ToError(errKeystrokeUsed)
		}
		keystrokes = []*shortcuts.Keystroke{ks}
	}

	if _useWayland {
		err := m.processWaylandCustomShortcut(id, cmd, keystroke)
		if err != nil {
			return err
		}
	}

	// modify then save
	customShortcut.SetName(name)
	customShortcut.Cmd = cmd
	m.shortcutManager.ModifyShortcutKeystrokes(shortcut, keystrokes)
	err := customShortcut.Save()
	if err != nil {
		return dbusutil.ToError(err)
	}
	m.emitShortcutSignal(shortcutSignalChanged, shortcut)
	return nil
}

func (m *Manager) AddShortcutKeystroke(id string, type0 int32, keystroke string) *dbus.Error {
	logger.Debug("AddShortcutKeystroke", id, type0, keystroke)
	shortcut := m.shortcutManager.GetByIdType(id, type0)
	if shortcut == nil {
		return dbusutil.ToError(ErrShortcutNotFound{id, type0})
	}
	if !shortcut.GetKeystrokesModifiable() {
		return dbusutil.ToError(errShortcutKeystrokesUnmodifiable)
	}

	ks, err := shortcuts.ParseKeystroke(keystroke)
	if err != nil {
		// parse keystroke error
		return dbusutil.ToError(err)
	}
	logger.Debug("keystroke:", ks.DebugString())

	if type0 == shortcuts.ShortcutTypeWM && ks.Mods == 0 {
		keyLower := strings.ToLower(ks.Keystr)
		if keyLower == "super_l" || keyLower == "super_r" {
			return dbusutil.ToError(errors.New(
				"keystroke of shortcut which type is wm can not be set to the Super key"))
		}
	}

	conflictKeystroke, err := m.shortcutManager.FindConflictingKeystroke(ks)
	if err != nil {
		return dbusutil.ToError(err)
	}
	if conflictKeystroke == nil {
		m.shortcutManager.AddShortcutKeystroke(shortcut, ks)
		err := shortcut.SaveKeystrokes()
		if err != nil {
			return dbusutil.ToError(err)
		}
		if shortcut.ShouldEmitSignalChanged() {
			m.emitShortcutSignal(shortcutSignalChanged, shortcut)
		}
	} else if conflictKeystroke.Shortcut != shortcut {
		return dbusutil.ToError(errKeystrokeUsed)
	}

	return nil
}

func (m *Manager) DeleteShortcutKeystroke(id string, type0 int32, keystroke string) *dbus.Error {
	logger.Debug("DeleteShortcutKeystroke", id, type0, keystroke)
	shortcut := m.shortcutManager.GetByIdType(id, type0)
	if shortcut == nil {
		return dbusutil.ToError(ErrShortcutNotFound{id, type0})
	}
	if !shortcut.GetKeystrokesModifiable() {
		return dbusutil.ToError(errShortcutKeystrokesUnmodifiable)
	}

	ks, err := shortcuts.ParseKeystroke(keystroke)
	if err != nil {
		// parse keystroke error
		return dbusutil.ToError(err)
	}
	logger.Debug("keystroke:", ks.DebugString())

	m.shortcutManager.DeleteShortcutKeystroke(shortcut, ks)
	err = shortcut.SaveKeystrokes()
	if err != nil {
		return dbusutil.ToError(err)
	}
	if shortcut.ShouldEmitSignalChanged() {
		m.emitShortcutSignal(shortcutSignalChanged, shortcut)
	}
	return nil
}

func (m *Manager) GetShortcut(id string, type0 int32) (shortcut string, busErr *dbus.Error) {
	s := m.shortcutManager.GetByIdType(id, type0)
	if s == nil {
		return "", dbusutil.ToError(ErrShortcutNotFound{id, type0})
	}
	detail, err := s.Marshal()
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	return detail, nil
}

func (m *Manager) SelectKeystroke() *dbus.Error {
	logger.Debug("SelectKeystroke")
	err := m.selectKeystroke()
	return dbusutil.ToError(err)
}

func (m *Manager) SetNumLockState(state int32) *dbus.Error {
	logger.Debug("SetNumLockState", state)
	if _useWayland {
		err := setNumLockWl(m.waylandOutputMgr, m.conn, NumLockState(state))
		m.handleKeyEventByWayland("numlock")
		return dbusutil.ToError(err)
	}

	return dbusutil.ToError(setNumLockX11(m.conn, m.keySymbols, NumLockState(state)))
}

func (m *Manager) SearchShortcuts(query string) (shortcuts string, busErr *dbus.Error) {
	list := m.shortcutManager.Search(query)
	ret, err := util.MarshalJSON(list)
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	return ret, nil
}

func (m *Manager) GetCapsLockState() (state int32, busErr *dbus.Error) {
	lockState, err := queryCapsLockState(m.conn)
	return int32(lockState), dbusutil.ToError(err)
}

func (m *Manager) SetCapsLockState(state int32) *dbus.Error {
	logger.Debug("SetCapsLockState", state)
	err := setCapsLockState(m.conn, m.keySymbols, CapsLockState(state))
	return dbusutil.ToError(err)
}
