// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/accounts1/users"
	polkit "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.policykit1"
	"github.com/linuxdeepin/go-lib/encoding/kv"
	"github.com/linuxdeepin/go-lib/graphic"
	"github.com/linuxdeepin/go-lib/utils"
)

// #nosec G101
const (
	polkitActionUserAdministration     = "org.deepin.dde.accounts.user-administration"
	polkitActionChangeOwnData          = "org.deepin.dde.accounts.change-own-user-data"
	polkitActionEnableAutoLogin        = "org.deepin.dde.accounts.enable-auto-login"
	polkitActionDisableAutoLogin       = "org.deepin.dde.accounts.disable-auto-login"
	polkitActionEnableNoPasswordLogin  = "org.deepin.dde.accounts.enable-nopass-login"
	polkitActionDisableNoPasswordLogin = "org.deepin.dde.accounts.disable-nopass-login"
	polkitActionEnableWechatAuth       = "org.deepin.dde.accounts.enable-wechat-auth"
	polkitActionDisableWechatAuth      = "org.deepin.dde.accounts.disable-wechat-auth"
	polkitActionSetKeyboardLayout      = "org.deepin.dde.accounts.set-keyboard-layout"
	polkitActionEnableQuickLogin       = "org.deepin.dde.accounts.enable-quick-login"
	polkitActionDisableQuickLogin      = "org.deepin.dde.accounts.disable-quick-login"

	systemLocaleFile  = "/etc/default/locale"
	systemdLocaleFile = "/etc/locale.conf"
	defaultLocale     = "en_US.UTF-8"

	layoutDelimiter   = ";"
	defaultLayout     = "us" + layoutDelimiter
	defaultLayoutFile = "/etc/default/keyboard"
)

type ErrCodeType int32

const (
	// 未知错误
	ErrCodeUnkown ErrCodeType = iota
	// 权限认证失败
	ErrCodeAuthFailed
	// 执行命令失败
	ErrCodeExecFailed
	// 传入的参数不合法
	ErrCodeParamInvalid
)

func (code ErrCodeType) String() string {
	switch code {
	case ErrCodeUnkown:
		return "Unkown error"
	case ErrCodeAuthFailed:
		return "Policykit authentication failed"
	case ErrCodeExecFailed:
		return "Exec command failed"
	case ErrCodeParamInvalid:
		return "Invalid parameters"
	}

	return "Unkown error"
}

// return icons uris
func getUserIcons() ([]string, []string) {
	var paths []string
	var icons []string
	var customIcons []string

	if err := filepath.Walk(userIconsDir,
		func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}

			if info.IsDir() && info.Name() != "icons" {
				paths = append(paths, path)
			}

			return nil
		},
	); err != nil {
		logger.Warning("failed to walk usr icon path", err)
		return icons, customIcons
	}

	for _, path := range paths {
		imgs, err := graphic.GetImagesInDir(path)
		if err != nil {
			logger.Warning("failed to get user icon images", err)
			return nil, nil
		}

		for _, img := range imgs {
			img = utils.EncodeURI(img, utils.SCHEME_FILE)
			if strings.Contains(img, userCustomIconsDir) {
				customIcons = append(customIcons, img)
			} else {
				icons = append(icons, img)
			}
		}
	}

	return icons, customIcons
}

// 从系统的用户头像中随机获取一张用户图片
// NOTE: 此处需要安装新的dde-account-faces包, 否则会造成找不到用户头像的问题
func getRandomIcon() string {
	if len(_userStandardIcons) > 0 {
		return _userStandardIcons[rand.Intn(len(_userStandardIcons))]
	}

	return ""
}

func getNewUserCustomIconDest(username string) string {
	ns := time.Now().UnixNano()
	base := username + "-" + strconv.FormatInt(ns, 36) + ".png"
	return filepath.Join(userCustomIconsDir, base)
}

func isStrInArray(str string, array []string) bool {
	for _, v := range array {
		if v == str {
			return true
		}
	}

	return false
}

func isStrvEqual(l1, l2 []string) bool {
	if len(l1) != len(l2) {
		return false
	}

	sort.Strings(l1)
	sort.Strings(l2)
	for i, v := range l1 {
		if v != l2[i] {
			return false
		}
	}
	return true
}

func checkAccountType(accountType int) error {
	switch accountType {
	case users.UserTypeStandard, users.UserTypeAdmin:
		return nil
	default:
		return fmt.Errorf("invalid account type %d", accountType)
	}
}

func checkAuth(actionId string, sysBusName string) error {
	ret, err := checkAuthByPolkit(actionId, sysBusName)
	if err != nil {
		return err
	}
	if !ret.IsAuthorized {
		inf, err := getDetailsKey(ret.Details, "polkit.dismissed")
		if err == nil {
			if dismiss, ok := inf.(string); ok {
				if dismiss != "" {
					return errors.New("")
				}
			}
		}
		return fmt.Errorf(ErrCodeAuthFailed.String())
	}
	return nil
}

func checkAuthByPolkit(actionId string, sysBusName string) (ret polkit.AuthorizationResult, err error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return
	}
	authority := polkit.NewAuthority(systemBus)
	subject := polkit.MakeSubject(polkit.SubjectKindSystemBusName)
	subject.SetDetail("name", sysBusName)

	ret, err = authority.CheckAuthorization(0, subject,
		actionId, nil,
		polkit.CheckAuthorizationFlagsAllowUserInteraction, "")
	if err != nil {
		logger.Warningf("call check auth failed, err: %v", err)
		return
	}
	logger.Debugf("call check auth success, ret: %v", ret)
	return
}

func getDetailsKey(details map[string]dbus.Variant, key string) (interface{}, error) {
	result, ok := details[key]
	if !ok {
		return nil, errors.New("key dont exist in details")
	}
	if utils.IsInterfaceNil(result) {
		return nil, errors.New("result is nil")
	}
	return result.Value(), nil
}

func getDefaultLocale() (locale string) {
	files := [...]string{
		systemLocaleFile,
		systemdLocaleFile,
	}
	for _, file := range files {
		locale = getLocaleFromFile(file)
		if locale != "" {
			// get locale success
			break
		}
	}
	if locale == "" {
		return defaultLocale
	}

	return strings.Trim(locale, "\"'")
}

func getLocaleFromFile(file string) string {
	f, err := os.Open(file)
	if err != nil {
		return ""
	}
	defer f.Close()

	r := kv.NewReader(f)
	r.Delim = '='
	r.Comment = '#'
	r.TrimSpace = kv.TrimLeadingTailingSpace
	for {
		pair, err := r.Read()
		if err == io.EOF {
			break
		} else if err != nil {
			return ""
		}

		if pair.Key == "LANG" {
			return pair.Value
		}
	}
	return ""
}

func getDefaultLayout() string {
	layout, err := getSystemLayout(defaultLayoutFile)
	if err != nil {
		logger.Warning("failed to get system default layout:", err)
		return defaultLayout
	}
	return layout
}

func getSystemLayout(file string) (string, error) {
	fr, err := os.Open(file)
	if err != nil {
		return "", err
	}
	defer fr.Close()

	var (
		found   int
		layout  string
		variant string

		regLayout  = regexp.MustCompile(`^XKBLAYOUT=`)
		regVariant = regexp.MustCompile(`^XKBVARIANT=`)

		scanner = bufio.NewScanner(fr)
	)
	for scanner.Scan() {
		if found == 2 {
			break
		}

		var line = scanner.Text()
		if regLayout.MatchString(line) {
			layout = strings.Trim(getValueFromLine(line, "="), "\"")
			found += 1
			continue
		}

		if regVariant.MatchString(line) {
			variant = strings.Trim(getValueFromLine(line, "="), "\"")
			found += 1
		}
	}

	if len(layout) == 0 {
		return "", fmt.Errorf("not found default layout")
	}

	return layout + layoutDelimiter + variant, nil
}

func getValueFromLine(line, delim string) string {
	array := strings.Split(line, delim)
	if len(array) != 2 {
		return ""
	}

	return strings.TrimSpace(array[1])
}

// Get available shells from '/etc/shells'
func getAvailableShells(file string) []string {
	contents, err := ioutil.ReadFile(file)
	if err != nil || len(contents) == 0 {
		return nil
	}
	var shells []string
	lines := strings.Split(string(contents), "\n")
	for _, line := range lines {
		if line == "" || line[0] == '#' {
			continue
		}
		shells = append(shells, line)
	}
	return shells
}

// 修复目录 ownership
func recoverOwnership(u *User) error {
	uid, err := strconv.Atoi(u.Uid)
	if err != nil {
		return err
	}
	gid, err := strconv.Atoi(u.Gid)
	if err != nil {
		return err
	}
	return filepath.Walk(u.HomeDir, func(name string, info os.FileInfo, err error) error {
		if err == nil {
			err = os.Chown(name, uid, gid)
		}
		return err
	})
}
