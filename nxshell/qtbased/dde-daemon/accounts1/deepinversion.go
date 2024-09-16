// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const (
	versionFile = "/etc/deepin-version"
)

func getDeepinReleaseType() string {
	keyFile, err := dutils.NewKeyFileFromFile(versionFile)
	if err != nil {
		return ""
	}
	defer keyFile.Free()
	releaseType, _ := keyFile.GetString("Release", "Type")
	return releaseType
}
