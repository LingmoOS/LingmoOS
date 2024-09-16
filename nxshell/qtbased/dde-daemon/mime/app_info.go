// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package mime

import (
	"fmt"
	"os"
	"path"

	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/mime"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

type AppInfo struct {
	// Desktop id
	Id string
	// App name
	Name string
	// Display name
	DisplayName string
	// Comment
	Description string
	// Icon
	Icon string
	// Commandline
	Exec      string
	CanDelete bool

	fileName string
}

type AppInfos []*AppInfo

func GetDefaultAppInfo(mimeType string) (*AppInfo, error) {
	id, err := mime.GetDefaultApp(mimeType, false)
	if err != nil {
		return nil, err
	}

	info, err := newAppInfoById2(id, mimeType)
	if err != nil {
		return nil, err
	}
	return info, nil
}

func (infos AppInfos) Add(id string) (AppInfos, error) {
	for _, info := range infos {
		if info.Id == id {
			return infos, nil
		}
	}
	tmp, err := newAppInfoById(id)
	if err != nil {
		return nil, err
	}
	infos = append(infos, tmp)
	return infos, nil
}

func (infos AppInfos) Delete(id string) AppInfos {
	var ret AppInfos
	for _, info := range infos {
		if info.Id == id {
			continue
		}
		ret = append(ret, info)
	}
	return ret
}

func SetAppInfo(ty, id string) error {
	return mime.SetDefaultApp(ty, id)
}

func GetAppInfos(mimeType string) AppInfos {
	var infos AppInfos
	for _, id := range mime.GetAppList(mimeType) {
		appInfo, err := newAppInfoById2(id, mimeType)
		if err != nil {
			logger.Warning(err)
			continue
		}
		infos = append(infos, appInfo)
	}
	return infos
}

func getAppName(dai *desktopappinfo.DesktopAppInfo) (name string) {
	xDeepinVendor, _ := dai.GetString(desktopappinfo.MainSection, "X-Deepin-Vendor")
	if xDeepinVendor == "deepin" {
		name = dai.GetGenericName()
	} else {
		name = dai.GetName()
	}
	if name == "" {
		name = dai.GetId()
	}
	return
}

func newAppInfoByIdAux(id string, fn func(dai *desktopappinfo.DesktopAppInfo, appInfo *AppInfo)) (*AppInfo, error) {
	dai := desktopappinfo.NewDesktopAppInfo(id)
	if dai == nil {
		return nil, fmt.Errorf("NewDesktopAppInfo failed: id %v", id)
	}
	if !dai.ShouldShow() {
		return nil, fmt.Errorf("app %q should not show", id)
	}

	name := getAppName(dai)
	var appInfo = &AppInfo{
		Id:          id,
		Name:        name,
		DisplayName: name,
		Description: dai.GetComment(),
		Exec:        dai.GetCommandline(),
		fileName:    dai.GetFileName(),
		Icon:        dai.GetIcon(),
	}

	if fn != nil {
		fn(dai, appInfo)
	}

	return appInfo, nil
}

func newAppInfoById2(id string, mimeType string) (*AppInfo, error) {
	// 可以填写 CanDelete 字段
	gInfo, err := newAppInfoByIdAux(id, func(dai *desktopappinfo.DesktopAppInfo, appInfo *AppInfo) {
		appInfo.CanDelete = canDeleteAssociation(dai, mimeType)
	})
	return gInfo, err
}

func newAppInfoById(id string) (*AppInfo, error) {
	return newAppInfoByIdAux(id, nil)
}

func canDeleteAssociation(appInfo *desktopappinfo.DesktopAppInfo, mimeType string) bool {
	mimeTypes := appInfo.GetMimeTypes()
	for _, mt := range mimeTypes {
		if mt == mimeType {
			return false
		}
	}
	return true
}

func findFilePath(file string) string {
	data := path.Join(os.Getenv("HOME"), ".local/share", file)
	if dutils.IsFileExist(data) {
		return data
	}

	data = path.Join("/usr/local/share", file)
	if dutils.IsFileExist(data) {
		return data
	}

	return path.Join("/usr/share", file)
}
