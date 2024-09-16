// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"time"

	"github.com/godbus/dbus/v5"
)

type GrubManager struct {
	dbusDest              string
	dbusPath              dbus.ObjectPath
	dbusInterface         string
	editAuthDBusPath      dbus.ObjectPath
	editAuthDBusInterface string
}

func Init() *GrubManager {
	var m GrubManager
	osVersion, err := util.GetOSInfo("", "MajorVersion")
	if nil != err {
		logger.Warning("failed get new version, err:", err)
	}
	// default version is  v23
	if len(osVersion) == 0 || osVersion == "23" {
		m.dbusDest = "org.deepin.daemon.Grub2"
		m.dbusPath = "/org/deepin/daemon/Grub2"
		m.dbusInterface = m.dbusDest
	} else {
		m.dbusDest = "com.deepin.daemon.Grub2"
		m.dbusPath = "/com/deepin/daemon/Grub2"
		m.dbusInterface = m.dbusDest
	}
	/*
		// default version is  v20
		m.dbusDest = "com.deepin.daemon.Grub2"
		m.dbusPath = "/com/deepin/daemon/Grub2"
		m.dbusInterface = m.dbusDest
	*/

	m.editAuthDBusPath = m.dbusPath + "/EditAuthentication"
	m.editAuthDBusInterface = m.dbusInterface + ".EditAuthentication"

	_, err = m.TimeOut()
	if err != nil {
		m = *m.ChangeDbusDest()
	}

	return &m
}

func (m *GrubManager) ChangeDbusDest() *GrubManager {
	if m.dbusDest == "com.deepin.daemon.Grub2" {
		m.dbusDest = "org.deepin.daemon.Grub2"
		m.dbusPath = "/org/deepin/daemon/Grub2"
		m.dbusInterface = m.dbusDest
	} else {
		m.dbusDest = "com.deepin.daemon.Grub2"
		m.dbusPath = "/com/deepin/daemon/Grub2"
		m.dbusInterface = m.dbusDest
	}
	m.editAuthDBusPath = m.dbusPath + "/EditAuthentication"
	m.editAuthDBusInterface = m.dbusInterface + ".EditAuthentication"
	return m
}

func (m *GrubManager) Reset() error {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	grubServiceObj := sysBus.Object(m.dbusDest,
		m.dbusPath)
	metho := m.dbusDest + ".Reset"

	err = grubServiceObj.Call(metho, 0).Store()
	if err != nil {
		return err
	}
	return nil
}

func (m *GrubManager) Join() error {
	ch := make(chan bool)
	go func(ch chan bool) {
		for {
			time.Sleep(100 * time.Millisecond)
			canExit, err := m.IsUpdating()
			if !canExit || err != nil {
				ch <- true
			}
		}
	}(ch)
	canExit, err := m.IsUpdating()
	if err != nil {
		logger.Warning("failed get properties, %v", err)
	}
	if canExit && nil == err {
		ticker := time.NewTicker(3 * time.Minute)
		for {
			select {
			case <-ticker.C:
				return nil
			case <-ch:
				return nil
			}
		}
	}
	return nil
}

func (m *GrubManager) SetTimeout(timeout uint32) error {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	grubServiceObj := sysBus.Object(m.dbusDest,
		m.dbusPath)
	metho := m.dbusDest + ".SetTimeout"

	err = grubServiceObj.Call(metho, 0, timeout).Store()
	if err != nil {
		return err
	}
	return nil
}

func (m *GrubManager) TimeOut() (uint32, error) {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return 0, err
	}
	grubServiceObj := sysBus.Object(m.dbusDest,
		m.dbusPath)

	var ret dbus.Variant
	err = grubServiceObj.Call("org.freedesktop.DBus.Properties.Get", 0, m.dbusInterface, "Timeout").Store(&ret)
	if err != nil {
		return 0, err
	}
	v := ret.Value().(uint32)

	return v, nil
}

func (m *GrubManager) IsUpdating() (bool, error) {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return false, err
	}
	grubServiceObj := sysBus.Object(m.dbusDest,
		m.dbusPath)

	var ret dbus.Variant
	err = grubServiceObj.Call("org.freedesktop.DBus.Properties.Get", 0, m.dbusInterface, "Updating").Store(&ret)
	if err != nil {
		return false, err
	}
	v := ret.Value().(bool)
	return v, nil
}

func (m *GrubManager) GetEnabledUsers() ([]string, error) {
	var userList []string
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return userList, err
	}

	grubServiceObj := sysBus.Object(m.dbusDest,
		m.editAuthDBusPath)

	var ret dbus.Variant
	err = grubServiceObj.Call("org.freedesktop.DBus.Properties.Get", 0, m.editAuthDBusInterface, "EnabledUsers").Store(&ret)
	if err != nil {
		return userList, err
	}
	v := ret.Value().([]string)
	return v, nil
}
