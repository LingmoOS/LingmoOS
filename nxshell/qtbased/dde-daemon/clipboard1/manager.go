// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package clipboard

import (
	"bytes"
	"errors"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/xfixes"
)

//go:generate dbusutil-gen em -type Manager

var (
	atomClipboardManager     x.Atom
	atomClipboard            x.Atom
	atomSaveTargets          x.Atom
	atomTargets              x.Atom
	atomMultiple             x.Atom
	atomDelete               x.Atom
	atomInsertProperty       x.Atom
	atomInsertSelection      x.Atom
	atomAtomPair             x.Atom //nolint
	atomIncr                 x.Atom
	atomTimestamp            x.Atom
	atomNull                 x.Atom //nolint
	atomTimestampProp        x.Atom
	atomFromClipboardManager x.Atom

	selectionMaxSize int
)

const (
	dSettingsAppID                      = "org.deepin.dde.daemon"
	dSettingsClipboardName              = "org.deepin.dde.daemon.clipboard"
	dSettingsKeySaveAtomIncrDataEnabled = "saveAtomIncrDataEnabled"
)

func initAtoms(xConn *x.Conn) {
	atomClipboardManager, _ = xConn.GetAtom("CLIPBOARD_MANAGER")
	atomClipboard, _ = xConn.GetAtom("CLIPBOARD")
	atomSaveTargets, _ = xConn.GetAtom("SAVE_TARGETS")
	atomTargets, _ = xConn.GetAtom("TARGETS")
	atomMultiple, _ = xConn.GetAtom("MULTIPLE")
	atomDelete, _ = xConn.GetAtom("DELETE")
	atomInsertProperty, _ = xConn.GetAtom("INSERT_PROPERTY")
	atomInsertSelection, _ = xConn.GetAtom("INSERT_SELECTION")
	atomAtomPair, _ = xConn.GetAtom("ATOM_PAIR")
	atomIncr, _ = xConn.GetAtom("INCR")
	atomTimestamp, _ = xConn.GetAtom("TIMESTAMP")
	atomTimestampProp, _ = xConn.GetAtom("_TIMESTAMP_PROP")
	atomNull, _ = xConn.GetAtom("NULL")
	atomFromClipboardManager, _ = xConn.GetAtom("FROM_DEEPIN_CLIPBOARD_MANAGER")
	selectionMaxSize = 65432
	logger.Debug("selectionMaxSize:", selectionMaxSize)
}

type TargetData struct {
	Target x.Atom
	Type   x.Atom
	Format uint8
	Data   []byte
}

func (td *TargetData) needINCR() bool {
	return len(td.Data) > selectionMaxSize
}

type Manager struct {
	xc                        XClient
	window                    x.Window
	ec                        *eventCaptor
	clipboardManagerAcquireTs x.Timestamp // 获取 CLIPBOARD_MANAGER selection 的时间戳
	clipboardManagerLostTs    x.Timestamp // 丢失 CLIPBOARD_MANAGER selection 的时间戳
	clipboardAcquireTs        x.Timestamp // 获取 CLIPBOARD selection 的时间戳
	clipboardLostTs           x.Timestamp // 丢失 CLIPBOARD selection 的时间戳

	contentMu sync.Mutex
	content   []*TargetData

	saveTargetsMu           sync.Mutex
	saveTargetsSuccessTime  time.Time
	saveTargetsRequestor    x.Window
	dsClipboardManager      ConfigManager.Manager
	saveAtomIncrDataEnabled bool
}

func (m *Manager) getTargetData(target x.Atom) *TargetData {
	m.contentMu.Lock()
	defer m.contentMu.Unlock()

	for _, td := range m.content {
		if td.Target == target {
			return td
		}
	}
	return nil
}

func (m *Manager) setContent(targetDataMap map[x.Atom]*TargetData) {
	// 给剪贴板数据带上特殊标记，为了让前端 dde-clipboard 知道是本程序给出的剪贴板数据
	targetDataMap[atomFromClipboardManager] = &TargetData{
		Target: atomFromClipboardManager,
		Type:   x.AtomString,
		Format: 8,
		Data:   []byte("1"),
	}
	for _, data := range targetDataMap {
		logger.Debugf("content target %s len: %v",
			getAtomDesc(m.xc.Conn(), data.Target), len(data.Data))
	}
	targetDataSlice := mapToSliceTargetData(targetDataMap)
	m.contentMu.Lock()
	m.content = targetDataSlice
	m.contentMu.Unlock()
}

func mapToSliceTargetData(dataMap map[x.Atom]*TargetData) []*TargetData {
	result := make([]*TargetData, 0, len(dataMap))
	for _, data := range dataMap {
		result = append(result, data)
	}
	return result
}

func (m *Manager) getConfigFromDSettings() error {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}

	ds := ConfigManager.NewConfigManager(sysBus)
	dsPath, err := ds.AcquireManager(0, dSettingsAppID, dSettingsClipboardName, "")
	if err != nil {
		return err
	}

	m.dsClipboardManager, err = ConfigManager.NewManager(sysBus, dsPath)
	if err != nil {
		return err
	}

	systemSigLoop := dbusutil.NewSignalLoop(sysBus, 10)
	systemSigLoop.Start()
	m.dsClipboardManager.InitSignalExt(systemSigLoop, true)

	m.saveAtomIncrDataEnabled = true

	getSaveAtomIncrDataEnabled := func() {
		v, err := m.dsClipboardManager.Value(0, dSettingsKeySaveAtomIncrDataEnabled)
		if err == nil {
			logger.Infof("get saveAtomIncrDataEnabled  %t", v.Value().(bool))
			m.saveAtomIncrDataEnabled = v.Value().(bool)
		}
	}
	if err != nil {
		logger.Warning(err)
	}

	_, err = m.dsClipboardManager.ConnectValueChanged(func(key string) {
		switch key {
		case dSettingsKeySaveAtomIncrDataEnabled:
			getSaveAtomIncrDataEnabled()
		}
	})

	if err != nil {
		logger.Warning(err)
	}

	getSaveAtomIncrDataEnabled()

	return nil
}

func (m *Manager) start() error {
	// 初始化配置
	err := m.getConfigFromDSettings()
	if err != nil {
		logger.Warning(err)
	}

	owner, err := m.xc.GetSelectionOwner(atomClipboardManager)
	if err != nil {
		return err
	}
	if owner != 0 {
		return fmt.Errorf("another clipboard manager is already running, owner: %d", owner)
	}

	m.window, err = m.xc.CreateWindow()
	if err != nil {
		return err
	}
	logger.Debug("m.window:", m.window)

	err = m.xc.SelectSelectionInputE(m.window, atomClipboard,
		xfixes.SelectionEventMaskSetSelectionOwner|
			xfixes.SelectionEventMaskSelectionClientClose|
			xfixes.SelectionEventMaskSelectionWindowDestroy)
	if err != nil {
		logger.Warning(err)
	}

	err = m.xc.SelectSelectionInputE(m.window, atomClipboardManager,
		xfixes.SelectionEventMaskSetSelectionOwner)
	if err != nil {
		logger.Warning(err)
	}

	m.ec = newEventCaptor()
	eventChan := make(chan x.GenericEvent, 50)
	m.xc.Conn().AddEventChan(eventChan)
	go func() {
		for ev := range eventChan {
			m.handleEvent(ev)
		}
	}()

	ts, err := m.getTimestamp()
	if err != nil {
		return err
	}

	logger.Debug("ts:", ts)
	err = setSelectionOwner(m.xc, m.window, atomClipboardManager, ts)
	if err != nil {
		return err
	}

	err = announceManageSelection(m.xc.Conn(), m.window, atomClipboardManager, ts)
	if err != nil {
		return err
	}

	return nil
}

func (m *Manager) handleEvent(ev x.GenericEvent) {
	xConn := m.xc.Conn()
	xfixesExtData := xConn.GetExtensionData(xfixes.Ext())
	code := ev.GetEventCode()
	switch code {
	case x.SelectionRequestEventCode:
		event, _ := x.NewSelectionRequestEvent(ev)
		logger.Debug(selReqEventToString(event, xConn))

		if event.Selection == atomClipboardManager {
			go m.convertClipboardManager(event)
		} else if event.Selection == atomClipboard {
			go m.convertClipboard(event)
		}

	case x.PropertyNotifyEventCode:
		event, _ := x.NewPropertyNotifyEvent(ev)

		if m.ec.handleEvent(event) {
			logger.Debug("->", propNotifyEventToString(event, xConn))
			return
		}
		logger.Debug(">>", propNotifyEventToString(event, xConn))

	case x.SelectionNotifyEventCode:
		event, _ := x.NewSelectionNotifyEvent(ev)

		if m.ec.handleEvent(event) {
			logger.Debug("->", selNotifyEventToString(event, xConn))
			return
		}
		logger.Debug(">>", selNotifyEventToString(event, xConn))

	case x.DestroyNotifyEventCode:
		event, _ := x.NewDestroyNotifyEvent(ev)

		logger.Debug(destroyNotifyEventToString(event))

	case x.SelectionClearEventCode:
		event, _ := x.NewSelectionClearEvent(ev)
		logger.Debug(selClearEventToString(event))

	case xfixes.SelectionNotifyEventCode + xfixesExtData.FirstEvent:
		event, _ := xfixes.NewSelectionNotifyEvent(ev)
		logger.Debug(xfixesSelNotifyEventToString(event))
		switch event.Subtype {
		case xfixes.SelectionEventSetSelectionOwner:
			if event.Selection == atomClipboard {
				if event.Owner == m.window {
					logger.Debug("i have become the owner of CLIPBOARD selection, ts:", event.SelectionTimestamp)
					m.clipboardAcquireTs = event.SelectionTimestamp
					m.clipboardLostTs = 0
				} else {
					logger.Debug("other app have become the owner of CLIPBOARD selection, ts:", event.SelectionTimestamp)
					if event.SelectionTimestamp >= m.clipboardAcquireTs {
						m.clipboardLostTs = event.SelectionTimestamp
					}
					const delay = 300 * time.Millisecond
					time.AfterFunc(delay, func() {
						// 等300ms，等 clipboard manager 的 SAVE_TARGETS 转换开始, 如果已经开始了则不再进行主动的数据保存。
						m.saveTargetsMu.Lock()
						defer func() {
							m.saveTargetsRequestor = 0
							m.saveTargetsMu.Unlock()
						}()

						shouldIgnore := m.saveTargetsRequestor == event.Owner &&
							time.Since(m.saveTargetsSuccessTime) < time.Second

						if shouldIgnore {
							logger.Debug("do not call handleClipboardUpdated")
							return
						}
						err := m.handleClipboardUpdated(event.SelectionTimestamp)
						if err != nil {
							logger.Warning("handle clipboard updated err:", err)
						}
					})
				}
			} else if event.Selection == atomClipboardManager {
				if event.Owner == m.window {
					logger.Debug("i have become the owner of CLIPBOARD_MANAGER selection, ts:", event.SelectionTimestamp)
					m.clipboardManagerAcquireTs = event.SelectionTimestamp
					m.clipboardManagerLostTs = 0
				} else {
					if event.SelectionTimestamp >= m.clipboardManagerAcquireTs {
						m.clipboardManagerLostTs = event.SelectionTimestamp
					}
				}
			}

		case xfixes.SelectionEventSelectionWindowDestroy, xfixes.SelectionEventSelectionClientClose:
			if event.Selection == atomClipboard {
				err := m.becomeClipboardOwner(event.Timestamp)
				if err != nil {
					logger.Warning(err)
				}
			}
		}
	}
}

// 处理剪贴板数据更新
func (m *Manager) handleClipboardUpdated(ts x.Timestamp) error {
	logger.Debug("handleClipboardUpdated", ts)

	targets, err := m.getClipboardTargets(ts)
	if err != nil {
		return err
	}
	logger.Debug("targets:", targets)

	// 过滤云桌面复制的数据
	// 判断是否从wps复制的数据并且是否包含text格式
	var tmpTarget x.Atom
	hasKingsoftData := false
	hasTextData := false

	for _, target := range targets {
		targetName, err := m.xc.GetAtomName(target)
		targetName = strings.ToLower(targetName)
		if err == nil {
			if targetName == "uos/remote-copy" {
				return nil
			} else if targetName == "text/plain" {
				hasTextData = true
				tmpTarget = target
			} else if strings.Contains(targetName, "kingsoft") {
				hasKingsoftData = true
			}
		}
	}

	logger.Debug("hasKingsoftData:", hasKingsoftData, ", hasTextData:", hasTextData)

	// 如果 是从wps复制的数据并且包含text格式数据，则先读取text数据大小
	// 如果text数据大小超过10M，则只缓存text数据，否则所有格式都缓存
	if hasKingsoftData && hasTextData {
		td, err := m.saveTarget(tmpTarget, ts)
		if err == nil && len(td.Data) > 10*1024*1024 {
			m.setContent(map[x.Atom]*TargetData{
				td.Target: td,
			})

			logger.Debug("handleClipboardUpdated  wps text format finish", ts)
			return nil
		}
	}

	targetDataMap := m.saveTargets(targets, ts)
	m.setContent(targetDataMap)

	logger.Debug("handleClipboardUpdated all format finish", ts)
	return nil
}

func setSelectionOwner(xc XClient, win x.Window, selection x.Atom, ts x.Timestamp) error {
	xc.SetSelectionOwner(win, selection, ts)
	owner, err := xc.GetSelectionOwner(selection)
	if err != nil {
		return err
	}
	if owner != win {
		return errors.New("failed to set selection owner")
	}
	return nil
}

func (m *Manager) becomeClipboardOwner(ts x.Timestamp) error {
	err := setSelectionOwner(m.xc, m.window, atomClipboard, ts)
	if err != nil {
		return err
	}
	logger.Debug("set clipboard selection owner to me")
	return nil
}

// 转换 CLIPBOARD selection 的 TARGETS target，剪贴板获取支持的所有 targets。
func (m *Manager) getClipboardTargets(ts x.Timestamp) ([]x.Atom, error) {
	selNotifyEvent, err := m.ec.captureSelectionNotifyEvent(func() error {
		m.xc.ConvertSelection(m.window, atomClipboard,
			atomTargets, atomTargets, ts)
		return m.xc.Flush()
	}, func(event *x.SelectionNotifyEvent) bool {
		return event.Target == atomTargets &&
			event.Selection == atomClipboard &&
			event.Requestor == m.window
	})
	if err != nil {
		return nil, err
	}

	if selNotifyEvent.Property == x.None {
		return nil, errors.New("failed to convert clipboard targets")
	}

	propReply, err := m.getProperty(m.window, selNotifyEvent.Property, true)
	if err != nil {
		return nil, err
	}

	targets, err := getAtomListFormReply(propReply)
	if err != nil {
		return nil, err
	}

	return targets, nil
}

func canConvertSelection(acquireTs, lostTs, evTs x.Timestamp) bool {
	logger.Debug("canConvertSelection", acquireTs, lostTs, evTs)
	// evTs == 0 表示现在
	if acquireTs == 0 {
		// 未获取
		return false
	}

	if lostTs == 0 {
		// 现在未失去
		if acquireTs <= evTs || evTs == 0 {
			return true
		}

	} else {
		// 现在已经失去
		if evTs == 0 {
			return false
		}

		if acquireTs <= evTs && evTs < lostTs {
			return true
		}
	}
	return false
}

// 处理 CLIPBOARD_MANAGER selection 的转换请求,
// target 支持：SAVE_TARGETS, TARGETS, TIMESTAMP
func (m *Manager) convertClipboardManager(ev *x.SelectionRequestEvent) {
	logger.Debug("convert CLIPBOARD_MANAGER selection")
	if !canConvertSelection(m.clipboardManagerAcquireTs, m.clipboardManagerLostTs, ev.Time) {
		logger.Debug("can not covert selection, ts invalid")
		m.finishSelectionRequest(ev, false)
		return
	}

	switch ev.Target {
	case atomSaveTargets:
		err := m.covertClipboardManagerSaveTargets(ev)
		if err != nil {
			logger.Warning("covert ClipboardManager saveTargets err:", err)
		}
		m.finishSelectionRequest(ev, err == nil)

	case atomTargets:
		w := x.NewWriter()
		w.Write4b(uint32(atomTargets))
		w.Write4b(uint32(atomSaveTargets))
		w.Write4b(uint32(atomTimestamp))
		err := m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor,
			ev.Property, x.AtomAtom, 32, w.Bytes())
		if err != nil {
			logger.Warning(err)
		}
		m.finishSelectionRequest(ev, err == nil)

	case atomTimestamp:
		w := x.NewWriter()
		w.Write4b(uint32(m.clipboardManagerAcquireTs))
		err := m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor,
			ev.Property, x.AtomInteger, 32, w.Bytes())
		if err != nil {
			logger.Warning(err)
		}
		m.finishSelectionRequest(ev, err == nil)

	default:
		// 不支持的 target
		m.finishSelectionRequest(ev, false)
	}
}

func (m *Manager) covertClipboardManagerSaveTargets(ev *x.SelectionRequestEvent) error {
	m.saveTargetsMu.Lock()
	defer m.saveTargetsMu.Unlock()

	err := m.xc.ChangeWindowEventMask(ev.Requestor, x.EventMaskStructureNotify)
	if err != nil {
		return err
	}

	var targets []x.Atom
	var replyType x.Atom
	if ev.Property != x.None {
		reply, err := m.xc.GetProperty(true, ev.Requestor, ev.Property,
			x.AtomAtom, 0, 0x1FFFFFFF)
		if err != nil {
			return err
		}

		replyType = reply.Type
		if reply.Type != x.None {
			targets, err = getAtomListFormReply(reply)
			if err != nil {
				return err
			}
		}
	}

	if replyType == x.None {
		logger.Debug("need convert clipboard targets")
		targets, err = m.getClipboardTargets(ev.Time)
		if err != nil {
			return err
		}
	}

	logger.Debug("targets:", targets)

	// 过滤云桌面复制的数据
	// 判断是否从wps复制的数据并且是否包含text格式
	var tmpTarget x.Atom
	hasKingsoftData := false
	hasTextData := false

	for _, target := range targets {
		targetName, err := m.xc.GetAtomName(target)
		targetName = strings.ToLower(targetName)
		if err == nil {
			if targetName == "uos/remote-copy" {
				return nil
			} else if targetName == "text/plain" {
				hasTextData = true
				tmpTarget = target
			} else if strings.Contains(targetName, "kingsoft") {
				hasKingsoftData = true
			}
		}
	}

	logger.Debug("hasKingsoftData:", hasKingsoftData, ", hasTextData:", hasTextData)

	// 如果 是从wps复制的数据并且包含text格式数据，则先读取text数据大小
	// 如果text数据大小超过10M，则只缓存text数据，否则所有格式都缓存
	if hasKingsoftData && hasTextData {
		td, err := m.saveTarget(tmpTarget, ev.Time)
		if err == nil && len(td.Data) > 10*1024*1024 {
			m.setContent(map[x.Atom]*TargetData{
				td.Target: td,
			})

			logger.Debug("covertClipboardManagerSaveTargets  text format finish", ev.Time)
			m.saveTargetsRequestor = ev.Requestor
			m.saveTargetsSuccessTime = time.Now()
			return nil
		}
	}

	targetDataMap := m.saveTargets(targets, ev.Time)
	m.setContent(targetDataMap)

	m.saveTargetsRequestor = ev.Requestor
	m.saveTargetsSuccessTime = time.Now()
	return nil
}

// 处理 CLIPBOARD selection 的转换请求
func (m *Manager) convertClipboard(ev *x.SelectionRequestEvent) {
	targetName, _ := m.xc.GetAtomName(ev.Target)
	logger.Debugf("convert clipboard target %s|%d", targetName, ev.Target)

	if !canConvertSelection(m.clipboardAcquireTs, m.clipboardLostTs, ev.Time) {
		logger.Debug("can not covert selection, ts invalid")
		m.finishSelectionRequest(ev, false)
		return
	}

	switch ev.Target {
	case atomTargets:
		// TARGETS
		w := x.NewWriter()
		w.Write4b(uint32(atomTargets))
		w.Write4b(uint32(atomTimestamp))
		m.contentMu.Lock()
		for _, targetData := range m.content {
			w.Write4b(uint32(targetData.Target))
		}
		m.contentMu.Unlock()

		err := m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor,
			ev.Property, x.AtomAtom, 32, w.Bytes())
		if err != nil {
			logger.Warning(err)
		}
		m.finishSelectionRequest(ev, err == nil)
	case atomTimestamp:
		// TIMESTAMP
		w := x.NewWriter()
		w.Write4b(uint32(m.clipboardAcquireTs))
		err := m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor,
			ev.Property, x.AtomInteger, 32, w.Bytes())
		if err != nil {
			logger.Warning(err)
		}
		m.finishSelectionRequest(ev, err == nil)
		// TODO 支持 MULTIPLE target
	default:
		targetData := m.getTargetData(ev.Target)
		if targetData == nil {
			m.finishSelectionRequest(ev, false)
			return
		}
		logger.Debugf("target %d len: %v, needINCR: %v", targetData.Target, len(targetData.Data),
			targetData.needINCR())

		if targetData.needINCR() {
			err := m.sendTargetIncr(targetData, ev)
			if err != nil {
				logger.Warning(err)
			}
		} else {
			err := m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor,
				ev.Property, targetData.Type, targetData.Format, targetData.Data)
			if err != nil {
				logger.Warning(err)
			}
			m.finishSelectionRequest(ev, err == nil)
		}
	}
}

// NOTE: 需要在这个函数调用 finishSelectionRequest
func (m *Manager) sendTargetIncr(targetData *TargetData, ev *x.SelectionRequestEvent) error {
	err := m.xc.ChangeWindowEventMask(ev.Requestor, x.EventMaskPropertyChange)
	if err != nil {
		m.finishSelectionRequest(ev, false)
		return err
	}

	// 函数返回时还原请求窗口的 event mask
	defer func() {
		err := m.xc.ChangeWindowEventMask(ev.Requestor, 0)
		if err != nil {
			logger.Warning("reset requestor window event mask err:", err)
		}
	}()

	_, err = m.ec.capturePropertyNotifyEvent(func() error {
		// 把 target 数据长度通过属性 ev.Property 告知请求者。
		w := x.NewWriter()
		w.Write4b(uint32(len(targetData.Data)))
		err = m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor, ev.Property,
			atomIncr, 32, w.Bytes())
		if err != nil {
			logger.Warning(err)
		}
		// NOTE: 一定要在开始传输具体数据之前 finish selection request
		m.finishSelectionRequest(ev, err == nil)
		return err
	}, func(pev *x.PropertyNotifyEvent) bool {
		return pev.Window == ev.Requestor &&
			pev.State == x.PropertyDelete &&
			pev.Atom == ev.Property
	})

	if err != nil {
		return err
	}

	var offset int
	for {
		data := targetData.Data[offset:]
		length := len(data)
		if length > selectionMaxSize {
			length = selectionMaxSize
		}
		offset += length

		_, err = m.ec.capturePropertyNotifyEvent(func() error {
			logger.Debug("send incr data", length)
			err = m.xc.ChangePropertyE(x.PropModeReplace, ev.Requestor, ev.Property,
				targetData.Type, targetData.Format, data[:length])
			if err != nil {
				logger.Warning(err)
			}
			return err
		}, func(pev *x.PropertyNotifyEvent) bool {
			return pev.Window == ev.Requestor &&
				pev.State == x.PropertyDelete &&
				pev.Atom == ev.Property
		})
		if err != nil {
			return err
		}

		if length == 0 {
			break
		}
	}

	return nil
}

func (m *Manager) finishSelectionRequest(ev *x.SelectionRequestEvent, success bool) {
	var property x.Atom
	if success {
		property = ev.Property
	}

	event := &x.SelectionNotifyEvent{
		Time:      ev.Time,
		Requestor: ev.Requestor,
		Selection: ev.Selection,
		Target:    ev.Target,
		Property:  property,
	}

	err := m.xc.SendEventE(false, ev.Requestor, x.EventMaskNoEvent,
		event)
	if err != nil {
		logger.Warning(err)
	}

	if logger.GetLogLevel() == log.LevelDebug {
		successStr := "success"
		if !success {
			successStr = "fail"
		}
		xConn := m.xc.Conn()
		logger.Debugf("finish selection request %s {Requestor: %d, Selection: %s,"+
			" Target: %s, Property: %s}",
			successStr, ev.Requestor,
			getAtomDesc(xConn, ev.Selection),
			getAtomDesc(xConn, ev.Target),
			getAtomDesc(xConn, ev.Property))
	}
}

func (m *Manager) saveTargets(targets []x.Atom, ts x.Timestamp) map[x.Atom]*TargetData {
	result := make(map[x.Atom]*TargetData, len(targets))

	for _, target := range targets {
		targetName, err := m.xc.GetAtomName(target)
		if err != nil {
			logger.Warning(err)
			continue
		}
		if shouldIgnoreSaveTarget(target, targetName) {
			logger.Debugf("ignore target %s|%d", targetName, target)
			continue
		}

		logger.Debugf("save target %s|%d", targetName, target)
		td, err := m.saveTarget(target, ts)
		if err != nil {
			logger.Warningf("save target failed %s|%d, err: %v", targetName, target, err)
		} else {
			result[td.Target] = td
			logger.Debugf("save target success %s|%d", targetName, target)
		}
	}
	return result
}

func shouldIgnoreSaveTarget(target x.Atom, targetName string) bool {
	switch target {
	case atomTargets, atomSaveTargets,
		atomTimestamp, atomMultiple, atomDelete,
		atomInsertProperty, atomInsertSelection,
		x.AtomPixmap:
		return true
	}
	if strings.HasPrefix(targetName, "image/") {
		switch targetName {
		case "image/jpeg", "image/png", "image/bmp":
			return false
		default:
			return true
		}
	}
	return false
}

func (m *Manager) saveTarget(target x.Atom, ts x.Timestamp) (targetData *TargetData, err error) {
	selNotifyEvent, err := m.ec.captureSelectionNotifyEvent(func() error {
		m.xc.ConvertSelection(m.window, atomClipboard, target, target, ts)
		return m.xc.Flush()
	}, func(event *x.SelectionNotifyEvent) bool {
		return event.Selection == atomClipboard &&
			event.Requestor == m.window &&
			event.Target == target
	})
	if err != nil {
		return
	}
	if selNotifyEvent.Property == x.None {
		err = errors.New("failed to convert target")
		return
	}

	propReply, err := m.getProperty(m.window, selNotifyEvent.Property, false)
	if err != nil {
		return
	}

	if propReply.Type == atomIncr {
		if m.saveAtomIncrDataEnabled {
			targetData, err = m.receiveTargetIncr(target, selNotifyEvent.Property)
		}
	} else {
		err = m.xc.DeletePropertyE(m.window, selNotifyEvent.Property)
		if err != nil {
			return
		}
		logger.Debug("data len:", len(propReply.Value))
		targetData = &TargetData{
			Target: target,
			Type:   propReply.Type,
			Format: propReply.Format,
			Data:   propReply.Value,
		}
	}
	return
}

func (m *Manager) getProperty(win x.Window, propertyAtom x.Atom, delete bool) (*x.GetPropertyReply, error) {
	propReply, err := m.xc.GetProperty(false, win, propertyAtom,
		x.GetPropertyTypeAny, 0, 0)
	if err != nil {
		return nil, err
	}

	propReply, err = m.xc.GetProperty(delete, win, propertyAtom,
		x.GetPropertyTypeAny,
		0,
		(propReply.BytesAfter+uint32(x.Pad(int(propReply.BytesAfter))))/4,
	)
	if err != nil {
		return nil, err
	}
	return propReply, nil
}

func (m *Manager) receiveTargetIncr(target, prop x.Atom) (targetData *TargetData, err error) {
	logger.Debug("start receiveTargetIncr", target)
	var data [][]byte
	t0 := time.Now()
	total := 0
	for {
		var propNotifyEvent *x.PropertyNotifyEvent
		propNotifyEvent, err = m.ec.capturePropertyNotifyEvent(func() error {
			err := m.xc.DeletePropertyE(m.window, prop)
			if err != nil {
				logger.Warning(err)
			}
			return err

		}, func(event *x.PropertyNotifyEvent) bool {
			return event.State == x.PropertyNewValue && event.Window == m.window &&
				event.Atom == prop
		})
		if err != nil {
			logger.Warning(err)
			return
		}

		var propReply *x.GetPropertyReply
		propReply, err = m.xc.GetProperty(false, propNotifyEvent.Window, propNotifyEvent.Atom,
			x.GetPropertyTypeAny,
			0, 0)
		if err != nil {
			logger.Warning(err)
			return
		}
		propReply, err = m.xc.GetProperty(false, propNotifyEvent.Window, propNotifyEvent.Atom,
			x.GetPropertyTypeAny, 0,
			(propReply.BytesAfter+uint32(x.Pad(int(propReply.BytesAfter))))/4,
		)
		if err != nil {
			logger.Warning(err)
			return
		}

		if propReply.ValueLen == 0 {
			logger.Debugf("end receiveTargetIncr %d, took %v, total size: %d",
				target, time.Since(t0), total)

			err = m.xc.DeletePropertyE(propNotifyEvent.Window, propNotifyEvent.Atom)
			if err != nil {
				logger.Warning(err)
				return
			}

			targetData = &TargetData{
				Target: target,
				Type:   propReply.Type,
				Format: propReply.Format,
				Data:   bytes.Join(data, nil),
			}
			return
		}
		if logger.GetLogLevel() == log.LevelDebug {
			logger.Debugf("incr receive data size: %d", len(propReply.Value))
		}
		total += len(propReply.Value)
		data = append(data, propReply.Value)
	}
}

func (m *Manager) getTimestamp() (x.Timestamp, error) {
	propNotifyEvent, err := m.ec.capturePropertyNotifyEvent(func() error {
		return m.xc.ChangePropertyE(x.PropModeReplace, m.window, atomTimestampProp,
			x.AtomInteger, 32, nil)
	}, func(event *x.PropertyNotifyEvent) bool {
		return event.Window == m.window &&
			event.Atom == atomTimestampProp &&
			event.State == x.PropertyNewValue
	})

	if err != nil {
		return 0, err
	}

	return propNotifyEvent.Time, nil
}

func (m *Manager) GetInterfaceName() string {
	return dbusServiceName
}
