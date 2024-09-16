// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package plymouth

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"strconv"
	"time"
)

var progress int

func UpdateProgress(pro int) {
	if pro == 0 {
		update()
	} else {
		progress = pro
	}
}

func update() {
	go func() {
		for {
			time.Sleep(100 * time.Millisecond)
			if progress == 100 {
				return
			}
			pro := strconv.Itoa(progress)
			update := "--progress=" + pro
			err := util.ExecCommand("/usr/bin/plymouth", []string{"system-update", update})
			if err != nil {
				logger.Warningf("update cmd action:%s,args:%v", "/usr/bin/plymouth, err: %v", []string{"system-update", update}, err)
				return
			}
		}
	}()
}
