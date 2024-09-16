// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

//#cgo pkg-config: x11 gtk+-3.0
//#cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
//#include <X11/Xlib.h>
//#include <gtk/gtk.h>
//void init(){XInitThreads();gtk_init(0,0);}
import "C"
import (
	"bufio"
	"flag"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"os/user"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/soundutils"
	"github.com/linuxdeepin/dde-api/userenv"
	"github.com/linuxdeepin/dde-daemon/loader"
	soundthemeplayer "github.com/linuxdeepin/go-dbus-factory/system/com.deepin.api.soundthemeplayer"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	. "github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/proxy"
	"github.com/linuxdeepin/go-lib/utils"
	"github.com/linuxdeepin/go-lib/xdg/basedir"
)

var logger = log.NewLogger("daemon/dde-session-daemon")
var hasDDECookie bool
var hasTreeLand bool

var treeLandNotAllowModules = []string{"x-event-monitor", "keybinding", "trayicon", "screensaver", "inputdevices", "power"}

func isInShutdown() bool {
	bus, err := dbus.SystemBus()
	if err != nil {
		return false
	}

	manager := login1.NewManager(bus)

	val, err := manager.PreparingForShutdown().Get(0)
	if err != nil {
		return false
	}

	return val
}

func allowRun() bool {
	if os.Getenv("DDE_SESSION_PROCESS_COOKIE_ID") != "" {
		hasDDECookie = true
		return true
	}

	systemBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		os.Exit(1)
	}
	sessionManagerObj := systemBus.Object("org.deepin.dde.SessionManager1",
		"/org/deepin/dde/SessionManager1")
	var allowRun bool
	err = sessionManagerObj.Call("org.deepin.dde.SessionManager1.AllowSessionDaemonRun",
		dbus.FlagNoAutoStart).Store(&allowRun)
	if err != nil {
		logger.Warning(err)
		return false
	}

	return allowRun
}

var _options struct {
	verbose  bool
	logLevel string
	list     string
	enable   string
	disable  string
	ignore   bool
	force    bool

	enablingModules []string
	disableModules  []string
}

func toLogLevel(name string) (log.Priority, error) {
	name = strings.ToLower(name)
	logLevel := log.LevelInfo
	var err error
	switch name {
	case "":
		logLevel = log.LevelInfo
	case "error":
		logLevel = log.LevelError
	case "warn":
		logLevel = log.LevelWarning
	case "info":
		logLevel = log.LevelInfo
	case "debug":
		logLevel = log.LevelDebug
	case "no":
		logLevel = log.LevelDisable
	default:
		err = fmt.Errorf("%s is not support", name)
	}

	return logLevel, err
}

func init() {
	rand.Seed(time.Now().UnixNano())

	// -v | -verbose
	const verboseUsage = "Show much more message, shorthand for --loglevel debug."
	flag.BoolVar(&_options.verbose, "v", false, verboseUsage)
	flag.BoolVar(&_options.verbose, "verbose", false, verboseUsage)

	// -l | -loglevel
	const logLevelUsage = "Set log level, possible value is error/warn/info/debug/no, info is default"
	flag.StringVar(&_options.logLevel, "l", "", logLevelUsage)
	flag.StringVar(&_options.logLevel, "loglevel", "", logLevelUsage)

	// -f | -force
	const forceUsage = "Force start disabled module."
	flag.BoolVar(&_options.force, "f", false, forceUsage)
	flag.BoolVar(&_options.force, "force", false, forceUsage)

	// -i | -ignore
	const ignoreUsage = "Ignore missing modules."
	flag.BoolVar(&_options.ignore, "i", true, ignoreUsage)
	flag.BoolVar(&_options.ignore, "ignore", true, ignoreUsage)

	// -list
	flag.StringVar(&_options.list, "list", "",
		"List all the modules or the dependencies of one module. The argument can be all or the name of the module.")

	// -enable
	flag.StringVar(&_options.enable, "enable", "",
		"Enable modules and their dependencies, ignore settings.")

	// -disable
	flag.StringVar(&_options.disable, "disable", "", "Disable modules, ignore settings.")

}

func main() {
	logger.SetLogLevel(log.LevelInfo)
	if !allowRun() {
		logger.Warning("session manager does not allow me to run")
		os.Exit(1)
	}

	if isInShutdown() {
		logger.Warning("system is in shutdown, no need to run")
		os.Exit(1)
	}

	flag.Parse()
	InitI18n()
	BindTextdomainCodeset("dde-daemon", "UTF-8")
	Textdomain("dde-daemon")

	if _options.verbose {
		_options.logLevel = "debug"
	}

	logLevel, err := toLogLevel(_options.logLevel)
	if err != nil {
		logger.Warning("failed to parse loglevel:", err)
		os.Exit(1)
	}

	if _options.enable != "" {
		_options.enablingModules = strings.Split(_options.enable, ",")
	}
	if _options.disable != "" {
		_options.disableModules = strings.Split(_options.disable, ",")
	}

	if os.Getenv("DDE_CURRENT_COMPOSITOR") == "TreeLand" {
		hasTreeLand = true
	}

	logger.Infof("env DDE_CURRENT_COMPOSITOR is %s", os.Getenv("DDE_CURRENT_COMPOSITOR"))

	usr, err := user.Current()
	if err == nil {
		_ = os.Chdir(usr.HomeDir)
	}

	C.init()
	proxy.SetupProxy()

	app := NewSessionDaemon(logger)
	if app == nil {
		return
	}
	service, err := dbusutil.NewSessionService()
	if err != nil {
		logger.Fatal(err)
	}

	if err = app.register(service); err != nil {
		logger.Info(err)
		os.Exit(0)
	}

	loader.SetService(service)

	if _options.logLevel == "" &&
		(utils.IsEnvExists(log.DebugLevelEnv) || utils.IsEnvExists(log.DebugMatchEnv)) {
		logger.Info("Log level is none and debug env exists, so do not call loader.SetLogLevel")
	} else {
		logger.Info("App log level:", _options.logLevel)
		// set all modules log level to logLevel
		loader.SetLogLevel(logLevel)
	}

	// Ensure each module and mainloop in the same thread
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	if _options.list != "" {
		err = app.listModule(_options.list)
		if err != nil {
			logger.Warning(err)
			os.Exit(1)
		}
		os.Exit(0)
	} else if len(_options.enablingModules) > 0 {
		err = app.enableModules(_options.enablingModules)
	} else if len(_options.disableModules) > 0 {
		err = app.disableModules(_options.disableModules)
	} else if hasTreeLand {
		err = app.disableModules(treeLandNotAllowModules)
	} else {
		app.execDefaultAction()
	}

	if err != nil {
		logger.Warning(err)
		os.Exit(1)
	}

	err = migrateUserEnv()
	if err != nil {
		logger.Warning("failed to migrate user env:", err)
	}

	err = syncConfigToSoundThemePlayer()
	if err != nil {
		logger.Warning(err)
	}

	runMainLoop()
}

// migrate user env from ~/.pam_environment to ~/.dde-env
func migrateUserEnv() error {
	_, err := os.Stat(userenv.DefaultFile())
	if os.IsNotExist(err) {
		// when ~/.dde-env does not exist, read ~/.pam_environment,
		// remove the key we set before, and save it back.
		pamEnvFile := filepath.Join(basedir.GetUserHomeDir(), ".pam_environment")
		pamEnv, err := loadPamEnv(pamEnvFile)
		if os.IsNotExist(err) {
			return nil
		} else if err != nil {
			return err
		}

		var reservedPamEnv []pamEnvKeyValue
		for _, kv := range pamEnv {
			switch kv.key {
			case "LANG", "LANGUAGE", "QT_SCALE_FACTOR", "_JAVA_OPTIONS":
			// ignore it
			default:
				reservedPamEnv = append(reservedPamEnv, kv)
			}
		}

		if len(reservedPamEnv) == 0 {
			err = os.Remove(pamEnvFile)
		} else {
			err = savePamEnv(pamEnvFile, reservedPamEnv)
		}
		if err != nil {
			return err
		}

		// save the current env to ~/.dde-env
		currentEnv := make(map[string]string)
		for _, envKey := range []string{"LANG", "LANGUAGE"} {
			envValue, ok := os.LookupEnv(envKey)
			if ok {
				currentEnv[envKey] = envValue
			}
		}
		err = userenv.Save(currentEnv)
		return err
	} else if err != nil {
		return err
	}
	return nil
}

type pamEnvKeyValue struct {
	key, value string
}

func loadPamEnv(filename string) ([]pamEnvKeyValue, error) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	lines := strings.Split(string(content), "\n")
	var result []pamEnvKeyValue
	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" || line[0] == '#' {
			continue
		}

		fields := strings.SplitN(line, "=", 2)
		if len(fields) == 2 {
			result = append(result, pamEnvKeyValue{
				key:   fields[0],
				value: fields[1],
			})
		}
	}
	return result, nil
}

func savePamEnv(filename string, pamEnv []pamEnvKeyValue) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()

	bw := bufio.NewWriterSize(f, 256)
	for _, kv := range pamEnv {
		_, err = fmt.Fprintf(bw, "%s=%s\n", kv.key, kv.value)
		if err != nil {
			return err
		}
	}

	err = bw.Flush()
	return err
}

// 同步所有涉及系统级设置的音效开关
func syncConfigToSoundThemePlayer() error {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	player := soundthemeplayer.NewSoundThemePlayer(sysBus)
	soundEffectGs := gio.NewSettings("com.deepin.dde.sound-effect")
	defer soundEffectGs.Unref()

	for _, name := range []string{"", soundutils.EventDesktopLogin,
		soundutils.EventSystemShutdown} {
		gsKey := name
		if name == "" {
			// name 为空表示音效总开关
			gsKey = "enabled"
		}

		enabled := soundEffectGs.GetBoolean(gsKey)
		err = player.EnableSound(0, name, enabled)
		if err != nil {
			return err
		}
	}

	return err
}
