// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package logined

import (
	"github.com/godbus/dbus/v5"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
)

// SessionInfo Show logined session info, if type is tty or ssh, no desktop and display
type SessionInfo struct {
	// Active  bool
	Uid     uint32
	Desktop string
	Display string

	sessionPath dbus.ObjectPath
}

// SessionInfos Logined session list
type SessionInfos []*SessionInfo

func newSessionInfo(sessionPath dbus.ObjectPath) (*SessionInfo, error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	core, err := login1.NewSession(systemBus, sessionPath)
	if err != nil {
		return nil, err
	}

	userInfo, err := core.User().Get(0)
	if err != nil {
		return nil, err
	}

	desktop, _ := core.Desktop().Get(0)
	display, _ := core.Display().Get(0)

	var info = SessionInfo{
		Uid:         userInfo.UID,
		Desktop:     desktop,
		Display:     display,
		sessionPath: sessionPath,
	}

	return &info, nil
}

// Add Add user to list, if exist and equal, return false
// else replace it, return true
func (infos SessionInfos) Add(info *SessionInfo) (SessionInfos, bool) {
	idx := infos.Index(info.sessionPath)
	if idx != -1 {
		if infos[idx].Equal(info) {
			return infos, false
		}
		infos[idx] = info
	} else {
		infos = append(infos, info)
	}
	return infos, true
}

// Index Find the user position in list, if not found, return -1
func (infos SessionInfos) Index(p dbus.ObjectPath) int32 {
	for i, v := range infos {
		if v.sessionPath != p {
			continue
		}

		return int32(i)
	}
	return -1
}

// Delete Delete user from list, if deleted, return true
func (infos SessionInfos) Delete(p dbus.ObjectPath) (SessionInfos, bool) {
	var (
		tmp     SessionInfos
		deleted = false
	)
	for _, v := range infos {
		if v.sessionPath == p {
			deleted = true
			v = nil
			continue
		}
		tmp = append(tmp, v)
	}
	return tmp, deleted
}

// Equal Check whether equal with target
func (info *SessionInfo) Equal(target *SessionInfo) bool {
	if info.sessionPath != target.sessionPath || info.Uid != target.Uid ||
		info.Desktop != target.Desktop || info.Display != target.Display {
		return false
	}
	return true
}
