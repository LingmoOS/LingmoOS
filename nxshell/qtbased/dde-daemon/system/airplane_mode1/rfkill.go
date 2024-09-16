// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package airplane_mode

import (
	"bytes"
	"encoding/binary"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"syscall"
	"time"
)

var endian binary.ByteOrder = getByteOrder()

type rfkillType uint8

const (
	rfkillTypeAll rfkillType = iota
	rfkillTypeWifi
	rfkillTypeBT
)

type rfkillOp uint8

const (
	rfkillOpAdd rfkillOp = iota
	rfkillOpDel
	rfkillOpChange
	rfkillOpChangeAll
)

type rfkillState uint8

const (
	rfkillStateUnblock rfkillState = iota
	rfkillStateBlock
)

// RfkillEvent rfkill event
type RfkillEvent struct {
	Index uint32
	Typ   rfkillType
	Op    rfkillOp
	Soft  rfkillState
	Hard  rfkillState
}

func (mgr *Manager) listenRfkill() {
	getRfkill := func() (fd int, err error) {
		return syscall.Open("/dev/rfkill", syscall.O_RDWR, 0777)
	}
	fd, err := getRfkill()
	if err != nil {
		// 个别机器文件创建较晚，尝试多读几次
		var checkTimer = time.NewTimer(time.Second)
		count := 0
		for range checkTimer.C {
			if count == 5 {
				logger.Warning("failed to open /dev/rfkill:", err)
				return
			} else {
				fd, err = getRfkill()
				if err == nil {
					break
				}
				count++
				checkTimer.Reset(time.Second)
			}
		}
	}
	defer syscall.Close(fd)

	event := &RfkillEvent{}
	b := make([]byte, 512)

	for {
		n, err := syscall.Read(fd, b)
		if err != nil {
			logger.Warning("failed to read /dev/rfkill:", err)
			continue
		}
		if n != binary.Size(event) {
			logger.Warning("wrong size:", n)
			continue
		}

		info := bytes.NewBuffer(b)
		err = binary.Read(info, endian, event)
		if err != nil {
			logger.Warning("failed to read event info:", err)
			continue
		}

		if event.Typ == rfkillTypeWifi &&
			event.Hard != 0 &&
			event.Soft != 0 {
			// 开启了hard block，就解除soft block
			time.AfterFunc(100*time.Millisecond, func() {
				mgr.btDevicesMu.Lock()
				numBt := len(mgr.btRfkillDevices)
				mgr.btDevicesMu.Unlock()
				if numBt == 0 {
					mgr.block(rfkillTypeAll, false)
				} else {
					mgr.block(rfkillTypeWifi, false)
				}
			})
		}
		mgr.handleBTRfkillEvent(event)
		mgr.handleWifIRfkillEvent(event)
	}
}

func (mgr *Manager) handleWifIRfkillEvent(event *RfkillEvent) {
	if event.Typ != rfkillTypeWifi {
		return
	}

	if mgr.hasNmWirelessDevices {
		// 当飞行模式的wifi设备状态为禁用而rfkill 状态为unblock状态时，以rfkill状态为准
		if mgr.WifiEnabled && event.Soft == rfkillStateUnblock {
			mgr.block(rfkillTypeWifi, false)
		}
	}
}

// 初始化获取rfkill BT设备
func (mgr *Manager) initBTRfkillDevice() {
	bin := "/usr/sbin/rfkill"
	// -n: don't print headings
	// --output ID,TYPE,SOFT,HARD: 只输出id、类型、SOFT,HARD信息
	args := "-n --output ID,TYPE,SOFT,HARD"
	output, err := exec.Command(bin, args).Output()
	if err != nil {
		logger.Warning("run rfkill err:", err)
		return
	}
	mgr.btDevicesMu.RLock()
	defer mgr.btDevicesMu.RUnlock()
	lines := strings.Split(string(output), "\n")
	getState := func(str string) rfkillState {
		if str == "blocked" {
			return rfkillStateBlock
		}
		return rfkillStateUnblock
	}
	for _, line := range lines {
		logger.Debug("rfkill device info:", line)
		fields := strings.Fields(line)
		if len(fields) != 4 {
			continue
		}
		if fields[1] == "bluetooth" {
			id, err := strconv.Atoi(fields[0])
			if err != nil {
				continue
			}
			mgr.btRfkillDevices[uint32(id)] = device{
				typ:  rfkillTypeBT,
				soft: getState(fields[2]),
				hard: getState(fields[3]),
			}
		}
	}
}

func (mgr *Manager) updateAllState() {
	if mgr.hasNmWirelessDevices {
		if len(mgr.btRfkillDevices) > 0 {
			mgr.setPropEnabled(mgr.BluetoothEnabled && mgr.WifiEnabled)
		} else {
			mgr.setPropEnabled(mgr.WifiEnabled)

		}
	} else if len(mgr.btRfkillDevices) > 0 {
		mgr.setPropEnabled(mgr.BluetoothEnabled)
	} else {
		logger.Info("rfkill device is empty")
		mgr.setPropEnabled(false)
	}

	// 仅保存 soft block 的状态
	allSoftBlocked := false
	wifiBlocked := false
	states, err := getRfkillState(rfkillTypeWifi)
	if err == nil {
		// nm 无线控制关闭，可能是hard关闭，dde只控制soft。
		allWifiSoftBlocked := true
		for _, v := range states {
			if v.Soft == 0 {
				allWifiSoftBlocked = false
				break
			}
		}
		wifiBlocked = allWifiSoftBlocked
	} else {
		logger.Warning(err)
		wifiBlocked = mgr.hasNmWirelessDevices && mgr.WifiEnabled
	}

	if mgr.hasNmWirelessDevices && len(mgr.btRfkillDevices) > 0 {
		allSoftBlocked = wifiBlocked && mgr.BluetoothEnabled
	} else if mgr.hasNmWirelessDevices {
		allSoftBlocked = wifiBlocked
	} else if len(mgr.btRfkillDevices) > 0 {
		allSoftBlocked = mgr.BluetoothEnabled
	}

	mgr.config.SetBlocked(rfkillTypeAll, allSoftBlocked)
	logger.Debug("refresh all blocked state:", allSoftBlocked)
}

// 只处理bluetooth设备
func (mgr *Manager) handleBTRfkillEvent(event *RfkillEvent) {
	logger.Warning("handleBTRfkillEvent=======>", event.Typ, event.Op, event.Op == rfkillOpDel)
	if event.Typ != rfkillTypeBT {
		return
	}
	mgr.btDevicesMu.Lock()
	defer mgr.btDevicesMu.Unlock()
	if event.Op == rfkillOpDel {
		delete(mgr.btRfkillDevices, event.Index)
		if event.Soft != 0 {
			// 飞行模式开启的时候，如果rfkill有hard block，
			// 解除 all soft block，让硬件接管。
			// FIXME: 优化
			time.AfterFunc(300*time.Millisecond, func() {
				hardEnable, err := mgr.nmManager.WirelessHardwareEnabled().Get(0)
				if err != nil {
					logger.Warning(err)
				}

				logger.Debug("wifi hardware status:", hardEnable)
				if !hardEnable {
					mgr.block(rfkillTypeAll, false)
				}
			})
		}
	} else {
		mgr.btRfkillDevices[event.Index] = device{
			typ:  rfkillTypeBT,
			soft: event.Soft,
			hard: event.Hard,
		}
	}
	btCnt := len(mgr.btRfkillDevices)
	blockBtCnt := 0
	softBtBlockCnt := 0
	for _, device := range mgr.btRfkillDevices {
		if device.soft == rfkillStateBlock || device.hard == rfkillStateBlock {
			blockBtCnt++
			if device.soft == rfkillStateBlock {
				softBtBlockCnt++
			}
		}
	}
	btBlocked := btCnt == blockBtCnt
	btSoftBlocked := btCnt == softBtBlockCnt
	mgr.setPropHasAirplaneMode(btCnt != 0 || mgr.hasNmWirelessDevices)
	mgr.setPropBluetoothEnabled(btBlocked && btCnt != 0)
	mgr.updateAllState()
	logger.Debug("refresh bluetooth blocked state:", btSoftBlocked)
	mgr.config.SetBlocked(rfkillTypeBT, btSoftBlocked)
	// save rfkill key event result to config file
	err := mgr.config.SaveConfig()
	if err != nil {
		logger.Warningf("save rfkill config file failed, err: %v", err)
	}
	logger.Debugf("rfkill state, bluetooth: %v, wifi: %v, airplane: %v", mgr.BluetoothEnabled, mgr.WifiEnabled, mgr.Enabled)
}

// rfkillAction2 write rfkill
// write op to /dev/rfkill so kernel will exec ref command
func rfkillAction(typ rfkillType, action rfkillState) error {
	// open rfkill file
	file, err := os.OpenFile("/dev/rfkill", os.O_RDWR, 0644)
	if err != nil {
		logger.Warningf("cant open rfkill, err: %v", err)
		return err
	}
	defer file.Close()
	// create event action
	event := &RfkillEvent{
		Typ:  typ,
		Op:   rfkillOpChangeAll,
		Soft: action,
	}
	// write command to file
	err = binary.Write(file, endian, event)
	if err != nil {
		logger.Warningf("set rfkill state failed, type: %v, action: %v, err: %v", typ, action, err)
		return err
	}
	logger.Infof("set rfkill state success, type: %v, action: %v", typ, action)
	return nil
}

func getRfkillState(typ rfkillType) ([]*RfkillEvent, error) {
	ret := []*RfkillEvent{}
	// open rfkill file
	file, err := os.Open("/dev/rfkill")
	if err != nil {
		logger.Warningf("cant open rfkill, err: %v", err)
		return ret, err
	}
	// close file
	defer file.Close()
	// get fd
	fd := int(file.Fd())
	// set non-block
	err = syscall.SetNonblock(fd, true)
	if err != nil {
		logger.Warningf("cant set non-block, err: %v", err)
		return ret, err
	}
	// create event action
	event := &RfkillEvent{
		Typ: typ,
	}
	// create reader
	buf := make([]byte, 512)
	for {
		// call to read rfkill info
		_, err = syscall.Read(fd, buf)
		if err != nil {
			// since fd is non-block, eagain means read end
			if err == syscall.EAGAIN {
				break
			}
			logger.Warningf("read rfkill failed, err: %v", err)
			return ret, err
		}
		// unmarshal to event
		info := bytes.NewBuffer(buf)
		// read event from info
		err = binary.Read(info, endian, event)
		if err != nil {
			logger.Warningf("binary read rfkill failed, err: %v", err)
			return ret, err
		}
		// check type here
		// if type is all type, all type match
		// if type is not all type, ref type match
		// or ignore this device
		if typ != rfkillTypeAll && typ != event.Typ {
			continue
		}

		ret = append(ret, event)
	}

	return ret, nil
}
