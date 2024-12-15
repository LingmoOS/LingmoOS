// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

const (
	dbusServiceName = "org.lingmo.ocean.InputDevices1"
	dbusPath        = "/org/lingmo/ocean/InputDevices1"
	dbusInterface   = dbusServiceName

	kbdDBusPath      = "/org/lingmo/ocean/InputDevice1/Keyboard"
	kbdDBusInterface = "org.lingmo.ocean.InputDevice1.Keyboard"

	mouseDBusPath           = "/org/lingmo/ocean/InputDevice1/Mouse"
	mouseDBusInterface      = "org.lingmo.ocean.InputDevice1.Mouse"
	trackPointDBusInterface = "org.lingmo.ocean.InputDevice1.TrackPoint"

	touchPadDBusPath      = "/org/lingmo/ocean/InputDevice1/TouchPad"
	touchPadDBusInterface = "org.lingmo.ocean.InputDevice1.TouchPad"

	wacomDBusPath      = "/org/lingmo/ocean/InputDevice1/Wacom"
	wacomDBusInterface = "org.lingmo.ocean.InputDevice1.Wacom"
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
