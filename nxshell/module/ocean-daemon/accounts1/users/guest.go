// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package users

import (
	"math/rand"
	"time"
)

func CreateGuestUser() (string, error) {
	shell, _ := getDefaultShell(defaultConfigShell)
	if len(shell) == 0 {
		shell = "/bin/bash"
	}

	username := getGuestUserName()
	var args = []string{"-m", "-d", "/tmp/" + username,
		"-s", shell,
		"-l", "-p", EncodePasswd(""), username}
	err := doAction(userCmdAdd, args)
	if err != nil {
		return "", err
	}

	return username, nil
}

func getGuestUserName() string {
	var (
		seedStr = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

		l    = len(seedStr)
		name = "guest-"
	)

	for i := 0; i < 6; i++ {
		rand.Seed(time.Now().UnixNano())
		index := rand.Intn(l) // #nosec G404
		name += string(seedStr[index])
	}

	return name
}
