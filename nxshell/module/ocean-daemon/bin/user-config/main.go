// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"fmt"
	"os"
	"os/user"
	"strings"
)

func helper() {
	fmt.Println("Initialize the user configuration, if the configuration files exist out directly.")
	fmt.Println("\nUsage: user-config [username]")
	fmt.Println("\tIf the user is not specified, will configure the current user.")
}

func getUsername(args []string) (string, bool, error) {
	if len(args) == 1 {
		u, err := user.Current()
		if err != nil {
			return "", false, err
		}
		return u.Username, false, nil
	}

	var arg = strings.ToLower(args[1])
	if arg == "-h" || arg == "--help" {
		return "", true, nil
	}

	return args[1], false, nil
}

func main() {
	name, help, err := getUsername(os.Args)
	if err != nil {
		fmt.Println("Parse arguments failed:", err)
		return
	}

	if help {
		helper()
		return
	}

	fmt.Printf("Start init '%s' configuration.\n", name)
	CopyUserDatas(name)
	fmt.Printf("Init '%s' configuration over.\n", name)
}
