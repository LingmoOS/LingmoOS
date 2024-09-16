// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/grub_common"
	polkit "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.policykit1"
)

func quoteString(str string) string {
	return strconv.Quote(str)
}

func checkGfxmode(v string) error {
	if v == "auto" {
		return nil
	}

	_, err := grub_common.ParseGfxmode(v)
	return err
}

func getStringIndexInArray(a string, list []string) int {
	for i, b := range list {
		if b == a {
			return i
		}
	}
	return -1
}

var noCheckAuth bool

func allowNoCheckAuth() {
	if os.Getenv("NO_CHECK_AUTH") == "1" {
		noCheckAuth = true
		return
	}
}

func checkAuth(sysBusName, actionId string) (bool, error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return false, err
	}
	authority := polkit.NewAuthority(systemBus)
	subject := polkit.MakeSubject(polkit.SubjectKindSystemBusName)
	subject.SetDetail("name", sysBusName)
	result, err := authority.CheckAuthorization(0, subject, actionId, nil,
		polkit.CheckAuthorizationFlagsAllowUserInteraction, "")
	if err != nil {
		return false, err
	}

	return result.IsAuthorized, nil
}

var errAuthFailed = errors.New("authentication failed")

const (
	systemLocaleFile  = "/etc/default/locale"
	systemdLocaleFile = "/etc/locale.conf"
)

func getSystemLocale() (locale string) {
	files := []string{
		systemLocaleFile,
		systemdLocaleFile, // It is used by systemd to store system-wide locale settings
	}

	for _, file := range files {
		locale, _ = getLocaleFromFile(file)
		if locale != "" {
			// get locale success
			break
		}
	}
	return
}

func getLocaleFromFile(filename string) (string, error) {
	// TODO: 重构代码，代码复制自 langselector 模块
	infos, err := readEnvFile(filename)
	if err != nil {
		return "", err
	}

	var locale string
	for _, info := range infos {
		if info.key != "LANG" {
			continue
		}
		locale = info.value
	}

	locale = strings.Trim(locale, " '\"")
	return locale, nil
}

type envInfo struct {
	key   string
	value string
}
type envInfos []envInfo

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

func getTempDir() (string, error) {
	return ioutil.TempDir("", "grub2_theme_*")
}

func copyBgSource(src, dst string) error {
	srcFile := filepath.Join(src, "background_source")
	dstFile := filepath.Join(dst, "background_source")
	err := os.Remove(dstFile)
	if err != nil && !os.IsNotExist(err) {
		return err
	}
	err = os.MkdirAll(filepath.Dir(dstFile), 0755)
	if err != nil {
		return err
	}
	_, err = copyFile(srcFile, dstFile)
	return err
}

func copyFile(src, dst string) (int64, error) {
	sourceFileStat, err := os.Stat(src)
	if err != nil {
		return 0, err
	}

	if !sourceFileStat.Mode().IsRegular() {
		return 0, fmt.Errorf("%s is not a regular file", src)
	}

	source, err := os.Open(src)
	if err != nil {
		return 0, err
	}
	defer source.Close()

	destination, err := os.Create(dst)
	if err != nil {
		return 0, err
	}
	defer destination.Close()
	nBytes, err := io.Copy(destination, source)
	return nBytes, err
}

func replaceAndBackupDir(src, dst string) error {
	bakDir := src + ".bak"
	err := os.RemoveAll(bakDir)
	if err != nil && !os.IsNotExist(err) {
		return err
	}
	err = os.Rename(src, bakDir)
	if err != nil && !os.IsNotExist(err) {
		return err
	}
	err = os.Rename(dst, src)
	if err != nil {
		os.Rename(bakDir, src)
		return err
	}

	return nil
}
