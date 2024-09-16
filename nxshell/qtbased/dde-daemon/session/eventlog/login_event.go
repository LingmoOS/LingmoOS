// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"encoding/json"
	"errors"
	"os/exec"
	"strings"
	"time"

	"github.com/linuxdeepin/go-lib/dbusutil"
)

type loginEventCollector struct {
	service         *dbusutil.Service
	writeEventLogFn writeEventLogFunc
}

type loginTimeInfo struct {
	Tid          TidTyp
	BootTime     int64
	ShutdownTime int64
}

const (
	SystemBootTid TidTyp = 1000600001
)

func init() {
	register("login", newLoginEventCollector())
}

func newLoginEventCollector() *loginEventCollector {
	c := &loginEventCollector{}
	return c
}

func (c *loginEventCollector) Init(service *dbusutil.Service, fn writeEventLogFunc) error {
	logger.Info("Login event collector init")
	if service == nil || fn == nil {
		return errors.New("failed to init dconfigLogCollector: error args")
	}
	c.service = service
	c.writeEventLogFn = fn
	return nil
}

func (c *loginEventCollector) Collect() error {
	err := c.collectLoginTimeInfo()
	if err != nil {
		return err
	}
	return nil
}
func genLastRebootCmd() *exec.Cmd {
	args := []string{"last", "-x", "reboot", "--time-format", "iso", "|", "awk", "'NR==2 {print}'"}
	// create command
	return exec.Command("/bin/bash", "-c", strings.Join(args, " "))
}

func parseLastRebootCmdOutPut(content []byte) (*loginTimeInfo, error) {
	const (
		cmdOutputPrefix    = "reboot   system boot  "
		cmdErrOutputSuffix = "still running"
	)
	cmdOutput := string(content)
	if !strings.HasPrefix(cmdOutput, cmdOutputPrefix) {
		logger.Warning(cmdOutput)
		return nil, errors.New("last command result is not reboot")
	}

	lastRebootContent := strings.Replace(cmdOutput, cmdOutputPrefix, "", -1)
	sl := strings.Split(lastRebootContent, " ")
	if len(sl) < 6 || strings.HasSuffix(lastRebootContent, cmdErrOutputSuffix) {
		return nil, errors.New("last command don't record reboot data")
	}
	// parse boot time
	boot, err := time.ParseInLocation("2006-01-02T15:04:05+08:00", sl[1], time.Local)
	if err != nil {
		logger.Warningf("last command parse shutdown time failed, err: %v", err)
		return nil, err
	}
	info := &loginTimeInfo{}
	info.Tid = SystemBootTid
	info.ShutdownTime = boot.UnixNano() / 1e6

	shut, err := time.ParseInLocation("2006-01-02T15:04:05+08:00", sl[3], time.Local)
	if err != nil {
		logger.Warningf("last command parse shutdown time failed, err: %v", err)
		return nil, err
	}
	info.BootTime = shut.UnixNano() / 1e6
	return info, nil
}

func (c *loginEventCollector) collectLoginTimeInfo() error {
	out, err := genLastRebootCmd().Output()
	if err != nil {
		logger.Warningf("last command execute failed, out: %v, err: %v", string(out), err)
		return err
	}
	info, err := parseLastRebootCmdOutPut(out)
	if err != nil {
		return err
	}
	c.writeLoginLog(info)
	return nil
}

func (c *loginEventCollector) Stop() error {
	return nil
}

func (c *loginEventCollector) writeLoginLog(info *loginTimeInfo) error {
	content, err := json.Marshal(info)
	if err != nil {
		return err
	}
	if c.writeEventLogFn != nil {
		c.writeEventLogFn(string(content))
	}

	return nil
}
