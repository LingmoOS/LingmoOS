// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	dutils "github.com/lingmoos/go-lib/utils"
)

const (
	versionFile = "/etc/lingmo-version"
)

func getLingmoReleaseType() string {
	keyFile, err := dutils.NewKeyFileFromFile(versionFile)
	if err != nil {
		return ""
	}
	defer keyFile.Free()
	releaseType, _ := keyFile.GetString("Release", "Type")
	return releaseType
}
