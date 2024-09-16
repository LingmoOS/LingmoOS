// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package fprintd1

import (
	"errors"
	"path"
	"strings"

	"github.com/godbus/dbus/v5"
	fprint "github.com/linuxdeepin/go-dbus-factory/system/net.reactivated.fprint"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

const (
	actionIdEnroll = "org.deepin.dde.fprintd.enroll"
	actionIdDelete = "org.deepin.dde.fprintd.delete-enrolled-fingers"
)

type IDevice interface {
	destroy()
	getCorePath() dbus.ObjectPath
	getPath() dbus.ObjectPath
	dbusutil.Implementer

	isFree() (bool, error)
}

type Device struct {
	service *dbusutil.Service
	core    fprint.Device

	ScanType string
}

type Devices []IDevice

func newDevice(objPath dbus.ObjectPath, service *dbusutil.Service,
	systemSigLoop *dbusutil.SignalLoop) *Device {
	var dev Device
	dev.service = service
	dev.core, _ = fprint.NewDevice(systemSigLoop.Conn(), objPath)
	dev.ScanType, _ = dev.core.ScanType().Get(0)
	dev.listenDBusSignals(systemSigLoop)
	return &dev
}

func (dev *Device) listenDBusSignals(sigLoop *dbusutil.SignalLoop) {
	dev.core.InitSignalExt(sigLoop, true)
	_, err := dev.core.ConnectEnrollStatus(func(status string, ok bool) {
		err := dev.service.Emit(dev, "EnrollStatus", status, ok)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = dev.core.ConnectVerifyStatus(func(status string, ok bool) {
		err := dev.service.Emit(dev, "VerifyStatus", status, ok)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	_, err = dev.core.ConnectVerifyFingerSelected(func(finger string) {
		err := dev.service.Emit(dev, "VerifyFingerSelected", finger)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (dev *Device) destroy() {
	dev.core.RemoveHandler(proxy.RemoveAllHandlers)
	err := dev.service.StopExport(dev)
	if err != nil {
		logger.Warning(err)
	}
}

func (dev *Device) isFree() (bool, error) {
	err := dev.core.Claim(0, "root")
	if err == nil {
		err = dev.core.Release(dbus.FlagNoAutoStart)
		if err != nil {
			logger.Warningf("failed to release device %q: %v", dev.getCorePath(), err)
		}
		return true, nil

	} else {
		if strings.Contains(err.Error(), "already claimed") {
			return false, nil
		}
		return false, err
	}
}

func (dev *Device) Claim(username string) *dbus.Error {
	err := dev.core.Claim(0, username)
	return dbusutil.ToError(err)
}

func (dev *Device) Release() *dbus.Error {
	err := dev.core.Release(0)
	return dbusutil.ToError(err)
}

func (dev *Device) EnrollStart(sender dbus.Sender, finger string) *dbus.Error {
	err := checkAuth(actionIdEnroll, string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}

	err = dev.core.EnrollStart(0, finger)
	return dbusutil.ToError(err)
}

func (dev *Device) EnrollStop(sender dbus.Sender) *dbus.Error {
	err := dev.core.EnrollStop(0)
	return dbusutil.ToError(err)
}

func (dev *Device) VerifyStart(finger string) *dbus.Error {
	err := dev.core.VerifyStart(0, finger)
	return dbusutil.ToError(err)
}

func (dev *Device) VerifyStop() *dbus.Error {
	err := dev.core.VerifyStop(0)
	return dbusutil.ToError(err)
}

func (dev *Device) DeleteEnrolledFingers(sender dbus.Sender, username string) *dbus.Error {
	err := checkAuth(actionIdDelete, string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}

	err = dev.core.DeleteEnrolledFingers(0, username)
	return dbusutil.ToError(err)
}

func (dev *Device) DeleteEnrolledFinger(sender dbus.Sender, username string, finger string) *dbus.Error {
	return dbusutil.ToError(errors.New("can not delete fprintd single finger"))
}

func (dev *Device) GetCapabilities() (caps []string, dbusErr *dbus.Error) {
	return nil, nil
}

func (dev *Device) ClaimForce(sender dbus.Sender, username string) *dbus.Error {
	return dbusutil.ToError(errors.New("can not claim force"))
}

func (dev *Device) ListEnrolledFingers(username string) (fingers []string, busErr *dbus.Error) {
	fingers, err := dev.core.ListEnrolledFingers(0, username)
	if err != nil {
		return nil, dbusutil.ToError(err)
	}
	return fingers, nil
}

func (*Device) GetInterfaceName() string {
	return dbusDeviceInterface
}

func (dev *Device) getPath() dbus.ObjectPath {
	return convertFPrintPath(dev.core.Path_())
}

func (dev *Device) getCorePath() dbus.ObjectPath {
	return dev.core.Path_()
}

func destroyDevices(list Devices) {
	for _, dev := range list {
		dev.destroy()
	}
}

func (devList Devices) Add(objPath dbus.ObjectPath, service *dbusutil.Service,
	systemSigLoop *dbusutil.SignalLoop) Devices {
	var v = newDevice(objPath, service, systemSigLoop)
	err := service.Export(v.getPath(), v)
	if err != nil {
		logger.Warning("failed to export:", objPath)
		return devList
	}

	devList = append(devList, v)
	return devList
}

func (devList Devices) Get(objPath dbus.ObjectPath) IDevice {
	for _, dev := range devList {
		if dev.getCorePath() == objPath {
			return dev
		}
	}
	return nil
}

func (devList Devices) Delete(objPath dbus.ObjectPath) Devices {
	var (
		list Devices
		v    IDevice
	)
	for _, dev := range devList {
		if dev.getCorePath() == objPath {
			v = dev
			continue
		}
		list = append(list, dev)
	}
	if v != nil {
		v.destroy()
	}
	return list
}

func convertFPrintPath(objPath dbus.ObjectPath) dbus.ObjectPath {
	return dbus.ObjectPath(dbusPath + "/Device/" + path.Base(string(objPath)))
}
