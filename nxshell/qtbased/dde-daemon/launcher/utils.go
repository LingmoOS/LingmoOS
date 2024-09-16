// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"os"
	"path"
	"path/filepath"
	"strings"
	"syscall"

	"github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/xdg/basedir"
	"github.com/linuxdeepin/go-lib/xdg/userdir"
)

const (
	AppDirName                 = "applications"
	DirDefaultPerm os.FileMode = 0755
)

// appInDesktop returns the destination when the desktop file is
// sent to the user's desktop direcotry.
func appInDesktop(appId string) string {
	appId = strings.Replace(appId, "/", "-", -1)
	return filepath.Join(getUserDesktopDir(), appId+desktopExt)
}

func isZH() bool {
	lang := gettext.QueryLang()
	return strings.HasPrefix(lang, "zh")
}

func getUserDesktopDir() string {
	return userdir.Get(userdir.Desktop)
}

// return $HOME/.local/share/applications
func getUserAppDir() string {
	userDataDir := basedir.GetUserDataDir()
	return filepath.Join(userDataDir, AppDirName)
}

func getDataDirsForWatch() []string {
	userDataDir := basedir.GetUserDataDir()
	sysDataDirs := basedir.GetSystemDataDirs()
	return append(sysDataDirs, userDataDir)
}

// The default applications module of the DDE Control Center
// creates the desktop file with the file name beginning with
// "deepin-custom" in the applications directory under the XDG
// user data directory.
func isDeepinCustomDesktopFile(file string) bool {
	dir := filepath.Dir(file)
	base := filepath.Base(file)
	userAppDir := getUserAppDir()

	return dir == userAppDir && strings.HasPrefix(base, "deepin-custom-")
}

func getAppDirs() []string {
	dataDirs := basedir.GetSystemDataDirs()
	dataDirs = append(dataDirs, basedir.GetUserDataDir())
	var dirs []string
	for _, dir := range dataDirs {
		dirs = append(dirs, path.Join(dir, AppDirName))
	}
	return dirs
}

func getAppIdByFilePath(file string, appDirs []string) string {
	file = filepath.Clean(file)
	var desktopId string
	isLingLong := false
	for _, dir := range appDirs {
		if strings.HasPrefix(file, dir) {
			desktopId, _ = filepath.Rel(dir, file)
			if strings.Contains(dir, "linglong") {
				isLingLong = true
			}
			break
		}
	}
	if desktopId == "" {
		return ""
	}
	appId := strings.TrimSuffix(desktopId, desktopExt)
	if isLingLong {
		appId = appId + "_Linglong"
	}
	return appId
}

func runeSliceDiff(key, current []rune) (popCount int, runesPush []rune) {
	var i int
	kLen := len(key)
	cLen := len(current)
	if kLen == 0 {
		popCount = cLen
		return
	}
	if cLen == 0 {
		runesPush = key
		return
	}

	for {
		k := key[i]
		c := current[i]
		//logger.Debugf("[%v] k %v c %v", i, k, c)

		if k == c {
			i++
			if i == kLen {
				//logger.Debug("i == key len")
				break
			}
			if i == cLen {
				//logger.Debug("i == current len")
				break
			}

		} else {
			break
		}
	}
	popCount = cLen - i
	for j := i; j < kLen; j++ {
		runesPush = append(runesPush, key[j])
	}

	//logger.Debug("i:", i)
	return
}

func getFileCTime(filename string) (int64, error) {
	var stat syscall.Stat_t
	err := syscall.Stat(filename, &stat)
	if err != nil {
		return 0, err
	}
	return int64(stat.Ctim.Sec), nil
}
