// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package langselector

import (
	"io/ioutil"
	"strings"
)

type envInfo struct {
	key   string
	value string
}
type envInfos []envInfo

const (
	systemLocaleFile  = "/etc/default/locale"
	systemdLocaleFile = "/etc/locale.conf"

	defaultLocale = "en_US.UTF-8"
)

func LocalLangEnv() (result []string) {
	result = append(result, "LANG="+getCurrentUserLocale("LANG"))
	result = append(result, "LANGUAGE="+getCurrentUserLocale("LANGUAGE"))
	return
}

func getCurrentUserLocale(key string) (locale string) {
	files := []string{
		systemLocaleFile,
		systemdLocaleFile, // It is used by systemd to store system-wide locale settings
	}

	for _, file := range files {
		locale, _ = getLocaleFromFile(file, key)
		if locale != "" {
			// get locale success
			break
		}
	}

	if locale == "" {
		return defaultLocale
	}
	return locale
}

func getLocaleFromFile(filename, key string) (string, error) {
	infos, err := readEnvFile(filename)
	if err != nil {
		return "", err
	}

	var locale string
	for _, info := range infos {
		if info.key != key {
			continue
		}
		locale = info.value
	}

	locale = strings.Trim(locale, " '\"")
	return locale, nil
}

func readEnvFile(file string) (envInfos, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var (
		infos envInfos
		lines = strings.Split(string(content), "\n")
	)
	for _, line := range lines {
		var array = strings.Split(line, "=")
		if len(array) != 2 {
			continue
		}

		infos = append(infos, envInfo{
			key:   array[0],
			value: array[1],
		})
	}

	return infos, nil
}
