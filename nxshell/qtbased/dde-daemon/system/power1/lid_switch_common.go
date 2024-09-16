// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

/*
#include <linux/input.h>
*/
import "C"

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"os"
	"strconv"
	"strings"
	"syscall"
	"unsafe"

	upower "github.com/linuxdeepin/go-dbus-factory/org.freedesktop.upower"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
)

const (
	SW_LID = C.SW_LID
	SW_CNT = C.SW_CNT
	SW_MAX = C.SW_MAX
	EV_SW  = C.EV_SW

	bitsPerLong = int(unsafe.Sizeof(C.long(0))) * 8
)

// NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
func NBITS(x int) int {
	return (((x) - 1) / bitsPerLong) + 1
}

// #define LONG(x) ((x)/BITS_PER_LONG)
func LONG(x int) int {
	return x / bitsPerLong
}

// #define OFF(x)  ((x)%BITS_PER_LONG)
func OFF(x int) int {
	return x % bitsPerLong
}

// #define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)
func testBit(bit int, array []int) bool {
	v := (array[LONG(bit)] >> uint(OFF(bit))) & 1
	return v != 0
}

func upInputStrToBitmask(s string, bitmask []int) int {
	var numBitsSet int
	maxSize := len(bitmask)
	v := strings.SplitN(s, " ", maxSize)

	j := 0
	for i := len(v) - 1; i >= 0; i-- {
		val, _ := strconv.ParseUint(v[i], 16, 64)
		bitmask[j] = int(val)

		for val != 0 {
			numBitsSet++
			val &= val - 1
		}

		j++
	}
	return numBitsSet
}

type InputEvent struct {
	Time  syscall.Timeval // time in seconds since epoch at which event occurred
	Type  uint16          // event type - one of ecodes.EV_*
	Code  uint16          // event code related to the event type
	Value int32           // event value related to the event type
}

// Get a useful description for an input event. Example:
//
//	event at 1347905437.435795, code 01, type 02, val 02
func (ev *InputEvent) String() string {
	return fmt.Sprintf("event at %d.%d, code %02d, type %02d, val %02d",
		ev.Time.Sec, ev.Time.Usec, ev.Code, ev.Type, ev.Value)
}

const eventSize = int(unsafe.Sizeof(InputEvent{}))

func (m *Manager) findLidSwitch() string {
	devices := m.gudevClient.QueryBySubsystem("input")

	defer func() {
		// free devices
		for _, device := range devices {
			device.Unref()
		}
	}()

	for _, device := range devices {
		name := device.GetName()
		if !strings.HasPrefix(name, "event") {
			continue
		}

		capSw := device.GetSysfsAttr("../capabilities/sw")

		bitmask := make([]int, NBITS(SW_MAX))
		numBits := upInputStrToBitmask(capSw, bitmask)

		if numBits == 0 || numBits >= SW_CNT {
			// invalid bitmask entry
			continue
		}

		if !testBit(SW_LID, bitmask) {
			// not a lid
			continue
		}

		deviceFile := device.GetDeviceFile()
		if deviceFile != "" {
			return deviceFile
		}
	}
	return ""
}

func (m *Manager) initLidSwitchCommon() {
	devFile := m.findLidSwitch()
	if devFile == "" {
		logger.Info("Not found lid switch")
		return
	}
	logger.Infof("find dev file %q", devFile)
	// open
	f, err := os.Open(devFile)
	if err != nil {
		logger.Warningf("os.Open %q err : %v", devFile, err)
		return
	}
	m.HasLidSwitch = true

	go func() {
		for {
			events, err := readLidSwitchEvent(f)
			if err != nil {
				logger.Warning(err)
				continue
			}
			for _, ev := range events {
				logger.Debugf("%v", &ev)
				if ev.Type == EV_SW && ev.Code == SW_LID {
					logger.Infof("lid switch event value: %v", ev.Value)
					var closed bool
					switch ev.Value {
					case 1:
						closed = true
					case 0:
						closed = false
					default:
						logger.Warningf("unknown lid switch event value %v", ev.Value)
						continue
					}
					m.handleLidSwitchEvent(closed)
				}
			}
		}
	}()
}

func (m *Manager) initLidSwitchByUPower() error {
	const UPowerServiceName = "org.freedesktop.UPower"
	sysBusObj := ofdbus.NewDBus(m.service.Conn())
	hasOwner, err := sysBusObj.NameHasOwner(0, UPowerServiceName)
	if err != nil {
		return err
	}
	if !hasOwner {
		return fmt.Errorf("%v not export", UPowerServiceName)
	}
	uPowerObj := upower.NewUPower(m.service.Conn())
	uPowerObj.InitSignalExt(m.systemSigLoop, true)
	err = uPowerObj.LidIsClosed().ConnectChanged(func(hasValue bool, isClosed bool) {
		if !hasValue {
			return
		}
		m.handleLidSwitchEvent(isClosed)
	})
	if err != nil {
		return err
	}

	hasLidSwitch, err := uPowerObj.LidIsPresent().Get(0)
	if err != nil {
		return err
	}

	m.HasLidSwitch = hasLidSwitch
	return nil
}

func readLidSwitchEvent(f *os.File) ([]InputEvent, error) {
	// read
	count := 16

	events := make([]InputEvent, count)
	buffer := make([]byte, eventSize*count)

	_, err := f.Read(buffer)
	if err != nil {
		logger.Warning("f read err:", err)
		return nil, err
	}

	b := bytes.NewBuffer(buffer)
	err = binary.Read(b, binary.LittleEndian, &events)
	if err != nil {
		logger.Warning("binary read err:", err)
		return nil, err
	}

	// remove trailing structures
	for i := range events {
		logger.Debug("i", i)
		if events[i].Time.Sec == 0 {
			events = events[:i]
			break
		}
	}
	return events, nil
}
