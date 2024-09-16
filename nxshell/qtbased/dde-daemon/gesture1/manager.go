// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"encoding/json"
	"fmt"

	"io/ioutil"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	dock "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.dde.daemon.dock"
	notification "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.dde.notification"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	clipboard "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.clipboard1"
	display "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.display1"
	sessionmanager "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.sessionmanager1"
	sessionwatcher "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.sessionwatcher1"
	daemon "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.daemon1"
	gesture "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.gesture1"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
	"github.com/linuxdeepin/go-lib/gsettings"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

//go:generate dbusutil-gen em -type Manager

const (
	tsSchemaID              = "com.deepin.dde.touchscreen"
	tsSchemaKeyLongPress    = "longpress-duration"
	tsSchemaKeyShortPress   = "shortpress-duration"
	tsSchemaKeyEdgeMoveStop = "edgemovestop-duration"
	tsSchemaKeyBlacklist    = "longpress-blacklist"
)

type deviceType int32 // 设备类型(触摸屏，触摸板)

const (
	deviceTouchPad deviceType = iota
	deviceTouchScreen
)

var _useWayland bool

func setUseWayland(value bool) {
	_useWayland = value
}

type Manager struct {
	wm                 wm.Wm
	sysDaemon          daemon.Daemon
	systemSigLoop      *dbusutil.SignalLoop
	mu                 sync.RWMutex
	userFile           string
	builtinSets        map[string]func() error
	gesture            gesture.Gesture
	dock               dock.Dock
	display            display.Display
	setting            *gio.Settings
	tsSetting          *gio.Settings
	touchPadEnabled    bool
	touchScreenEnabled bool
	Infos              gestureInfos
	sessionmanager     sessionmanager.SessionManager
	clipboard          clipboard.Clipboard
	notification       notification.Notification

	longPressEnable       bool
	oneFingerBottomEnable bool
	oneFingerLeftEnable   bool
	oneFingerRightEnable  bool
	configManagerPath     dbus.ObjectPath
	sessionWatcher        sessionwatcher.SessionWatcher
}

func newManager() (*Manager, error) {
	setUseWayland(len(os.Getenv("WAYLAND_DISPLAY")) != 0)
	sessionConn, err := dbus.SessionBus()
	if err != nil {
		return nil, err
	}

	systemConn, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}

	var filename = configUserPath
	if !dutils.IsFileExist(configUserPath) {
		filename = configSystemPath
	}

	infos, err := newGestureInfosFromFile(filename)
	if err != nil {
		return nil, err
	}
	// for touch long press
	infos = append(infos, &gestureInfo{
		Event: EventInfo{
			Name:      "touch right button",
			Direction: "down",
			Fingers:   0,
		},
		Action: ActionInfo{
			Type:   ActionTypeCommandline,
			Action: "xdotool mousedown 3",
		},
	})
	infos = append(infos, &gestureInfo{
		Event: EventInfo{
			Name:      "touch right button",
			Direction: "up",
			Fingers:   0,
		},
		Action: ActionInfo{
			Type:   ActionTypeCommandline,
			Action: "xdotool mouseup 3",
		},
	})

	setting, err := dutils.CheckAndNewGSettings(gestureSchemaId)
	if err != nil {
		return nil, err
	}

	tsSetting, err := dutils.CheckAndNewGSettings(tsSchemaID)
	if err != nil {
		return nil, err
	}

	m := &Manager{
		userFile:           configUserPath,
		Infos:              infos,
		setting:            setting,
		tsSetting:          tsSetting,
		touchPadEnabled:    setting.GetBoolean(gsKeyTouchPadEnabled),
		touchScreenEnabled: setting.GetBoolean(gsKeyTouchScreenEnabled),
		wm:                 wm.NewWm(sessionConn),
		dock:               dock.NewDock(sessionConn),
		display:            display.NewDisplay(sessionConn),
		sysDaemon:          daemon.NewDaemon(systemConn),
		sessionmanager:     sessionmanager.NewSessionManager(sessionConn),
		clipboard:          clipboard.NewClipboard(sessionConn),
		notification:       notification.NewNotification(sessionConn),
	}

	systemConnObj := systemConn.Object(configManagerId, "/")
	err = systemConnObj.Call(configManagerId+".acquireManager", 0, "org.deepin.dde.daemon", "org.deepin.dde.daemon.gesture", "").Store(&m.configManagerPath)
	if err != nil {
		logger.Warning(err)
	}
	m.longPressEnable = m.getGestureConfigValue("longPressEnable")
	m.oneFingerBottomEnable = m.getGestureConfigValue("oneFingerBottomEnable")
	m.oneFingerLeftEnable = m.getGestureConfigValue("oneFingerLeftEnable")
	m.oneFingerRightEnable = m.getGestureConfigValue("oneFingerRightEnable")

	if _useWayland {
		setLongPressEnable(m.longPressEnable)
	}

	m.gesture = gesture.NewGesture(systemConn)
	m.systemSigLoop = dbusutil.NewSignalLoop(systemConn, 10)

	if _useWayland {
		m.sessionWatcher = sessionwatcher.NewSessionWatcher(sessionConn)
	}
	return m, nil
}

func setLongPressEnable(enable bool) {
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}
	Obj := sessionBus.Object("org.kde.KWin", "/KWin")
	err = Obj.Call("org.kde.KWin.setEnableTouchLongPress", 0, enable).Err
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) getGestureConfigValue(key string) bool {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		return true
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", m.configManagerPath)
	var val bool
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, key).Store(&val)
	if err != nil {
		logger.Warning(err)
		return true
	}
	return val
}

func (m *Manager) destroy() {
	m.gesture.RemoveHandler(proxy.RemoveAllHandlers)
	m.systemSigLoop.Stop()
	m.setting.Unref()
}

func (m *Manager) init() {
	m.initBuiltinSets()
	err := m.sysDaemon.SetLongPressDuration(0, uint32(m.tsSetting.GetInt(tsSchemaKeyLongPress)))
	if err != nil {
		logger.Warning("call SetLongPressDuration failed:", err)
	}
	err = m.gesture.SetShortPressDuration(0, uint32(m.tsSetting.GetInt(tsSchemaKeyShortPress)))
	if err != nil {
		logger.Warning("call SetShortPressDuration failed:", err)
	}
	err = m.gesture.SetEdgeMoveStopDuration(0, uint32(m.tsSetting.GetInt(tsSchemaKeyEdgeMoveStop)))
	if err != nil {
		logger.Warning("call SetEdgeMoveStopDuration failed:", err)
	}

	systemConn, err := dbus.SystemBus()
	if err != nil {
		logger.Error(err)
	}
	err = dbusutil.NewMatchRuleBuilder().Type("signal").
		PathNamespace(string(m.configManagerPath)).
		Interface("org.desktopspec.ConfigManager.Manager").
		Member("valueChanged").Build().AddTo(systemConn)
	if err != nil {
		logger.Warning(err)
	}

	m.systemSigLoop.Start()
	m.gesture.InitSignalExt(m.systemSigLoop, true)
	_, err = m.gesture.ConnectEvent(func(name string, direction string, fingers int32) {
		should, err := m.shouldHandleEvent(deviceTouchPad)
		if err != nil {
			logger.Error("shouldHandleEvent failed:", err)
			return
		}
		if !should {
			return
		}

		err = m.Exec(EventInfo{
			Name:      name,
			Direction: direction,
			Fingers:   fingers,
		})
		if err != nil {
			logger.Error("Exec failed:", err)
		}
	})

	m.systemSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.desktopspec.ConfigManager.Manager.valueChanged",
	}, func(sig *dbus.Signal) {
		if strings.Contains(sig.Name, "org.desktopspec.ConfigManager.Manager.valueChanged") &&
			strings.Contains(string(sig.Path), "org_deepin_dde_daemon_gesture") && len(sig.Body) >= 1 {
			key, ok := sig.Body[0].(string)
			if !ok {
				logger.Warning("Get key of body failed.")
				return
			}
			switch key {
			case "oneFingerBottomEnable":
				m.oneFingerBottomEnable = m.getGestureConfigValue("oneFingerBottomEnable")
				logger.Info("DConfig of oneFingerBottomEnable : ", m.oneFingerBottomEnable)
			case "longPressEnable":
				m.longPressEnable = m.getGestureConfigValue("longPressEnable")
				logger.Info("DConfig of longPressEnable : ", m.longPressEnable)
				if _useWayland {
					setLongPressEnable(m.longPressEnable)
				}
			case "oneFingerLeftEnable":
				m.oneFingerLeftEnable = m.getGestureConfigValue("oneFingerLeftEnable")
				logger.Info("DConfig of oneFingerLeftEnable : ", m.oneFingerLeftEnable)
			case "oneFingerRightEnable":
				m.oneFingerRightEnable = m.getGestureConfigValue("oneFingerRightEnable")
				logger.Info("DConfig of oneFingerRightEnable : ", m.oneFingerRightEnable)
			default:
				logger.Warning("Not use key : ", key)
			}
		}
	})

	if err != nil {
		logger.Error("connect gesture event failed:", err)
	}

	_, err = m.gesture.ConnectTouchEdgeMoveStopLeave(func(direction string, scaleX float64, scaleY float64, duration int32) {
		should, err := m.shouldHandleEvent(deviceTouchScreen)
		if err != nil {
			logger.Error("shouldHandleEvent failed:", err)
			return
		}
		if !should {
			return
		}

		context, pointFn, err := m.getTouchScreenRotationContext()
		if err != nil {
			logger.Error("getTouchScreenRotationContext failed:", err)
		}
		p := &point{X: scaleX, Y: scaleY}
		pointFn(p)

		err = m.handleTouchEdgeMoveStopLeave(context, direction, p, duration)
		if err != nil {
			logger.Error("handleTouchEdgeMoveStopLeave failed:", err)
		}
	})
	if err != nil {
		logger.Error("connect TouchEdgeMoveStopLeave failed:", err)
	}

	_, err = m.gesture.ConnectTouchEdgeEvent(func(direction string, scaleX float64, scaleY float64) {
		should, err := m.shouldHandleEvent(deviceTouchScreen)
		if err != nil {
			logger.Error("shouldHandleEvent failed:", err)
			return
		}
		if !should {
			return
		}
		context, pointFn, err := m.getTouchScreenRotationContext()
		if err != nil {
			logger.Error("getTouchScreenRotationContext failed:", err)
		}
		p := &point{X: scaleX, Y: scaleY}
		pointFn(p)
		err = m.handleTouchEdgeEvent(context, direction, p)
		if err != nil {
			logger.Error("handleTouchEdgeEvent failed:", err)
		}
	})
	if err != nil {
		logger.Error("connect handleTouchEdgeEvent failed:", err)
	}

	_, err = m.gesture.ConnectTouchMovementEvent(func(direction string, fingers int32, startScaleX float64, startScaleY float64, endScaleX float64, endScaleY float64) {
		should, err := m.shouldHandleEvent(deviceTouchScreen)
		if err != nil {
			logger.Error("shouldHandleEvent failed:", err)
			return
		}
		if !should {
			return
		}

		context, pointFn, err := m.getTouchScreenRotationContext()
		if err != nil {
			logger.Error("getTouchScreenRotationContext failed:", err)
		}

		startP := &point{X: startScaleX, Y: startScaleY}
		endP := &point{X: endScaleX, Y: endScaleY}
		pointFn(startP)
		pointFn(endP)

		err = m.handleTouchMovementEvent(context, direction, fingers, startP, endP)
		if err != nil {
			logger.Error("handleTouchMovementEvent failed:", err)
		}
	})
	if err != nil {
		logger.Error("connect handleTouchMovementEvent failed:", err)
	}
	m.listenGSettingsChanged()
}

func (m *Manager) shouldIgnoreGesture(info *gestureInfo) bool {
	// allow right button up when kbd grabbed
	if (info.Event.Name != "touch right button" || info.Event.Direction != "up") && isKbdAlreadyGrabbed() {
		// 多任务窗口下，不应该忽略手势操作
		isShowMultiTask, err := m.wm.GetMultiTaskingStatus(0)
		if err != nil {
			logger.Warning(err)
		} else if isShowMultiTask && info.Event.Name == "swipe" {
			logger.Debug("should not ignore swipe event, because we are in multi task")
			return false
		}
		logger.Debug("another process grabbed keyboard, not exec action")
		return true
	}

	// TODO(jouyouyun): improve touch right button handler
	if info.Event.Name == "touch right button" {
		// filter google chrome
		if isInWindowBlacklist(getCurrentActionWindowCmd(), m.tsSetting.GetStrv(tsSchemaKeyBlacklist)) {
			logger.Debug("the current active window in blacklist")
			return true
		}
	} else if strings.HasPrefix(info.Event.Name, "touch") {
		return true
	}

	return false
}

func (m *Manager) Exec(evInfo EventInfo) error {
	if _useWayland {
		if !isSessionActive("/org/freedesktop/login1/session/self") {
			active, err := m.sessionWatcher.IsActive().Get(0)
			if err != nil || !active {
				logger.Debug("Gesture had been disabled or session inactive")
				return nil
			}
		}
	}

	info := m.Infos.Get(evInfo)
	if info == nil {
		logger.Infof("[Exec]: not found event info: %s", evInfo.toString())
		return nil
	}

	logger.Debugf("[Exec]: event info:%s  action info:%s", info.Event.toString(), info.Action.toString())
	if m.shouldIgnoreGesture(info) {
		return nil
	}

	if (!m.longPressEnable || _useWayland) && strings.Contains(string(info.Event.Name), "touch right button") {
		return nil
	}

	var cmd = info.Action.Action
	switch info.Action.Type {
	case ActionTypeCommandline:
		break
	case ActionTypeShortcut:
		cmd = fmt.Sprintf("xdotool key %s", cmd)
	case ActionTypeBuiltin:
		return m.handleBuiltinAction(cmd)
	default:
		return fmt.Errorf("invalid action type: %s", info.Action.Type)
	}

	// #nosec G204
	out, err := exec.Command("/bin/sh", "-c", cmd).CombinedOutput()
	if err != nil {
		return fmt.Errorf("%s", string(out))
	}
	return nil
}

func (m *Manager) Write() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	// #nosec G301
	err := os.MkdirAll(filepath.Dir(m.userFile), 0755)
	if err != nil {
		return err
	}
	data, err := json.Marshal(m.Infos)
	if err != nil {
		return err
	}
	// #nosec G306
	return ioutil.WriteFile(m.userFile, data, 0644)
}

func (m *Manager) listenGSettingsChanged() {
	gsettings.ConnectChanged(gestureSchemaId, gsKeyTouchPadEnabled, func(key string) {
		m.mu.Lock()
		m.touchPadEnabled = m.setting.GetBoolean(key)
		m.mu.Unlock()
	})

	gsettings.ConnectChanged(gestureSchemaId, gsKeyTouchScreenEnabled, func(key string) {
		m.mu.Lock()
		m.touchScreenEnabled = m.setting.GetBoolean(key)
		m.mu.Unlock()
	})
}

func (m *Manager) handleBuiltinAction(cmd string) error {
	fn := m.builtinSets[cmd]
	if fn == nil {
		return fmt.Errorf("invalid built-in action %q", cmd)
	}
	return fn()
}

func (*Manager) GetInterfaceName() string {
	return dbusServiceIFC
}

type TouchScreensRotation uint16

// counterclockwise
const (
	Normal       TouchScreensRotation = 1
	Rotation_90  TouchScreensRotation = 2
	Rotation_180 TouchScreensRotation = 4
	Rotation_270 TouchScreensRotation = 8
)

// 获取触摸屏的旋转
func (m *Manager) getTouchScreenRotation() (display.Monitor, TouchScreensRotation) {
	// 读取触屏列表，取第一个触屏（目前触摸手势事件中不包含所属屏幕，因此不支持多个触摸屏）
	touchScreens, err := m.display.TouchscreensV2().Get(0)
	if err != nil {
		logger.Warning(err)
	}

	// 读取触摸屏映射
	touchMap, err := m.display.TouchMap().Get(0)
	if err != nil {
		logger.Warning(err)
	}

	// 读取触摸屏的名字
	var touchScreen string
	if len(touchScreens) > 0 && len(touchMap) > 0 {
		touchScreen = touchMap[touchScreens[0].UUID]
	}

	// 读取失败，把主屏当做触摸屏
	if touchScreen == "" {
		logger.Warning("failed to find the touch screen, assume the primary as the touch screen")
		touchScreen, err = m.display.Primary().Get(0)
		if err != nil {
			logger.Warning(err)
		}
	}

	// 遍历显示器，查找触摸屏的旋转角度
	monitors, err := m.display.Monitors().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return nil, Normal
	}
	for _, path := range monitors {
		monitor, err := display.NewMonitor(sessionBus, path)
		if err != nil {
			logger.Warning(err)
			continue
		}

		name, err := monitor.Name().Get(0)
		if err != nil {
			logger.Warning(err)
			continue
		}

		if name == touchScreen {
			rotation, err := monitor.Rotation().Get(0)
			if err != nil {
				logger.Warning(err)
				break
			}
			return monitor, TouchScreensRotation(rotation)
		}
	}

	// 查找失败，当做没有旋转
	return nil, Normal
}

// struct point represents a point on a touchScreen
// X is a float64 in [0,1], which is the horizontal index
// Y is a float64 in [0,1], which is the vertical index
// left-top corner represents in struct point is{X:0,Y:0}
type point struct {
	X float64
	Y float64
}

// struct touchEventContext is a struct try to handle the context of touchScreen Gesture after rotation
// for example after Rotation_90 context.top is "left", and context.screenHeight is always the vertical height of screen
// see func getTouchScreenRotationContext for details
type touchEventContext struct {
	top, bot, left, right     string
	screenWidth, screenHeight uint16
}

// func getTouchScreenRotationContext return a context represents the current touchScreen's rotation, and a func to transform point
func (m *Manager) getTouchScreenRotationContext() (context *touchEventContext, pointTransformFn func(*point), err error) {
	monitor, rotation := m.getTouchScreenRotation()

	var screenWidth, screenHeight uint16
	if monitor == nil { // 如果获取失败则当作用户只有一个显示屏, 直接使用 x 的画布大小当作触摸屏大小
		screenWidth, err = m.display.ScreenWidth().Get(0)
		if err != nil {
			logger.Error("get display.ScreenWidth failed:", err)
			return
		}
		screenHeight, err = m.display.ScreenHeight().Get(0)
		if err != nil {
			logger.Error("get display.ScreenWidth failed:", err)
			return
		}
	} else {
		screenWidth, err = monitor.Width().Get(0)
		if err != nil {
			logger.Error("get monitor.Width failed:", err)
			return
		}
		screenHeight, err = monitor.Height().Get(0)
		if err != nil {
			logger.Error("get monitor.Height failed:", err)
			return
		}
	}

	pointFn := func(p *point) {}
	top, bot, left, right := "top", "bot", "left", "right"
	switch rotation {
	case Rotation_90:
		top, bot, left, right = "left", "right", "bot", "top"
		screenHeight, screenWidth = screenWidth, screenHeight
		pointFn = func(p *point) {
			p.X, p.Y = 1-p.Y, p.X
		}
	case Rotation_180:
		top, bot, left, right = "bot", "top", "right", "left"
		pointFn = func(p *point) {
			p.X, p.Y = 1-p.X, 1-p.Y
		}
	case Rotation_270:
		top, bot, left, right = "right", "left", "top", "bot"
		screenHeight, screenWidth = screenWidth, screenHeight
		pointFn = func(p *point) {
			p.X, p.Y = p.Y, 1-p.X
		}
	}
	context = &touchEventContext{
		screenWidth:  screenWidth,
		screenHeight: screenHeight,
		top:          top,
		bot:          bot,
		left:         left,
		right:        right,
	}
	pointTransformFn = pointFn
	return
}

// param @edge: swipe to touchscreen edge
// edge: 该手势来自屏幕的哪条边
// p:    该手势的终点
func (m *Manager) handleTouchEdgeMoveStopLeave(context *touchEventContext, edge string, p *point, duration int32) error {
	logger.Debugf("handleTouchEdgeMoveStopLeave: context:%+v edge:%s p: %+v", *context, edge, *p)

	if edge == context.bot && m.oneFingerBottomEnable {
		position, err := m.dock.Position().Get(0)
		if err != nil {
			logger.Error("get dock.Position failed:", err)
			return err
		}

		if position >= 0 {
			rect, err := m.dock.FrontendWindowRect().Get(0)
			if err != nil {
				logger.Error("get dock.FrontendWindowRect failed:", err)
				return err
			}

			var dockPly uint32 = 0
			if position == positionTop || position == positionBottom {
				dockPly = rect.Height
			} else if position == positionRight || position == positionLeft {
				dockPly = rect.Width
			}

			if (1-p.Y)*float64(context.screenHeight) > float64(dockPly) {
				logger.Debug("show work space")
				return m.handleBuiltinAction("ShowWorkspace")
			}
		}
	}
	return nil
}

func (m *Manager) showWidgets(show bool) error {
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return err
	}
	obj := sessionBus.Object("org.deepin.dde.Widgets", "/org/deepin/dde/Widgets")
	if show {
		err = obj.Call("org.deepin.dde.Widgets.Show", 0).Err
	} else {
		err = obj.Call("org.deepin.dde.Widgets.Hide", 0).Err
	}
	if err != nil {
		logger.Warning(err)
	}
	return err
}

// edge: 该手势来自屏幕的哪条边
// p:    该手势的终点
func (m *Manager) handleTouchEdgeEvent(context *touchEventContext, edge string, p *point) error {
	logger.Debugf("handleTouchEdgeEvent: context:%+v edge:%s p:%+v", *context, edge, *p)
	switch edge {
	case context.left:
		if p.X*float64(context.screenHeight) > 100 && m.oneFingerLeftEnable {
			return m.clipboard.Show(0)
		}
	case context.right:
		if (1-p.X)*float64(context.screenWidth) > 100 && m.oneFingerRightEnable {
			return m.showWidgets(true)
		}
	}
	return nil
}

// direction: 该手势的方向
// fingers:   手指的数量
// startP:    该手势的起点
// endP:      该手势的终点
func (m *Manager) handleTouchMovementEvent(context *touchEventContext, direction string, fingers int32, startP *point, endP *point) error {
	logger.Debugf("handleTouchMovementEvent: context:%+v direction:%s startP:%+v endP:%+v", *context, direction, *startP, *endP)

	if fingers == 1 {
		// sensitivity check
		// TODO maybe write a function for this
		sensitivityThreshold := 0.05

		if math.Abs(startP.X-endP.X) < sensitivityThreshold {
			logger.Debug("sensitivity check fail, gesture will not be triggered")
			return nil
		}

		switch direction {
		case context.left:
			if m.oneFingerLeftEnable {
				return m.clipboard.Hide(0)
			}
		case context.right:
			if m.oneFingerRightEnable {
				return m.showWidgets(false)
			}
		}
	}

	return nil
}

// touchpad double click down
func (m *Manager) handleDbclickDown(fingers int32) error {
	if fingers == 3 {
		return m.wm.TouchToMove(0, 0, 0)
	}
	return nil
}

// touchpad swipe move
func (m *Manager) handleSwipeMoving(fingers int32, accelX float64, accelY float64) error {
	if fingers == 3 {
		return m.wm.TouchToMove(0, int32(accelX), int32(accelY))
	}
	return nil
}

// touchpad swipe stop or interrupted
func (m *Manager) handleSwipeStop(fingers int32) error {
	if fingers == 3 {
		return m.wm.ClearMoveStatus(0)
	}
	return nil
}

// 多用户存在，防止非当前用户响应触摸屏手势
func (m *Manager) shouldHandleEvent(devType deviceType) (bool, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	switch devType {
	case deviceTouchPad:
		if !m.touchPadEnabled {
			logger.Debug("touch pad is disabled, do not handle touchpad gesture event")
			return false, nil
		}
	case deviceTouchScreen:
		if !m.touchScreenEnabled {
			logger.Debug("touch screen is disabled, do not handle touchscreen gesture event")
			return false, nil
		}
	default:
		logger.Warningf("Unknown device type: %v, do not handle gesture event", devType)
		return false, nil
	}

	isLocked, err := m.sessionmanager.Locked().Get(0)
	if err != nil {
		return false, fmt.Errorf("get login1 session locked failed: %v", err)
	}

	if isLocked {
		return false, nil
	}

	currentSessionPath, err := m.sessionmanager.CurrentSessionPath().Get(0)
	if err != nil {
		return false, fmt.Errorf("get login1 session path failed: %v", err)
	}

	if !isSessionActive(currentSessionPath) {
		return false, nil
	}

	return true, nil
}
