// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"log"
	"os"
	"os/exec"

	"github.com/godbus/dbus/v5"
	appmanager "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.application1"
	newAppmanager "github.com/linuxdeepin/go-dbus-factory/session/org.desktopspec.applicationmanager1"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

const (
	gsSchemaDefaultTerminal = "com.deepin.desktop.default-applications.terminal"
	gsKeyAppId              = "app-id"
	gsKeyExec               = "exec"
)

const (
	appManagerDBusServiceName = "org.desktopspec.ApplicationManager1"
	appManagerDBusPath        = "/org/desktopspec/ApplicationManager1"
)

var terms = []string{
	"deepin-terminal",
	"gnome-terminal",
	"terminator",
	"xfce4-terminal",
	"rxvt",
	"xterm",
}

func getPresetTerminalPath() string {
	for _, exe := range terms {
		file, _ := exec.LookPath(exe)
		if file != "" {
			return file
		}
	}
	return ""
}

func init() {
	log.SetFlags(log.Lshortfile)
}

func main() {
	settings := gio.NewSettings(gsSchemaDefaultTerminal)
	defer settings.Unref()

	appId := settings.GetString(gsKeyAppId)
	appInfo := desktopappinfo.NewDesktopAppInfo(appId)

	if len(os.Args) == 1 {
		if appInfo != nil {
			sessionBus, err := dbus.SessionBus()
			if err != nil {
				log.Fatal(err)
			}

			workDir, err := os.Getwd()
			if err != nil {
				log.Println("warning: failed to get work dir:", err)
			}

			options := map[string]dbus.Variant{
				"path": dbus.MakeVariant(workDir),
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
				obj, err := desktopappinfo.GetDBusObjectFromAppDesktop(appId, appManagerDBusServiceName, appManagerDBusPath)
				if err != nil {
					log.Println("warning: get dbus object error:", err)
				}

				appManagerAppObj, err := newAppmanager.NewApplication(sessionBus, obj)
				if err != nil {
					log.Println("warning: new appManager error:", err)
				}

				_, err = appManagerAppObj.Launch(0, "", []string{}, options)
				if err != nil {
					log.Println("warning: launch app error:", err)
				}
			} else {
				appManager := appmanager.NewManager(sessionBus)
				err = appManager.LaunchAppWithOptions(0, appInfo.GetFileName(), 0,
					nil, options)
			}

			if err != nil {
				log.Println(err)
				runFallbackTerm()
			}
		} else {
			runFallbackTerm()
		}
	} else {
		// define -e option
		termExec := settings.GetString(gsKeyExec)
		termPath, _ := exec.LookPath(termExec)
		if termPath == "" {
			// try again
			termPath = getPresetTerminalPath()
			if termPath == "" {
				log.Fatal("failed to get terminal path")
			}
		}

		args := os.Args[1:]
		cmd := exec.Command(termPath, args...) // #nosec G204
		cmd.Stderr = os.Stderr
		cmd.Stdout = os.Stdout
		err := cmd.Run()
		if err != nil {
			log.Fatal(err)
		}
	}
}

func runFallbackTerm() {
	termPath := getPresetTerminalPath()
	if termPath == "" {
		log.Println("failed to get terminal path")
		return
	}
	cmd := exec.Command(termPath) // #nosec G204
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	err := cmd.Run()
	if err != nil {
		log.Println(err)
	}
}
