// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package x_event_monitor

import (
	"encoding/json"
	"errors"
	"fmt"
	"sync"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/strv"
	dutils "github.com/linuxdeepin/go-lib/utils"
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/ge"
	"github.com/linuxdeepin/go-x11-client/ext/input"
	"github.com/linuxdeepin/go-x11-client/ext/xfixes"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

//go:generate dbusutil-gen em -type Manager

const fullscreenId = "d41d8cd98f00b204e9800998ecf8427e"

var errAreasRegistered = errors.New("the areas has been registered")
var errAreasNotRegistered = errors.New("the areas has not been registered yet")

type coordinateInfo struct {
	areas        []coordinateRange
	moveIntoFlag bool
	motionFlag   bool //是否发送鼠标在区域中移动的信号
	buttonFlag   bool
	keyFlag      bool
}

type coordinateRange struct {
	X1 int32
	Y1 int32
	X2 int32
	Y2 int32
}

type Manager struct {
	hideCursorWhenTouch bool
	cursorShowed        bool
	xConn               *x.Conn
	keySymbols          *keysyms.KeySymbols
	service             *dbusutil.Service
	sessionSigLoop      *dbusutil.SignalLoop
	//nolint
	signals *struct {
		CancelAllArea                     struct{}
		CursorInto, CursorOut, CursorMove struct {
			x, y int32
			id   string
		}
		ButtonPress, ButtonRelease struct {
			button, x, y int32
			id           string
		}
		KeyPress, KeyRelease struct {
			key  string
			x, y int32
			id   string
		}
		CursorShowAgain struct{}
	}

	pidAidsMap            map[uint32]strv.Strv
	idAreaInfoMap         map[string]*coordinateInfo
	idReferCountMap       map[string]int32
	fullscreenMotionCount int32
	cursorMask            uint32

	mu sync.Mutex

	CursorX int32
	CursorY int32
}

const (
	buttonLeft  = 272
	buttonRight = 273
	leftBit     = 0
	rightBit    = 1
	midBit      = 2
	x11BtnLeft  = 1
	x11BtnRight = 3
	x11BtnMid   = 2
)

func newManager(service *dbusutil.Service) (*Manager, error) {
	xConn, err := x.NewConn()
	if err != nil {
		return nil, err
	}
	keySymbols := keysyms.NewKeySymbols(xConn)
	m := &Manager{
		xConn:               xConn,
		hideCursorWhenTouch: true,
		cursorShowed:        true,
		keySymbols:          keySymbols,
		service:             service,
		pidAidsMap:          make(map[uint32]strv.Strv),
		idAreaInfoMap:       make(map[string]*coordinateInfo),
		idReferCountMap:     make(map[string]int32),
	}
	sessionBus := m.service.Conn()
	m.sessionSigLoop = dbusutil.NewSignalLoop(sessionBus, 10)
	m.sessionSigLoop.Start()
	m.cursorMask = 0
	return m, nil
}

func (m *Manager) queryPointer() (*x.QueryPointerReply, error) {
	root := m.xConn.GetDefaultScreen().Root
	reply, err := x.QueryPointer(m.xConn, root).Reply(m.xConn)
	return reply, err
}

func (m *Manager) selectXInputEvents() {
	logger.Debug("select input events")
	var evMask uint32 = input.XIEventMaskRawMotion |
		input.XIEventMaskRawButtonPress |
		input.XIEventMaskRawButtonRelease |
		input.XIEventMaskRawKeyPress |
		input.XIEventMaskRawKeyRelease |
		input.XIEventMaskRawTouchBegin |
		input.XIEventMaskRawTouchEnd
	err := m.doXISelectEvents(evMask)
	if err != nil {
		logger.Warning(err)
	}
}

const evMaskForHideCursor uint32 = input.XIEventMaskRawMotion | input.XIEventMaskRawTouchBegin

func (m *Manager) listenGlobalCursorPressed() error {
	sessionBus := m.service.Conn()
	logger.Debug("[test global key] sessionBus", sessionBus)
	err := sessionBus.Object("org.deepin.dde.KWayland1",
		"/org/deepin/dde/KWayland1/Output").AddMatchSignal("org.deepin.dde.KWayland1.Output", "ButtonPress").Err
	if err != nil {
		logger.Warning(err)
		return err
	}

	m.sessionSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.deepin.dde.KWayland1.Output.ButtonPress",
	}, func(sig *dbus.Signal) {
		if len(sig.Body) > 1 {
			key := sig.Body[0].(uint32)
			x := sig.Body[1].(uint32)
			y := sig.Body[2].(uint32)
			m.CursorX = int32(x)
			m.CursorY = int32(y)
			if key == buttonLeft || key == 0 {
				m.cursorMask |= uint32(uint32(1) << leftBit)
				key = x11BtnLeft
			} else if key == buttonRight {
				m.cursorMask |= uint32(uint32(1) << rightBit)
				key = x11BtnRight
			} else {
				m.cursorMask |= uint32(uint32(1) << midBit)
				key = x11BtnMid
			}
			m.handleButtonEvent(int32(key), true, int32(x), int32(y))
		}
	})
	return nil
}

func (m *Manager) listenGlobalCursorRelease() error {
	sessionBus := m.service.Conn()
	err := sessionBus.Object("org.deepin.dde.KWayland1",
		"/org/deepin/dde/KWayland1/Output").AddMatchSignal("org.deepin.dde.KWayland1.Output", "ButtonRelease").Err
	if err != nil {
		logger.Warning(err)
		return err
	}

	m.sessionSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.deepin.dde.KWayland1.Output.ButtonRelease",
	}, func(sig *dbus.Signal) {
		if len(sig.Body) > 1 {
			key := sig.Body[0].(uint32)
			x := sig.Body[1].(uint32)
			y := sig.Body[2].(uint32)
			m.CursorX = int32(x)
			m.CursorY = int32(y)

			if key == buttonLeft || key == 0 {
				m.cursorMask &= ^(uint32(1) << leftBit)
				key = x11BtnLeft
			} else if key == buttonRight {
				m.cursorMask &= ^(uint32(1) << rightBit)
				key = x11BtnRight
			} else {
				m.cursorMask &= ^(uint32(1) << midBit)
				key = x11BtnMid
			}
			m.handleButtonEvent(int32(key), false, int32(x), int32(y))
		}
	})
	return nil
}

func (m *Manager) listenGlobalCursorMove() error {
	sessionBus := m.service.Conn()
	err := sessionBus.Object("org.deepin.dde.KWayland1",
		"/org/deepin/dde/KWayland1/Output").AddMatchSignal("org.deepin.dde.KWayland1.Output", "CursorMove").Err
	if err != nil {
		logger.Warning(err)
		return err
	}

	m.sessionSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.deepin.dde.KWayland1.Output.CursorMove",
	}, func(sig *dbus.Signal) {
		if len(sig.Body) > 1 {
			x := sig.Body[0].(uint32)
			y := sig.Body[1].(uint32)
			m.CursorX = int32(x)
			m.CursorY = int32(y)

			//m.cursorMask |= (1 << cursorBit)
			var hasPress = false
			if m.cursorMask > 0 {
				hasPress = true
			}

			//logger.Debug("[test global cursor] get CursorMove", x, y)
			m.handleCursorEvent(int32(x), int32(y), hasPress)
		}
	})
	return nil
}

func (m *Manager) deselectXInputEvents() {
	var evMask uint32
	if m.hideCursorWhenTouch {
		evMask = evMaskForHideCursor
	}

	logger.Debug("deselect input events")
	err := m.doXISelectEvents(evMask)
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) doXISelectEvents(evMask uint32) error {
	root := m.xConn.GetDefaultScreen().Root
	err := input.XISelectEventsChecked(m.xConn, root, []input.EventMask{
		{
			DeviceId: input.DeviceAllMaster,
			Mask:     []uint32{evMask},
		},
	}).Check(m.xConn)
	return err
}

func (m *Manager) BeginTouch() *dbus.Error {
	m.beginTouch()
	return nil
}

func (m *Manager) beginMoveMouse() {
	if m.cursorShowed {
		return
	}
	err := m.doShowCursor(true)
	if err != nil {
		logger.Warning(err)
	}
	m.cursorShowed = true
	err = m.service.Emit(m, "CursorShowAgain")
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) beginTouch() {
	if !m.cursorShowed {
		return
	}
	err := m.doShowCursor(false)
	if err != nil {
		logger.Warning(err)
	}
	m.cursorShowed = false
}

func (m *Manager) doShowCursor(show bool) error {
	rootWin := m.xConn.GetDefaultScreen().Root
	var cookie x.VoidCookie
	if show {
		logger.Debug("xfixes show cursor")
		cookie = xfixes.ShowCursorChecked(m.xConn, rootWin)
	} else {
		logger.Debug("xfixes hide cursor")
		cookie = xfixes.HideCursorChecked(m.xConn, rootWin)
	}
	err := cookie.Check(m.xConn)
	return err
}

func (m *Manager) initXExtensions() {
	_, err := xfixes.QueryVersion(m.xConn, xfixes.MajorVersion, xfixes.MinorVersion).Reply(m.xConn)
	if err != nil {
		logger.Warning(err)
	}

	_, err = ge.QueryVersion(m.xConn, ge.MajorVersion, ge.MinorVersion).Reply(m.xConn)
	if err != nil {
		logger.Warning(err)
		return
	}

	_, err = input.XIQueryVersion(m.xConn, input.MajorVersion, input.MinorVersion).Reply(m.xConn)
	if err != nil {
		logger.Warning(err)
		return
	}

	if m.hideCursorWhenTouch {
		err = m.doXISelectEvents(evMaskForHideCursor)
		if err != nil {
			logger.Warning(err)
		}
	}
}

func (m *Manager) handleXEvent() {
	eventChan := make(chan x.GenericEvent, 10)
	m.xConn.AddEventChan(eventChan)
	inputExtData := m.xConn.GetExtensionData(input.Ext())

	for ev := range eventChan {
		switch ev.GetEventCode() {
		case x.MappingNotifyEventCode:
			logger.Debug("mapping notify event")
			event, _ := x.NewMappingNotifyEvent(ev)
			m.keySymbols.RefreshKeyboardMapping(event)

		case x.GeGenericEventCode:
			geEvent, _ := x.NewGeGenericEvent(ev)
			if geEvent.Extension == inputExtData.MajorOpcode {
				switch geEvent.EventType {
				case input.RawMotionEventCode:
					//logger.Debug("raw motion event")
					if m.hideCursorWhenTouch {
						m.beginMoveMouse()
					}
					m.mu.Lock()
					_, ok := m.idReferCountMap[fullscreenId]
					m.mu.Unlock()
					if len(m.idAreaInfoMap) == 0 && !ok {
						break
					}
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						/**
						mouse left press: mask = 256
						mouse right press: mask = 512
						mouse middle press: mask = 1024
						 **/

						var press bool
						if qpReply.Mask >= 256 {
							press = true
						}
						m.handleCursorEvent(int32(qpReply.RootX), int32(qpReply.RootY), press)
					}

				case input.RawKeyPressEventCode:
					e, _ := input.NewRawKeyPressEvent(geEvent.Data)
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						m.handleKeyboardEvent(int32(e.Detail), true, int32(qpReply.RootX),
							int32(qpReply.RootY))
					}
				case input.RawKeyReleaseEventCode:
					e, _ := input.NewRawKeyReleaseEvent(geEvent.Data)
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						m.handleKeyboardEvent(int32(e.Detail), false, int32(qpReply.RootX),
							int32(qpReply.RootY))
					}

				case input.RawButtonPressEventCode:
					e, _ := input.NewRawButtonPressEvent(geEvent.Data)
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						m.handleButtonEvent(int32(e.Detail), true, int32(qpReply.RootX),
							int32(qpReply.RootY))
					}

				case input.RawButtonReleaseEventCode:
					e, _ := input.NewRawButtonReleaseEvent(geEvent.Data)
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						m.handleButtonEvent(int32(e.Detail), false, int32(qpReply.RootX),
							int32(qpReply.RootY))
					}

				case input.RawTouchBeginEventCode:
					//logger.Debug("raw touch begin event")
					if m.hideCursorWhenTouch {
						m.beginTouch()
					}
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						m.handleButtonEvent(1, true, int32(qpReply.RootX),
							int32(qpReply.RootY))
					}

				case input.RawTouchEndEventCode:
					qpReply, err := m.queryPointer()
					if err != nil {
						logger.Warning(err)
					} else {
						m.handleButtonEvent(1, false, int32(qpReply.RootX),
							int32(qpReply.RootY))
					}
				}
			}
		}
	}
}

func (m *Manager) handleCursorEvent(x, y int32, press bool) {
	m.mu.Lock()
	defer m.mu.Unlock()

	press = !press
	inList, outList := m.getIdList(x, y)
	for _, id := range inList {
		areaInfo, ok := m.idAreaInfoMap[id]
		if !ok {
			continue
		}

		if !areaInfo.moveIntoFlag {
			if press {
				err := m.service.Emit(m, "CursorInto", x, y, id)
				if err != nil {
					logger.Warning(err)
				}
				areaInfo.moveIntoFlag = true
			}
		}

		if areaInfo.motionFlag {
			err := m.service.Emit(m, "CursorMove", x, y, id)
			if err != nil {
				logger.Warning(err)
			}
		}
	}

	for _, id := range outList {
		areaInfo, ok := m.idAreaInfoMap[id]
		if !ok {
			continue
		}

		if areaInfo.moveIntoFlag {
			err := m.service.Emit(m, "CursorOut", x, y, id)
			if err != nil {
				logger.Warning(err)
			}
			areaInfo.moveIntoFlag = false
		}
	}

	_, ok := m.idReferCountMap[fullscreenId]
	if ok && m.fullscreenMotionCount > 0 {
		err := m.service.Emit(m, "CursorMove", x, y, fullscreenId)
		if err != nil {
			logger.Warning(err)
		}
	}
}

func (m *Manager) handleButtonEvent(button int32, press bool, x, y int32) {
	m.mu.Lock()
	defer m.mu.Unlock()

	list, _ := m.getIdList(x, y)
	for _, id := range list {
		array, ok := m.idAreaInfoMap[id]
		if !ok || !array.buttonFlag {
			continue
		}

		if press {
			err := m.service.Emit(m, "ButtonPress", button, x, y, id)
			if err != nil {
				logger.Warning("Emit error:", err)
			}
		} else {
			err := m.service.Emit(m, "ButtonRelease", button, x, y, id)
			if err != nil {
				logger.Warning("Emit error:", err)
			}
		}
	}

	_, ok := m.idReferCountMap[fullscreenId]
	if !ok {
		return
	}

	if press {
		err := m.service.Emit(m, "ButtonPress", button, x, y, fullscreenId)
		if err != nil {
			logger.Warning("Emit error:", err)
		}
	} else {
		err := m.service.Emit(m, "ButtonRelease", button, x, y, fullscreenId)
		if err != nil {
			logger.Warning("Emit error:", err)
		}
	}
}

func (m *Manager) keyCode2Str(key int32) string {
	str, _ := m.keySymbols.LookupString(x.Keycode(key), 0)
	return str
}

func (m *Manager) handleKeyboardEvent(code int32, press bool, x, y int32) {
	m.mu.Lock()
	defer m.mu.Unlock()

	list, _ := m.getIdList(x, y)
	for _, id := range list {
		array, ok := m.idAreaInfoMap[id]
		if !ok || !array.keyFlag {
			continue
		}

		if press {
			err := m.service.Emit(m, "KeyPress", m.keyCode2Str(code), x, y, id)
			if err != nil {
				logger.Warning("Emit error:", err)
			}
		} else {
			err := m.service.Emit(m, "KeyRelease", m.keyCode2Str(code), x, y, id)
			if err != nil {
				logger.Warning("Emit error:", err)
			}
		}
	}

	_, ok := m.idReferCountMap[fullscreenId]
	if ok {
		if press {
			err := m.service.Emit(m, "KeyPress", m.keyCode2Str(code), x, y,
				fullscreenId)
			if err != nil {
				logger.Warning("Emit error:", err)
			}
		} else {
			err := m.service.Emit(m, "KeyRelease", m.keyCode2Str(code), x, y,
				fullscreenId)
			if err != nil {
				logger.Warning("Emit error:", err)
			}
		}
	}
}

func (m *Manager) isPidAreaRegistered(pid uint32, areasId string) bool {
	areasIds := m.pidAidsMap[pid]
	return areasIds.Contains(areasId)
}

func (m *Manager) registerPidArea(pid uint32, areasId string) {
	areasIds := m.pidAidsMap[pid]
	areasIds, _ = areasIds.Add(areasId)
	m.pidAidsMap[pid] = areasIds

	m.selectXInputEvents()
}

func (m *Manager) unregisterPidArea(pid uint32, areasId string) {
	areasIds := m.pidAidsMap[pid]
	areasIds, _ = areasIds.Delete(areasId)
	if len(areasIds) > 0 {
		m.pidAidsMap[pid] = areasIds
	} else {
		delete(m.pidAidsMap, pid)
	}

	if len(m.pidAidsMap) == 0 {
		m.deselectXInputEvents()
	}
}

func (m *Manager) RegisterArea(sender dbus.Sender, x1, y1, x2, y2, flag int32) (id string, busErr *dbus.Error) {
	return m.RegisterAreas(sender,
		[]coordinateRange{{x1, y1, x2, y2}},
		flag)
}

func (m *Manager) RegisterAreas(sender dbus.Sender, areas []coordinateRange, flag int32) (id string, busErr *dbus.Error) {
	md5Str, ok := m.sumAreasMd5(areas, flag)
	if !ok {
		busErr = dbusutil.ToError(fmt.Errorf("sumAreasMd5 failed: %v", areas))
		return
	}
	id = md5Str
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		busErr = dbusutil.ToError(err)
		return
	}
	logger.Debugf("RegisterAreas id %q pid %d", id, pid)

	m.mu.Lock()
	defer m.mu.Unlock()

	if m.isPidAreaRegistered(pid, id) {
		logger.Warningf("RegisterAreas id %q pid %d failed: %v", id, pid, errAreasRegistered)
		return "", dbusutil.ToError(errAreasRegistered)
	}
	m.registerPidArea(pid, id)

	_, ok = m.idReferCountMap[id]
	if ok {
		m.idReferCountMap[id] += 1
		return id, nil
	}

	info := &coordinateInfo{}
	info.areas = areas
	info.motionFlag = hasMotionFlag(flag)
	info.buttonFlag = hasButtonFlag(flag)
	info.keyFlag = hasKeyFlag(flag)

	m.idAreaInfoMap[id] = info
	m.idReferCountMap[id] = 1

	return id, nil
}

func (m *Manager) RegisterFullScreen(sender dbus.Sender) (id string, busErr *dbus.Error) {
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		busErr = dbusutil.ToError(err)
		return
	}
	logger.Debugf("RegisterFullScreen pid %d", pid)

	m.mu.Lock()
	defer m.mu.Unlock()

	if m.isPidAreaRegistered(pid, fullscreenId) {
		logger.Warningf("RegisterFullScreen pid %d failed: %v", pid, errAreasRegistered)
		return "", dbusutil.ToError(errAreasRegistered)
	}

	_, ok := m.idReferCountMap[fullscreenId]
	if !ok {
		m.idReferCountMap[fullscreenId] = 1
	} else {
		m.idReferCountMap[fullscreenId] += 1
	}
	m.registerPidArea(pid, fullscreenId)
	return fullscreenId, nil
}

func (m *Manager) UnregisterArea(sender dbus.Sender, id string) (ok bool, busErr *dbus.Error) {
	m.mu.Lock()
	defer m.mu.Unlock()

	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		return false, dbusutil.ToError(err)
	}
	logger.Debugf("UnregisterArea id %q pid %d", id, pid)
	if !m.isPidAreaRegistered(pid, id) {
		logger.Warningf("UnregisterArea id %q pid %d failed: %v", id, pid, errAreasNotRegistered)
		return false, nil
	}

	m.unregisterPidArea(pid, id)

	_, ok1 := m.idReferCountMap[id]
	if !ok1 {
		logger.Warningf("not found key %q in idReferCountMap", id)
		return false, nil
	}

	m.idReferCountMap[id] -= 1
	if m.idReferCountMap[id] == 0 {
		delete(m.idReferCountMap, id)
		delete(m.idAreaInfoMap, id)
	}
	logger.Debugf("area %q unregistered by pid %d", id, pid)
	return true, nil
}

func (m *Manager) RegisterFullScreenMotionFlag(sender dbus.Sender) *dbus.Error {
	err := m.changeFullscreenMotionCount(true, sender)
	return dbusutil.ToError(err)
}

func (m *Manager) UnregisterFullScreenMotionFlag(sender dbus.Sender) *dbus.Error {
	err := m.changeFullscreenMotionCount(false, sender)
	return dbusutil.ToError(err)
}

func (m *Manager) changeFullscreenMotionCount(add bool, sender dbus.Sender) (err error) {
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		return
	}
	var (
		count    int32
		funcName string
	)
	if add {
		count = 1
		funcName = "RegisterFullScreenMotionFlag"
	} else {
		count = -1
		funcName = "UnregisterFullScreenMotionFlag"
	}
	logger.Debugf("%s pid %d", funcName, pid)

	m.mu.Lock()
	defer m.mu.Unlock()
	aidList, ok := m.pidAidsMap[pid]
	if !ok || !aidList.Contains(fullscreenId) {
		err = fmt.Errorf("%s err: pid %d hasn't registed fullscreen motion", funcName, pid)
		return
	}
	m.fullscreenMotionCount += count

	return nil
}

func (m *Manager) getIdList(x, y int32) ([]string, []string) {
	var inList []string
	var outList []string

	for id, array := range m.idAreaInfoMap {
		inFlag := false
		for _, area := range array.areas {
			if isInArea(x, y, area) {
				inFlag = true
				if !isInIdList(id, inList) {
					inList = append(inList, id)
				}
			}
		}
		if !inFlag {
			if !isInIdList(id, outList) {
				outList = append(outList, id)
			}
		}
	}

	return inList, outList
}

func (m *Manager) GetInterfaceName() string {
	return dbusInterface
}

func (m *Manager) sumAreasMd5(areas []coordinateRange, flag int32) (md5Str string, ok bool) {
	if len(areas) < 1 {
		return
	}

	content := ""
	for _, area := range areas {
		if len(content) > 1 {
			content += "-"
		}
		content += fmt.Sprintf("%v-%v-%v-%v", area.X1, area.Y1, area.X2, area.Y2)
	}
	content += fmt.Sprintf("-%v", flag)

	logger.Debug("areas content:", content)
	md5Str, ok = dutils.SumStrMd5(content)

	return
}

func (m *Manager) DebugGetPidAreasMap() (pidAreasMapJSON string, busErr *dbus.Error) {
	m.mu.Lock()
	data, err := json.Marshal(m.pidAidsMap)
	m.mu.Unlock()
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	return string(data), nil
}

func (m *Manager) listenGlobalAxisChanged() error {
	sessionBus := m.service.Conn()
	err := sessionBus.Object("org.deepin.dde.KWayland1",
		"/org/deepin/dde/KWayland1/Output").AddMatchSignal("org.deepin.dde.KWayland1.Output", "AxisChanged").Err
	if err != nil {
		logger.Warning(err)
		return err
	}

	m.sessionSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.deepin.dde.KWayland1.Output.AxisChanged",
	}, func(sig *dbus.Signal) {
		if len(sig.Body) > 1 {
			x := sig.Body[1].(float64)
			up := 4
			down := 5

			if x < 0 {
				m.handleButtonEvent(int32(up), true, m.CursorX, m.CursorY)
				m.handleButtonEvent(int32(up), false, m.CursorX, m.CursorY)
			} else {
				m.handleButtonEvent(int32(down), true, m.CursorX, m.CursorY)
				m.handleButtonEvent(int32(down), false, m.CursorX, m.CursorY)
			}
		}
	})
	return nil
}
