// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// funtion for atomic upgrade module
package atomic

import (
	"lingmo-system-upgrade-daemon/pkg/module/dbustools"
	"errors"
	"os/exec"
	"strings"

	"github.com/lingmoos/go-lib/log"
)

var logger = log.NewLogger("lingmo-system-upgrade/module/atomic")

func GetOStreeRepoInfo(infoType string) (string, error) {
	repoUUid, err := dbustools.GetProperty("org.lingmo.AtomicUpgrade1", "org.lingmo.AtomicUpgrade1", "RepoUUID", "/org/lingmo/AtomicUpgrade1")
	if err != nil {
		logger.Warning("failed to get repo uuid:", err)
		return "", err
	}

	out, err := exec.Command("/usr/bin/findmnt", "-rn", "-S", "UUID="+repoUUid.String(), "-o", infoType).CombinedOutput()
	if err != nil {
		logger.Warning("failed to findmnt:", string(out))
		return "", err
	}
	mps := strings.Split(string(out), "\n")
	if len(mps) < 1 {
		return "", errors.New("get empty repo location")
	}
	return mps[0], nil
}
