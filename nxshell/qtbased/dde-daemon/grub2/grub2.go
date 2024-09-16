// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"unicode"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/grub_common"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/procfs"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const grubScriptFile = "/boot/grub/grub.cfg"

var logger *log.Logger

func SetLogger(v *log.Logger) {
	logger = v
}

type gfxmodeDetectState int

const (
	gfxmodeDetectStateNone gfxmodeDetectState = iota
	gfxmodeDetectStateDetecting
	gfxmodeDetectStateFailed
)

//go:generate dbusutil-gen -type Grub2,Theme,EditAuth grub2.go theme.go edit_auth.go
//go:generate dbusutil-gen em -type Grub2,Theme,EditAuth

type Grub2 struct {
	service            *dbusutil.Service
	modifyManager      *modifyManager
	entries            []Entry
	theme              *Theme
	editAuth           *EditAuth
	fstart             *Fstart
	gfxmodeDetectState gfxmodeDetectState
	inhibitFd          dbus.UnixFD
	PropsMu            sync.RWMutex
	dbusObj            ofdbus.DBus
	sysLoop            *dbusutil.SignalLoop
	// props:
	ThemeFile    string
	DefaultEntry string
	EnableTheme  bool
	Gfxmode      string
	Timeout      uint32
	Updating     bool
}

// return -1 for failed
func (g *Grub2) defaultEntryStr2Idx(str string) int {
	entriesLv1 := g.getEntryTitlesLv1()
	return getStringIndexInArray(str, entriesLv1)
}

func (g *Grub2) defaultEntryIdx2Str(idx int) (string, error) {
	entriesLv1 := g.getEntryTitlesLv1()
	length := len(entriesLv1)
	if length == 0 {
		return "", errors.New("no entry")
	}
	if 0 <= idx && idx < length {
		return entriesLv1[idx], nil
	} else {
		return "", errors.New("index out of range")
	}
}

func (g *Grub2) applyParams(params map[string]string) {
	if grub_common.InGfxmodeDetectionMode(params) {
		g.gfxmodeDetectState = gfxmodeDetectStateDetecting
	} else if grub_common.IsGfxmodeDetectFailed(params) {
		g.gfxmodeDetectState = gfxmodeDetectStateFailed
	}

	// timeout
	timeout := getTimeout(params)
	if timeout < 0 {
		timeout = 999
	}
	g.Timeout = uint32(timeout)

	// enable theme
	var enableTheme bool
	g.ThemeFile = getTheme(params)
	if g.ThemeFile != "" {
		enableTheme = true
	}
	g.EnableTheme = enableTheme

	g.Gfxmode = getGfxMode(params)

	// default entry
	defaultEntry := getDefaultEntry(params)

	defaultEntryIdx, err := strconv.Atoi(defaultEntry)
	if err == nil {
		// is a num
		g.DefaultEntry, _ = g.defaultEntryIdx2Str(defaultEntryIdx)
	} else {
		// not a num
		if defaultEntry == "saved" {
			// TODO saved
			g.DefaultEntry, _ = g.defaultEntryIdx2Str(0)
		} else {
			g.DefaultEntry = defaultEntry
		}
	}
}

type modifyTask struct {
	paramsModifyFunc func(map[string]string)
	adjustTheme      bool
	adjustThemeLang  string
}

func getModifyTaskEnableTheme(enable bool, lang string, gfxmodeDetectState gfxmodeDetectState) modifyTask {
	if enable {
		f := func(params map[string]string) {
			if gfxmodeDetectState == gfxmodeDetectStateNone {
				// normal
				params[grubTheme] = quoteString(defaultGrubTheme)
				params[grubBackground] = quoteString(defaultGrubBackground)
			} else {
				// detecting or failed
				params[grubTheme] = quoteString(fallbackGrubTheme)
				params[grubBackground] = quoteString(fallbackGrubBackground)
			}
		}
		return modifyTask{
			paramsModifyFunc: f,
			adjustTheme:      gfxmodeDetectState == gfxmodeDetectStateNone,
			adjustThemeLang:  lang,
		}
	} else {
		f := func(params map[string]string) {
			params[grubTheme] = ""
			params[grubBackground] = ""
		}
		return modifyTask{
			paramsModifyFunc: f,
		}
	}
}

func getModifyTaskTimeout(timeout uint32) modifyTask {
	f := func(params map[string]string) {
		params[grubTimeout] = strconv.Itoa(int(timeout))
	}
	return modifyTask{
		paramsModifyFunc: f,
	}
}

func getModifyTaskGfxmode(val string, lang string) modifyTask {
	f := func(params map[string]string) {
		params[grubGfxmode] = quoteString(val)
	}
	return modifyTask{
		paramsModifyFunc: f,
		adjustTheme:      true,
		adjustThemeLang:  lang,
	}
}

func getModifyTaskDefaultEntry(idx int) modifyTask {
	f := func(params map[string]string) {
		params[grubDefault] = strconv.Itoa(idx)
	}
	return modifyTask{
		paramsModifyFunc: f,
	}
}

func joinGfxmodesForDetect(gfxmodes grub_common.Gfxmodes) string {
	const gfxmodeDelimiter = ","
	var buf bytes.Buffer
	for _, m := range gfxmodes {
		buf.WriteString(m.String())
		buf.WriteString(gfxmodeDelimiter)
	}

	buf.WriteString("auto")
	return buf.String()
}

func getModifyFuncPrepareGfxmodeDetect(gfxmodesStr string) func(map[string]string) {
	f := func(params map[string]string) {
		if params[grubTheme] != "" {
			// theme enabled
			params[grubTheme] = fallbackGrubTheme
			params[grubBackground] = fallbackGrubBackground
		} else {
			delete(params, grubTheme)
			params[grubBackground] = ""
		}

		params[grub_common.DeepinGfxmodeDetect] = "1"
		delete(params, grub_common.DeepinGfxmodeAdjusted)
		delete(params, grub_common.DeepinGfxmodeNotSupported)
		if gfxmodesStr != "auto" || params[grubGfxmode] == "" {
			params[grubGfxmode] = gfxmodesStr
		}
	}
	return f
}

func getModifyTaskPrepareGfxmodeDetect(gfxmodesStr string) modifyTask {
	f := getModifyFuncPrepareGfxmodeDetect(gfxmodesStr)
	return modifyTask{
		paramsModifyFunc: f,
	}
}

func (g *Grub2) finishGfxmodeDetect(params map[string]string) {
	logger.Debug("finish gfxmode detect")

	currentGfxmode, _, err := grub_common.GetBootArgDeepinGfxmode()
	if err != nil {
		g.PropsMu.Lock()
		g.gfxmodeDetectState = gfxmodeDetectStateFailed
		g.PropsMu.Unlock()
		logger.Warning("failed to get current gfxmode:", err)
		task := modifyTask{
			paramsModifyFunc: func(params map[string]string) {
				params[grub_common.DeepinGfxmodeDetect] = "2"
			},
		}
		g.addModifyTask(task)
		return
	}
	g.PropsMu.Lock()
	g.gfxmodeDetectState = gfxmodeDetectStateNone
	g.PropsMu.Unlock()
	logger.Debug("currentGfxmode:", currentGfxmode)

	var maxGfxmode grub_common.Gfxmode
	detectGfxmodes := strings.Split(params[grubGfxmode], ",")
	logger.Debug("detectGfxmodes:", detectGfxmodes)
	if len(detectGfxmodes) > 0 {
		maxGfxmode, err = grub_common.ParseGfxmode(detectGfxmodes[0])
		if err != nil {
			logger.Warning(err)
		}
	} else {
		logger.Warning("failed to get detect gfxmodes")
	}
	logger.Debug("maxGfxmode:", maxGfxmode)
	notMax := maxGfxmode.Width != 0 && currentGfxmode != maxGfxmode

	themeEnabled := params[grubTheme] != ""

	currentGfxmodeStr := currentGfxmode.String()
	g.PropsMu.Lock()
	g.setPropGfxmode(currentGfxmodeStr)
	if themeEnabled {
		g.setPropThemeFile(defaultGrubTheme)
	} else {
		g.setPropThemeFile("")
	}
	g.PropsMu.Unlock()
	g.theme.emitSignalBackgroundChanged()

	task := modifyTask{
		paramsModifyFunc: func(params map[string]string) {
			if themeEnabled {
				params[grubTheme] = quoteString(defaultGrubTheme)
				params[grubBackground] = quoteString(defaultGrubBackground)
			}
			params[grubGfxmode] = currentGfxmodeStr
			params[grub_common.DeepinGfxmodeAdjusted] = "1"
			delete(params, grub_common.DeepinGfxmodeDetect)
			if notMax {
				params[grub_common.DeepinGfxmodeNotSupported] = maxGfxmode.String()
			}
		},
		adjustTheme: themeEnabled,
	}
	g.addModifyTask(task)
}

func NewGrub2(service *dbusutil.Service) *Grub2 {
	g := &Grub2{
		service:   service,
		inhibitFd: -1,
	}
	g.dbusObj = ofdbus.NewDBus(service.Conn())
	g.sysLoop = dbusutil.NewSignalLoop(service.Conn(), 10)
	g.sysLoop.Start()
	g.dbusObj.InitSignalExt(g.sysLoop, true)
	err := g.readEntries()
	if err != nil {
		logger.Warning("readEntries Failed:", err)
	}

	params, err := grub_common.LoadDDEGrubParams()
	if err != nil {
		logger.Warning(err)
	}
	_, ok := params[grubDefault]
	if !ok {
		defaultParams, err := grub_common.LoadGrubParams()
		if err != nil {
			logger.Warning(err)
		} else {
			params[grubDefault] = defaultParams[grubDefault]
		}
	}
	g.applyParams(params)
	g.modifyManager = newModifyManager()
	g.modifyManager.g = g
	g.modifyManager.stateChangeCb = func(running bool) {
		// state change callback
		if running {
			g.preventShutdown()
		} else {
			g.enableShutdown()
		}
		g.PropsMu.Lock()
		g.setPropUpdating(running)
		g.PropsMu.Unlock()
	}
	go g.modifyManager.loop()

	// init theme
	g.theme = NewTheme(g)

	g.fstart = NewFstart(g)

	jobLog, err := loadLog()
	if err != nil {
		if !os.IsNotExist(err) {
			logger.Warning(err)
		}
	}
	if jobLog != nil {
		if !jobLog.isJobDone(logJobMkConfig) {
			task := modifyTask{}

			if jobLog.hasJob(logJobAdjustTheme) &&
				!jobLog.isJobDone(logJobAdjustTheme) {
				task.adjustTheme = true
			}
			g.addModifyTask(task)
		}
	}

	// init edit auth
	g.editAuth = NewEditAuth(g)
	g.editAuth.init()

	return g
}

func (grub *Grub2) readEntries() (err error) {
	fileContent, err := ioutil.ReadFile(grubScriptFile)
	if err != nil {
		logger.Error(err)
		return
	}
	entries, err := parseEntries(string(fileContent))
	if err != nil {
		logger.Error(err)
		grub.resetEntries()
		return
	}
	grub.entries = entries
	if len(grub.entries) == 0 {
		logger.Warningf("there is no menu entry in %s", grubScriptFile)
	}
	return
}

func (grub *Grub2) resetEntries() {
	grub.entries = make([]Entry, 0)
}

// getAllEntriesLv1 return all entires titles in level one.
func (grub *Grub2) getEntryTitlesLv1() (entryTitles []string) {
	for _, entry := range grub.entries {
		if entry.parentSubMenu == nil {
			entryTitles = append(entryTitles, entry.getFullTitle())
		}
	}
	return
}

func parseEntries(fileContent string) ([]Entry, error) {
	var entries []Entry

	inMenuEntry := false
	level := 0
	numCount := make(map[int]int)
	numCount[0] = 0
	parentMenus := make([]*Entry, 0)
	parentMenus = append(parentMenus, nil)
	sl := bufio.NewScanner(strings.NewReader(fileContent))
	sl.Split(bufio.ScanLines)
	for sl.Scan() {
		line := sl.Text()
		line = strings.TrimSpace(line)
		if strings.HasPrefix(line, "menuentry ") {
			if inMenuEntry {
				err := fmt.Errorf("a 'menuentry' directive was detected inside the scope of a menuentry")
				return nil, err
			}
			title, ok := parseTitle(line)
			if ok {
				entry := Entry{MENUENTRY, title, numCount[level], parentMenus[len(parentMenus)-1]}
				entries = append(entries, entry)
				logger.Debugf("found entry: [%d] %s %s", level, strings.Repeat(" ", level*2), title)

				numCount[level]++
				inMenuEntry = true
				continue
			} else {
				err := fmt.Errorf("parse entry title failed from: %q", line)
				return nil, err
			}
		} else if strings.HasPrefix(line, "submenu ") {
			if inMenuEntry {
				err := fmt.Errorf("a 'submenu' directive was detected inside the scope of a menuentry")
				return nil, err
			}
			title, ok := parseTitle(line)
			if ok {
				entry := Entry{SUBMENU, title, numCount[level], parentMenus[len(parentMenus)-1]}
				entries = append(entries, entry)
				parentMenus = append(parentMenus, &entry)
				logger.Debugf("found entry: [%d] %s %s", level, strings.Repeat(" ", level*2), title)

				level++
				numCount[level] = 0
				continue
			} else {
				err := fmt.Errorf("parse entry title failed from: %q", line)
				return nil, err
			}
		} else if line == "}" {
			if inMenuEntry {
				inMenuEntry = false
			} else if level > 0 {
				level--

				// delete last parent submenu
				i := len(parentMenus) - 1
				copy(parentMenus[i:], parentMenus[i+1:])
				parentMenus[len(parentMenus)-1] = nil
				parentMenus = parentMenus[:len(parentMenus)-1]
			}
		}
	}
	err := sl.Err()
	if err != nil {
		return nil, err
	}
	return entries, nil
}

var (
	entryRegexpSingleQuote = regexp.MustCompile(`^ *(menuentry|submenu) +'(.*?)'.*$`)
	entryRegexpDoubleQuote = regexp.MustCompile(`^ *(menuentry|submenu) +"(.*?)".*$`)
)

func parseTitle(line string) (string, bool) {
	line = strings.TrimLeftFunc(line, unicode.IsSpace)
	if entryRegexpSingleQuote.MatchString(line) {
		return entryRegexpSingleQuote.FindStringSubmatch(line)[2], true
	} else if entryRegexpDoubleQuote.MatchString(line) {
		return entryRegexpDoubleQuote.FindStringSubmatch(line)[2], true
	} else {
		return "", false
	}
}

func (g *Grub2) canSafelyExit() bool {
	logger.Debug("call canSafelyExit")
	g.PropsMu.RLock()
	can := !g.Updating
	g.PropsMu.RUnlock()
	return can
}

func (g *Grub2) checkAuth(sender dbus.Sender, actionId string) error {
	if noCheckAuth {
		logger.Warning("check auth disabled")
		return nil
	}

	isAuthorized, err := checkAuth(string(sender), actionId)
	if err != nil {
		return err
	}
	if !isAuthorized {
		return errAuthFailed
	}
	return nil
}

func (g *Grub2) addModifyTask(task modifyTask) {
	g.modifyManager.ch <- task
}

func (g *Grub2) getSenderLang(sender dbus.Sender) (string, error) {
	pid, err := g.service.GetConnPID(string(sender))
	if err != nil {
		return "", err
	}

	p := procfs.Process(pid)
	environ, err := p.Environ()
	if err != nil {
		return "", err
	}

	return environ.Get("LANG"), nil
}

func getXEnvWithSender(service *dbusutil.Service, sender dbus.Sender) (map[string]string, error) {
	environ := make(map[string]string)
	pid, err := service.GetConnPID(string(sender))
	if err != nil {
		return nil, err
	}
	p := procfs.Process(pid)
	envVars, err := p.Environ()
	if err != nil {
		return nil, err
	}
	environ["DISPLAY"] = envVars.Get("DISPLAY")
	environ["XAUTHORITY"] = envVars.Get("XAUTHORITY")
	return environ, nil
}

func (g *Grub2) getGfxmodesFromXRandr(sender dbus.Sender) (grub_common.Gfxmodes, error) {
	xEnv, err := getXEnvWithSender(g.service, sender)
	if err != nil {
		return nil, err
	}
	for key, value := range xEnv {
		os.Setenv(key, value)
	}

	return grub_common.GetGfxmodesFromXRandr()
}

func (g *Grub2) getAvailableGfxmodes(sender dbus.Sender) (grub_common.Gfxmodes, error) {
	randrGfxmodes, err := g.getGfxmodesFromXRandr(sender)
	if err != nil {
		return nil, err
	}
	logger.Debug("randrGfxmodes:", randrGfxmodes)

	grubGfxmodes, err := getGfxmodesFromBootArg()
	if err != nil {
		logger.Warning(err)
	}
	logger.Debug("grubGfxmodes:", grubGfxmodes)

	if len(grubGfxmodes) == 0 {
		return randrGfxmodes, nil
	}

	return randrGfxmodes.Intersection(grubGfxmodes), nil
}

func getGfxmodesFromBootArg() (grub_common.Gfxmodes, error) {
	_, allGfxmodes, err := grub_common.GetBootArgDeepinGfxmode()
	if err != nil {
		return nil, err
	}
	return allGfxmodes, nil
}

var ignoreString = []string{"System setup", "UOS Restore"}

func getOSNum(entries []Entry) uint32 {
	var systemNum uint32
	var shouldIgnore bool
	for _, entry := range entries {
		shouldIgnore = false
		if entry.parentSubMenu == nil && entry.entryType == MENUENTRY {
			for _, str := range ignoreString {
				if strings.Contains(entry.title, str) {
					shouldIgnore = true
					break
				}
			}
			if !shouldIgnore {
				systemNum++
			}
		}
	}
	return systemNum
}

func checkInvokePermission(service *dbusutil.Service, sender dbus.Sender) error {
	uid, err := service.GetConnUID(string(sender))
	if err != nil {
		return err
	}
	if uid != 0 {
		pid, err := service.GetConnPID(string(sender))
		if err != nil {
			return err
		}
		p := procfs.Process(pid)
		cmd, err := p.Exe()
		if err != nil {
			// 当调用者在使用过程中发生了更新,则在获取该进程的exe时,会出现lstat xxx (deleted)此类的error,如果发生的是覆盖,则该路径依旧存在,因此增加以下判断
			pErr, ok := err.(*os.PathError)
			if ok {
				if os.IsNotExist(pErr.Err) {
					errExecPath := strings.Replace(pErr.Path, "(deleted)", "", -1)
					oldExecPath := strings.TrimSpace(errExecPath)
					if dutils.IsFileExist(oldExecPath) {
						cmd = oldExecPath
						err = nil
					}
				}
			} else {
				return err
			}
		}
		if cmd == "/usr/bin/dde-control-center" || cmd == "/usr/libexec/lastore-daemon/lastore-daemon" {
			return nil
		} else {
			return fmt.Errorf("not allow %v call this method", sender)
		}
	} else {
		return nil
	}
}

func getFstartState() (bool, int) {
	if !dutils.IsFileExist(deepinFstartFile) {
		return false, -1
	}
	content, err := ioutil.ReadFile(deepinFstartFile)
	if err != nil {
		return false, -1
	}

	lines := strings.Split(string(content), "\n")
	for i, line := range lines {
		line = strings.TrimPrefix(strings.TrimSpace(string(line)), "export ")
		if line == "" || line[0] == '#' {
			continue
		}
		equalIndex := strings.Index(line, "=")
		if equalIndex != -1 {
			key := line[:equalIndex]
			if key != deepinFstart {
				continue
			}
			value := strings.Trim(strings.TrimSpace(line[strings.Index(line, "=")+1:]), "\"")
			logger.Debug("FstartState:", value)
			return value == "true", i
		}
	}
	return false, -1
}

func setFstartState(state bool) error {
	value, lineNum := getFstartState()
	if lineNum == -1 {
		return errors.New(deepinFstartFile + "is illegal!")
	}
	if value == state {
		logger.Debug("current state is same : ", state)
		return nil
	}
	content, err := ioutil.ReadFile(deepinFstartFile)
	if err != nil {
		return err
	}
	lines := strings.Split(string(content), "\n")
	line := strings.TrimPrefix(strings.TrimSpace(string(lines[lineNum])), "export ")
	if !strings.Contains(line, deepinFstart) {
		return errors.New(deepinFstartFile + "is illegal!")
	}
	arg := "true"
	if !state {
		arg = "false"
	}
	if strings.Trim(strings.TrimSpace(line[strings.Index(line, "=")+1:]), "\"") != arg {
		lines[lineNum] = "export " + deepinFstart + "=\"" + arg + "\""
		err := ioutil.WriteFile(deepinFstartFile, []byte(strings.Join(lines, "\n")), 0644)
		if err != nil {
			return dbusutil.ToError(err)
		}
	}
	return nil
}
