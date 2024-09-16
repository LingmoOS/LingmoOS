// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

// #cgo pkg-config: libinput libudev
// #cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
// #include <libinput.h>
// #include <libudev.h>
// #include "dde-libinput.h"
import "C"
import (
	"unsafe"

	"github.com/linuxdeepin/go-lib/log"
)

//export log_handler_go
func log_handler_go(priority C.enum_libinput_log_priority, cstr *C.char) {
	str := C.GoString(cstr)
	switch priority {
	case C.LIBINPUT_LOG_PRIORITY_DEBUG:
		logger.Debug(str)
	case C.LIBINPUT_LOG_PRIORITY_INFO:
		logger.Info(str)
	case C.LIBINPUT_LOG_PRIORITY_ERROR:
		logger.Error(str)
	}
}

//export handle_device_added
func handle_device_added(event *C.struct_libinput_event, userdata unsafe.Pointer) {
	dev := C.libinput_event_get_device(event)
	notify_device_changed(event, userdata, true)
	if C.libinput_device_has_capability(dev, C.LIBINPUT_DEVICE_CAP_TOUCH) == 0 {
		return
	}

	l := (*libinput)(userdata)
	l.i.newTouchscreen(newLibinputDevice(dev))
}

//export handle_device_removed
func handle_device_removed(event *C.struct_libinput_event, userdata unsafe.Pointer) {
	dev := C.libinput_event_get_device(event)
	notify_device_changed(event, userdata, false)
	if C.libinput_device_has_capability(dev, C.LIBINPUT_DEVICE_CAP_TOUCH) == 0 {
		return
	}

	l := (*libinput)(userdata)
	l.i.removeTouchscreen(newLibinputDevice(dev))
}

func notify_device_changed(event *C.struct_libinput_event, userdata unsafe.Pointer, state bool) {
	dev := C.libinput_event_get_device(event)
	devName := C.GoString(C.libinput_device_get_name(dev))
	udev := C.libinput_device_get_udev_device(dev)
	devNode := C.GoString(C.udev_device_get_devnode(udev))
	devPath := C.GoString(C.udev_device_get_property_value(udev, C.CString("DEVPATH")))
	l := (*libinput)(userdata)
	err := l.i.service.Emit(l.i, "EventChanged", devNode, devName, devPath, state)
	if err != nil {
		logger.Warning(" [notify_device_changed] err : ", err)
	} else {
		logger.Debugf(" [notify_device_changed] devNode : %s; devName : %s; dir : %s; state : %v ", devNode, devName, devPath, state)
		l.i.updateSupportWakeupDevices()
	}
}

type libinput struct {
	data   *C.struct_data
	stopCh chan struct{}

	i *InputDevices
}

func newLibinput(i *InputDevices) *libinput {
	var priority C.enum_libinput_log_priority = C.LIBINPUT_LOG_PRIORITY_ERROR

	debugLevel := logger.GetLogLevel()
	switch debugLevel {
	case log.LevelInfo:
		priority = C.LIBINPUT_LOG_PRIORITY_INFO
	case log.LevelDebug:
		priority = C.LIBINPUT_LOG_PRIORITY_DEBUG
	}

	l := new(libinput)
	l.i = i
	l.data = C.new_libinput((*C.char)(unsafe.Pointer(l)), priority)

	return l
}

func (l *libinput) start() {
	l.stopCh = make(chan struct{}, 1)

	go func() {
		C.start(l.data)

		l.stopCh <- struct{}{}
	}()
}

func (l *libinput) stop() {
	C.stop(l.data)

	<-l.stopCh
}

type libinputDevice struct {
	dev *C.struct_libinput_device
}

func newLibinputDevice(dev *C.struct_libinput_device) *libinputDevice {
	return &libinputDevice{
		dev: dev,
	}
}

func (d *libinputDevice) GetName() string {
	return C.GoString(C.libinput_device_get_name(d.dev))
}

func (d *libinputDevice) GetOutputName() string {
	return C.GoString(C.libinput_device_get_output_name(d.dev))
}

func (d *libinputDevice) GetSize() (float64, float64) {
	var width C.double
	var height C.double
	C.libinput_device_get_size(d.dev, &width, &height)

	return float64(width), float64(height)
}

func (d *libinputDevice) GetPhys() string {
	udev := C.libinput_device_get_udev_device(d.dev)
	v := C.udev_device_get_sysattr_value(udev, C.CString("phys"))
	if v == nil {
		parent := C.udev_device_get_parent(udev)
		v = C.udev_device_get_sysattr_value(parent, C.CString("phys"))
	}
	C.udev_device_unref(udev)

	return C.GoString(v)
}

func (d *libinputDevice) GetDevNode() string {
	udev := C.libinput_device_get_udev_device(d.dev)
	v := C.GoString(C.udev_device_get_devnode(udev))
	C.udev_device_unref(udev)
	return v
}
func (d *libinputDevice) GetSysAttr(k string) string {
	udev := C.libinput_device_get_udev_device(d.dev)
	v := C.udev_device_get_sysattr_value(udev, C.CString(k))
	C.udev_device_unref(udev)
	return C.GoString(v)
}

func (d *libinputDevice) GetProperty(k string) string {
	udev := C.libinput_device_get_udev_device(d.dev)
	v := C.GoString(C.udev_device_get_property_value(udev, C.CString(k)))
	C.udev_device_unref(udev)
	return v
}
