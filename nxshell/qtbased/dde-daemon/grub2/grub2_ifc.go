// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"errors"
	"io/ioutil"
	"strings"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/grub_common"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/procfs"
)

const (
	dbusServiceName = "org.deepin.dde.Grub2"
	dbusPath        = "/org/deepin/dde/Grub2"
	dbusInterface   = dbusServiceName

	polikitActionIdCommon               = "org.deepin.dde.grub2"
	polikitActionIdPrepareGfxmodeDetect = "org.deepin.dde.grub2.prepare-gfxmode-detect"

	timeoutMax = 10
)

func (*Grub2) GetInterfaceName() string {
	return dbusInterface
}

// GetSimpleEntryTitles return entry titles only in level one and will
// filter out some useless entries such as sub-menus and "memtest86+".
func (grub *Grub2) GetSimpleEntryTitles(sender dbus.Sender) (titles []string, busErr *dbus.Error) {
	err := checkInvokePermission(grub.service, sender)
	if err != nil {
		return nil, dbusutil.ToError(err)
	}
	grub.service.DelayAutoQuit()

	grub.readEntries()

	for _, entry := range grub.entries {
		if entry.parentSubMenu == nil && entry.entryType == MENUENTRY {
			title := entry.getFullTitle()
			if !strings.Contains(title, "memtest86+") {
				titles = append(titles, title)
			}
		}
	}
	if len(titles) == 0 {
		logger.Warningf("there is no menu entry in %q", grubScriptFile)
	}
	return titles, nil
}

func (g *Grub2) GetAvailableGfxmodes(sender dbus.Sender) (gfxModes []string, busErr *dbus.Error) {
	err := checkInvokePermission(g.service, sender)
	if err != nil {
		return nil, dbusutil.ToError(err)
	}
	pid, err := g.service.GetConnPID(string(sender))
	if err != nil {
		return nil, dbusutil.ToError(err)
	}

	p := procfs.Process(pid)
	envVars, err := p.Environ()
	if err != nil {
		return nil, dbusutil.ToError(err)
	}

	sessionType := envVars.Get("XDG_SESSION_TYPE")

	if sessionType == "wayland" {
		logger.Debug("wayland desktop environment, can not acquire output info")
	} else if sessionType == "x11" {
		g.service.DelayAutoQuit()
		modes, err := g.getAvailableGfxmodes(sender)
		if err != nil {
			logger.Warning(err)
			return nil, dbusutil.ToError(err)
		}
		modes.SortDesc()
		gfxModes = make([]string, len(modes))
		for idx, m := range modes {
			gfxModes[idx] = m.String()
		}
	} else {
		logger.Debug("unkown session type, can not acquire output info")
	}

	return gfxModes, nil
}

func (g *Grub2) SetDefaultEntry(sender dbus.Sender, entry string) *dbus.Error {
	err := checkInvokePermission(g.service, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	g.service.DelayAutoQuit()

	err = g.checkAuth(sender, polikitActionIdCommon)
	if err != nil {
		return dbusutil.ToError(err)
	}

	idx := g.defaultEntryStr2Idx(entry)
	if idx == -1 {
		return dbusutil.ToError(errors.New("invalid entry"))
	}

	g.PropsMu.Lock()
	if g.setPropDefaultEntry(entry) {
		g.addModifyTask(getModifyTaskDefaultEntry(idx))
	}
	g.PropsMu.Unlock()
	return nil
}

var errInGfxmodeDetect = errors.New("in gfxmode detection mode")

func (g *Grub2) SetEnableTheme(sender dbus.Sender, enabled bool) *dbus.Error {
	err := checkInvokePermission(g.service, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	g.service.DelayAutoQuit()

	err = g.checkAuth(sender, polikitActionIdCommon)
	if err != nil {
		return dbusutil.ToError(err)
	}

	lang, err := g.getSenderLang(sender)
	if err != nil {
		logger.Warning("failed to get sender lang:", err)
	}

	g.PropsMu.Lock()

	if g.setPropEnableTheme(enabled) {
		var themeFile string
		if enabled {
			if g.gfxmodeDetectState == gfxmodeDetectStateNone {
				themeFile = defaultGrubTheme
			} else {
				themeFile = fallbackGrubTheme
			}
		}
		g.setPropThemeFile(themeFile)
		task := getModifyTaskEnableTheme(enabled, lang, g.gfxmodeDetectState)
		logger.Debugf("SetEnableTheme task: %#v", task)
		g.addModifyTask(task)
	}

	g.PropsMu.Unlock()
	return nil
}

func (g *Grub2) SetGfxmode(sender dbus.Sender, gfxmode string) *dbus.Error {
	err := checkInvokePermission(g.service, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	g.service.DelayAutoQuit()

	err = g.checkAuth(sender, polikitActionIdCommon)
	if err != nil {
		return dbusutil.ToError(err)
	}

	err = checkGfxmode(gfxmode)
	if err != nil {
		return dbusutil.ToError(err)
	}

	lang, err := g.getSenderLang(sender)
	if err != nil {
		logger.Warning("failed to get sender lang:", err)
	}

	g.PropsMu.Lock()
	if g.gfxmodeDetectState == gfxmodeDetectStateDetecting {
		g.PropsMu.Unlock()
		return dbusutil.ToError(errInGfxmodeDetect)
	}

	if g.setPropGfxmode(gfxmode) {
		g.addModifyTask(getModifyTaskGfxmode(gfxmode, lang))
	}
	g.PropsMu.Unlock()
	return nil
}

func (g *Grub2) SetTimeout(sender dbus.Sender, timeout uint32) *dbus.Error {
	err := checkInvokePermission(g.service, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	g.service.DelayAutoQuit()

	/*
		err := g.checkAuth(sender, polikitActionIdCommon)
		if err != nil {
			return dbusutil.ToError(err)
		}
	*/

	if timeout > timeoutMax {
		return dbusutil.ToError(errors.New("exceeded the maximum value"))
	}

	g.PropsMu.Lock()
	if g.setPropTimeout(timeout) {
		g.addModifyTask(getModifyTaskTimeout(timeout))
	}
	g.PropsMu.Unlock()
	return nil
}

// Reset reset all configuration.
func (g *Grub2) Reset(sender dbus.Sender) *dbus.Error {
	g.service.DelayAutoQuit()

	const defaultEnableTheme = true

	err := g.checkAuth(sender, polikitActionIdCommon)
	if err != nil {
		return dbusutil.ToError(err)
	}

	lang, err := g.getSenderLang(sender)
	if err != nil {
		logger.Warning("failed to get sender lang:", err)
	}

	var modifyTasks []modifyTask

	g.PropsMu.Lock()
	if g.setPropTimeout(defaultGrubTimeoutInt) {
		modifyTasks = append(modifyTasks, getModifyTaskTimeout(defaultGrubTimeoutInt))
	}

	if g.setPropEnableTheme(defaultEnableTheme) {
		modifyTasks = append(modifyTasks,
			getModifyTaskEnableTheme(defaultEnableTheme, lang, g.gfxmodeDetectState))
	}

	g.setPropThemeFile(defaultGrubTheme)

	cfgDefaultEntry, _ := g.defaultEntryIdx2Str(defaultGrubDefaultInt)
	if g.setPropDefaultEntry(cfgDefaultEntry) {
		modifyTasks = append(modifyTasks, getModifyTaskDefaultEntry(defaultGrubDefaultInt))
	}
	g.PropsMu.Unlock()

	if len(modifyTasks) > 0 {
		compoundModifyFunc := func(params map[string]string) {
			for _, task := range modifyTasks {
				task.paramsModifyFunc(params)
			}
		}
		g.addModifyTask(modifyTask{
			paramsModifyFunc: compoundModifyFunc,
			adjustTheme:      true,
		})
	}

	return nil
}

func (g *Grub2) PrepareGfxmodeDetect(sender dbus.Sender) *dbus.Error {
	g.service.DelayAutoQuit()

	gfxmodes, err := g.getGfxmodesFromXRandr(sender)
	if err != nil {
		logger.Debug("failed to get gfxmodes from XRandr:", err)
	}

	gfxmodes.SortDesc()
	gfxmodesStr := joinGfxmodesForDetect(gfxmodes)

	g.PropsMu.RLock()
	gfxmodeDetectState := g.gfxmodeDetectState
	g.PropsMu.RUnlock()

	defaultParams, err := grub_common.LoadGrubParams()
	if err != nil {
		logger.Warning("failed to load grub params:", err)
		return dbusutil.ToError(err)
	}

	params := make(map[string]string)
	for _, key := range []string{grubBackground, grubGfxmode, grubTheme, grubTimeout} {
		if v, ok := defaultParams[key]; ok {
			params[key] = v
		}
	}

	if gfxmodeDetectState == gfxmodeDetectStateDetecting {
		return dbusutil.ToError(errors.New("already in detection mode"))
	} else if gfxmodeDetectState == gfxmodeDetectStateFailed {
		cur, _, err := grub_common.GetBootArgDeepinGfxmode()
		if err == nil {
			if params[grubGfxmode] == gfxmodesStr ||
				(len(gfxmodes) > 0 && gfxmodes[0] == cur) {
				g.finishGfxmodeDetect(params)
				return nil
			}
		}
	}

	themeFile := getTheme(params)
	g.PropsMu.Lock()
	g.gfxmodeDetectState = gfxmodeDetectStateDetecting
	if themeFile != "" {
		g.setPropThemeFile(fallbackGrubTheme)
		g.setPropEnableTheme(true)
	} else {
		g.setPropThemeFile("")
		g.setPropEnableTheme(false)
	}
	g.setPropGfxmode(gfxmodesStr)
	g.PropsMu.Unlock()
	g.theme.emitSignalBackgroundChanged()

	g.addModifyTask(getModifyTaskPrepareGfxmodeDetect(gfxmodesStr))

	err = ioutil.WriteFile(grub_common.GfxmodeDetectReadyPath, nil, 0644)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}
