// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"fmt"
	"sort"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
)

// 每个适配器最大 worker 数量，最大同时连接设备数。
const maxNumWorkerPerAdapter = 1

// 自动连接管理器
type autoConnectManager struct {
	mu               sync.Mutex
	devices          map[dbus.ObjectPath]*autoDeviceInfo // key 是 device path
	adapters         map[dbus.ObjectPath]*acmAdapterData // key 是 adapter path
	connectCb        func(adapter, device dbus.ObjectPath, wId int) error
	startDiscoveryCb func(adapter dbus.ObjectPath)
}

// 和单个适配器相关的数据
type acmAdapterData struct {
	workers map[int]bool
	// 需要等待的由设备主动重连接的设备集合
	activeReconnectDevices map[dbus.ObjectPath]struct{}
	timer                  *time.Timer
}

func newAutoConnectManager() *autoConnectManager {
	return &autoConnectManager{
		devices:  make(map[dbus.ObjectPath]*autoDeviceInfo),
		adapters: make(map[dbus.ObjectPath]*acmAdapterData),
	}
}

// 自动连接设备信息
type autoDeviceInfo struct {
	alias              string
	connectDurationMax time.Duration // 连接时长最大限制
	connectDuration    time.Duration
	adapter            dbus.ObjectPath
	device             dbus.ObjectPath
	wId                int
	priority           int // 优先级，越小越高
	count              int // 尝试连接次数，从1开始。
}

func (adi *autoDeviceInfo) String() string {
	return fmt.Sprintf("autoDeviceInfo{cd: %v, cdMax: %v, count: %v, adapter: %q, alias: %q, device: %q, wId: %d, p: %d}",
		adi.connectDuration, adi.connectDurationMax, adi.count, adi.adapter, adi.alias, adi.device, adi.wId, adi.priority)
}

func (adi *autoDeviceInfo) canRetry() bool {
	return adi.connectDuration < adi.connectDurationMax
}

// isTaken 设备是否正在被 worker 拿走，正在被自动连接。
func (adi *autoDeviceInfo) isTaken() bool {
	return adi.wId != 0
}

// getDevice 获取设备用于自动连接
func (acm *autoConnectManager) getDevice(workerId int) (result autoDeviceInfo) {
	acm.mu.Lock()
	defer acm.mu.Unlock()

	defer func() {
		if result.device != "" {
			logger.Debugf("[%d] getDevice success, adi: %v", workerId, &result)
		} else {
			logger.Debugf("[%d] getDevice failed", workerId)
		}
	}()

	var tmpDeviceInfos []*autoDeviceInfo
	for _, info := range acm.devices {
		if info.canRetry() && !info.isTaken() {
			tmpDeviceInfos = append(tmpDeviceInfos, info)
		}
	}

	if len(tmpDeviceInfos) == 0 {
		return autoDeviceInfo{}
	}

	sort.SliceStable(tmpDeviceInfos, func(i, j int) bool {
		return tmpDeviceInfos[i].priority < tmpDeviceInfos[j].priority
	})

	info := tmpDeviceInfos[0]
	info.wId = workerId
	info.count++
	// 失败 3 次则把优先级降到最低
	if info.count >= 3 {
		// 影响第4次连接时的优先级
		info.priority = priorityLowest
	}
	if !info.canRetry() {
		delete(acm.devices, info.device)
	}

	return *info
}

// putDevice 归还设备
func (acm *autoConnectManager) putDevice(d autoDeviceInfo, connectDuration time.Duration) {
	acm.mu.Lock()
	defer acm.mu.Unlock()

	v, ok := acm.devices[d.device]
	if !ok {
		// 设备已被删除
		return
	}
	if v.wId != d.wId {
		return
	}
	v.wId = 0
	// 更新 connectDuration
	v.connectDuration += connectDuration
}

// removeDevice 移除设备，比如设备连接成功。
func (acm *autoConnectManager) removeDevice(d autoDeviceInfo) {
	acm.mu.Lock()
	defer acm.mu.Unlock()
	delete(acm.devices, d.device)
}

// addDevices 添加自动连接设备，devices 中所有 adapter 都要是 adapterPath。
func (acm *autoConnectManager) addDevices(adapterPath dbus.ObjectPath, devices []autoDeviceInfo,
	activeReconnectDevices []*device) {

	if len(devices) == 0 {
		acm.startDiscoveryCb(adapterPath)
		return
	}

	acm.mu.Lock()
	defer acm.mu.Unlock()

	if _, ok := acm.adapters[adapterPath]; !ok {
		return
	}

	for _, d := range devices {
		if d.adapter != adapterPath {
			continue
		}
		currentD, ok := acm.devices[d.device]
		if ok {
			// 保留 wId
			d.wId = currentD.wId
			// 覆盖其他，比如 count, connectDuration
		}
		dCopy := d
		acm.devices[d.device] = &dCopy
	}

	if len(activeReconnectDevices) == 0 {
		// 不用等待
		acm.startWorkers(adapterPath)
		return
	}
	acm.delayStartWorkers(adapterPath, activeReconnectDevices)
}

// delayStartWorkers 等待由设备主动连接的设备连接成功，或者超时，再调用 startWorkers 。
func (acm *autoConnectManager) delayStartWorkers(adapterPath dbus.ObjectPath, activeReconnectDevices []*device) {
	// NOTE: 不要加锁
	adapterData, ok := acm.adapters[adapterPath]
	if !ok {
		logger.Warning("invalid adapter path", adapterPath)
		return
	}

	adapterData.activeReconnectDevices = make(map[dbus.ObjectPath]struct{})
	for _, d := range activeReconnectDevices {
		adapterData.activeReconnectDevices[d.Path] = struct{}{}
	}
	if adapterData.timer == nil {
		adapterData.timer = time.AfterFunc(delayStartDuration, func() {
			acm.mu.Lock()
			defer acm.mu.Unlock()
			logger.Debug("timer expired, startWorkers", adapterPath)
			acm.startWorkers(adapterPath)

			adapterData, ok := acm.adapters[adapterPath]
			if !ok {
				return
			}
			adapterData.activeReconnectDevices = nil
		})
	}
	logger.Debug("delayStartWorkers", adapterPath, activeReconnectDevices)
	adapterData.timer.Reset(delayStartDuration)
}

// handleDeviceEvent 处理设备连接和删除事件
func (acm *autoConnectManager) handleDeviceEvent(d *device) {
	acm.mu.Lock()
	defer acm.mu.Unlock()

	adapterData, ok := acm.adapters[d.AdapterPath]
	if !ok {
		logger.Warning("invalid adapter path", d.AdapterPath)
		return
	}

	if len(adapterData.activeReconnectDevices) == 0 {
		return
	}
	// 所有由设备重连接的设备都连接（或删除）了，则可以继续后续步骤。
	delete(adapterData.activeReconnectDevices, d.Path)
	if len(adapterData.activeReconnectDevices) == 0 {
		adapterData.timer.Stop()
		logger.Debug("[handleDeviceEvent] startWorkers", d.AdapterPath)
		acm.startWorkers(d.AdapterPath)
	}
}

const delayStartDuration = 12 * time.Second

// evalNumWorkers 评估期待新启动的 worker 数量
func (acm *autoConnectManager) evalNumWorkers(adapterPath dbus.ObjectPath) int {
	// NOTE: 不要加锁
	num := 0
	for _, d := range acm.devices {
		if d.adapter == adapterPath && !d.isTaken() && d.canRetry() {
			num++
			if num == maxNumWorkerPerAdapter {
				break
			}
		}
	}
	adapterData := acm.adapters[adapterPath]
	if adapterData == nil {
		return 0
	}
	idMap := adapterData.workers
	if idMap != nil {
		count := 0
		for _, exist := range idMap {
			if exist {
				count++
			}
		}
		limit := maxNumWorkerPerAdapter - count
		if num > limit {
			num = limit
		}
	}
	return num
}

// getId 分配 worker 的 id，并可以限制 worker 的数量。
// 返回 0 表示分配失败，大于 0 的表示成功。
func (acm *autoConnectManager) getId(adapterPath dbus.ObjectPath) int {
	// NOTE: 不要加锁
	adapterData, ok := acm.adapters[adapterPath]
	if !ok {
		return 0
	}

	idMap := adapterData.workers
	if idMap == nil {
		return 0
	}
	for i := 1; i <= maxNumWorkerPerAdapter; i++ {
		if !idMap[i] {
			idMap[i] = true
			return i
		}
	}
	return 0
}

// putId 归还 worker 使用的 id
func (acm *autoConnectManager) putId(adapterPath dbus.ObjectPath, id int) {
	acm.mu.Lock()
	defer acm.mu.Unlock()

	adapterData, ok := acm.adapters[adapterPath]
	if !ok {
		return
	}

	idMap := adapterData.workers
	if idMap == nil {
		return
	}
	idMap[id] = false
}

// startWorkers 为适配器启动 worker 们
func (acm *autoConnectManager) startWorkers(adapterPath dbus.ObjectPath) {
	num := acm.evalNumWorkers(adapterPath)
	// NOTE: 不要加锁
	for i := 0; i < num; i++ {
		id := acm.getId(adapterPath)
		if id != 0 {
			w := newAutoConnectWorker(adapterPath, id, acm)
			logger.Debug("new worker", id)
			w.start()
		}
	}
}

// addAdapter 响应适配器被添加事件
func (acm *autoConnectManager) addAdapter(adapterPath dbus.ObjectPath) {
	acm.mu.Lock()
	defer acm.mu.Unlock()

	if _, ok := acm.adapters[adapterPath]; ok {
		return
	}

	acm.adapters[adapterPath] = &acmAdapterData{
		workers: make(map[int]bool),
	}
}

// removeAdapter 响应适配器被删除事件
func (acm *autoConnectManager) removeAdapter(adapterPath dbus.ObjectPath) {
	acm.mu.Lock()
	defer acm.mu.Unlock()

	adapterData, ok := acm.adapters[adapterPath]
	if !ok {
		return
	}

	if adapterData.timer != nil {
		adapterData.timer.Stop()
	}
	delete(acm.adapters, adapterPath)
	for devPath, info := range acm.devices {
		if info.adapter == adapterPath {
			delete(acm.devices, devPath)
		}
	}
}

// 自动连接的 worker，代表一个 go routine。
type autoConnectWorker struct {
	id      int
	adapter dbus.ObjectPath
	m       *autoConnectManager
}

func newAutoConnectWorker(adapterPath dbus.ObjectPath, id int, m *autoConnectManager) *autoConnectWorker {
	w := &autoConnectWorker{
		id:      id,
		adapter: adapterPath,
		m:       m,
	}
	return w
}

// start 开始工作
func (w *autoConnectWorker) start() {
	go func() {
		d := w.m.getDevice(w.id)
		for {
			if d.device != "" {
				logger.Debugf("worker %d before connect", w.id)
				connectStart := time.Now()
				err := w.m.connectCb(d.adapter, d.device, w.id)
				connectDuration := time.Since(connectStart)

				if err != nil {
					logger.Warning(err)
					// 连接失败
					if err == errInvalidDevicePath {
						// 设备被移除，b.getDevice 失败
						w.m.removeDevice(d)
					} else {
						// 归还设备，以便再次尝试
						w.m.putDevice(d, connectDuration)
					}
				} else {
					// 连接成功，或被忽略
					w.m.removeDevice(d)
				}
				logger.Debugf("worker %d after connect", w.id)

				d = w.m.getDevice(w.id)
				if d.device != "" {
					continue
				}
			}
			// 无工作可做，退出，回收资源。
			logger.Debugf("[%d] quit", w.id)
			w.m.putId(w.adapter, w.id)

			if w.m.adapters[w.adapter] == nil {
				// 设备被移除后直接返回，防止后续works空指针崩溃
				logger.Debug("adapter removed")
				return
			}

			// 如果启用多个work，需要等待所有work全部回连结束再开始扫描其他设备
			idMap := w.m.adapters[w.adapter].workers
			if idMap != nil {
				for _, exist := range idMap {
					if exist {
						return
					}
				}

				w.m.startDiscoveryCb(w.adapter)
			}
			return
		}
	}()
}
