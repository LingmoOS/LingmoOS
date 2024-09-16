// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"errors"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	fstartDBusPath      = dbusPath + "/Fstart"
	fstartDBusInterface = dbusInterface + ".Fstart"
)

const (
	deepinFstartFile = "/etc/default/grub.d/15_deepin_fstart.cfg"
	deepinFstart     = "DEEPIN_FSTART"
)

func (*Fstart) GetInterfaceName() string {
	return fstartDBusInterface
}

func (f *Fstart) SkipGrub(sender dbus.Sender, enabled bool) *dbus.Error {
	err := checkInvokePermission(f.service, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	f.service.DelayAutoQuit()

	f.g.PropsMu.Lock()
	defer f.g.PropsMu.Unlock()
	isEnabled, lineNum := getFstartState()
	if lineNum == -1 {
		return dbusutil.ToError(errors.New(deepinFstartFile + "is illegal!"))
	}
	if isEnabled == enabled {
		logger.Debug("current state is same : ", enabled)
		return nil
	}
	err = setFstartState(enabled)
	if err != nil {
		return dbusutil.ToError(err)
	}

	if f.setPropIsSkipGrub(enabled) {
		f.g.addModifyTask(modifyTask{})
	}
	return nil
}
