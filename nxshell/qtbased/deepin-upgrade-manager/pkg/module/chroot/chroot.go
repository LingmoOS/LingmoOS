// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package chroot

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/dirinfo"
	"deepin-upgrade-manager/pkg/module/mountpoint"
	"deepin-upgrade-manager/pkg/module/util"
	"errors"
	"os"
	"path/filepath"
	"syscall"
)

type Manager struct {
	target string
	mounts []string
	oldf   *os.File
}

func Start(root string) (Manager, error) {
	logger.Infof("start chroot %s", root)
	var m Manager
	if !util.IsExists(root) {
		return m, errors.New("root dir isn't exit")
	}
	m.target = root
	m.mounts = append(m.mounts, []string{"/proc", "/dev", "/sys", "/run"}...)

	err := m.HandleBind()
	if err != nil {
		return m, err
	}
	m.oldf, err = os.Open("/")
	if err != nil {
		return m, err
	}
	return m, syscall.Chroot(m.target)
}

func (m Manager) HandleBind() error {
	for _, v := range m.mounts {
		dst := filepath.Join(m.target, v)
		newInfo := &mountpoint.MountPoint{
			Src:  v,
			Dest: dst,
			Bind: true,
		}
		err := newInfo.Mount()
		if err != nil {
			return nil
		}
	}
	return nil
}

func (m Manager) HandleUnBind() error {
	for _, v := range m.mounts {
		dst := filepath.Join(m.target, v)
		newInfo := &mountpoint.MountPoint{
			Src:  v,
			Dest: dst,
			Bind: true,
		}
		err := newInfo.Umount()
		if err != nil {
			return nil
		}
	}
	return nil
}

func (m Manager) Exit() (err error) {
	logger.Infof("exit chroot %s", m.target)
	defer func() {
		if m.oldf != nil {
			if closeErr := m.oldf.Close(); err != nil {
				err = closeErr
			}
		}
	}()
	err = m.oldf.Chdir()
	if err != nil {
		return 
	}
	err = syscall.Chroot(".")
	if err != nil {
		return 
	}
	err = m.HandleUnBind()
	return 
}

func IsEnv() bool {
	part, err := dirinfo.GetDirPartition("/")
	if err != nil {
		return true
	}
	if !util.IsExists(part) {
		return true
	}
	return false
}
