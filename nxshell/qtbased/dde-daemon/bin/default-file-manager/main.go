// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"errors"
	"log"
	"os/user"
	"path/filepath"
	"strings"

	dbus "github.com/godbus/dbus/v5"
	appmanager "github.com/linuxdeepin/go-dbus-factory/session/org.desktopspec.applicationmanager1"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	appManagerDBusServiceName = "org.desktopspec.ApplicationManager1"
	appManagerDBusPath        = "/org/desktopspec/ApplicationManager1"
)

const (
	fileManagerMimeType = "inode/directory"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func queryAppDesktopByMime() (string, error) {
	appInfo := gio.AppInfoGetDefaultForType(fileManagerMimeType, false)
	if appInfo == nil {
		return "", errors.New("failed to get appInfo")
	}

	defer appInfo.Unref()

	dAppInfo := gio.ToDesktopAppInfo(appInfo)

	return strings.TrimSpace(filepath.Base(dAppInfo.GetFilename())), nil
}

func main() {
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		log.Fatal(err)
	}

	session, err := dbusutil.NewSessionService()
	if err != nil {
		log.Fatal(err)
	}

	has, err := session.NameHasOwner(appManagerDBusServiceName)
	if err != nil {
		log.Println("warning: call name has owner error:", err)
	}

	if has {
		var dBusObjPath dbus.ObjectPath
		var mimeType string

		mimeManagerAppObj := appmanager.NewMimeManager(sessionBus)

		if mimeManagerAppObj != nil {
			cur, err := user.Current()
			if err != nil {
				log.Fatal(err)
			}

			mimeType, dBusObjPath, err = mimeManagerAppObj.QueryDefaultApplication(0, cur.HomeDir)
			if err != nil {
				log.Println("warning: query default application error:", err)
			}
		}

		if mimeType != fileManagerMimeType || dBusObjPath == "/" {
			log.Println("warning: can not get default file manager from AM, query from xdg-mime:", err)
			appDesktop, err := queryAppDesktopByMime()
			if err != nil {
				log.Fatal(err)
			}

			dBusObjPath, err = desktopappinfo.GetDBusObjectFromAppDesktop(appDesktop, appManagerDBusServiceName, appManagerDBusPath)
			if err != nil {
				log.Println("warning: get dbus object path error:", err)
				log.Fatal(err)
			}
		}

		appManagerAppObj, err := appmanager.NewApplication(sessionBus, dBusObjPath)
		if err != nil {
			log.Println("warning: new appManager error:", err)
		}

		_, err = appManagerAppObj.Launch(0, "", []string{}, make(map[string]dbus.Variant))
		if err != nil {
			log.Println("warning: launch app error:", err)
		}
	}
}
