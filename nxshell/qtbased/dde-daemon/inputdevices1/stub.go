// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

const (
	dbusServiceName = "org.deepin.dde.InputDevices1"
	dbusPath        = "/org/deepin/dde/InputDevices1"
	dbusInterface   = dbusServiceName

	kbdDBusPath      = "/org/deepin/dde/InputDevice1/Keyboard"
	kbdDBusInterface = "org.deepin.dde.InputDevice1.Keyboard"

	mouseDBusPath           = "/org/deepin/dde/InputDevice1/Mouse"
	mouseDBusInterface      = "org.deepin.dde.InputDevice1.Mouse"
	trackPointDBusInterface = "org.deepin.dde.InputDevice1.TrackPoint"

	touchPadDBusPath      = "/org/deepin/dde/InputDevice1/TouchPad"
	touchPadDBusInterface = "org.deepin.dde.InputDevice1.TouchPad"

	wacomDBusPath      = "/org/deepin/dde/InputDevice1/Wacom"
	wacomDBusInterface = "org.deepin.dde.InputDevice1.Wacom"
)

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func (*Keyboard) GetInterfaceName() string {
	return kbdDBusInterface
}

func (*Mouse) GetInterfaceName() string {
	return mouseDBusInterface
}

func (*TrackPoint) GetInterfaceName() string {
	return trackPointDBusInterface
}

func (*Touchpad) GetInterfaceName() string {
	return touchPadDBusInterface
}

func (*Wacom) GetInterfaceName() string {
	return wacomDBusInterface
}
