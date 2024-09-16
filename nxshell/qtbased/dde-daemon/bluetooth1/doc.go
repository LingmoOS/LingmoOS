// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
Package bluetooth is a high level dbus warp for bluez5. You SHOULD always use dbus to call
the interface of Bluetooth.

The dbus interface of bluetooth to operate bluez is designed to asynchronous, and all result is returned by signal.
Other interface to get adapter/device informations will return immediately, because it was cached.
*/
package bluetooth
