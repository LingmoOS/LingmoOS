// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import "github.com/godbus/dbus/v5"

type NotifyMsg struct {
	Icon    string
	Summary *LocalizeStr
	Body    *LocalizeStr
}

type LocalizeStr struct {
	Format string
	Args   []string
}

const (
	ErrNameRejected = "org.bluez.Error.Rejected"
	ErrNameCanceled = "org.bluez.Error.Canceled"
)

var ErrRejected = &dbus.Error{
	Name: ErrNameRejected,
	Body: []interface{}{"rejected"},
}

var ErrCanceled = &dbus.Error{
	Name: ErrNameCanceled,
	Body: []interface{}{"canceled"},
}

// SessionAgentPath 目前唯一支持的标准的 session agent 对象路径
const SessionAgentPath = "/org/deepin/dde/Bluetooth1/Agent"
