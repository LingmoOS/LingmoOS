// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Make directory hardlink
package hardlink

import (
	"deepin-upgrade-manager/pkg/module/util"
)

func HardlinkDir(srcDir, dstDir string) error {

	return util.CopyDir(srcDir, dstDir, []string{}, []string{}, true)
}
