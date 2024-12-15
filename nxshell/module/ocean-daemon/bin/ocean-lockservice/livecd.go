// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"fmt"
	"os"
	"strings"
)

func isInLiveCD(username string) bool {
	cmdline, err := os.ReadFile("/proc/cmdline")
	if err != nil {
		fmt.Println("failed to read /proc/cmdline")
		return false
	}
	return strings.Contains(string(cmdline), "boot=live")
}
