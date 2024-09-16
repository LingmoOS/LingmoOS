// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"os"
	"path"

	"github.com/linuxdeepin/go-lib/utils"
)

const (
	// 50MB
	filesystemMinFreeSize = 50 * 1024 * 1024
)

// checkLeftSpace Check disk left space, if no space left, then remove '~/.cache'
func (u *User) checkLeftSpace() {
	info, err := utils.QueryFilesytemInfo(u.HomeDir)
	if err != nil {
		logger.Warning("Failed to get filesystem info:", err, u.HomeDir)
		return
	}
	logger.Debugf("--------User '%s' left space: %#v", u.UserName, info)
	if info.AvailSize > filesystemMinFreeSize {
		return
	}

	logger.Debugf("No space left, will remove %s's .cache", u.UserName)
	u.removeCache()
}

func (u *User) removeCache() {
	var file = path.Join(u.HomeDir + ".cache")
	logger.Debug("-------Will remove:", file)
	err := os.RemoveAll(file)
	if err != nil {
		logger.Warningf("Failed to remove %s's .cache: %s", u.UserName, err.Error())
	}
}
