// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package users

import (
	"fmt"
	"os/exec"
	"strconv"

	"github.com/linuxdeepin/go-lib/keyfile"
)

func isStrInArray(str string, array []string) bool {
	for _, v := range array {
		if v == str {
			return true
		}
	}

	return false
}

func doAction(cmd string, args []string) error {
	fmt.Println("run cmd", cmd)
	out, err := exec.Command(cmd, args...).CombinedOutput()
	if err != nil {
		fmt.Printf("[doAction] exec '%s' failed: %s, %v\n", cmd, string(out), err)
	}
	return err
}

func strToInt(str string, defaultVal int) int {
	if str == "" {
		return defaultVal
	}
	val, err := strconv.Atoi(str)
	if err != nil {
		val = defaultVal
	}
	return val
}

func systemType() string {
	kf := keyfile.NewKeyFile()
	err := kf.LoadFromFile("/etc/deepin-version")
	if err != nil {
		fmt.Println("load version file failed, err: ", err)
		return ""
	}
	typ, err := kf.GetString("Release", "Type")
	if err != nil {
		fmt.Println("get version type failed, err: ", err)
		return ""
	}
	return typ
}
