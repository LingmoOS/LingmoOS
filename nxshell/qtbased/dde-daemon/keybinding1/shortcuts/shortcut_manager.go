// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"
	"sync"
	"time"
	"unicode"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/keybinding1/util"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	daemon "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.daemon1"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/keyfile"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/strv"
	dutils "github.com/linuxdeepin/go-lib/utils"
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/record"
	"github.com/linuxdeepin/go-x11-client/util/keybind"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
	"github.com/linuxdeepin/go-x11-client/util/wm/ewmh"
)

var logger *log.Logger

func GetQtKeycodeMap() map[string]string {
	var qtKeycodMap = map[string]string{
		"messenger":           "Qt::Key_Messenger",             // XF86Messenger
		"save":                "Qt::Key_Save",                  // XF86Save
		"new":                 "Qt::Key_New",                   // XF86New
		"wake-up":             "Qt::Key_WakeUp",                // XF86WakeUp
		"audio-rewind":        "Qt::Key_AudioRewind",           // XF86AudioRewind
		"audio-mute":          "Qt::Key_VolumeMute",            // XF86AudioMute
		"mon-brightness-up":   "Qt::Key_MonBrightnessUp",       // XF86MonBrightnessUp
		"wlan":                "Qt::Key_WLAN",                  // XF86WLAN
		"audio-media":         "Qt::Key_AudioMedia",            // XF86AudioMedia
		"reply":               "Qt::Key_Reply",                 // XF86Reply
		"favorites":           "Qt::Key_Favorites",             // XF86Favorites
		"audio-play":          "Qt::Key_MediaPlay",             // XF86AudioPlay
		"audio-mic-mute":      "Qt::Key_MicMute",               // XF86AudioMicMute
		"audio-pause":         "Qt::Key_MediaPause",            // XF86AudioPause
		"audio-stop":          "Qt::Key_AudioStop",             // XF86AudioStop
		"documents":           "Qt::Key_Documents",             // XF86Documents
		"game":                "Qt::Key_Game",                  // XF86Game
		"search":              "<Super><Shift>Touchpad Toggle", // XF86Search
		"audio-record":        "Qt::Key_AudioRecord",           // XF86AudioRecord
		"display":             "Qt::Key_Display",               // XF86Display
		"reload":              "Qt::Key_Reload",                // XF86Reload
		"explorer":            "Qt::Key_Explorer",              // XF86Explorer
		"calculator":          "Qt::Key_Launch1",               // XF86Calculator
		"calendar":            "Qt::Key_Calendar",              // XF86Calendar
		"forward":             "Qt::Key_Forward",               // XF86Forward
		"cut":                 "Qt::Key_Cut",                   // XF86Cut
		"mon-brightness-down": "Qt::Key_MonBrightnessDown",     // XF86MonBrightnessDown
		"copy":                "Qt::Key_Copy",                  // XF86Copy
		"tools":               "Qt::Key_Tools",                 // XF86Tools
		"audio-raise-volume":  "Qt::Key_VolumeUp",              // XF86AudioRaiseVolume
		"media-close":         "Qt::Key_Close",                 // XF86Close
		"www":                 "Qt::Key_WWW",                   // XF86WWW
		"home-page":           "Qt::Key_HomePage",              // XF86HomePage
		"sleep":               "Qt::Key_Sleep",                 // XF86Sleep
		"audio-lower-volume":  "Qt::Key_VolumeDown",            // XF86AudioLowerVolume
		"audio-prev":          "Qt::Key_MediaPrevious",         // XF86AudioPrev
		"audio-next":          "Qt::Key_MediaNext",             // XF86AudioNext
		"paste":               "Qt::Key_Paste",                 // XF86Paste
		"open":                "Qt::Key_Open",                  // XF86Open
		"send":                "Qt::Key_Send",                  // XF86Send
		"my-computer":         "Qt::Key_MyComputer",            // XF86MyComputer
		"mail":                "Qt::Key_MailForward",           // XF86Mail
		"adjust-brightness":   "Qt::Key_BrightnessAdjust",      // XF86BrightnessAdjust
		"log-off":             "Qt::Key_LogOff",                // XF86LogOff
		"pictures":            "Qt::Key_Pictures",              // XF86Pictures
		"terminal":            "Qt::Key_Terminal",              // XF86Terminal
		"video":               "Qt::Key_Video",                 // XF86Video
		"music":               "Qt::Key_Music",                 // XF86Music
		"app-left":            "Qt::Key_ApplicationLeft",       // XF86ApplicationLeft
		"app-right":           "Qt::Key_ApplicationRight",      // XF86ApplicationRight
		"meeting":             "Qt::Key_Meeting",               // XF86Meeting
	}
	return qtKeycodMap
}

const (
	SKLCtrlShift uint32 = 1 << iota
	SKLAltShift
	SKLSuperSpace
)

const (
	versionFile = "/etc/deepin-version"
)

func SetLogger(l *log.Logger) {
	logger = l
}

var _useWayland bool

func setUseWayland(value bool) {
	_useWayland = value
}

type KeyEventFunc func(ev *KeyEvent)

type ShortcutManager struct {
	conn         *x.Conn
	dataConn     *x.Conn // conn for receive record event
	daemonDaemon daemon.Daemon

	idShortcutMap     map[string]Shortcut
	idShortcutMapMu   sync.Mutex
	keyKeystrokeMap   map[Key]*Keystroke
	keyKeystrokeMapMu sync.Mutex
	keySymbols        *keysyms.KeySymbols

	recordEnable        bool
	recordEnableMu      sync.Mutex
	recordContext       record.Context
	xRecordEventHandler *XRecordEventHandler
	eventCb             KeyEventFunc
	eventCbMu           sync.Mutex
	layoutChanged       chan struct{}
	pinyinEnabled       bool

	ConflictingKeystrokes []*Keystroke
	EliminateConflictDone bool

	WaylandCustomShortCutMap map[string]string
}

type KeyEvent struct {
	Mods     Modifiers
	Code     Keycode
	Shortcut Shortcut
}

func NewShortcutManager(conn *x.Conn, keySymbols *keysyms.KeySymbols, eventCb KeyEventFunc) *ShortcutManager {
	setUseWayland(strings.Contains(os.Getenv("XDG_SESSION_TYPE"), "wayland"))
	ss := &ShortcutManager{
		idShortcutMap:            make(map[string]Shortcut),
		eventCb:                  eventCb,
		conn:                     conn,
		keySymbols:               keySymbols,
		recordEnable:             true,
		keyKeystrokeMap:          make(map[Key]*Keystroke),
		layoutChanged:            make(chan struct{}),
		pinyinEnabled:            isZH(),
		WaylandCustomShortCutMap: make(map[string]string),
	}

	ss.xRecordEventHandler = NewXRecordEventHandler(keySymbols)
	ss.xRecordEventHandler.modKeyReleasedCb = func(code uint8, mods uint16) {
		isGrabbed := isKbdAlreadyGrabbed(ss.conn)
		switch mods {
		case keysyms.ModMaskCapsLock, keysyms.ModMaskSuper:
			// caps_lock, supper
			if isGrabbed {
				return
			}
			ss.emitKeyEvent(0, Key{Code: Keycode(code)})

		case keysyms.ModMaskNumLock:
			// num_lock
			ss.emitKeyEvent(0, Key{Code: Keycode(code)})

		case keysyms.ModMaskControl | keysyms.ModMaskShift:
			// ctrl-shift
			if isGrabbed {
				return
			}
			ss.emitFakeKeyEvent(&Action{Type: ActionTypeSwitchKbdLayout, Arg: SKLCtrlShift})

		case keysyms.ModMaskAlt | keysyms.ModMaskShift:
			// alt-shift
			if isGrabbed {
				return
			}
			ss.emitFakeKeyEvent(&Action{Type: ActionTypeSwitchKbdLayout, Arg: SKLAltShift})
		}
	}
	// init record
	err := ss.initRecord()
	if err == nil {
		logger.Debug("start record event loop")
		go ss.recordEventLoop()
	} else {
		logger.Warning("init record failed: ", err)
	}

	err = ss.initSysDaemon()
	if err != nil {
		logger.Warning("init system D-BUS failed: ", err)
	}

	return ss
}

func (sm *ShortcutManager) recordEventLoop() {
	// enable context
	cookie := record.EnableContext(sm.dataConn, sm.recordContext)

	for {
		reply, err := cookie.Reply(sm.dataConn)
		if err != nil {
			logger.Warning(err)
			return
		}
		if !sm.isRecordEnabled() {
			logger.Debug("record disabled!")
			continue
		}

		if reply.ClientSwapped {
			logger.Warning("reply.ClientSwapped is true")
			continue
		}
		if len(reply.Data) == 0 {
			continue
		}

		ge := x.GenericEvent(reply.Data)

		switch ge.GetEventCode() {
		case x.KeyPressEventCode:
			event, _ := x.NewKeyPressEvent(ge)
			//logger.Debug(event)
			sm.handleXRecordKeyEvent(true, uint8(event.Detail), event.State)

		case x.KeyReleaseEventCode:
			event, _ := x.NewKeyReleaseEvent(ge)
			//logger.Debug(event)
			sm.handleXRecordKeyEvent(false, uint8(event.Detail), event.State)

		case x.ButtonPressEventCode:
			//event, _ := x.NewButtonPressEvent(ge)
			//logger.Debug(event)
			sm.handleXRecordButtonEvent(true)
		case x.ButtonReleaseEventCode:
			//event, _ := x.NewButtonReleaseEvent(ge)
			//logger.Debug(event)
			sm.handleXRecordButtonEvent(false)
		default:
			logger.Debug(ge)
		}

	}
}

func (sm *ShortcutManager) initRecord() error {
	ctrlConn := sm.conn
	dataConn, err := x.NewConn()
	if err != nil {
		return err
	}

	_, err = record.QueryVersion(ctrlConn, record.MajorVersion, record.MinorVersion).Reply(ctrlConn)
	if err != nil {
		return err
	}
	_, err = record.QueryVersion(dataConn, record.MajorVersion, record.MinorVersion).Reply(dataConn)
	if err != nil {
		return err
	}

	xid, err := ctrlConn.AllocID()
	if err != nil {
		return err
	}
	ctx := record.Context(xid)
	logger.Debug("record context id:", ctx)

	// create context
	clientSpec := []record.ClientSpec{record.ClientSpec(record.CSAllClients)}
	ranges := []record.Range{
		{
			DeviceEvents: record.Range8{
				First: x.KeyPressEventCode,
				Last:  x.ButtonReleaseEventCode,
			},
		},
	}

	err = record.CreateContextChecked(ctrlConn, ctx, record.ElementHeader(0),
		clientSpec, ranges).Check(ctrlConn)
	if err != nil {
		return err
	}

	sm.recordContext = ctx
	sm.dataConn = dataConn
	return nil
}

func (sm *ShortcutManager) EnableRecord(val bool) {
	sm.recordEnableMu.Lock()
	sm.recordEnable = val
	sm.recordEnableMu.Unlock()
}

func (sm *ShortcutManager) isRecordEnabled() bool {
	sm.recordEnableMu.Lock()
	ret := sm.recordEnable
	sm.recordEnableMu.Unlock()
	return ret
}

func (sm *ShortcutManager) NotifyLayoutChanged() {
	sm.layoutChanged <- struct{}{}
}

func (sm *ShortcutManager) Destroy() {
	// TODO
}

func (sm *ShortcutManager) List() (list []Shortcut) {
	sm.idShortcutMapMu.Lock()
	defer sm.idShortcutMapMu.Unlock()

	for _, shortcut := range sm.idShortcutMap {
		list = append(list, shortcut)
	}
	return
}

func (sm *ShortcutManager) ListByType(type0 int32) (list []Shortcut) {
	sm.idShortcutMapMu.Lock()
	defer sm.idShortcutMapMu.Unlock()

	for _, shortcut := range sm.idShortcutMap {
		if type0 == shortcut.GetType() {
			list = append(list, shortcut)
		}
	}
	return
}

func Contains(str1 string, str2 string) bool {
	return strings.Contains(strings.ToLower(str1), strings.ToLower(str2))
}

func (sm *ShortcutManager) Search(query string) (list []Shortcut) {
	sm.idShortcutMapMu.Lock()
	defer sm.idShortcutMapMu.Unlock()

	for _, shortcut := range sm.idShortcutMap {
		if sm.matchShortcut(shortcut, query) {
			list = append(list, shortcut)
		}
	}
	return list
}

func (sm *ShortcutManager) matchShortcut(shortcut Shortcut, query string) bool {
	name := strings.TrimRightFunc(shortcut.GetName(), unicode.IsSpace)
	if Contains(name, query) {
		return true
	}

	if sm.pinyinEnabled {
		nameBlocks := shortcut.GetNameBlocks()
		if nameBlocks.Match(query) {
			return true
		}
	}

	keystrokes := shortcut.GetKeystrokes()
	for _, keystroke := range keystrokes {
		if Contains(keystroke.searchString(), query) {
			return true
		}
	}

	return false
}

func (sm *ShortcutManager) storeConflictingKeystroke(ks *Keystroke) {
	sm.ConflictingKeystrokes = append(sm.ConflictingKeystrokes, ks)
}

func (sm *ShortcutManager) grabKeystroke(shortcut Shortcut, ks *Keystroke, dummy bool) {
	keyList, err := ks.ToKeyList(sm.keySymbols)
	if err != nil {
		logger.Debugf("grabKeystroke failed, shortcut: %v, ks: %v, err: %v", shortcut.GetId(), ks, err)
		return
	}

	var conflictCount int
	var idx = -1
	for i, key := range keyList {
		sm.keyKeystrokeMapMu.Lock()
		conflictKeystroke, ok := sm.keyKeystrokeMap[key]
		sm.keyKeystrokeMapMu.Unlock()

		logger.Debugf("grabKeystroke shortcut: %s, ks: %s, key: %s, dummy: %v", shortcut.GetId(), ks, key, dummy)

		if ok {
			// conflict
			if conflictKeystroke.Shortcut != nil {
				conflictCount++
				logger.Debugf("key %v is grabbed by %v", key, conflictKeystroke.Shortcut.GetId())
			} else {
				logger.Warningf("key %v is grabbed, conflictKeystroke.Shortcut is nil", key)
			}
			continue
		}

		// no conflict
		if !dummy {
			err = key.Grab(sm.conn)
			if err != nil {
				logger.Debug(err)
				// Rollback
				idx = i
				break
			}
		}
		sm.keyKeystrokeMapMu.Lock()
		sm.keyKeystrokeMap[key] = ks
		sm.keyKeystrokeMapMu.Unlock()
	}

	// Rollback
	if idx != -1 {
		for i := 0; i <= idx; i++ {
			keyList[i].Ungrab(sm.conn)
		}
	}

	// Delete completely conflicting key
	if conflictCount == len(keyList) && !sm.EliminateConflictDone {
		sm.storeConflictingKeystroke(ks)
	}
}

func (sm *ShortcutManager) ungrabKeystroke(ks *Keystroke, dummy bool) {
	keyList, err := ks.ToKeyList(sm.keySymbols)
	if err != nil {
		logger.Debug(err)
		return
	}
	if len(keyList) == 0 {
		return
	}

	sm.keyKeystrokeMapMu.Lock()
	defer sm.keyKeystrokeMapMu.Unlock()
	for _, key := range keyList {
		delete(sm.keyKeystrokeMap, key)
		if !dummy {
			key.Ungrab(sm.conn)
		}
	}
}

func (sm *ShortcutManager) grabShortcut(shortcut Shortcut) {
	if _useWayland && shortcut.GetId() == "launcher" {
		return
	}
	//logger.Debug("grabShortcut shortcut id:", shortcut.GetId())
	for _, ks := range shortcut.GetKeystrokes() {
		dummy := dummyGrab(shortcut, ks)
		sm.grabKeystroke(shortcut, ks, dummy)
		ks.Shortcut = shortcut
	}
}

func (sm *ShortcutManager) ungrabShortcut(shortcut Shortcut) {

	for _, ks := range shortcut.GetKeystrokes() {
		dummy := dummyGrab(shortcut, ks)
		sm.ungrabKeystroke(ks, dummy)
		ks.Shortcut = nil
	}
}

func (sm *ShortcutManager) ModifyShortcutKeystrokes(shortcut Shortcut, newVal []*Keystroke) {
	logger.Debug("ShortcutManager.ModifyShortcutKeystrokes", shortcut, newVal)
	sm.ungrabShortcut(shortcut)
	shortcut.setKeystrokes(newVal)
	sm.grabShortcut(shortcut)
}

func (sm *ShortcutManager) AddShortcutKeystroke(shortcut Shortcut, ks *Keystroke) {
	logger.Debug("ShortcutManager.AddShortcutKeystroke", shortcut, ks.DebugString())
	oldVal := shortcut.GetKeystrokes()
	notExist := true
	for _, ks0 := range oldVal {
		if ks.Equal(sm.keySymbols, ks0) {
			notExist = false
			break
		}
	}
	if notExist {
		shortcut.setKeystrokes(append(oldVal, ks))
		logger.Debug("shortcut.Keystrokes append", ks.DebugString())

		// grab keystroke
		dummy := dummyGrab(shortcut, ks)
		sm.grabKeystroke(shortcut, ks, dummy)
	}
	ks.Shortcut = shortcut
}

func (sm *ShortcutManager) DeleteShortcutKeystroke(shortcut Shortcut, ks *Keystroke) {
	logger.Debug("ShortcutManager.DeleteShortcutKeystroke", shortcut, ks.DebugString())
	oldVal := shortcut.GetKeystrokes()
	var newVal []*Keystroke
	for _, ks0 := range oldVal {
		// Leaving unequal values
		if !ks.Equal(sm.keySymbols, ks0) {
			newVal = append(newVal, ks0)
		}
	}
	shortcut.setKeystrokes(newVal)
	logger.Debugf("shortcut.Keystrokes  %v -> %v", oldVal, newVal)

	// ungrab keystroke
	dummy := dummyGrab(shortcut, ks)
	sm.ungrabKeystroke(ks, dummy)
	ks.Shortcut = nil
}

func dummyGrab(shortcut Shortcut, ks *Keystroke) bool {
	if shortcut.GetType() == ShortcutTypeWM {
		return true
	}

	switch strings.ToLower(ks.Keystr) {
	case "super_l", "super_r", "caps_lock", "num_lock":
		return true
	}
	return false
}

func (sm *ShortcutManager) UngrabAll() {
	sm.keyKeystrokeMapMu.Lock()
	// ungrab all grabbed keys
	for key, keystroke := range sm.keyKeystrokeMap {
		dummy := dummyGrab(keystroke.Shortcut, keystroke)
		if !dummy {
			key.Ungrab(sm.conn)
		}
	}
	// new map
	count := len(sm.keyKeystrokeMap)
	sm.keyKeystrokeMap = make(map[Key]*Keystroke, count)
	sm.keyKeystrokeMapMu.Unlock()
}

func (sm *ShortcutManager) GrabAll() {
	sm.idShortcutMapMu.Lock()
	defer sm.idShortcutMapMu.Unlock()

	// re-grab all shortcuts
	for _, shortcut := range sm.idShortcutMap {
		sm.grabShortcut(shortcut)
	}
}

func (sm *ShortcutManager) regrabAll() {
	logger.Debug("regrabAll")
	sm.UngrabAll()
	sm.GrabAll()
}

func (sm *ShortcutManager) ReloadAllShortcutsKeystrokes() []Shortcut {
	sm.idShortcutMapMu.Lock()
	defer sm.idShortcutMapMu.Unlock()

	var changes []Shortcut
	for _, shortcut := range sm.idShortcutMap {
		changed := shortcut.ReloadKeystrokes()
		if changed {
			changes = append(changes, shortcut)
		}
	}
	return changes
}

// shift, control, alt(mod1), super(mod4)
func getConcernedMods(state uint16) uint16 {
	var mods uint16
	if state&keysyms.ModMaskShift > 0 {
		mods |= keysyms.ModMaskShift
	}
	if state&keysyms.ModMaskControl > 0 {
		mods |= keysyms.ModMaskControl
	}
	if state&keysyms.ModMaskAlt > 0 {
		mods |= keysyms.ModMaskAlt
	}
	if state&keysyms.ModMaskSuper > 0 {
		mods |= keysyms.ModMaskSuper
	}
	return mods
}

func GetConcernedModifiers(state uint16) Modifiers {
	return Modifiers(getConcernedMods(state))
}

func combineStateCode2Key(state uint16, code uint8) Key {
	mods := GetConcernedModifiers(state)
	key := Key{
		Mods: mods,
		Code: Keycode(code),
	}
	return key
}

func (sm *ShortcutManager) callEventCallback(ev *KeyEvent) {
	sm.eventCbMu.Lock()
	sm.eventCb(ev)
	sm.eventCbMu.Unlock()
}

func (sm *ShortcutManager) handleKeyEvent(pressed bool, detail x.Keycode, state uint16) {
	key := combineStateCode2Key(state, uint8(detail))
	logger.Debug("event key:", key)

	if pressed {
		// key press
		sm.emitKeyEvent(Modifiers(state), key)
	}
}

func (sm *ShortcutManager) emitFakeKeyEvent(action *Action) {
	keyEvent := &KeyEvent{
		Shortcut: NewFakeShortcut(action),
	}
	sm.callEventCallback(keyEvent)
}

func (sm *ShortcutManager) emitKeyEvent(mods Modifiers, key Key) {
	sm.keyKeystrokeMapMu.Lock()
	keystroke, ok := sm.keyKeystrokeMap[key]
	sm.keyKeystrokeMapMu.Unlock()
	if ok {
		logger.Debugf("emitKeyEvent keystroke: %#v", keystroke)
		keyEvent := &KeyEvent{
			Mods:     mods,
			Code:     key.Code,
			Shortcut: keystroke.Shortcut,
		}

		sm.callEventCallback(keyEvent)
	} else {
		logger.Debug("keystroke not found")
	}
}

func isKbdAlreadyGrabbed(conn *x.Conn) bool {
	var grabWin x.Window

	rootWin := conn.GetDefaultScreen().Root
	if activeWin, _ := ewmh.GetActiveWindow(conn).Reply(conn); activeWin == 0 {
		grabWin = rootWin
	} else {
		// check viewable
		attrs, err := x.GetWindowAttributes(conn, activeWin).Reply(conn)
		if err != nil {
			grabWin = rootWin
		} else if attrs.MapState != x.MapStateViewable {
			// err is nil and activeWin is not viewable
			grabWin = rootWin
		} else {
			// err is nil, activeWin is viewable
			grabWin = activeWin
		}
	}

	err := keybind.GrabKeyboard(conn, grabWin)
	if err == nil {
		// grab keyboard successful
		err = keybind.UngrabKeyboard(conn)
		if err != nil {
			logger.Warning("ungrabKeyboard Failed:", err)
		}
		return false
	}

	logger.Warningf("grabKeyboard win %d failed: %v", grabWin, err)

	gkErr, ok := err.(keybind.GrabKeyboardError)
	if ok && gkErr.Status == x.GrabStatusAlreadyGrabbed {
		return true
	}
	return false
}

func (sm *ShortcutManager) SetAllModKeysReleasedCallback(cb func()) {
	sm.xRecordEventHandler.allModKeysReleasedCb = cb
}

// get Active Window pid
// 注意: 从D-BUS启动dde-system-daemon的时候x会取不到环境变量,只能把获取pid放到dde-session-daemon
func (sm *ShortcutManager) getActiveWindowPid() (uint32, error) {
	activeWin, err := ewmh.GetActiveWindow(sm.conn).Reply(sm.conn)
	if err != nil {
		logger.Warning(err)
		return 0, err
	}

	pid, err1 := ewmh.GetWMPid(sm.conn, activeWin).Reply(sm.conn)
	if err1 != nil {
		logger.Warning(err1)
		return 0, err1
	}

	return uint32(pid), nil
}

func (sm *ShortcutManager) isPidVirtualMachine(pid uint32) (bool, error) {
	ret, err := sm.daemonDaemon.IsPidVirtualMachine(0, pid)
	if err != nil {
		logger.Warning(err)
		return ret, err
	}

	return ret, nil
}

// 初始化go-dbus-factory system DBUS : org.deepin.dde.Daemon1
func (sm *ShortcutManager) initSysDaemon() error {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return err
	}
	sm.daemonDaemon = daemon.NewDaemon(sysBus)
	return nil
}

func (sm *ShortcutManager) getActiveWindowCmd() string {
	win, err := ewmh.GetActiveWindow(sm.conn).Reply(sm.conn)
	if err != nil {
		logger.Warning("Failed to get current active window:", err)
		return ""
	}
	pid, err := ewmh.GetWMPid(sm.conn, win).Reply(sm.conn)
	if err != nil {
		logger.Warning("Failed to get current window pid:", err)
		return ""
	}
	data, err := ioutil.ReadFile(fmt.Sprintf("/proc/%d/cmdline", pid))
	if err != nil {
		logger.Warning("Failed to read cmdline:", err)
		return ""
	}
	return string(data)
}

func (sm *ShortcutManager) handleXRecordKeyEvent(pressed bool, code uint8, state uint16) {
	sm.xRecordEventHandler.handleKeyEvent(pressed, code, state)

	if pressed {
		// Special handling screenshot* shortcuts
		key := combineStateCode2Key(state, code)
		sm.keyKeystrokeMapMu.Lock()
		keystroke, ok := sm.keyKeystrokeMap[key]
		sm.keyKeystrokeMapMu.Unlock()
		if ok {
			shortcut := keystroke.Shortcut
			if shortcut != nil && shortcut.GetType() == ShortcutTypeSystem &&
				(strings.HasPrefix(shortcut.GetId(), "screenshot") ||
					strings.HasPrefix(shortcut.GetId(), "deepin-screen-recorder")) &&
				!strings.Contains(sm.getActiveWindowCmd(), "/usr/bin/abrecovery") {
				keyEvent := &KeyEvent{
					Mods:     key.Mods,
					Code:     key.Code,
					Shortcut: shortcut,
				}
				logger.Debug("handleXRecordKeyEvent: emit key event for screenshot* shortcuts")
				sm.callEventCallback(keyEvent)
			}
			// 显示桌面快捷键是窗管控制，此处需要隐藏启动器
			if shortcut != nil && shortcut.GetType() == ShortcutTypeWM && shortcut.GetId() == "show-desktop" {
				go func() {
					cmd := "dbus-send --print-reply --dest=org.deepin.dde.Launcher1 /org/deepin/dde/Launcher1 org.deepin.dde.Launcher1.Hide"
					err := exec.Command("/bin/sh", "-c", cmd).Run()
					if err != nil {
						logger.Warning(err)
					}
				}()
			}
		}
	}
}

func (sm *ShortcutManager) handleXRecordButtonEvent(pressed bool) {
	sm.xRecordEventHandler.handleButtonEvent(pressed)
}

func (sm *ShortcutManager) EventLoop() {
	eventChan := make(chan x.GenericEvent, 500)
	sm.conn.AddEventChan(eventChan)
	for ev := range eventChan {
		switch ev.GetEventCode() {
		case x.KeyPressEventCode:
			event, _ := x.NewKeyPressEvent(ev)
			logger.Debug(event)
			sm.handleKeyEvent(true, event.Detail, event.State)
		case x.KeyReleaseEventCode:
			event, _ := x.NewKeyReleaseEvent(ev)
			logger.Debug(event)
			sm.handleKeyEvent(false, event.Detail, event.State)
		case x.MappingNotifyEventCode:
			event, _ := x.NewMappingNotifyEvent(ev)
			logger.Debug(event)
			if sm.keySymbols.RefreshKeyboardMapping(event) {
				go func() {
					select {
					case _, ok := <-sm.layoutChanged:
						if !ok {
							logger.Error("Invalid layout changed event")
							return
						}

						sm.regrabAll()
					case _, ok := <-time.After(3 * time.Second):
						if !ok {
							logger.Error("Invalid time event")
							return
						}

						logger.Debug("layout not changed")
					}
				}()
			}
		}
	}
}

func (sm *ShortcutManager) Add(shortcut Shortcut) {
	logger.Debug("add", shortcut)
	uid := shortcut.GetUid()

	sm.idShortcutMapMu.Lock()
	sm.idShortcutMap[uid] = shortcut
	sm.idShortcutMapMu.Unlock()

	sm.grabShortcut(shortcut)
}

func (sm *ShortcutManager) addWithoutLock(shortcut Shortcut) {
	logger.Debug("add", shortcut)
	uid := shortcut.GetUid()

	sm.idShortcutMap[uid] = shortcut

	sm.grabShortcut(shortcut)
}

func (sm *ShortcutManager) Delete(shortcut Shortcut) {
	uid := shortcut.GetUid()

	sm.idShortcutMapMu.Lock()
	delete(sm.idShortcutMap, uid)
	sm.idShortcutMapMu.Unlock()

	sm.ungrabShortcut(shortcut)
}

func (sm *ShortcutManager) GetByIdType(id string, type0 int32) Shortcut {
	uid := idType2Uid(id, type0)

	sm.idShortcutMapMu.Lock()
	shortcut := sm.idShortcutMap[uid]
	sm.idShortcutMapMu.Unlock()

	return shortcut
}

func (sm *ShortcutManager) GetByUid(uid string) Shortcut {
	sm.idShortcutMapMu.Lock()
	shortcut := sm.idShortcutMap[uid]
	sm.idShortcutMapMu.Unlock()
	return shortcut
}

// ret0: Conflicting keystroke
// ret1: error
func (sm *ShortcutManager) FindConflictingKeystroke(ks *Keystroke) (*Keystroke, error) {
	keyList, err := ks.ToKeyList(sm.keySymbols)
	if err != nil {
		return nil, err
	}
	if len(keyList) == 0 {
		return nil, nil
	}

	logger.Debug("ShortcutManager.FindConflictingKeystroke", ks.DebugString())
	logger.Debug("key list:", keyList)

	sm.keyKeystrokeMapMu.Lock()
	defer sm.keyKeystrokeMapMu.Unlock()
	var count = 0
	var ks1 *Keystroke
	for _, key := range keyList {
		tmp, ok := sm.keyKeystrokeMap[key]
		if !ok {
			continue
		}
		count++
		ks1 = tmp
	}

	if count == len(keyList) {
		return ks1, nil
	}
	return nil, nil
}

func systemType() string {
	kf := keyfile.NewKeyFile()
	err := kf.LoadFromFile("/etc/os-version")
	if err != nil {
		fmt.Println("load version file failed, err: ", err)
		return ""
	}
	typ, err := kf.GetString("Version", "EditionName")
	if err != nil {
		fmt.Println("get version type failed, err: ", err)
		return ""
	}

	return typ
}

func arr2set(in []string) map[string]bool {
	out := make(map[string]bool)
	for _, v := range in {
		out[v] = true
	}

	return out
}

func strvLower(in []string) []string {
	out := make([]string, len(in))
	for i, v := range in {
		out[i] = strings.ToLower(v)
	}
	return out
}

// 检测一个系统快捷键是否配置为可用，true可用，false不可用
func (sm *ShortcutManager) CheckSystem(gsPlatform, gsEnable *gio.Settings, id string) bool {
	platformSet := arr2set(gsPlatform.ListKeys())
	enableSet := arr2set(gsEnable.ListKeys())
	sysType := strings.ToLower(systemType())
	assistiveToolsShortcut := []string{
		"ai-assistant",
		"speech-to-text",
		"text-to-speech",
		"translation",
	}
	// 判断是否是支持的平台
	if platformSet[id] {
		plats := gsPlatform.GetStrv(id)

		platSet := arr2set(strvLower(plats))
		if !platSet["all"] && !platSet[sysType] {
			return false
		}
	}
	if sysType == "community" && strv.Strv(assistiveToolsShortcut).Contains(id) {
		return false
	}

	// 判断是否配置开启
	if enableSet[id] && !gsEnable.GetBoolean(id) {
		logger.Debugf("%s is disabled", id)
		return false
	}

	return true
}

func (sm *ShortcutManager) AddSystemById(gsettings *gio.Settings, wmObj wm.Wm, id string) {
	shortcut := sm.GetByIdType(id, ShortcutTypeSystem)
	if shortcut != nil {
		logger.Debugf("%s is exist", id)
		return
	}

	idNameMap := getSystemIdNameMap()
	name := idNameMap[id]
	if name == "" {
		name = id
	}

	cmd := getSystemActionCmd(id)
	if id == "terminal-quake" && strings.Contains(cmd, "deepin-terminal") {
		termPath, _ := exec.LookPath("deepin-terminal")
		if termPath == "" {
			return
		}
	}

	keystrokes := gsettings.GetStrv(id)
	gs := NewGSettingsShortcut(gsettings, wmObj, id, ShortcutTypeSystem, keystrokes, name)
	sysShortcut := &SystemShortcut{
		GSettingsShortcut: gs,
		arg: &ActionExecCmdArg{
			Cmd: cmd,
		},
	}
	sm.addWithoutLock(sysShortcut)
}

func (sm *ShortcutManager) DelSystemById(id string) {
	shortcut := sm.GetByIdType(id, ShortcutTypeSystem)
	if shortcut == nil {
		logger.Debugf("%s is not exist", id)
		return
	}
	sm.Delete(shortcut)
}

func (sm *ShortcutManager) AddSystem(gsettings, gsPlatform, gsEnable *gio.Settings, wmObj wm.Wm) {
	logger.Debug("AddSystem")
	allow, err := wmObj.CompositingAllowSwitch().Get(0)
	if err != nil {
		logger.Warning(err)
		allow = false
	}
	for _, id := range gsettings.ListKeys() {
		if id == "deepin-screen-recorder" || id == "wm-switcher" {
			if !allow && id == "wm-switcher" {
				logger.Debugf("com.deepin.wm.compositingAllowSwitch is false, filter %s", id)
				continue
			}

			if _useWayland {
				logger.Debugf("XDG_SESSION_TYPE is wayland, filter %s", id)
				continue
			}
		}

		if !sm.CheckSystem(gsPlatform, gsEnable, id) {
			continue
		}

		sm.AddSystemById(gsettings, wmObj, id)
	}
}

func (sm *ShortcutManager) AddWM(gsettings *gio.Settings, wmObj wm.Wm) {
	logger.Debug("AddWM")
	idNameMap := getWMIdNameMap()
	releaseType := getDeepinReleaseType()
	for _, id := range gsettings.ListKeys() {
		if releaseType == "Server" && strings.Contains(id, "workspace") {
			logger.Debugf("release type is server filter '%s'", id)
			continue
		}
		if id == "expose-all-windows" || id == "expose-windows" {
			logger.Debugf("'%s' is abandoned!", id)
			continue
		}
		name := idNameMap[id]
		if name == "" {
			name = id
		}
		keystrokes := gsettings.GetStrv(id)
		gs := NewGSettingsShortcut(gsettings, wmObj, id, ShortcutTypeWM, keystrokes, name)
		sm.addWithoutLock(gs)
	}
}

func (sm *ShortcutManager) AddMedia(gsettings *gio.Settings, wmObj wm.Wm) {
	logger.Debug("AddMedia")
	idNameMap := getMediaIdNameMap()
	for _, id := range gsettings.ListKeys() {
		if id == "media-close" {
			continue
		}
		name := idNameMap[id]
		if name == "" {
			name = id
		}
		keystrokes := gsettings.GetStrv(id)
		gs := NewGSettingsShortcut(gsettings, wmObj, id, ShortcutTypeMedia, keystrokes, name)
		mediaShortcut := &MediaShortcut{
			GSettingsShortcut: gs,
		}
		sm.addWithoutLock(mediaShortcut)
	}
}

func (sm *ShortcutManager) AddCustom(csm *CustomShortcutManager, wmObj wm.Wm) {
	csm.pinyinEnabled = sm.pinyinEnabled
	logger.Debug("AddCustom")
	if _useWayland {
		for _, shortcut := range csm.List() {
			id := shortcut.GetId()
			keystrokesStrv := shortcut.getKeystrokesStrv()
			action := shortcut.GetAction()
			var cmd string
			switch arg := action.Arg.(type) {
			case *ActionExecCmdArg:
				cmd = arg.Cmd
			case string:
				cmd = arg
			}
			if cmd == "" {
				logger.Warning(ErrTypeAssertionFail, id, action)
				continue
			}
			logger.Debugf("customshort: %+v", cmd)
			ok, err := setShortForWayland(shortcut, wmObj)
			if !ok {
				logger.Warning("failed to setShortForWayland:", err)
				continue
			}
			sm.WaylandCustomShortCutMap[id+"-cs"] = cmd
			cs := newCustomShort(id, id, cmd, keystrokesStrv, wmObj, csm)
			sm.addWithoutLock(cs)
		}
	} else {
		for _, shortcut := range csm.List() {
			sm.addWithoutLock(shortcut)
		}
	}
}

func setShortForWayland(shortcut Shortcut, wmObj wm.Wm) (bool, error) {
	id := shortcut.GetId()
	isCustom := shortcut.GetType() == ShortcutTypeCustom
	if isCustom {
		id += "-cs"
	}
	keystrokesStrv := shortcut.getKeystrokesStrv()
	logger.Debugf("Id: %+v, keystrokesStrv: %+v", id, keystrokesStrv)
	accelJson, err := util.MarshalJSON(util.KWinAccel{
		Id:         id,
		Keystrokes: keystrokesStrv,
	})
	if err != nil {
		return false, err
	}
	ok, err := wmObj.SetAccel(0, accelJson)
	if !ok {
		logger.Warning("failed to set KWin accels:", id, keystrokesStrv, err)
		return false, err
	}
	if isCustom {
		sessionBus, err := dbus.SessionBus()
		if err != nil {
			logger.Warning(err)
			return false, err
		}
		obj := sessionBus.Object("org.kde.kglobalaccel", "/kglobalaccel")
		err = obj.Call("org.kde.KGlobalAccel.setActiveByUniqueName", 0, id, true).Err
		if err != nil {
			logger.Warning(err)
			return false, err
		}
	}
	return true, nil
}

func (sm *ShortcutManager) AddSpecial() {
	idNameMap := getSpecialIdNameMap()

	// add SwitchKbdLayout <Super>space
	s0 := NewFakeShortcut(&Action{Type: ActionTypeSwitchKbdLayout, Arg: SKLSuperSpace})
	ks, err := ParseKeystroke("<Super>space")
	if err != nil {
		panic(err)
	}
	s0.Id = "switch-kbd-layout"
	s0.Name = idNameMap[s0.Id]
	s0.Keystrokes = []*Keystroke{ks}
	sm.addWithoutLock(s0)
}

func (sm *ShortcutManager) AddKWin(wmObj wm.Wm) {
	logger.Debug("AddKWin")
	accels, err := util.GetAllKWinAccels(wmObj)
	if err != nil {
		logger.Warning("failed to get all KWin accels:", err)
		return
	}

	idNameMap := getWMIdNameMap()
	releaseType := getDeepinReleaseType()

	for _, accel := range accels {
		if releaseType == "Server" && strings.Contains(accel.Id, "workspace") {
			logger.Debugf("release type is server filter '%s'", accel.Id)
			continue
		}
		if accel.Id == "expose-all-windows" || accel.Id == "expose-windows" {
			logger.Debugf("'%s' is abandoned!", accel.Id)
			continue
		}
		name := idNameMap[accel.Id]
		if name == "" {
			name = accel.Id
		}

		ks := newKWinShortcut(accel.Id, name, accel.Keystrokes, wmObj)
		sm.addWithoutLock(ks)
	}
}

func (sm *ShortcutManager) AddKWinForWayland(wmObj wm.Wm) {
	logger.Debug("AddKWinForWayland")
	accels, err := util.GetAllKWinAccels(wmObj)
	if err != nil {
		logger.Warning("failed to get all KWin accels:", err)
		return
	}
	idNameMap := getWMIdNameMap()
	for _, accel := range accels {
		if accel.Id == "color-picker" || accel.Id == "switch-kbd-layout" ||
			accel.Id == "expose-all-windows" || accel.Id == "expose-windows" {
			continue
		}
		if getSystemIdNameMap()[accel.Id] != "" || getMediaIdNameMap()[accel.Id] != "" {
			continue
		}

		name := idNameMap[accel.Id]
		if name == "" {
			name = accel.Id
		}
		keystrokes := accel.Keystrokes
		if keystrokes == nil {
			keystrokes = accel.DefaultKeystrokes
		}
		//Del
		if accel.Id == "logout" {
			for i := 0; i < len(keystrokes); i++ {
				keystrokes[i] = strings.Replace(keystrokes[i], "Del", "Delete", 1)
			}
		}
		//minimize
		if accel.Id == "minimize" {
			if accel.DefaultKeystrokes != nil {
				keystrokes = accel.DefaultKeystrokes
			}
		}
		//launcher
		if accel.Id == "launcher" {
			if keystrokes == nil {
				keystrokes = append(keystrokes, "<Super_L>")
			}
		}
		//system-monitor
		if accel.Id == "system-monitor" {
			for i := 0; i < len(keystrokes); i++ {
				keystrokes[i] = strings.Replace(keystrokes[i], "Esc", "Escape", 1)
			}
		}
		ks := newKWinShortcut(accel.Id, name, keystrokes, wmObj)
		sm.addWithoutLock(ks)
	}
}

func (sm *ShortcutManager) AddSystemToKwin(gsettings *gio.Settings, wmObj wm.Wm) {
	logger.Debug("AddSystemToKwin")
	idNameMap := getSystemIdNameMap()
	for _, id := range gsettings.ListKeys() {
		name := idNameMap[id]
		if name == "" {
			name = id
		}

		accelJson, err := util.MarshalJSON(util.KWinAccel{
			Id:         id,
			Keystrokes: gsettings.GetStrv(id),
		})

		if id == "screenshot-window" {
			accelJson = `{"Id":"screenshot-window","Accels":["SysReq"]}` //+ Alt+print对应kwin识别的键SysReq
		}
		if id == "launcher" {
			accelJson = `{"Id":"launcher","Accels":["Super_L"]}`
		}
		if id == "system_monitor" {
			accelJson = `{"Id":"system_monitor","Accels":["<Crtl><Alt>Escape"]}`
		}
		if err != nil {
			logger.Warning("failed to get json:", err)
			continue
		}
		ok, err := wmObj.SetAccel(0, accelJson)
		if !ok {
			logger.Warning("failed to set KWin accels:", id, gsettings.GetStrv(id), err)
		}
		sm.AddSystemById(gsettings, wmObj, id)
	}
}

func (sm *ShortcutManager) AddMediaToKwin(gsettings *gio.Settings, wmObj wm.Wm) {
	logger.Debug("AddMediaToKwin")
	idNameMap := getMediaIdNameMap()
	for _, id := range gsettings.ListKeys() {
		if id == "close" {
			continue
		}
		name := idNameMap[id]
		logger.Warning("+++ gsetting KWin accels ID , NANE:", id, name)

		if name == "" {
			name = id
		}
		if len(os.Getenv("WAYLAND_DISPLAY")) != 0 {
			if name == "Capslock" || name == "Numlock" || name == "switch-monitors" {
				continue
			}
		}

		accelJson, err := util.MarshalJSON(util.KWinAccel{
			Id:         id,
			Keystrokes: []string{GetQtKeycodeMap()[id]}, //gsettings.GetStrv(id),
		})
		if err != nil {
			logger.Warning("failed to get json:", err)
			continue
		}

		ok, err := wmObj.SetAccel(0, accelJson)
		if !ok {
			logger.Warning("failed to set KWin accels:", accelJson, err)
		}
		keystrokes := gsettings.GetStrv(id)
		gs := NewGSettingsShortcut(gsettings, wmObj, id, ShortcutTypeMedia, keystrokes, name)
		mediaShortcut := &MediaShortcut{
			GSettingsShortcut: gs,
		}
		sm.addWithoutLock(mediaShortcut)
	}
}

func (sm *ShortcutManager) AddSpecialToKwin(wmObj wm.Wm) {
	accelJson, err := util.MarshalJSON(util.KWinAccel{
		Id:         "switch-kbd-layout",
		Keystrokes: []string{"<Super>space"},
	})
	if err != nil {
		logger.Warning("failed to get json:", err)
	}

	ok, err := wmObj.SetAccel(0, accelJson)
	if !ok {
		logger.Warning("failed to set KWin accels:", accelJson, err)
	}
	logger.Warning("set sepical KWin accels:", accelJson)

}

func isZH() bool {
	lang := gettext.QueryLang()
	return strings.HasPrefix(lang, "zh")
}

func getDeepinReleaseType() string {
	keyFile, err := dutils.NewKeyFileFromFile(versionFile)
	if err != nil {
		logger.Warningf("failed to open '%s' : %s", versionFile, err)
		return ""
	}
	defer keyFile.Free()
	releaseType, err := keyFile.GetString("Release", "Type")
	if err != nil {
		logger.Warningf("failed to get release type : %s", err)
		return ""
	}

	logger.Debugf("release type is %s", releaseType)
	return releaseType
}
