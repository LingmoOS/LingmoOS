// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"fmt"
	"os/exec"
	"sync"

	"github.com/linuxdeepin/go-gir/gio-2.0"
)

const (
	xsettingsSchema   = "com.deepin.xsettings"
	xsPropBlinkTimeut = "cursor-blink-time"
	xsPropDoubleClick = "double-click-time"
	xsPropDragThres   = "dnd-drag-threshold"
)

var (
	xsLocker  sync.Mutex
	xsSetting = gio.NewSettings(xsettingsSchema)
)

func xsSetInt32(prop string, value int32) {
	xsLocker.Lock()
	if value == xsSetting.GetInt(prop) {
		xsLocker.Unlock()
		return
	}
	xsSetting.SetInt(prop, value)
	xsLocker.Unlock()
}

func addItemToList(item string, list []string) ([]string, bool) {
	if isItemInList(item, list) {
		return list, false
	}

	list = append(list, item)
	return list, true
}

func delItemFromList(item string, list []string) ([]string, bool) {
	var (
		found bool
		ret   []string
	)

	for _, v := range list {
		if v == item {
			found = true
			continue
		}
		ret = append(ret, v)
	}

	return ret, found
}

func filterSpaceStr(list []string) []string {
	var ret []string
	for _, v := range list {
		if v != "" {
			ret = append(ret, v)
		}
	}
	return ret
}

func isItemInList(item string, list []string) bool {
	for _, v := range list {
		if v == item {
			return true
		}
	}
	return false
}

func doAction(cmd string) error {
	out, err := exec.Command("/bin/sh", "-c", cmd).CombinedOutput()
	if err != nil {
		return fmt.Errorf(string(out))
	}
	return nil
}
