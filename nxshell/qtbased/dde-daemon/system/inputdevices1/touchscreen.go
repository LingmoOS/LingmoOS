// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/utils"
)

type Touchscreen struct {
	service *dbusutil.Service

	id         uint32
	UUID       string
	Name       string
	Phys       string
	DevNode    string
	BusType    string
	Serial     string
	OutputName string
	Width      float64
	Height     float64
}

const touchscreenDBusPath = "/org/deepin/dde/InputDevices1/Touchscreen/"
const touchscreenDBusInterface = "org.deepin.dde.InputDevices1.Touchscreen"

func newTouchscreen(dev *libinputDevice, service *dbusutil.Service, id uint32) *Touchscreen {
	t := &Touchscreen{
		service: service,

		id:         id,
		Name:       dev.GetName(),
		Phys:       dev.GetPhys(),
		DevNode:    dev.GetDevNode(),
		BusType:    dev.GetProperty("ID_BUS"),
		Serial:     dev.GetProperty("ID_SERIAL"),
		OutputName: dev.GetOutputName(),
	}

	t.UUID = genTouchscreenUUID(t.Phys, t.Serial)
	t.Width, t.Height = dev.GetSize()

	return t
}

func (t *Touchscreen) GetInterfaceName() string {
	return touchscreenDBusInterface
}

func (t *Touchscreen) export(path dbus.ObjectPath) error {
	err := t.service.Export(path, t)
	if err != nil {
		logger.Warning(err)
		return err
	}

	return nil
}

func (t *Touchscreen) stopExport() error {
	t.service.StopExport(t)
	return nil
}

func genTouchscreenUUID(phys, serial string) string {
	res, _ := utils.SumStrMd5(phys + serial)
	return res
}
