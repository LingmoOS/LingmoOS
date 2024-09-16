// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"os"
	"os/exec"
	"strings"
	"sync"
	"syscall"

	"github.com/linuxdeepin/dde-daemon/grub_common"
)

const (
	grubMkconfigCmd = "grub-mkconfig"
	updateGrubCmd   = "update-grub"
	adjustThemeCmd  = "/usr/lib/deepin-api/adjust-grub-theme"
)

func init() {
	_ = os.Setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin")
}

type modifyManager struct {
	g           *Grub2
	ch          chan modifyTask
	modifyTasks []modifyTask

	running       bool
	stateChangeCb func(running bool)

	mu sync.Mutex
}

func newModifyManager() *modifyManager {
	m := &modifyManager{
		ch: make(chan modifyTask),
	}
	return m
}

func (m *modifyManager) notifyStateChange() {
	if m.stateChangeCb != nil {
		m.stateChangeCb(m.running)
	}
}

func (m *modifyManager) loop() {
	for {
		t, ok := <-m.ch
		if !ok {
			return
		}
		m.mu.Lock()

		if m.running {
			m.modifyTasks = append(m.modifyTasks, t)
		} else {
			m.start(t)
		}

		m.mu.Unlock()
	}
}

func (m *modifyManager) start(tasks ...modifyTask) {
	logger.Infof("modifyManager start")
	defer logger.Infof("modifyManager start return")

	params, _ := grub_common.LoadDDEGrubParams()

	logger.Debug("modifyManager.start len(tasks):", len(tasks))
	var adjustTheme bool
	var adjustThemeLang string
	for _, task := range tasks {
		f := task.paramsModifyFunc
		if f != nil {
			f(params)
		}
		if task.adjustTheme {
			adjustTheme = true
			adjustThemeLang = task.adjustThemeLang
		}
	}
	err := writeGrubParams(params)
	if err != nil {
		logger.Warning("failed to write grub params:", err)
		return
	}

	logStart()
	m.running = true
	m.notifyStateChange()
	go m.update(adjustTheme, adjustThemeLang)
}

func (m *modifyManager) update(adjustTheme bool, adjustThemeLang string) {
	if adjustTheme {
		logJobStart(logJobAdjustTheme)
		err := copyBgSource(defaultThemeDir, defaultThemeTmpDir)
		if err != nil && !os.IsNotExist(err) {
			logger.Warning("failed to copy background source:", err)
		} else {
			var args []string
			if adjustThemeLang != "" {
				args = append(args, "-lang", adjustThemeLang)
			}
			args = append(args, "-theme-output", themesTmpDir)
			cmd := exec.Command(adjustThemeCmd, args...)
			cmd.Stdout = os.Stdout
			cmd.Stderr = os.Stderr

			logger.Debugf("$ %s %s", adjustThemeCmd, strings.Join(args, " "))
			err = cmd.Run()
			if err != nil {
				logger.Warning("failed to adjust theme:", err)
			} else {
				syscall.Sync()
				err = replaceAndBackupDir(themesDir, themesTmpDir)
				if err != nil {
					logger.Warning("failed to replace and backup dir:", err)
				}
			}
		}
		logJobEnd(logJobAdjustTheme, err)
		m.g.theme.emitSignalBackgroundChanged()
	}

	logJobStart(logJobMkConfig)
	err := runUpdateGrub()
	if err != nil {
		logger.Warning("failed to make config:", err)
	}
	logJobEnd(logJobMkConfig, err)
	m.updateEnd()
}

func runUpdateGrub() error {
	updateGrubPath, err := exec.LookPath(updateGrubCmd)
	var cmd *exec.Cmd
	if err == nil {
		cmd = exec.Command(updateGrubPath)
		logger.Debugf("$ %s", updateGrubCmd)
	} else {
		// fallback to grub-mkconfig
		cmd = exec.Command(grubMkconfigCmd, "-o", grubScriptFile)
		logger.Debugf("$ %s -o %s", grubMkconfigCmd, grubScriptFile)
	}

	locale := getSystemLocale()
	if locale != "" {
		logger.Info("system locale:", locale)
		language := strings.Split(locale, ".")[0]
		cmd.Env = append(os.Environ(), "LANG="+locale, "LANGUAGE="+language)
	} else {
		logger.Warning("failed to get system locale")
	}

	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

func (m *modifyManager) updateEnd() {
	m.mu.Lock()

	logEnd()
	logger.Info("modifyManager update end")

	if len(m.modifyTasks) > 0 {
		m.start(m.modifyTasks...)
		m.modifyTasks = nil
	} else {
		// loop end
		syscall.Sync()
		m.running = false
		m.notifyStateChange()
	}

	m.mu.Unlock()
}

func (m *modifyManager) IsRunning() bool {
	m.mu.Lock()
	running := m.running
	m.mu.Unlock()
	return running
}
