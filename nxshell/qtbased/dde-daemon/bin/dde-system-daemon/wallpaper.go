// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"sync"

	dbus "github.com/godbus/dbus/v5"
	polkit "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.policykit1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const maxCount = 5
const maxSize = 32 * 1024 * 1024
const wallPaperDir = "/usr/share/wallpapers/custom-wallpapers/"
const solidWallPaperPath = "/usr/share/wallpapers/custom-solidwallpapers/"
const solidPrefix = "solid::"
const polkitActionUserAdministration = "org.deepin.dde.accounts.user-administration"

var wallPaperDirs = []string{
	wallPaperDir,
	solidWallPaperPath,
}

func checkPath(path string, dirs []string) string {
	for _, dir := range dirs {
		if strings.HasPrefix(path, dir) {
			return dir
		}
	}
	return ""
}

func GetUserDirs(username string) (dirs []string, err error) {
	dirs = make([]string, 0)
	for _, wallPaperDir := range wallPaperDirs {
		dir := filepath.Join(wallPaperDir, username)
		dir, err := filepath.Abs(dir)

		if err != nil {
			continue
		}

		if !strings.HasPrefix(dir, wallPaperDir) {
			continue
		}

		info, err := os.Stat(dir)

		if err != nil && !os.IsExist(err) {
			os.MkdirAll(dir, 0755)
		}

		if info != nil && !info.IsDir() {
			return nil, errors.New("UsernName is not a dir")
		}
		dirs = append(dirs, dir)
	}
	return dirs, nil
}

func RemoveOverflowWallPapers(username string, max int) error {
	dirs, err := GetUserDirs(username)
	if err != nil {
		logger.Warning(err)
		return err
	}
	for _, dir := range dirs {
		fileinfos, err := ioutil.ReadDir(dir)
		if err != nil {
			logger.Warning(err)
			continue
		}

		logger.Debugf("is count %d <= %d ?", len(fileinfos), max)
		if len(fileinfos) <= max {
			continue
		}

		sort.Slice(fileinfos, func(i, j int) bool { return fileinfos[i].ModTime().Before(fileinfos[j].ModTime()) })
		for i := 0; i < len(fileinfos)-max; i++ {
			err = os.Remove(filepath.Join(dir, fileinfos[i].Name()))
			if err != nil {
				logger.Warning(err)
			}
		}
	}
	return nil
}

func DeleteWallPaper(username string, file string) error {
	dirs, err := GetUserDirs(username)
	if err != nil {
		return err
	}

	path, err := filepath.Abs(file)
	if err != nil {
		return err
	}

	if checkPath(path, dirs) == "" {
		return fmt.Errorf("%s is not in %v", file, dirs)
	}

	return os.Remove(path)
}

var wallpaperMutex sync.Mutex

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
		return fmt.Errorf("Policykit authentication failed")
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
	if dutils.IsInterfaceNil(result) {
		return nil, errors.New("result is nil")
	}
	return result.Value(), nil
}

func (d *Daemon) isSelf(sender dbus.Sender, username string) error {
	uid, err := d.service.GetConnUID(string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}
	user, err := user.LookupId(strconv.Itoa(int(uid)))
	if err != nil {
		return dbusutil.ToError(err)
	}
	if user.Username != username {
		err = fmt.Errorf("%s not allowed to delete %s custom wallpaper", user.Username, username)
		return dbusutil.ToError(err)
	}
	return nil
}

func (d *Daemon) checkAuth(sender dbus.Sender) error {
	return checkAuth(polkitActionUserAdministration, string(sender))
}

func (d *Daemon) SaveCustomWallPaper(sender dbus.Sender, username string, file string) (string, *dbus.Error) {
	var err error
	var isSolid bool = false
	wallpaperMutex.Lock()
	defer wallpaperMutex.Unlock()
	if strings.HasPrefix(file, solidPrefix) {
		file = strings.TrimPrefix(file, solidPrefix)
		isSolid = true
	}
	info, err := os.Stat(file)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	if info.Size() > maxSize {
		err = fmt.Errorf("file size %d > %d", info.Size(), maxSize)
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}
	dirs, _ := GetUserDirs(username)

	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	if checkPath(file, dirs) != "" {
		return file, nil
	}

	uid, err := d.service.GetConnUID(string(sender))
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	user, err := user.LookupId(strconv.Itoa(int(uid)))
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	if user.Username != username && uid != 0 {
		err = fmt.Errorf("%s not allowed to set %s wallpaper", user.Username, username)
		return "", dbusutil.ToError(err)
	}
	md5sum, _ := dutils.SumFileMd5(file)

	var prefix string
	if isSolid {
		prefix = solidWallPaperPath
	} else {
		prefix = wallPaperDir
	}
	// 设置壁纸路径
	path := func() string {
		for _, dir := range dirs {
			if strings.HasPrefix(dir, prefix) {
				return dir
			}
		}
		return ""
	}()
	if path == "" {
		err = fmt.Errorf("unknown path: %s", prefix)
		return "", dbusutil.ToError(err)
	}
	destFile := filepath.Join(path, md5sum)
	destFile = destFile + filepath.Ext(file)
	if dutils.IsFileExist(destFile) {
		return destFile, nil
	}
	src, err := exec.Command("runuser", []string{
		"-u",
		username,
		"--",
		"cat",
		file,
	}...).Output()
	if err != nil {
		err = fmt.Errorf("permission denied, %s is not allowed to read this file:%s", username, file)
		return "", dbusutil.ToError(err)
	}

	err = ioutil.WriteFile(destFile, src, 0644)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	err = RemoveOverflowWallPapers(username, maxCount)
	if err != nil {
		logger.Warning(err)
	}

	return destFile, dbusutil.ToError(err)
}

func (d *Daemon) DeleteCustomWallPaper(sender dbus.Sender, username string, file string) *dbus.Error {
	err := d.isSelf(sender, username)
	if err != nil {
		logger.Warning(err)

		err = d.checkAuth(sender)
		if err != nil {
			return dbusutil.ToError(err)
		} else {
			return dbusutil.ToError(DeleteWallPaper(username, file))
		}
	}

	return dbusutil.ToError(DeleteWallPaper(username, file))
}

func (*Daemon) GetCustomWallPapers(username string) ([]string, *dbus.Error) {
	dirs, err := GetUserDirs(username)
	if err != nil {
		logger.Warning(err)
		return []string{}, dbusutil.ToError(err)
	}
	files := []string{}

	for _, dir := range dirs {

		fileinfos, err := ioutil.ReadDir(dir)
		if err != nil {
			logger.Warning(err)
			return []string{}, dbusutil.ToError(err)
		}

		for _, fileinfo := range fileinfos {
			files = append(files, filepath.Join(dir, fileinfo.Name()))
		}
	}

	return files, nil
}
