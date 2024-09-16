// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"fmt"
	"io/ioutil"
	"os"
	"time"

	"github.com/linuxdeepin/go-lib/encoding/kv"
)

const (
	dataDir     = "/var/cache/deepin"
	logFile     = dataDir + "/grub2.log"
	logFileMode = 0644

	logJobMkConfig    = "mkconfig"
	logJobAdjustTheme = "adjustTheme"
)

func logStart() {
	content := fmt.Sprintf("start=%s\n", time.Now())
	err := ioutil.WriteFile(logFile, []byte(content), logFileMode)
	if err != nil {
		logger.Warning("logStart write failed:", err)
	}
}

func logAppendText(text string) {
	f, err := os.OpenFile(logFile, os.O_APPEND|os.O_WRONLY, logFileMode)
	if err != nil {
		logger.Warning("logAppendText open failed:", err)
		return
	}
	defer f.Close()
	_, err = f.WriteString(text)
	if err != nil {
		logger.Warning("logAppendText write failed:", err)
	}
}

func logEnd() {
	logAppendText(fmt.Sprintf("end=%s\n", time.Now()))
}

func logJobStart(jobName string) {
	logAppendText(fmt.Sprintf("%sStart=%s\n", jobName, time.Now()))
}

func logJobEnd(jobName string, err error) {
	text := fmt.Sprintf("%sEnd=%s\n", jobName, time.Now())
	if err != nil {
		text += jobName + "Failed=1\n"
	}
	logAppendText(text)
}

type Log map[string]string

func loadLog() (Log, error) {
	f, err := os.Open(logFile)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	l := make(Log)
	reader := kv.NewReader(f)

	for {
		pair, err := reader.Read()
		if err != nil {
			break
		}
		l[pair.Key] = pair.Value
	}

	return l, nil
}

func (l Log) hasJob(jobName string) bool {
	_, ok := l[jobName+"Start"]
	return ok
}

func (l Log) isJobDone(jobName string) bool {
	_, ok := l[jobName+"End"]
	return ok
}
