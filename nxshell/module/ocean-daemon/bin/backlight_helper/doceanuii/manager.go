// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package doceanuii

import (
	"fmt"
	"sync"

	"github.com/godbus/dbus/v5"
	"github.com/lingmoos/go-lib/dbusutil"
	"github.com/lingmoos/go-lib/log"
	x "github.com/lingmoos/go-x11-client"
)

const (
	DbusPath      = "/org/lingmo/ocean/BacklightHelper1/DDCCI"
	dbusInterface = "org.lingmo.ocean.BacklightHelper1.DDCCI"
)

var logger = log.NewLogger("backlight_helper/doceanuii")

//go:generate dbusutil-gen em -type Manager
type Manager struct {
	service *dbusutil.Service
	doceanuii   *doceanuii

	PropsMu         sync.RWMutex
	configTimestamp x.Timestamp
}

func NewManager(service *dbusutil.Service) (*Manager, error) {
	m := &Manager{}
	m.service = service

	var err error
	m.doceanuii, err = newDDCCI()
	if err != nil {
		return nil, fmt.Errorf("brightness: failed to init ddc/ci: %s", err)
	}

	return m, nil
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func (m *Manager) CheckSupport(edidBase64 string) (bool, *dbus.Error) {
	if m.doceanuii == nil {
		return false, nil
	}

	return m.doceanuii.SupportBrightness(edidBase64), nil
}

func (m *Manager) GetBrightness(edidBase64 string) (int32, *dbus.Error) {
	if m.doceanuii == nil {
		return 0, nil
	}

	if !m.doceanuii.SupportBrightness(edidBase64) {
		err := fmt.Errorf("brightness: not support ddc/ci: %s", edidBase64)
		return 0, dbusutil.ToError(err)
	}

	brightness, err := m.doceanuii.GetBrightness(edidBase64)
	return int32(brightness), dbusutil.ToError(err)
}

func (m *Manager) SetBrightness(edidBase64 string, value int32) *dbus.Error {
	if m.doceanuii == nil {
		return nil
	}
	if !m.doceanuii.SupportBrightness(edidBase64) {
		err := fmt.Errorf("brightness: not support ddc/ci: %s", edidBase64)
		return dbusutil.ToError(err)
	}
	err := m.doceanuii.SetBrightness(edidBase64, int(value))
	return dbusutil.ToError(err)
}

func (m *Manager) RefreshDisplays() *dbus.Error {
	if m.doceanuii == nil {
		return nil
	}
	err := m.doceanuii.RefreshDisplays()
	return dbusutil.ToError(err)
}
