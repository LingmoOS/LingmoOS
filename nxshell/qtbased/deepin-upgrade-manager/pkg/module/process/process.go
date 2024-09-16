// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package process

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

type Process uint

func (p Process) getFile(name string) string {
	return fmt.Sprintf("/proc/%d/%s", uint(p), name)
}

func (p Process) Exist() bool {
	procDir := fmt.Sprintf("/proc/%d", uint(p))
	_, err := os.Stat(procDir)
	return err == nil
}

func (p Process) Cmdline() ([]string, error) {
	cmdlineFile := p.getFile("cmdline")
	bytes, err := ioutil.ReadFile(filepath.Clean(cmdlineFile))
	if err != nil {
		return nil, err
	}
	content := string(bytes)
	parts := strings.Split(content, "\x00")
	length := len(parts)
	if length >= 2 && parts[length-1] == "" {
		return parts[:length-1], nil
	}
	return parts, nil
}

func (p Process) Cwd() (string, error) {
	cwdFile := p.getFile("cwd")
	cwd, err := os.Readlink(cwdFile)
	return cwd, err
}

func (p Process) Exe() (string, error) {
	exeFile := p.getFile("exe")
	exe, err := filepath.EvalSymlinks(exeFile)
	return exe, err
}

type EnvVars []string

func (p Process) Environ() (EnvVars, error) {
	envFile := p.getFile("environ")
	contents, err := ioutil.ReadFile(filepath.Clean(envFile))
	if err != nil {
		return nil, err
	}
	return strings.Split(string(contents), "\x00"), nil
}

func (vars EnvVars) Lookup(key string) (string, bool) {
	prefix := key + "="
	for _, aVar := range vars {
		if strings.HasPrefix(aVar, prefix) {
			return aVar[len(prefix):], true
		}
	}
	return "", false
}

func (vars EnvVars) Get(key string) string {
	ret, _ := vars.Lookup(key)
	return ret
}
