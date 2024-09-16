// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package single

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
)

var (
	ErrProcessRunning = errors.New("process is running")
	ErrFileStale      = errors.New("pidfile exists but process is not running")
	ErrFileInvalid    = errors.New("pidfile has invalid contents")

	SingleInstance    string = "_single_instance_deepin-upgrade-manager"
	SingleInstanceDir string = "/run/deepin-upgrade-manager/"
)

func SetSingleInstance() bool {
	_ = os.MkdirAll(SingleInstanceDir, 0750)
	err := write(SingleInstanceDir + SingleInstance)
	return err != ErrProcessRunning
}

// Remove a pidfile
func Remove() error {
	return os.RemoveAll(SingleInstanceDir + SingleInstance)
}

// Write writes a pidfile, returning an error
// if the process is already running or pidfile is orphaned
func write(filename string) error {
	return writeControl(filename, os.Getpid(), true)
}

func writeControl(filename string, pid int, overwrite bool) error {
	// Check for existing pid
	oldpid, err := pidfileContents(filename)
	if err != nil && !os.IsNotExist(err) {
		return err
	}

	// We have a pid
	if err == nil {
		if pidIsRunning(oldpid) {
			return ErrProcessRunning
		}
		if !overwrite {
			return ErrFileStale
		}
	}

	// We're clear to (over)write the file
	return ioutil.WriteFile(filename, []byte(fmt.Sprintf("%d\n", pid)), 0600)
}

func pidfileContents(filename string) (int, error) {
	contents, err := ioutil.ReadFile(filepath.Clean(filename))
	if err != nil {
		return 0, err
	}

	oldpid, err := strconv.Atoi(strings.TrimSpace(string(contents)))
	if err != nil {
		return 0, ErrFileInvalid
	}

	return oldpid, nil
}

func pidIsRunning(pid int) bool {
	process, err := os.FindProcess(pid)
	if err != nil {
		return false
	}

	err = process.Signal(syscall.Signal(0))

	if err != nil && err.Error() == "no such process" {
		return false
	}

	if err != nil && err.Error() == "os: process already finished" {
		return false
	}

	return true
}
