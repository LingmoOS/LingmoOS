// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/strv"

	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	configManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	lockfront "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.dde.lockfront"
	shutdownfront "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.dde.shutdownfront"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	appmanager "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.application1"
	inputdevices "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.inputdevices1"
	kwayland "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.kwayland1"
	network "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.network1"
	sessionmanager "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.sessionmanager1"
	newAppmanager "github.com/linuxdeepin/go-dbus-factory/session/org.desktopspec.applicationmanager1"
	airplanemode "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.airplanemode1"
	backlight "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.backlighthelper1"
	keyevent "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.keyevent1"
	power "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.power1"
	systeminfo "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.systeminfo1"
	DisplayManager "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.DisplayManager"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	networkmanager "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/xdg/basedir"
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

//go:generate dbusutil-gen em -type Manager

const (
	// shortcut signals:
	shortcutSignalChanged = "Changed"
	shortcutSignalAdded   = "Added"
	shortcutSignalDeleted = "Deleted"

	gsSchemaKeyboard          = "com.deepin.dde.keyboard"
	gsKeyNumLockState         = "numlock-state"
	gsKeySaveNumLockState     = "save-numlock-state"
	gsKeyShortcutSwitchLayout = "shortcut-switch-layout"
	gsKeyShowCapsLockOSD      = "capslock-toggle"
	gsKeyUpperLayerWLAN       = "upper-layer-wlan"

	gsSchemaSystem         = "com.deepin.dde.keybinding.system"
	gsSchemaSystemPlatform = "com.deepin.dde.keybinding.system.platform"
	gsSchemaSystemEnable   = "com.deepin.dde.keybinding.system.enable"
	gsSchemaMediaKey       = "com.deepin.dde.keybinding.mediakey"
	gsSchemaGnomeWM        = "com.deepin.wrap.gnome.desktop.wm.keybindings"
	gsSchemaSessionPower   = "com.deepin.dde.power"

	customConfigFile = "deepin/dde-daemon/keybinding/custom.ini"
	CapslockKey      = 58
	NumlockKey       = 69
	KeyPress         = 1
)

const (
	DSettingsAppID                         = "org.deepin.dde.daemon"
	DSettingsKeyBindingName                = "org.deepin.dde.daemon.keybinding"
	DSettingsKeyWirelessControlEnable      = "wirelessControlEnable"
	DSettingsKeyNeedXrandrQDevices         = "need-xrandr-q-devices"
	DSettingsKeyDeviceManagerControlEnable = "deviceManagerControlEnable"
)

const ( // power按键事件的响应
	powerActionShutdown int32 = iota
	powerActionSuspend
	powerActionHibernate
	powerActionTurnOffScreen
	powerActionShowUI
)

var _useWayland bool

func setUseWayland(value bool) {
	_useWayland = value
}

const (
	appManagerDBusServiceName = "org.desktopspec.ApplicationManager1"
	appManagerDBusPath        = "/org/desktopspec/ApplicationManager1"
)

const (
	KeyType        = "Type"
	KeyVersion     = "Version"
	KeyName        = "Name"
	KeyGenericName = "GenericName"
	KeyNoDisplay   = "NoDisplay"
	KeyIcon        = "Icon"
	KeyExec        = "Exec"
	KeyPath        = "Path"
	KeyTerminal    = "Terminal"
	KeyMimeType    = "MimeType"
	KeyActions     = "Actions"
)

type Manager struct {
	service *dbusutil.Service
	// properties
	NumLockState         gsprop.Enum
	ShortcutSwitchLayout gsprop.Uint `prop:"access:rw"`

	conn       *x.Conn
	keySymbols *keysyms.KeySymbols

	gsKeyboard       *gio.Settings
	gsSystem         *gio.Settings
	gsSystemPlatform *gio.Settings
	gsSystemEnable   *gio.Settings
	gsMediaKey       *gio.Settings
	gsGnomeWM        *gio.Settings
	gsPower          *gio.Settings

	enableListenGSettings      bool
	delayNetworkStateChange    bool
	canExcuteSuspendOrHiberate bool
	prepareForSleep            bool
	clickNum                   uint32
	shortcutCmd                string
	shortcutKey                string
	shortcutKeyCmd             string
	customShortcutManager      *shortcuts.CustomShortcutManager

	lockFront     lockfront.LockFront
	shutdownFront shutdownfront.ShutdownFront

	sessionSigLoop *dbusutil.SignalLoop
	systemSigLoop  *dbusutil.SignalLoop
	//startManager              sessionmanager.StartManager
	appManager                appmanager.Manager
	sessionManager            sessionmanager.SessionManager
	airplane                  airplanemode.AirplaneMode
	networkmanager            networkmanager.Manager
	backlightHelper           backlight.Backlight
	keyboard                  inputdevices.Keyboard
	keyboardLayout            string
	wm                        wm.Wm
	waylandOutputMgr          kwayland.OutputManagement
	login1Manager             login1.Manager
	keyEvent                  keyevent.KeyEvent
	displayManager            DisplayManager.DisplayManager
	network                   network.Network
	specialKeycodeBindingList map[SpecialKeycodeMapKey]func()

	// controllers
	audioController       *AudioController
	mediaPlayerController *MediaPlayerController
	displayController     *DisplayController
	kbdLightController    *KbdLightController
	touchPadController    *TouchPadController

	shortcutManager *shortcuts.ShortcutManager
	// shortcut action handlers
	handlers             []shortcuts.KeyEventFunc
	lastKeyEventTime     time.Time
	lastMethodCalledTime time.Time
	delayUpdateRfTimer   *time.Timer
	grabScreenKeystroke  *shortcuts.Keystroke

	// for switch kbd layout
	switchKbdLayoutState SKLState
	sklWaitQuit          chan int

	// dsg config
	wifiControlEnable          bool
	needXrandrQDevice          []string
	useNewAppManager           bool
	deviceManagerControlEnable bool

	configManagerPath           dbus.ObjectPath
	DisabledSystemShortcutsList strv.Strv

	dmiInfo     systeminfo.DMIInfo
	rfkillState bool
	repeatCount int
	fnLockCount int
	fnLocking   bool

	// nolint
	signals *struct {
		Added, Deleted, Changed struct {
			id  string
			typ int32
		}

		KeyEvent struct {
			pressed   bool
			keystroke string
		}
	}
}

// SKLState Switch keyboard Layout state
type SKLState uint

const (
	SKLStateNone SKLState = iota
	SKLStateWait
	SKLStateOSDShown
)

func newManager(service *dbusutil.Service) (*Manager, error) {
	setUseWayland(strings.Contains(os.Getenv("XDG_SESSION_TYPE"), "wayland"))
	conn, err := x.NewConn()
	if err != nil {
		return nil, err
	}

	sessionBus := service.Conn()
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}

	var m = Manager{
		service:               service,
		enableListenGSettings: true,
		conn:                  conn,
		keySymbols:            keysyms.NewKeySymbols(conn),
		handlers:              make([]shortcuts.KeyEventFunc, shortcuts.ActionTypeCount),
	}

	m.sessionSigLoop = dbusutil.NewSignalLoop(sessionBus, 10)
	m.systemSigLoop = dbusutil.NewSignalLoop(sysBus, 10)

	if _useWayland {
		m.waylandOutputMgr = kwayland.NewOutputManagement(sessionBus)
	}
	m.login1Manager = login1.NewManager(sysBus)
	m.login1Manager.InitSignalExt(m.systemSigLoop, true)
	_, err = m.login1Manager.ConnectPrepareForSleep(func(isSleep bool) {
		logger.Debugf("PreparingForSleep status changed, isSleep: %v", isSleep)
		m.prepareForSleep = isSleep
		// 待机或休眠时，唤醒后的1秒内不响应待机和休眠操作，避免按电源键唤醒时再次进入待机或休眠
		if !isSleep {
			m.canExcuteSuspendOrHiberate = false
			time.AfterFunc(1*time.Second, func() {
				m.canExcuteSuspendOrHiberate = true
			})
		} else {
			m.canExcuteSuspendOrHiberate = false
		}
	})
	if err != nil {
		logger.Warning("failed to connect signal PrepareForSleep:", err)
	}
	m.prepareForSleep, _ = m.login1Manager.PreparingForSleep().Get(0)
	m.displayManager = DisplayManager.NewDisplayManager(sysBus)
	m.shutdownFront = shutdownfront.NewShutdownFront(sessionBus)
	m.gsKeyboard = gio.NewSettings(gsSchemaKeyboard)
	m.NumLockState.Bind(m.gsKeyboard, gsKeyNumLockState)
	m.ShortcutSwitchLayout.Bind(m.gsKeyboard, gsKeyShortcutSwitchLayout)
	m.sessionSigLoop.Start()
	m.systemSigLoop.Start()

	m.initNumLockState(sysBus)
	m.initDSettings(sysBus)

	m.init()

	return &m, nil
}

func (m *Manager) init() {
	sessionBus := m.service.Conn()
	sysBus, _ := dbus.SystemBus()
	m.delayNetworkStateChange = true
	m.canExcuteSuspendOrHiberate = true

	// init settings
	m.gsSystem = gio.NewSettings(gsSchemaSystem)
	m.gsSystemPlatform = gio.NewSettings(gsSchemaSystemPlatform)
	m.gsSystemEnable = gio.NewSettings(gsSchemaSystemEnable)
	m.gsMediaKey = gio.NewSettings(gsSchemaMediaKey)
	m.gsPower = gio.NewSettings(gsSchemaSessionPower)
	m.wm = wm.NewWm(sessionBus)
	m.keyEvent = keyevent.NewKeyEvent(sysBus)
	m.network = network.NewNetwork(sessionBus)

	m.shortcutManager = shortcuts.NewShortcutManager(m.conn, m.keySymbols, m.handleKeyEvent)

	// when session is locked, we need handle some keyboard function event
	m.lockFront = lockfront.NewLockFront(sessionBus)
	m.lockFront.InitSignalExt(m.sessionSigLoop, true)
	m.lockFront.ConnectChangKey(func(changKey string) {
		m.handleKeyEventFromLockFront(changKey)
	})

	m.shutdownFront = shutdownfront.NewShutdownFront(sessionBus)
	m.shutdownFront.InitSignalExt(m.sessionSigLoop, true)
	m.shutdownFront.ConnectChangKey(func(changKey string) {
		m.handleKeyEventFromShutdownFront(changKey)
	})

	if _useWayland {
		if shouldUseDDEKwin() {
			m.shortcutManager.AddSpecialToKwin(m.wm)
			m.shortcutManager.AddSystemToKwin(m.gsSystem, m.wm)
			m.shortcutManager.AddMediaToKwin(m.gsMediaKey, m.wm)
			m.shortcutManager.AddKWinForWayland(m.wm)
		} else {
			m.shortcutManager.AddSpecial()
			m.shortcutManager.AddSystem(m.gsSystem, m.gsSystemPlatform, m.gsSystemEnable, m.wm)
			m.shortcutManager.AddMedia(m.gsMediaKey, m.wm)
			m.gsGnomeWM = gio.NewSettings(gsSchemaGnomeWM)
			m.shortcutManager.AddWM(m.gsGnomeWM, m.wm)
		}
	} else {
		m.shortcutManager.AddSystem(m.gsSystem, m.gsSystemPlatform, m.gsSystemEnable, m.wm)
		m.shortcutManager.AddMedia(m.gsMediaKey, m.wm)
		if shouldUseDDEKwin() {
			logger.Debug("Use DDE KWin")
			m.shortcutManager.AddKWin(m.wm)
		} else {
			logger.Debug("Use gnome WM")
			m.gsGnomeWM = gio.NewSettings(gsSchemaGnomeWM)
			m.shortcutManager.AddWM(m.gsGnomeWM, m.wm)
		}
	}

	// init custom shortcuts
	customConfigFilePath := filepath.Join(basedir.GetUserConfigDir(), customConfigFile)
	m.customShortcutManager = shortcuts.NewCustomShortcutManager(customConfigFilePath)
	m.shortcutManager.AddCustom(m.customShortcutManager, m.wm)

	// init controllers
	m.backlightHelper = backlight.NewBacklight(sysBus)
	m.audioController = NewAudioController(sessionBus, m.backlightHelper)
	m.mediaPlayerController = NewMediaPlayerController(m.systemSigLoop, sessionBus)

	//m.startManager = sessionmanager.NewStartManager(sessionBus)
	m.airplane = airplanemode.NewAirplaneMode(sysBus)
	m.networkmanager = networkmanager.NewManager(sysBus)
	m.sessionManager = sessionmanager.NewSessionManager(sessionBus)
	m.keyboard = inputdevices.NewKeyboard(sessionBus)
	m.keyboard.InitSignalExt(m.sessionSigLoop, true)
	err := m.keyboard.CurrentLayout().ConnectChanged(func(hasValue bool, layout string) {
		if !hasValue {
			return
		}
		if m.keyboardLayout != layout {
			m.keyboardLayout = layout
			logger.Debug("keyboard layout changed:", layout)
			m.shortcutManager.NotifyLayoutChanged()
		}
	})

	if err != nil {
		logger.Warning("connect CurrentLayout property changed failed:", err)
	}

	m.displayController = NewDisplayController(m.backlightHelper, sessionBus, m)
	m.kbdLightController = NewKbdLightController(m.backlightHelper)
	m.touchPadController = NewTouchPadController(sessionBus)

	m.initSpecialKeycodeMap()
	m.keyEvent.InitSignalExt(m.systemSigLoop, true)
	_, err = m.keyEvent.ConnectKeyEvent(m.handleSpecialKeycode)
	if err != nil {
		logger.Warning(err)
	}

	sysInfo := systeminfo.NewSystemInfo(sysBus)
	dmiInfo, err := sysInfo.DMIInfo().Get(0)
	if err != nil {
		logger.Warning(err)
	} else {
		m.dmiInfo = dmiInfo
	}

	if _useWayland {
		m.initHandlers()
		m.clickNum = 0

		go m.listenGlobalAccel(sessionBus)
		go m.listenKeyboardEvent(sysBus)
	}

	m.rfkillState, err = m.airplane.Enabled().Get(0)
	if err != nil {
		logger.Warning(err)
	} else {
		logger.Info("init rfkillState : ", m.rfkillState)
	}

	hasOwner, err := m.service.NameHasOwner(appManagerDBusServiceName)
	if err != nil {
		logger.Warning("failed to call NameHasOwner:", err)
	}
	if hasOwner {
		m.useNewAppManager = true
	}
}

func (m *Manager) initDSettings(bus *dbus.Conn) {
	ds := configManager.NewConfigManager(bus)
	dsPath, err := ds.AcquireManager(0, DSettingsAppID, DSettingsKeyBindingName, "")
	if err != nil {
		logger.Warning(err)
		return
	}

	keybindingDS, err := configManager.NewManager(bus, dsPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	getWirelessControlEnableConfig := func() {
		v, err := keybindingDS.Value(0, DSettingsKeyWirelessControlEnable)
		if err != nil {
			logger.Warning(err)
			return
		}
		m.wifiControlEnable = v.Value().(bool)
	}
	getNeedXrandrQConfig := func() {
		v, err := keybindingDS.Value(0, DSettingsKeyNeedXrandrQDevices)
		if err != nil {
			logger.Warning(err)
			return
		}
		itemList := v.Value().([]dbus.Variant)
		for _, i := range itemList {
			m.needXrandrQDevice = append(m.needXrandrQDevice, i.Value().(string))
		}
	}

	getDeviceManagerControlEnableConfig := func() {
		v, err := keybindingDS.Value(0, DSettingsKeyDeviceManagerControlEnable)
		if err != nil {
			logger.Warning(err)
			return
		}
		m.deviceManagerControlEnable = v.Value().(bool)
	}
	getWirelessControlEnableConfig()
	getNeedXrandrQConfig()
	getDeviceManagerControlEnableConfig()

	keybindingDS.InitSignalExt(m.systemSigLoop, true)
	// 监听dsg配置变化
	_, err = keybindingDS.ConnectValueChanged(func(key string) {
		switch key {
		case DSettingsKeyWirelessControlEnable:
			getWirelessControlEnableConfig()
		case DSettingsKeyNeedXrandrQDevices:
			getNeedXrandrQConfig()
		case DSettingsKeyDeviceManagerControlEnable:
			getDeviceManagerControlEnableConfig()
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

var kwinSysActionCmdMap = map[string]string{
	"Launcher":              "launcher",               // Super_L Super_R
	"Terminal":              "terminal",               // <Control><Alt>T
	"Terminal Quake Window": "terminal-quake",         //
	"Lock screen":           "lock-screen",            // super+l
	"Shutdown interface":    "logout",                 // ctrl+alt+del
	"File manager":          "file-manager",           // super+e
	"Screenshot":            "screenshot",             // ctrl+alt+a
	"Full screenshot":       "screenshot-fullscreen",  // print
	"Window screenshot":     "screenshot-window",      // alt+print
	"Delay screenshot":      "screenshot-delayed",     // ctrl+print
	"Disable Touchpad":      "disable-touchpad",       //
	"Switch window effects": "wm-switcher",            // alt+tab
	"turn-off-screen":       "Fast Screen Off",        // <Shift><Super>L
	"Deepin Picker":         "color-picker",           // ctrl+alt+v
	"System Monitor":        "system-monitor",         // ctrl+alt+escape
	"Screen Recorder":       "deepin-screen-recorder", // deepin-screen-recorder ctrl+alt+r
	"Desktop AI Assistant":  "ai-assistant",           // ai-assistant [<Super>Q]q
	"Text to Speech":        "text-to-speech",
	"Speech to Text":        "speech-to-text",
	"Clipboard":             "clipboard",
	"Translation":           "translation",
	"Show/Hide the dock":    "show-dock",

	// cmd
	"Calculator":          "calculator",          // XF86Calculator
	"Search":              "search",              // XF86Search
	"Notification Center": "notification-center", // Meta M

	"ScreenshotScroll": "screenshot-scroll",
	"ScreenshotOcr":    "screenshot-ocr",
	"Global Search":    "global-search",
	"Switch monitors":  "switch-monitors",
}

var waylandMediaIdMap = map[string]string{
	"Messenger":         "messenger",           // XF86Messenger
	"Save":              "save",                // XF86Save
	"New":               "new",                 // XF86New
	"WakeUp":            "wake-up",             // XF86WakeUp
	"audio-rewind":      "AudioRewind",         // XF86AudioRewind
	"VolumeMute":        "audio-mute",          // XF86AudioMute  "AudioMute":
	"MonBrightnessUp":   "mon-brightness-up",   // XF86MonBrightnessUp
	"WLAN":              "wlan",                // XF86WLAN
	"AudioMedia":        "audio-media",         // XF86AudioMedia
	"reply":             "Reply",               // XF86Reply
	"favorites":         "Favorites",           // XF86Favorites
	"AudioPlay":         "audio-play",          // XF86AudioPlay
	"AudioMicMute":      "audio-mic-mute",      // XF86AudioMicMute
	"AudioPause":        "audio-pause",         // XF86AudioPause
	"AudioStop":         "audio-stop",          // XF86AudioStop
	"documents":         "Documents",           // XF86Documents
	"game":              "Game",                // XF86Game
	"AudioRecord":       "audio-record",        // XF86AudioRecord
	"Display":           "display",             // XF86Display
	"reload":            "Reload",              // XF86Reload
	"explorer":          "Explorer",            // XF86Explorer
	"calendar":          "Calendar",            // XF86Calendar
	"forward":           "Forward",             // XF86Forward
	"cut":               "Cut",                 // XF86Cut
	"MonBrightnessDown": "mon-brightness-down", // XF86MonBrightnessDown
	"Copy":              "copy",                // XF86Copy
	"Tools":             "tools",               // XF86Tools
	"VolumeUp":          "audio-raise-volume",  // XF86AudioRaiseVolume "AudioRaiseVolume":  "audio-raise-volume",
	"media-close":       "media-Close",         // XF86Close
	"WWW":               "www",                 // XF86WWW
	"HomePage":          "home-page",           // XF86HomePage
	"sleep":             "Sleep",               // XF86Sleep
	"VolumeDown":        "audio-lower-volume",  // XF86AudioLowerVolume  "AudioLowerVolume":  "audio-lower-volume",
	"AudioPrev":         "audio-prev",          // XF86AudioPrev
	"AudioNext":         "audio-next",          // XF86AudioNext
	"Paste":             "paste",               // XF86Paste
	"open":              "Open",                // XF86Open
	"send":              "Send",                // XF86Send
	"my-computer":       "MyComputer",          // XF86MyComputer
	"Mail":              "mail",                // XF86Mail
	"adjust-brightness": "BrightnessAdjust",    // XF86BrightnessAdjust
	"LogOff":            "log-off",             // XF86LogOff
	"pictures":          "Pictures",            // XF86Pictures
	"Terminal":          "terminal",            // XF86Terminal
	"video":             "Video",               // XF86Video
	"Music":             "music",               // XF86Music
	"app-left":          "ApplicationLeft",     // XF86ApplicationLeft
	"app-right":         "ApplicationRight",    // XF86ApplicationRight
	"meeting":           "Meeting",             // XF86Meeting
	"Numlock":           "numlock",
	"Capslock":          "capslock",
	"Switch kbd layout": "switch-kbd-layout",
}

func (m *Manager) listenGlobalAccel(sessionBus *dbus.Conn) error {
	err := sessionBus.Object("org.kde.kglobalaccel",
		"/component/kwin").AddMatchSignal("org.kde.kglobalaccel.Component", "globalShortcutPressed").Err
	if err != nil {
		logger.Warning(err)
		return err
	}

	m.sessionSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.kde.kglobalaccel.Component.globalShortcutPressed",
	}, func(sig *dbus.Signal) {
		if len(sig.Body) > 1 {
			const minKeyEventInterval = 200 * time.Millisecond
			now := time.Now()
			duration := now.Sub(m.lastKeyEventTime)
			if 0 < duration && duration < minKeyEventInterval {
				logger.Debug("ignore key event duration:", duration)
				return
			}
			m.lastKeyEventTime = now

			m.shortcutKey = sig.Body[0].(string)
			m.shortcutKeyCmd = sig.Body[1].(string)
			shortId := kwinSysActionCmdMap[m.shortcutKeyCmd]
			if shortId != "" && m.DisabledSystemShortcutsList.Contains(shortId) {
				logger.Warningf("shortcut id: %s is disabled", shortId)
				return
			}
			ok := strings.Compare(string("kwin"), m.shortcutKey)
			if ok == 0 {
				logger.Debug("[global key] get accel sig.Body[1]", m.shortcutKeyCmd)
				if m.shortcutKeyCmd == "" {
					// + 把响应一次的逻辑放到协程外执行，防止协程响应延迟
					m.handleKeyEventByWayland(waylandMediaIdMap[m.shortcutKeyCmd])
				} else {
					m.shortcutCmd = shortcuts.GetSystemActionCmd(kwinSysActionCmdMap[m.shortcutKeyCmd])
					if m.shortcutCmd == "" {
						m.shortcutCmd = m.shortcutManager.WaylandCustomShortCutMap[m.shortcutKeyCmd]
					}
					logger.Debug("WaylandCustomShortCutMap", m.shortcutCmd)
					if m.shortcutCmd == "" {
						m.handleKeyEventByWayland(waylandMediaIdMap[m.shortcutKeyCmd])
					} else {
						if strings.HasSuffix(m.shortcutCmd, ".desktop") {
							err := m.runDesktopFile(m.shortcutCmd)
							if err != nil {
								logger.Warning(err)
							}
						} else {
							go func() {
								err := m.execCmd(m.shortcutCmd, true)
								if err != nil {
									logger.Warning(err)
								}
							}()
						}
					}
				}
			}
		}
	})

	return nil
}

func (m *Manager) listenKeyboardEvent(systemBus *dbus.Conn) {
	err := systemBus.Object("org.deepin.dde.Gesture1",
		"/org/deepin/dde/Gesture1").AddMatchSignal("org.deepin.dde.Gesture1", "KeyboardEvent").Err
	if err != nil {
		logger.Warning(err)
	}
	m.systemSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.deepin.dde.Gesture1.KeyboardEvent",
	}, func(sig *dbus.Signal) {
		if len(sig.Body) > 1 {
			key := sig.Body[0].(uint32)
			value := sig.Body[1].(uint32)

			if key == CapslockKey && value == KeyPress {
				m.handleKeyEventByWayland("capslock")
			} else if key == NumlockKey && value == KeyPress {
				m.handleKeyEventByWayland("numlock")
			}
		}
	})
}

// 初始化 NumLock 数字锁定键状态
func (m *Manager) initNumLockState(sysBus *dbus.Conn) {
	// 从 gsettings 读取相关设置
	nlState := NumLockState(m.NumLockState.Get())
	saveStateEnabled := m.gsKeyboard.GetBoolean(gsKeySaveNumLockState)
	if nlState == NumLockUnknown {
		// 判断是否是笔记本, 只根据电池状态，有电池则是笔记本。
		isLaptop := false
		sysPower := power.NewPower(sysBus)
		hasBattery, err := sysPower.HasBattery().Get(0)
		if err != nil {
			logger.Warning("failed to get sysPower HasBattery property:", err)
		} else if hasBattery {
			isLaptop = true
		}

		state := NumLockUnknown
		logger.Debug("isLaptop:", isLaptop)
		if isLaptop {
			// 笔记本，默认关闭。
			state = NumLockOff
		} else {
			// 台式机等，默认开启。
			state = NumLockOn
		}

		if saveStateEnabled {
			// 保存新状态到 gsettings
			m.NumLockState.Set(int32(state))
		}

		err = setNumLockState(m.waylandOutputMgr, m.conn, m.keySymbols, state)
		if err != nil {
			logger.Warning("setNumLockState failed:", err)
		}
	} else {
		if saveStateEnabled {
			err := setNumLockState(m.waylandOutputMgr, m.conn, m.keySymbols, nlState)
			if err != nil {
				logger.Warning("setNumLockState failed:", err)
			}
		}
	}

}

// 检查快捷键时间间隔，如果间隔太短返回false，不应该响应快捷键
func (m *Manager) checkKeyEventInterval() bool {
	const minKeyEventInterval = 200 * time.Millisecond
	now := time.Now()
	duration := now.Sub(m.lastKeyEventTime)
	if 0 < duration && duration < minKeyEventInterval {
		logger.Debug("handleKeyEvent ignore key event duration:", duration)
		return false
	}
	m.lastKeyEventTime = now
	return true
}

func (m *Manager) handleKeyEventFromLockFront(changKey string) {
	if !m.checkKeyEventInterval() {
		return
	}
	logger.Debugf("Receive LockFront ChangKey Event %s", changKey)
	action := shortcuts.GetAction(changKey)

	// numlock/capslock
	if action.Type == shortcuts.ActionTypeShowNumLockOSD ||
		action.Type == shortcuts.ActionTypeShowCapsLockOSD ||
		action.Type == shortcuts.ActionTypeSystemShutdown {
		if handler := m.handlers[int(action.Type)]; handler != nil {
			handler(nil)
		} else {
			logger.Warning("handler is nil")
		}
	} else {
		cmd, ok := action.Arg.(shortcuts.ActionCmd)
		if !ok {
			logger.Warning(errTypeAssertionFail)
		} else {
			if action.Type == shortcuts.ActionTypeAudioCtrl {
				// audio-mute/audio-lower-volume/audio-raise-volume
				if m.audioController != nil {
					if err := m.audioController.ExecCmd(cmd); err != nil {
						logger.Warning(m.audioController.Name(), "Controller exec cmd err:", err)
					}
				}
			} else if action.Type == shortcuts.ActionTypeDisplayCtrl {
				// mon-brightness-up/mon-brightness-down
				if m.displayController != nil {
					if err := m.displayController.ExecCmd(cmd); err != nil {
						logger.Warning(m.displayController.Name(), "Controller exec cmd err:", err)
					}
				}
			} else if action.Type == shortcuts.ActionTypeTouchpadCtrl {
				// touchpad-toggle/touchpad-on/touchpad-off
				if m.touchPadController != nil {
					if err := m.touchPadController.ExecCmd(cmd); err != nil {
						logger.Warning(m.touchPadController.Name(), "Controller exec cmd err:", err)
					}
				}
			}
		}
	}
}

func (m *Manager) handleKeyEventByWayland(changKey string) {
	action := shortcuts.GetAction(changKey)
	var isWaylandGrabed bool = false
	if _useWayland {
		isWaylandGrabed = true
		if action.Type == shortcuts.ActionTypeShowNumLockOSD || action.Type == shortcuts.ActionTypeShowCapsLockOSD {
			sessionBus, err := dbus.SessionBus()
			if err != nil {
				return
			}
			sessionObj := sessionBus.Object("org.kde.KWin", "/KWin")
			err = sessionObj.Call("org.kde.KWin.xwaylandGrabed", 0).Store(&isWaylandGrabed)
			if err != nil {
				logger.Warning(err)
				return
			}
			logger.Debug("xwaylandGrabed: ", isWaylandGrabed)
		}
	}
	// numlock/capslock
	if action.Type == shortcuts.ActionTypeSystemShutdown {
		var powerPressAction int32
		systemBus, _ := dbus.SystemBus()
		systemPower := power.NewPower(systemBus)
		onBattery, err := systemPower.OnBattery().Get(0)
		if err != nil {
			logger.Error(err)
		}
		if onBattery {
			powerPressAction = m.gsPower.GetEnum("battery-press-power-button")
		} else {
			powerPressAction = m.gsPower.GetEnum("line-power-press-power-button")
		}
		logger.Debug("powerPressAction:", powerPressAction)
		switch powerPressAction {
		case powerActionShutdown:
			m.systemShutdown()
		case powerActionSuspend:
			systemSuspend()
		case powerActionHibernate:
			m.systemHibernate()
		case powerActionTurnOffScreen:
			m.systemTurnOffScreen()
		case powerActionShowUI:
			cmd := "dde-shutdown"
			go func() {
				err := m.execCmd(cmd, false)
				if err != nil {
					logger.Warning("execCmd error:", err)
				}
			}()
		}
	} else if action.Type == shortcuts.ActionTypeShowControlCenter {
		err := m.execCmd("dbus-send --session --dest=com.deepin.dde.ControlCenter  --print-reply /com/deepin/dde/ControlCenter com.deepin.dde.ControlCenter.Show",
			false)
		if err != nil {
			logger.Warning("failed to show control center:", err)
		}

	} else if action.Type == shortcuts.ActionTypeToggleWireless {
		// 如果已经开启了飞行模式，则不能通过快捷键去控制wifi的使能
		enabled, err := m.airplane.Enabled().Get(0)
		if err != nil {
			logger.Warningf("get airplane enabled failed, err: %v", err)
			return
		}

		if enabled {
			logger.Debug("airplane mode enabled, can not enable wireless by key")
			return
		}

		// check if allow set wireless
		// and check if Wifi shortcut effected by DDE software
		if m.gsMediaKey.GetBoolean(gsKeyUpperLayerWLAN) && m.wifiControlEnable {
			enabled, err := m.airplane.WifiEnabled().Get(0)
			if err != nil {
				logger.Warningf("get wireless enabled failed, err: %v", err)
				return
			}
			// FIXME:  修复NM WiFi无法恢复bug, 使快捷键能够恢复WiFi问题
			if !enabled {
				if devicesJson, err := m.network.Devices().Get(0); err == nil {
					networkDevices := make(map[string][]*networkDevice)
					json.Unmarshal([]byte(devicesJson), &networkDevices)
					for _, wifiDevice := range networkDevices["wireless"] {
						if wifiDevice.InterfaceFlags == 0 {
							// wifi里有未up的网络接口时尝试重置下wifi网络
							logger.Info("rest wifi, because some link down")
							if err := m.networkmanager.WirelessEnabled().Set(0, false); err != nil {
								logger.Warning(err)
							}
							if err := m.networkmanager.WirelessEnabled().Set(0, true); err != nil {
								logger.Warning(err)
							}
							break
						}
					}
				}
			}
			err = m.airplane.EnableWifi(0, !enabled)
			if err != nil {
				logger.Warningf("set wireless enabled failed, err: %v", err)
				return
			}
		}
	} else if action.Type == shortcuts.ActionTypeShowNumLockOSD {
		var state NumLockState
		if !isWaylandGrabed {
			if _useWayland {
				sessionBus, err := dbus.SessionBus()
				if err != nil {
					return
				}
				time.Sleep(200 * time.Millisecond) // + 添加200ms延时，保证在dde-system-daemon中先获取状态；
				sessionObj := sessionBus.Object("org.kde.KWin", "/Xkb")
				var ret int32
				err = sessionObj.Call("org.kde.kwin.Xkb.getLeds", 0).Store(&ret)
				if err != nil {
					logger.Warning(err)
					return
				}
				if 0 == (ret & 0x1) {
					state = NumLockOff
				} else {
					state = NumLockOn
				}
			} else {
				var err error
				state, err = queryNumLockState(m.conn)
				if err != nil {
					logger.Warning(err)
					return
				}
			}

			save := m.gsKeyboard.GetBoolean(gsKeySaveNumLockState)

			switch state {
			case NumLockOn:
				if save {
					m.NumLockState.Set(int32(NumLockOn))
				}
				showOSD("NumLockOn")
			case NumLockOff:
				if save {
					m.NumLockState.Set(int32(NumLockOff))
				}
				showOSD("NumLockOff")
			}
		}
	} else if action.Type == shortcuts.ActionTypeShowCapsLockOSD {
		if !m.shouldShowCapsLockOSD() {
			return
		}

		if !isWaylandGrabed {
			var state CapsLockState
			if _useWayland {
				sessionBus, err := dbus.SessionBus()
				if err != nil {
					return
				}
				time.Sleep(200 * time.Millisecond) // + 添加200ms延时，保证在dde-system-daemon中先获取状态；
				sessionObj := sessionBus.Object("org.kde.KWin", "/Xkb")
				var ret int32
				err = sessionObj.Call("org.kde.kwin.Xkb.getLeds", 0).Store(&ret)
				if err != nil {
					logger.Warning(err)
					return
				}
				if 0 == (ret & 0x2) {
					state = CapsLockOff
				} else {
					state = CapsLockOn
				}
			} else {
				state, err := queryCapsLockState(m.conn)
				if err != nil {
					logger.Warning(err)
					return
				}
				logger.Debug("caps:", state)
			}

			switch state {
			case CapsLockOff:
				showOSD("CapsLockOff")
			case CapsLockOn:
				showOSD("CapsLockOn")
			}
		}
	} else if action.Type == shortcuts.ActionTypeSwitchKbdLayout {
		switch m.switchKbdLayoutState {
		case SKLStateNone:
			m.switchKbdLayoutState = SKLStateWait
			go m.sklWait()

		case SKLStateWait:
			m.switchKbdLayoutState = SKLStateOSDShown
			m.terminateSKLWait()
			showOSD("SwitchLayout")

		case SKLStateOSDShown:
			showOSD("SwitchLayout")
		}
	} else {
		cmd, ok := action.Arg.(shortcuts.ActionCmd)
		if !ok {
			logger.Warning(errTypeAssertionFail)
		} else {
			if action.Type == shortcuts.ActionTypeAudioCtrl {
				// audio-mute/audio-lower-volume/audio-raise-volume
				if m.audioController != nil {
					if err := m.audioController.ExecCmd(cmd); err != nil {
						logger.Warning(m.audioController.Name(), "Controller exec cmd err:", err)
					}
				}
			} else if action.Type == shortcuts.ActionTypeDisplayCtrl {
				// mon-brightness-up/mon-brightness-down
				if m.displayController != nil {
					if err := m.displayController.ExecCmd(cmd); err != nil {
						logger.Warning(m.displayController.Name(), "Controller exec cmd err:", err)
					}
				}
			} else if action.Type == shortcuts.ActionTypeTouchpadCtrl {
				// touchpad-toggle/touchpad-on/touchpad-off
				if m.touchPadController != nil {
					if err := m.touchPadController.ExecCmd(cmd); err != nil {
						logger.Warning(m.touchPadController.Name(), "Controller exec cmd err:", err)
					}
				}
			} else if action.Type == shortcuts.ActionTypeSystemShutdown {

			} else if action.Type == shortcuts.ActionTypeMediaPlayerCtrl {
				// 增蓝牙耳机快捷键的处理
				if cmd == shortcuts.MediaPlayerPlay {
					m.clickNum = m.clickNum + 1
					if m.clickNum == 1 {
						time.AfterFunc(time.Millisecond*600, func() {
							m.playMeadiaByHeadphone()
						})
					}
				} else {
					if m.mediaPlayerController != nil {
						err := m.mediaPlayerController.ExecCmd(cmd)
						if err != nil {
							logger.Warning(m.mediaPlayerController.Name(), "Controller exec cmd err:", err)
						}
					}
				}

			}
		}
	}
}

func getMediaPlayAction(num uint32) shortcuts.ActionCmd {
	var cmd shortcuts.ActionCmd = shortcuts.MediaPlayerPlay
	if num == 2 {
		cmd = shortcuts.MediaPlayerNext
	} else if num == 3 {
		cmd = shortcuts.MediaPlayerPrevious
	} else {
		cmd = shortcuts.MediaPlayerPlay
	}
	return cmd
}

func (m *Manager) playMeadiaByHeadphone() {
	cmd := getMediaPlayAction(m.clickNum)
	m.clickNum = 0
	if m.mediaPlayerController != nil {
		err := m.mediaPlayerController.ExecCmd(cmd)
		if err != nil {
			logger.Warning(m.mediaPlayerController.Name(), "Controller exec cmd err:", err)
		}
	}
}

func (m *Manager) handleKeyEventFromShutdownFront(changKey string) {
	logger.Debugf("handleKeyEvent %s from ShutdownFront", changKey)
	action := shortcuts.GetAction(changKey)
	if action.Type == shortcuts.ActionTypeSystemShutdown {
		if handler := m.handlers[int(action.Type)]; handler != nil {
			handler(nil)
		} else {
			logger.Warning("handler [system shutdown] is nil")
		}
	}
}

func (m *Manager) destroy() {
	err := m.service.StopExport(m)
	if err != nil {
		logger.Warning("stop export failed:", err)
	}

	if m.shortcutManager != nil {
		m.shortcutManager.Destroy()
		m.shortcutManager = nil
	}

	// destroy settings
	if m.gsSystem != nil {
		m.gsSystem.Unref()
		m.gsSystem = nil
	}

	if m.gsMediaKey != nil {
		m.gsMediaKey.Unref()
		m.gsMediaKey = nil
	}

	if m.gsGnomeWM != nil {
		m.gsGnomeWM.Unref()
		m.gsGnomeWM = nil
	}

	if m.audioController != nil {
		m.audioController.Destroy()
		m.audioController = nil
	}

	if m.mediaPlayerController != nil {
		m.mediaPlayerController.Destroy()
		m.mediaPlayerController = nil
	}

	if m.keyboard != nil {
		m.keyboard.RemoveHandler(proxy.RemoveAllHandlers)
		m.keyboard = nil
	}

	if m.keyEvent != nil {
		m.keyEvent.RemoveHandler(proxy.RemoveAllHandlers)
		m.keyEvent = nil
	}

	if m.sessionSigLoop != nil {
		m.sessionSigLoop.Stop()
		m.sessionSigLoop = nil
	}

	if m.systemSigLoop != nil {
		m.systemSigLoop.Stop()
		m.systemSigLoop = nil
	}

	if m.conn != nil {
		m.conn.Close()
		m.conn = nil
	}
}

func (m *Manager) handleKeyEvent(ev *shortcuts.KeyEvent) {
	if !m.checkKeyEventInterval() {
		return
	}
	logger.Debugf("handleKeyEvent ev: %#v", ev)
	action := ev.Shortcut.GetAction()
	shortcutId := ev.Shortcut.GetId()
	logger.Debugf("shortcut id: %s, type: %v, action: %#v",
		shortcutId, ev.Shortcut.GetType(), action)
	if ev.Shortcut.GetType() == shortcuts.ShortcutTypeSystem && m.DisabledSystemShortcutsList.Contains(shortcutId) {
		logger.Warningf("shortcut id: %s is disabled", shortcutId)
		return
	}
	if action == nil {
		logger.Warning("action is nil")
		return
	}
	if len(m.handlers) == 0 {
		logger.Warning("handlers is nil")
		return
	}
	if handler := m.handlers[int(action.Type)]; handler != nil {
		handler(ev)
	} else {
		logger.Warning("handler is nil")
	}
}

func (m *Manager) emitShortcutSignal(signalName string, shortcut shortcuts.Shortcut) {
	logger.Debug("emit DBus signal", signalName, shortcut.GetId(), shortcut.GetType())
	err := m.service.Emit(m, signalName, shortcut.GetId(), shortcut.GetType())
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) enableListenGSettingsChanged(val bool) {
	m.enableListenGSettings = val
}

func (m *Manager) listenGSettingsChanged(schema string, settings *gio.Settings, type0 int32) {
	gsettings.ConnectChanged(schema, "*", func(key string) {
		if !m.enableListenGSettings {
			return
		}

		shortcut := m.shortcutManager.GetByIdType(key, type0)
		if shortcut == nil {
			return
		}

		keystrokes := settings.GetStrv(key)
		m.shortcutManager.ModifyShortcutKeystrokes(shortcut, shortcuts.ParseKeystrokes(keystrokes))
		m.emitShortcutSignal(shortcutSignalChanged, shortcut)
	})
}

func (m *Manager) listenSystemEnableChanged() {
	gsettings.ConnectChanged(gsSchemaSystemEnable, "*", func(key string) {
		if !m.enableListenGSettings {
			return
		}

		if m.shortcutManager.CheckSystem(m.gsSystemPlatform, m.gsSystemEnable, key) {
			m.shortcutManager.AddSystemById(m.gsSystem, m.wm, key)
		} else {
			m.shortcutManager.DelSystemById(key)
		}
	})
}

func (m *Manager) listenSystemPlatformChanged() {
	gsettings.ConnectChanged(gsSchemaSystemPlatform, "*", func(key string) {
		if !m.enableListenGSettings {
			return
		}

		if m.shortcutManager.CheckSystem(m.gsSystemPlatform, m.gsSystemEnable, key) {
			m.shortcutManager.AddSystemById(m.gsSystem, m.wm, key)
		} else {
			m.shortcutManager.DelSystemById(key)
		}
	})
}

func runCommand(cmd string) (string, error) {
	result, err := exec.Command("/bin/sh", "-c", cmd).Output()
	if err != nil {
		return "", err
	}
	return strings.TrimSpace(string(result)), err
}

func checkProRunning(serverName string) bool {
	cmd := `ps ux | awk '/` + serverName + `/ && !/awk/ {print $2}'`
	pid, err := runCommand(cmd)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return pid != ""
}

func (m *Manager) execCmd(cmd string, viaStartdde bool) error {
	if cmd == "" {
		logger.Debug("cmd is empty")
		return nil
	}
	if strings.HasPrefix(cmd, "dbus-send ") || !viaStartdde {
		logger.Debug("run cmd:", cmd)
		// #nosec G204
		return exec.Command("/bin/sh", "-c", cmd).Run()
	}

	logger.Debug("exec run cmd:", cmd)

	if m.useNewAppManager {
		desktopExt := ".desktop"
		sha256Hasher := sha256.New()
		_, err := sha256Hasher.Write([]byte(cmd))
		if err != nil {
			logger.Warning("generate sha256 hash failed with error: ", err)
			return err
		}
		desktopPre := sha256Hasher.Sum(nil)
		name := hex.EncodeToString(desktopPre)
		desktopFileName := "daemon-keybinding-" + name + desktopExt

		_, err = os.Stat(basedir.GetUserDataDir() + "/applications/" + desktopFileName)
		// 如果对应命令的desktop文件不存在，需要新建desktop文件
		if os.IsNotExist(err) {
			desktopInfoMap := map[string]dbus.Variant{
				KeyExec: dbus.MakeVariant(map[string]string{
					"default": cmd,
				}),
				KeyIcon: dbus.MakeVariant(map[string]string{
					"default-icon": "",
				}),
				KeyMimeType: dbus.MakeVariant([]string{""}),
				KeyName: dbus.MakeVariant(map[string]string{
					"default": name,
				}),
				KeyTerminal:  dbus.MakeVariant(false),
				KeyType:      dbus.MakeVariant("Application"),
				KeyVersion:   dbus.MakeVariant(1),
				KeyNoDisplay: dbus.MakeVariant(true),
			}

			appManager := newAppmanager.NewManager(m.sessionSigLoop.Conn())
			err := appManager.ReloadApplications(0)
			if err != nil {
				logger.Warning("reload applications error: ", err)
			}

			desktopFileName, err = appManager.AddUserApplication(0, desktopInfoMap, desktopFileName)
			if err != nil {
				logger.Warning("adding user application error: ", err)
				return err
			}
		}

		obj, err := desktopappinfo.GetDBusObjectFromAppDesktop(desktopFileName, appManagerDBusServiceName, appManagerDBusPath)
		if err != nil {
			logger.Warning("get dbus object error:", err)
			return err
		}

		appManagerAppObj, err := newAppmanager.NewApplication(m.sessionSigLoop.Conn(), obj)
		if err != nil {
			return err
		}

		_, err = appManagerAppObj.Launch(0, "", []string{}, make(map[string]dbus.Variant))

		if err != nil {
			logger.Warningf("launch keybinding cmd %s error: %v", cmd, err)
			return err
		}
	} else {
		err := m.appManager.RunCommand(0, "/bin/sh", []string{"-c", cmd})
		if err != nil {
			logger.Warningf("launch keybinding cmd %s error: %v", cmd, err)
			return err
		}
	}

	return nil
}

func (m *Manager) handleCheckCamera() error {
	cmd := "deepin-camera"
	viaStartdde := true
	if checkProRunning(cmd) {
		cmd = "killall deepin-camera"
		viaStartdde = false
	}
	return m.execCmd(cmd, viaStartdde)
}

func (m *Manager) runDesktopFile(desktop string) error {
	if m.useNewAppManager {
		obj, err := desktopappinfo.GetDBusObjectFromAppDesktop(desktop, appManagerDBusServiceName, appManagerDBusPath)
		if err != nil {
			logger.Warning("get dbus object error: ", err)
			return err
		}

		appManagerAppObj, err := newAppmanager.NewApplication(m.sessionSigLoop.Conn(), obj)
		if err != nil {
			return err
		}

		_, err = appManagerAppObj.Launch(0, "", []string{}, make(map[string]dbus.Variant))
		if err != nil {
			logger.Warning("failed to launch application", desktop)
			return err
		}
	} else {
		err := m.appManager.LaunchApp(0, desktop, 0, []string{})
		if err != nil {
			logger.Warning("failed to launch application", desktop)
		}
	}

	return nil
}

func (m *Manager) eliminateKeystrokeConflict() {
	for _, ks := range m.shortcutManager.ConflictingKeystrokes {
		shortcut := ks.Shortcut
		logger.Infof("eliminate conflict shortcut: %s keystroke: %s",
			ks.Shortcut.GetUid(), ks)
		err := m.DeleteShortcutKeystroke(shortcut.GetId(), shortcut.GetType(), ks.String())
		if err != nil {
			logger.Warning("delete shortcut keystroke failed:", err)
		}
	}

	m.shortcutManager.ConflictingKeystrokes = nil
	m.shortcutManager.EliminateConflictDone = true
}
