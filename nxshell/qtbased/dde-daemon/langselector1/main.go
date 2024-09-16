// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package langselector

import (
	"time"

	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/strv"
)

const (
	dbusServiceName = dbusInterface
)

var (
	logger = log.NewLogger("daemon/langselector")
)

func Run() {
	service, err := dbusutil.NewSessionService()
	if err != nil {
		logger.Fatal("failed to new session service:", err)
	}

	lang, err := newLangSelector(service)
	if err != nil {
		logger.Fatal("failed to new langSelector:", err)
	}
	err = service.Export(dbusPath, lang)
	if err != nil {
		logger.Fatal("failed to export:", err)
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Fatal("failed to request name:", err)
	}

	initNotifyTxt()
	lang.connectSettingsChanged()
	err = gsettings.StartMonitor()
	if err != nil {
		logger.Warning("failed to start monitor settings:", err)
	}
	service.SetAutoQuitHandler(time.Minute*5, func() bool {
		lang.PropsMu.RLock()
		canQuit := lang.LocaleState != LocaleStateChanging
		lang.PropsMu.RUnlock()
		return canQuit
	})
	service.Wait()
}

func GetLocales() []string {
	currentLocale := getCurrentUserLocale()
	settings := gio.NewSettings(gsSchemaLocale)
	locales := settings.GetStrv(gsKeyLocales)
	if !strv.Strv(locales).Contains(currentLocale) {
		locales = append(locales, currentLocale)
	}
	settings.Unref()
	return locales
}
