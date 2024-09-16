// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package image_effect

import (
	"errors"
	"fmt"
	"os"
	"os/user"
	"path/filepath"
	"strconv"
	"sync"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/procfs"
	"golang.org/x/xerrors"
)

//go:generate dbusutil-gen em -type ImageEffect

const (
	dbusServiceName = "org.deepin.dde.ImageEffect1"
	dbusInterface   = dbusServiceName
	dbusPath        = "/org/deepin/dde/ImageEffect1"

	cacheDir      = "/var/cache/deepin/dde-daemon/image-effect"
	effectPixmix  = "pixmix"
	defaultEffect = effectPixmix
)

var allEffects = []string{effectPixmix}

type effectTool interface {
	generate(userName, inputFile, outputFile string, envVars []string) error
}

type effectToolFunc func(userName, inputFile, outputFile string, envVars []string) error

func (etf effectToolFunc) generate(userName, inputFile, outputFile string, envVars []string) error {
	return etf(userName, inputFile, outputFile, envVars)
}

type ImageEffect struct {
	service *dbusutil.Service
	tools   map[string]effectTool
	tasks   map[taskKey]*Task
	tasksMu sync.Mutex
}

func (ie *ImageEffect) addTask(effect, filename string) (ch chan error) {
	key := taskKey{effect, filename}

	ie.tasksMu.Lock()

	task, taskExist := ie.tasks[key]
	if !taskExist {
		task = &Task{}
		ie.tasks[key] = task
	} else {
		ch = make(chan error)
		task.chs = append(task.chs, ch)
	}

	ie.tasksMu.Unlock()

	return
}

func (ie *ImageEffect) hasTask(effect, filename string) bool {
	key := taskKey{effect, filename}
	ie.tasksMu.Lock()
	_, taskExist := ie.tasks[key]
	ie.tasksMu.Unlock()

	return taskExist
}

func (ie *ImageEffect) finishTask(effect, filename string, err error) {
	key := taskKey{effect, filename}

	ie.tasksMu.Lock()
	task := ie.tasks[key]
	if task != nil {
		delete(ie.tasks, key)
	}
	ie.tasksMu.Unlock()

	if task != nil {
		for _, ch := range task.chs {
			ch <- err
		}
	}
}

type Task struct {
	chs []chan error
}

type taskKey struct {
	effect   string
	filename string
}

func (ie *ImageEffect) GetInterfaceName() string {
	return dbusInterface
}

func newImageEffect() *ImageEffect {
	ie := &ImageEffect{
		tools: make(map[string]effectTool),
		tasks: make(map[taskKey]*Task),
	}
	ie.tools[effectPixmix] = effectToolFunc(ddePixmix)
	return ie
}

func ddePixmix(userName, inputFile, outputFile string, envVars []string) error {

	return runCmdRedirectStdOut(userName, outputFile, []string{"dde-pixmix", "-o=-", inputFile}, envVars)
}

func (ie *ImageEffect) Get(sender dbus.Sender, effect, filename string) (outputFile string, busErr *dbus.Error) {
	logger.Debugf("Get sender: %s, effect: %q, filename: %q", sender, effect, filename)
	var err error
	defer func() {
		if err != nil {
			logger.Warning(err)
		}
		busErr = dbusutil.ToError(err)
	}()

	filenameResolved, err := filepath.EvalSymlinks(filename)
	if err != nil {
		err = xerrors.Errorf("failed to eval symlinks: %w", err)
		return
	} else {
		filename = filenameResolved
	}

	uid, err := ie.service.GetConnUID(string(sender))
	if err != nil {
		err = xerrors.Errorf("failed to get conn uid: %w", err)
		return
	}
	pid, err := ie.service.GetConnPID(string(sender))
	if err != nil {
		err = xerrors.Errorf("failed to get conn pid: %w", err)
		return
	}

	usr, err := user.LookupId(string(strconv.Itoa(int(uid))))
	if err != nil {
		err = xerrors.Errorf("failed to get user: %w", err)
		return
	}

	process := procfs.Process(pid)
	processEnv, err := process.Environ()
	if err != nil {
		err = xerrors.Errorf("failed to get process %d environ: %w", pid, err)
		return
	}

	var envVarNames = []string{"DISPLAY", "XDG_RUNTIME_DIR"}
	var envVars = make([]string, len(envVarNames))
	for idx, envVarName := range envVarNames {
		envVarVal := processEnv.Get(envVarName)
		envVars[idx] = envVarName + "=" + envVarVal
	}

	outputFile, err = ie.get(usr.Username, effect, filename, envVars)
	if err != nil {
		err = xerrors.Errorf("failed to get output file: %w", err)
		return
	}
	return
}

func (ie *ImageEffect) get(username, effect, filename string, envVars []string) (outputFile string, err error) {
	if effect == "" {
		effect = defaultEffect
	}

	tool := ie.tools[effect]
	if tool == nil {
		err = fmt.Errorf("invalid effect %q", effect)
		return
	}

	inputFileInfo, err := os.Stat(filename)
	if err != nil {
		err = xerrors.Errorf("failed to stat file: %w", err)
		return
	}

	outputFile = getOutputFile(effect, filename)
	outputDir := filepath.Dir(outputFile)
	err = os.MkdirAll(outputDir, 0755)
	if err != nil {
		err = xerrors.Errorf("failed to make output dir: %w", err)
		return
	}

	outputFileInfo, err := os.Stat(outputFile)
	if err == nil {
		if outputFileInfo.Size() == 0 {
			logger.Warningf("file %q already exists, but the content is empty", outputFile)
		} else {
			// check mod time
			if modTimeEqual(inputFileInfo.ModTime(), outputFileInfo.ModTime()) {
				logger.Debug("mod time equal")
				return
			}
		}
	} else if !os.IsNotExist(err) {
		err = xerrors.Errorf("failed to stat outputFile: %w", err)
		return
	}

	ch := ie.addTask(effect, filename)
	if ch != nil {
		err = <-ch
		return
	}
	// task not exist
	shouldDelete := false
	t0 := time.Now()
	err = tool.generate(username, filename, outputFile, envVars)
	elapsed := time.Since(t0)
	ie.finishTask(effect, filename, err)

	if err == nil {
		// generate success
		err = setFileModTime(outputFile, inputFileInfo.ModTime())
		if err != nil {
			err = xerrors.Errorf("failed to set file modify time: %w", err)
			return
		}

		// check outputFile
		var fileInfo os.FileInfo
		fileInfo, err = os.Stat(outputFile)
		if err != nil {
			err = xerrors.Errorf("failed to stat output file: %w", err)
			return
		}
		if fileInfo.Size() == 0 {
			shouldDelete = true
			err = errors.New("generate success but output file is empty")
		}
	} else {
		// generate failed
		shouldDelete = true
		err = xerrors.Errorf("generate failed: %w", err)
	}

	logger.Debug("cost time:", elapsed)

	if shouldDelete {
		rmErr := os.Remove(outputFile)
		if rmErr != nil && !os.IsNotExist(rmErr) {
			logger.Warningf("failed to remove output file %q: %v", outputFile, rmErr)
		}
	}
	return
}

func (ie *ImageEffect) Delete(effect, filename string) (busErr *dbus.Error) {
	logger.Debugf("Delete effect: %q, filename: %q", effect, filename)
	var err error
	defer func() {
		if err != nil {
			logger.Warning(err)
		}
		busErr = dbusutil.ToError(err)
	}()

	filenameResolved, err := filepath.EvalSymlinks(filename)
	if err != nil {
		logger.Warningf("failed to eval symlinks %q: %v", filename, err)
	} else {
		filename = filenameResolved
	}

	if effect == "all" {
		for _, effect := range allEffects {
			err = ie.delete(effect, filename)
			if err != nil {
				logger.Warning(err)
			}
		}
		err = nil
		return
	}

	err = ie.delete(effect, filename)
	return
}

func (ie *ImageEffect) delete(effect, filename string) (err error) {
	if effect == "" {
		effect = defaultEffect
	}

	has := ie.hasTask(effect, filename)
	if has {
		return errors.New("generation task is in progress")
	}

	outputFile := getOutputFile(effect, filename)
	logger.Debugf("delete file %q, effect: %q, source: %q", outputFile, effect, filename)
	err = os.Remove(outputFile)
	if err != nil {
		if os.IsNotExist(err) {
			err = nil
		} else {
			logger.Warningf("failed to delete file %q: %v", outputFile, err)
		}
	}
	return
}
