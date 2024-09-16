// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"errors"
	"fmt"
	"sort"
	"strconv"
	"strings"

	dbus "github.com/godbus/dbus/v5"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.notifications"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/pulse"
)

// 一次性读出所有事件
func (a *Audio) pollEvents() []*pulse.Event {
	events := make([]*pulse.Event, 0)

FOR:
	for {
		select {
		case event := <-a.eventChan:
			events = append(events, event)
		default:
			logger.Debugf("poll %d events", len(events))
			break FOR
		}
	}

	return events
}

// 事件分发
func (a *Audio) dispatchEvents(events []*pulse.Event) {
	logger.Debugf("dispatch %d events", len(events))
	for i, event := range events {
		logger.Debugf("dispatch %dth event: type<%d> index<%d>", i, event.Type, event.Index)
		switch event.Facility {
		case pulse.FacilityServer:
			a.handleServerEvent(event.Type)
			a.saveConfig()
		case pulse.FacilityCard:
			a.handleCardEvent(event.Type, event.Index)
			a.saveConfig()
		case pulse.FacilitySink:
			a.handleSinkEvent(event.Type, event.Index)
			a.saveConfig()
		case pulse.FacilitySource:
			a.handleSourceEvent(event.Type, event.Index)
			a.saveConfig()
		case pulse.FacilitySinkInput:
			a.handleSinkInputEvent(event.Type, event.Index)
		}
	}
	logger.Debug("dispatch events done")
}

func (a *Audio) handleEvent() {
	for {
		select {
		case event := <-a.eventChan:
			tail := a.pollEvents()
			events := make([]*pulse.Event, 0, 1+len(tail))
			events = append(events, event)
			events = append(events, tail...)
			a.refresh()
			GetPriorityManager().SetPorts(a.cards)
			GetPriorityManager().Save()
			a.dispatchEvents(events)

		case <-a.quit:
			logger.Debug("handleEvent return")
			return
		}
	}
}

func (a *Audio) handleStateChanged() {
	for {
		select {
		case state := <-a.stateChan:
			switch state {
			case pulse.ContextStateFailed:
				logger.Warning("pulseaudio context state failed")
				a.destroyCtxRelated()

				if !a.noRestartPulseAudio {
					logger.Debug("retry init")
					err := a.init()
					if err != nil {
						logger.Warning("failed to init:", err)
					}
					return
				} else {
					logger.Debug("do not restart pulseaudio")
				}
			}

		case <-a.quit:
			logger.Debug("handleStateChanged return")
			return
		}
	}
}

func (a *Audio) isCardIdValid(cardId uint32) bool {
	for _, card := range a.cards {
		if card.Id == cardId {
			return true
		}
	}
	return false
}

func (a *Audio) needAutoSwitchInputPort() bool {
	// 不支持自动切换端口
	if !a.canAutoSwitchPort() {
		return false
	}

	inputs := GetPriorityManager().Input
	firstPort := inputs.GetTheFirstPort()

	// 没有可用端口
	if firstPort.PortType == PortTypeInvalid {
		logger.Debug("no input port")
		return false
	}

	// 当前端口为空
	if a.defaultSource == nil {
		logger.Debug("current source is nil")
		return true
	}

	// 同端口切换次数超出限制(切换失败时反复切换同一端口)
	if a.inputAutoSwitchCount >= 10 &&
		(a.inputCardName == firstPort.CardName && a.inputPortName == firstPort.PortName) {
		logger.Debug("input auto switch tried too many times")
		return false
	}

	// 当前端口就是优先级最高的端口
	currentCardName := a.getCardNameById(a.defaultSource.Card)
	currentPortName := a.defaultSource.ActivePort.Name
	if currentCardName == firstPort.CardName && currentPortName == firstPort.PortName {
		logger.Debugf("current input<%s,%s> is already the first port",
			currentCardName, currentPortName)
		return false
	}

	logger.Debugf("will auto switch from input<%s,%s> to input<%s,%s>",
		currentCardName, currentPortName, firstPort.CardName, firstPort.PortName)
	return true
}

func (a *Audio) needAutoSwitchOutputPort() bool {
	// 不支持自动切换端口
	if !a.canAutoSwitchPort() {
		return false
	}

	outputs := GetPriorityManager().Output
	firstPort := outputs.GetTheFirstPort()

	// 没有可用端口
	if firstPort.PortType == PortTypeInvalid {
		logger.Debug("no output port")
		return false
	}

	// 当前端口为空
	if a.defaultSink == nil {
		logger.Debug("default sink is nil")
		return true
	}

	// 同端口切换次数超出限制(切换失败时反复切换同一端口)
	if a.outputAutoSwitchCount >= a.outputAutoSwitchCountMax &&
		(a.outputCardName == firstPort.CardName && a.outputPortName == firstPort.PortName) {
		logger.Debug("output auto switch tried too many times")
		return false
	}

	// 当前端口就是优先级最高的端口
	currentCardName := a.getCardNameById(a.defaultSink.Card)
	currentPortName := a.defaultSink.ActivePort.Name
	if currentCardName == firstPort.CardName && currentPortName == firstPort.PortName {
		logger.Debugf("current output<%s,%s> is already the first",
			currentCardName, currentPortName)
		return false
	}

	logger.Debugf("will auto switch from output<%s,%s> to output<%s,%s>",
		currentCardName, currentPortName, firstPort.CardName, firstPort.PortName)
	return true
}

func (a *Audio) autoSwitchPort() {
	if a.needAutoSwitchOutputPort() {
		outputs := GetPriorityManager().Output
		firstOutput := outputs.GetTheFirstPort()
		card, err := a.cards.getByName(firstOutput.CardName)

		if err == nil {
			logger.Debugf("auto switch output to #%d %s:%s", card.Id, card.core.Name, firstOutput.PortName)
			a.setPort(card.Id, firstOutput.PortName, pulse.DirectionSink)
		} else {
			logger.Warning(err)
		}

		// 自动切换计数
		if a.outputCardName == firstOutput.CardName && a.outputPortName == firstOutput.PortName {
			a.outputAutoSwitchCount++
		} else {
			a.outputAutoSwitchCount = 0
			a.outputCardName = firstOutput.CardName
			a.outputPortName = firstOutput.PortName
		}
	}

	if a.needAutoSwitchInputPort() {
		inputs := GetPriorityManager().Input
		firstInput := inputs.GetTheFirstPort()
		card, err := a.cards.getByName(firstInput.CardName)

		if err == nil {
			logger.Debugf("auto switch input to #%d %s:%s", card.Id, card.core.Name, firstInput.PortName)
			a.setPort(card.Id, firstInput.PortName, pulse.DirectionSource)
		} else {
			logger.Warning(err)
		}

		// 自动切换计数
		if a.inputCardName == firstInput.CardName && a.inputPortName == firstInput.PortName {
			a.inputAutoSwitchCount++
		} else {
			a.inputAutoSwitchCount = 0
			a.inputCardName = firstInput.CardName
			a.inputPortName = firstInput.PortName
		}
	}
}

func (a *Audio) handleCardEvent(eventType int, idx uint32) {
	switch eventType {
	case pulse.EventTypeNew: // 新增声卡
		a.handleCardAdded(idx)
	case pulse.EventTypeRemove: // 删除声卡
		a.handleCardRemoved(idx)
	case pulse.EventTypeChange: // 声卡属性变化
		a.handleCardChanged(idx)
	default:
		logger.Warningf("unhandled card event, card=%d, type=%d", idx, eventType)
	}

	// 这里写所有类型的card事件都需要触发的逻辑
	/* 新增声卡上的端口如果被处于禁用状态，进行横幅提示 */
	card, err := a.cards.get(idx)
	if err == nil {
		if isBluezAudio(card.core.Name) {
			logger.Debugf("notify bluez card %s", card.core.Name)
			a.notifyBluezCardPortInsert(card)
		} else {
			logger.Debugf("notify normal card %s", card.core.Name)
			a.notifyCardPortInsert(card)
		}
	} else {
		logger.Warning(err)
	}

	// 保存旧的cards
	a.oldCards = a.cards

	// 触发自动切换
	a.autoSwitchPort()
}

func (a *Audio) handleCardAdded(idx uint32) {
	// 数据更新在refreshCards中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("card %d added", idx)

	card, err := a.cards.get(idx)
	if err != nil {
		logger.Warningf("invalid card index #%d", idx)
		return
	}

	if isBluezAudio(card.core.Name) {
		card.AutoSetBluezMode()
	}
}

func (a *Audio) handleCardRemoved(idx uint32) {
	// 数据更新在refreshCards中统一处理，这里只做业务逻辑上的响应
	// 注意，此时idx已经失效了，无法获取已经失去的数据，如果业务需要，应当在refresh前进行数据备份
	logger.Debugf("card %d removed", idx)
}

func (a *Audio) handleCardChanged(idx uint32) {
	// 数据更新在refreshSinks中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("card %d changed", idx)

	card, err := a.cards.get(idx)
	if err != nil {
		logger.Warningf("invalid card index #%d", idx)
		return
	}

	// 如果发生变化的是当前输出所用的声卡，且是蓝牙声卡
	if a.defaultSink != nil && idx == a.defaultSink.Card && isBluetoothCard(card.core) {
		if strings.Contains(strings.ToLower(card.ActiveProfile.Name), bluezModeA2dp) {
			a.setPropBluetoothAudioMode(bluezModeA2dp)
		} else if strings.Contains(strings.ToLower(card.ActiveProfile.Name), bluezModeHeadset) {
			a.setPropBluetoothAudioMode(bluezModeHeadset)
		}
	}
}

func (a *Audio) handleSinkEvent(eventType int, idx uint32) {
	switch eventType {
	case pulse.EventTypeNew: // 新增sink
		a.handleSinkAdded(idx)
	case pulse.EventTypeRemove: // 删除sink
		a.handleSinkRemoved(idx)
	case pulse.EventTypeChange: // sink属性变化
		a.handleSinkChanged(idx)
	default:
		logger.Warningf("unhandled sink event, sink=%d, type=%d", idx, eventType)
	}

	// 这里写所有类型的sink事件都需要触发的逻辑

	// 触发自动切换
	a.autoSwitchPort()
}

func (a *Audio) handleSinkAdded(idx uint32) {
	// 数据更新在refreshSinks中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("sink %d added", idx)
}

func (a *Audio) handleSinkRemoved(idx uint32) {
	// 数据更新在refreshSinks中统一处理，这里只做业务逻辑上的响应
	// 注意，此时idx已经失效了，无法获取已经失去的数据，如果业务需要，应当在refresh前进行数据备份
	logger.Debugf("sink %d removed", idx)
}

func (a *Audio) handleSinkChanged(idx uint32) {
	// 数据更新在refreshSinks中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("sink %d changed", idx)

	// 蓝牙模式切换、触发sink change
	if a.defaultSink != nil && a.defaultSink.index == idx && isBluezAudio(a.defaultSink.Name) {
		card, err := a.cards.get(a.defaultSink.Card)
		if err == nil {
			a.setPropBluetoothAudioMode(card.BluezMode())
		} else {
			logger.Warningf("%d card not found", a.defaultSink.Card)
		}
	}
}

func (a *Audio) handleSourceEvent(eventType int, idx uint32) {
	switch eventType {
	case pulse.EventTypeNew:
		a.handleSourceAdded(idx)
	case pulse.EventTypeRemove:
		a.handleSourceRemoved(idx)
	case pulse.EventTypeChange:
		a.handleSourceChanged(idx)
	default:
		logger.Warningf("unhandled source event, sink=%d, type=%d", idx, eventType)
	}

	// 这里写所有类型的source事件都需要触发的逻辑

	// 触发自动切换
	a.autoSwitchPort()
}

func (a *Audio) handleSourceAdded(idx uint32) {
	// 数据更新在refreshSources中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("source %d added", idx)
}

func (a *Audio) handleSourceRemoved(idx uint32) {
	// 数据更新在refreshSources中统一处理，这里只做业务逻辑上的响应
	// 注意，此时idx已经失效了，无法获取已经失去的数据，如果业务需要，应当在refresh前进行数据备份
	logger.Debugf("source %d removed", idx)
}

func (a *Audio) handleSourceChanged(idx uint32) {
	// 数据更新在refreshSources中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("source %d changed", idx)
}

func (a *Audio) handleSinkInputEvent(eventType int, idx uint32) {
	switch eventType {
	case pulse.EventTypeNew:
		a.handleSinkInputAdded(idx)
	case pulse.EventTypeRemove:
		a.handleSinkInputRemoved(idx)
	case pulse.EventTypeChange:
		a.handleSinkInputChanged(idx)
	default:
		logger.Warningf("unhandled sink-input event, sink-input=%d, type=%d", idx, eventType)
	}

	// 这里写所有类型的sink-input事件都需要触发的逻辑
}

func (a *Audio) handleSinkInputAdded(idx uint32) {
	// 数据更新在refreshSinkInputs中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("sink-input %d added", idx)
}

func (a *Audio) handleSinkInputRemoved(idx uint32) {
	// 数据更新在refreshSinkInputs中统一处理，这里只做业务逻辑上的响应
	// 注意，此时idx已经失效了，无法获取已经失去的数据，如果业务需要，应当在refresh前进行数据备份
	logger.Debugf("sink-input %d removed", idx)
}

func (a *Audio) handleSinkInputChanged(idx uint32) {
	// 数据更新在refreshSinkInputs中统一处理，这里只做业务逻辑上的响应
	logger.Debugf("sink-input %d changed", idx)
}

/* 创建开启端口的命令，提供给notification调用 */
func makeNotifyCmdEnablePort(cardId uint32, portName string) string {
	dest := "org.deepin.dde.Audio1"
	path := "/org/deepin/dde/Audio1"
	method := "org.deepin.dde.Audio1.SetPortEnabled"
	return fmt.Sprintf("dbus-send,--type=method_call,--dest=%s,%s,%s,uint32:%d,string:%s,boolean:true",
		dest, path, method, cardId, portName)
}

/* 横幅提示端口被禁用,并提供开启的按钮 */
func (a *Audio) notifyPortDisabled(cardId uint32, port pulse.CardPortInfo) {
	session, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return
	}

	icon := "disabled-audio-output-plugged"
	if port.Direction == pulse.DirectionSource {
		icon = "disabled-audio-input-plugged"
	}

	cmd := makeNotifyCmdEnablePort(cardId, port.Name)
	message := fmt.Sprintf(gettext.Tr("%s had been disabled"), port.Description)
	actions := []string{"open", gettext.Tr("Open")}
	hints := map[string]dbus.Variant{"x-deepin-action-open": dbus.MakeVariant(cmd)}
	notify := notifications.NewNotifications(session)
	_, err = notify.Notify(
		0,
		gettext.Tr("dde-control-center"),
		0,
		icon,
		message,
		"",
		actions,
		hints,
		15*1000,
	)
	if err != nil {
		logger.Warning(err)
	}

}

func (a *Audio) updateObjPathsProp(type0 string, ids []int, setFn func(value []dbus.ObjectPath) bool) {
	sort.Ints(ids)
	paths := make([]dbus.ObjectPath, len(ids))
	for idx, id := range ids {
		paths[idx] = dbus.ObjectPath(dbusPath + "/" + type0 + strconv.Itoa(id))
	}
	a.PropsMu.Lock()
	setFn(paths)
	a.PropsMu.Unlock()
}

func (a *Audio) updatePropSinks() {
	var ids []int
	a.mu.Lock()
	for _, sink := range a.sinks {
		ids = append(ids, int(sink.index))
	}
	a.mu.Unlock()
	a.updateObjPathsProp("Sink", ids, a.setPropSinks)
}

func (a *Audio) updatePropSources() {
	var ids []int
	a.mu.Lock()
	for _, source := range a.sources {
		ids = append(ids, int(source.index))
	}
	a.mu.Unlock()
	a.updateObjPathsProp("Source", ids, a.setPropSources)
}

func (a *Audio) updatePropSinkInputs() {
	var ids []int
	a.mu.Lock()
	for _, sinkInput := range a.sinkInputs {
		if sinkInput.visible {
			ids = append(ids, int(sinkInput.index))
		}
	}
	a.mu.Unlock()
	a.updateObjPathsProp("SinkInput", ids, a.setPropSinkInputs)
}

func isPhysicalDevice(deviceName string) bool {
	for _, virtualDeviceKey := range []string{
		"echoCancelSource", "echo-cancel", "Echo-Cancel", // virtual key
	} {
		if strings.Contains(deviceName, virtualDeviceKey) {
			return false
		}
	}
	return true
}

func (a *Audio) handleServerEvent(eventType int) {
	switch eventType {
	case pulse.EventTypeChange:
		server, err := a.ctx.GetServer()
		if err != nil {
			logger.Error(err)
			return
		}
		logger.Debugf("[Event] server changed: default sink: %s, default source: %s",
			server.DefaultSinkName, server.DefaultSourceName)

		a.defaultSinkName = server.DefaultSinkName
		a.defaultSourceName = server.DefaultSourceName
		a.updateDefaultSink(server.DefaultSinkName)
		a.updateDefaultSource(server.DefaultSourceName)
		a.autoSwitchPort()
	}
}

func (a *Audio) listenGSettingVolumeIncreaseChanged() {
	gsettings.ConnectChanged(gsSchemaAudio, gsKeyVolumeIncrease, func(val string) {
		volInc := a.settings.GetBoolean(gsKeyVolumeIncrease)
		if volInc {
			a.MaxUIVolume = increaseMaxVolume
		} else {
			a.MaxUIVolume = normalMaxVolume
		}
		gMaxUIVolume = a.MaxUIVolume
		err := a.emitPropChangedMaxUIVolume(a.MaxUIVolume)
		if err != nil {
			logger.Warning("changed Max UI Volume failed: ", err)
		} else {
			sink := a.defaultSink
			GetConfigKeeper().SetIncreaseVolume(a.getCardNameById(sink.Card), sink.ActivePort.Name, volInc)
		}
	})
}

func (a *Audio) listenGSettingDeviceManageChanged() {
	gsettings.ConnectChanged(gsSchemaControlCenter, gsKeyDeviceManager, func(val string) {
		if a.enableAutoSwitchPort != a.controlCenterDeviceManager.Get() {
			a.controlCenterDeviceManager.Set(a.enableAutoSwitchPort)
			logger.Info("listenGSettingDeviceManageChanged set a.enableAutoSwitchPort to a.controlCenterDeviceManager. value : ", a.enableAutoSwitchPort)
		}
	})
}

// 外部修改ReducecNoise时触发回调，响应实际降噪开关
func (a *Audio) writeReduceNoise(write *dbusutil.PropertyWrite) *dbus.Error {
	reduce, ok := write.Value.(bool)
	if !ok {
		return dbusutil.ToError(errors.New("type is not bool"))
	}

	if reduce && isBluezAudio(a.defaultSource.Name) {
		logger.Debug("bluetooth audio device cannot open reduce-noise")
		a.ReduceNoise = false
		a.emitPropChangedReduceNoise(a.ReduceNoise)
		return dbusutil.ToError(errors.New("bluetooth audio device cannot open reduce-noise"))
	}

	// 这个配置属性本来应该放在降噪设置成功之后再设置的
	// 但是在开启降噪，切换到降噪的虚拟通道时，需要用对应主设备的配置进行配置恢复
	// 如果不放在前面，配置恢复时，主设备的配置里降噪还处于关闭状态
	// 配置恢复会自动关闭降噪
	source := a.defaultSource
	GetConfigKeeper().SetReduceNoise(a.getCardNameById(source.Card), source.ActivePort.Name, reduce)
	err := a.setReduceNoise(reduce)
	if err != nil {
		logger.Warning("set Reduce Noise failed: ", err)
	}
	a.inputAutoSwitchCount = 0
	return nil
}

func (a *Audio) writeKeyPausePlayer(write *dbusutil.PropertyWrite) *dbus.Error {
	pausePlayer, ok := write.Value.(bool)
	if !ok {
		return dbusutil.ToError(errors.New("type is not bool"))
	}
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return dbus.MakeFailedError(err)
	}

	systemConnObj := systemBus.Object("org.desktopspec.ConfigManager", a.configManagerPath)
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.setValue", 0, dsgkeyPausePlayer, dbus.MakeVariant(pausePlayer)).Err
	if err != nil {
		return dbusutil.ToError(errors.New("dconfig Cannot set value " + dsgkeyPausePlayer))
	}
	a.setPropPausePlayer(pausePlayer)
	return nil
}

func (a *Audio) notifyBluezCardPortInsert(card *Card) {
	logger.Debugf("notify bluez card %d:%s", card.Id, card.core.Name)
	oldCard, err := a.oldCards.getByName(card.core.Name)
	if err != nil {
		// oldCard不存在
		logger.Warning(err)
	}

	// 蓝牙模式配置出错情况过滤通知
	if card.ActiveProfile.Name == "off" {
		logger.Debugf("filter bluez notify, beacuse profile is off")
		return
	}

	// 蓝牙会根据模式过滤端口，因此忽略unknown状态
	for _, port := range card.Ports {
		if port.Available == pulse.AvailableTypeNo {
			// 当前状态为AvailableTypeNo，忽略
			logger.Debugf("port %s not insert", port.Name)
			continue
		}

		isInsert := false
		if oldCard == nil {
			// oldCard不存在，即新增声卡
			isInsert = true
		} else {
			oldPort, err := oldCard.getPortByName(port.Name)
			if err != nil {
				// oldPort不存在，例如A2DP切换到headset
				isInsert = true

				// 但是pulseaudio事件时序是乱的，所以可能会因为其它原因进来，导致bug
				logger.Warning(err)
			} else if oldPort.Available == pulse.AvailableTypeNo {
				isInsert = true
			}
		}

		if isInsert {
			logger.Debugf("port<%s,%s> inserted", card.core.Name, port.Name)
			_, portConfig := GetConfigKeeper().GetCardAndPortConfig(card.core.Name, port.Name)
			if !portConfig.Enabled {
				logger.Debugf("port<%s,%s> notify", card.core.Name, port.Name)
				a.notifyPortDisabled(card.Id, port)
			}
		}
	}
}

func (a *Audio) notifyCardPortInsert(card *Card) {
	logger.Debugf("notify card %d:%s", card.Id, card.core.Name)
	oldCard, err := a.oldCards.getByName(card.core.Name)
	if err != nil {
		// oldCard不存在
		logger.Warning(err)
	}

	for _, port := range card.Ports {
		if port.Available == pulse.AvailableTypeNo {
			// 当前状态为AvailableTypeNo，忽略
			logger.Debugf("port %s not insert", port.Name)
			continue
		}

		isInsert := false
		if oldCard == nil {
			// oldCard不存在，即新增声卡
			isInsert = true
		} else {
			oldPort, err := oldCard.getPortByName(port.Name)
			if err != nil {
				// oldPort不存在，例如A2DP切换到headset
				isInsert = true

				// 但是pulseaudio事件时序是乱的，所以可能会因为其它原因进来，导致bug
				logger.Warning(err)

			} else if oldPort.Available == pulse.AvailableTypeNo {
				logger.Debugf("port %s from AvailableTypeNo to %d", port.Name, port.Available)
				isInsert = true
			} else if oldPort.Available == pulse.AvailableTypeUnknow && port.Available == pulse.AvailableTypeYes {
				logger.Debugf("port %s from AvailableTypeUnknow to AvailableTypeYes", port.Name)
				isInsert = true
			}
		}

		if isInsert {
			logger.Debugf("port<%s,%s> inserted", card.core.Name, port.Name)
			_, portConfig := GetConfigKeeper().GetCardAndPortConfig(card.core.Name, port.Name)
			if !portConfig.Enabled {
				logger.Debugf("port<%s,%s> notify", card.core.Name, port.Name)
				a.notifyPortDisabled(card.Id, port)
			}
		}
	}
}
