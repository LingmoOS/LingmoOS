// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"sync"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/pulse"
)

type Meter struct {
	audio   *Audio
	service *dbusutil.Service
	PropsMu sync.RWMutex
	Volume  float64
	id      string
	alive   bool
	core    *pulse.SourceMeter
}

// TODO: use pulse.Meter instead of remove pulse.SourceMeter
func newMeter(id string, core *pulse.SourceMeter, audio *Audio) *Meter {
	m := &Meter{
		id:      id,
		core:    core,
		audio:   audio,
		service: audio.service,
	}
	err := m.Tick()
	if err != nil {
		logger.Warning(err)
	}
	go m.tryQuit()
	return m
}

func (m *Meter) quit() {
	m.audio.mu.Lock()
	delete(m.audio.meters, m.id)
	m.audio.mu.Unlock()

	err := m.service.StopExport(m)
	if err != nil {
		logger.Warning(err)
	}
	m.core.Destroy()
}

func (m *Meter) tryQuit() {
	defer m.quit()

	for {
		time.Sleep(time.Second * 10)
		m.PropsMu.Lock()
		if !m.alive {
			m.PropsMu.Unlock()
			return
		}
		m.alive = false
		m.PropsMu.Unlock()
	}
}

func (m *Meter) Tick() *dbus.Error {
	logger.Infof("dbus call Tick")

	m.PropsMu.Lock()
	m.alive = true
	m.PropsMu.Unlock()
	return nil
}

func (m *Meter) getPath() dbus.ObjectPath {
	return dbus.ObjectPath(dbusPath + "/Meter" + m.id)
}

func (*Meter) GetInterfaceName() string {
	return dbusInterface + ".Meter"
}
