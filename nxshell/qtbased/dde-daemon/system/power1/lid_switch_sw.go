// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"io/ioutil"
	"os"
	"strings"
	"time"
)

const (
	swLidOpen  = "1"
	swLidClose = "0"
)

const swLidStateFile = "/sys/bus/platform/devices/liddev/lid_state"

func isSWLidStateFileExist() bool {
	_, err := os.Stat(swLidStateFile)
	return err == nil
}

func (m *Manager) initLidSwitchSW() {
	m.HasLidSwitch = true
	go m.swLidSwitchCheckLoop()
}

func (m *Manager) swLidSwitchCheckLoop() {
	prevState := getLidStateSW()
	for {
		time.Sleep(time.Second * 3)
		newState := getLidStateSW()
		if prevState != newState {
			prevState = newState

			var closed bool
			switch newState {
			case swLidClose:
				closed = true
			case swLidOpen:
				closed = false
			default:
				logger.Warningf("unknown lid state %q", newState)
				continue
			}
			m.handleLidSwitchEvent(closed)
		}
	}
}

// lid_state content: '1\n'
func getLidStateSW() string {
	content, err := ioutil.ReadFile(swLidStateFile)
	if err != nil {
		logger.Warning(err)
		return swLidOpen
	}
	return strings.TrimRight(string(content), "\n")
}
