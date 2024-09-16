// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package logger

import (
	"fmt"
	"io"
	"log"
	"log/syslog"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

type Logger struct {
	name       string
	log        *log.Logger
	level      Priority
	isInStdout bool
	fd         *os.File
}

type Priority int

var logger Logger

const (
	defaultDebugMatchEnv = "DDE_DEBUG_MATCH"
)

const (
	LevelDisable Priority = iota
	LevelFatal
	LevelPanic
	LevelError
	LevelWarning
	LevelInfo
	LevelDebug

	OutInitrdFile = "/var/log/deepin/deepin-upgrade-manager/initramfs.log"
)

func IsEnvExists(envName string) (ok bool) {
	for _, e := range os.Environ() {
		if strings.HasPrefix(e, envName+"=") {
			ok = true
			break
		}
	}
	return
}

func Disable() {
	logger.level = LevelDisable
}

func LoggerFD() *os.File {
	if logger.fd != nil {
		return logger.fd
	}
	return nil
}

func NewLogger(name string, isInitramfs bool) {
	inStdOut := false
	if isInitramfs {
		logger.name = name
		logger.level = LevelDebug
		OutputInitrdFile()
	} else {
		var writer io.Writer
		lg := new(log.Logger)
		syslogger, err := syslog.New(syslog.LOG_INFO, name)
		if IsEnvExists(defaultDebugMatchEnv) && os.Getenv(defaultDebugMatchEnv) == name {
			writer = io.MultiWriter(os.Stderr, syslogger)
			inStdOut = true
			lg = log.New(writer, "", log.Ldate|log.Ltime)
		} else {
			if err == nil {
				writer = io.MultiWriter(syslogger)
				inStdOut = false
				lg = log.New(writer, "", log.Ltime)
			} else {
				writer = io.MultiWriter(os.Stderr)
				inStdOut = false
				lg = log.New(writer, "", log.Ldate|log.Ltime)
			}
		}
		logger = Logger{
			name:       name,
			log:        lg,
			level:      LevelDebug,
			isInStdout: inStdOut,
		}
	}
}

func OutPutFile(path string) {
	os.MkdirAll(filepath.Join(filepath.Dir(path)), 0644)
	fd, _ := os.OpenFile(path, os.O_TRUNC|os.O_WRONLY|os.O_CREATE, 0666)
	writer := io.MultiWriter(fd)
	writers := io.MultiWriter(writer)
	logger.log = log.New(writers, "", log.Ldate|log.Ltime)
	Save()
	logger.fd = fd
}

func OutputInitrdFile() {
	OutPutFile(OutInitrdFile)
}

func CopyLogFile(root string) {
	Save()
	srcList := []string{OutInitrdFile}
	os.MkdirAll(filepath.Join(root, filepath.Dir(OutInitrdFile)), 0644)
	for _, v := range srcList {
		cpCmd := exec.Command("cp", "-rf", v, filepath.Join(root, v))
		err := cpCmd.Run()
		if err != nil {
			fmt.Printf("failed save log, %v", err)
		}
	}
}

func Save() {
	if logger.fd != nil {
		logger.fd.Sync()
		logger.fd.Close()
	}
}

func (l *Logger) doLog(level Priority, v ...interface{}) {
	if !l.isNeedLog(level) {
		return
	}
	s := buildMsg(3, l.isNeedTraceMore(level), v...)
	l.log.Output(3, combineMsg(level, l.name, s))
}

func (l *Logger) doLogf(level Priority, format string, v ...interface{}) {
	if !l.isNeedLog(level) {
		return
	}
	s := buildFormatMsg(3, l.isNeedTraceMore(level), format, v...)

	l.log.Output(3, combineMsg(level, l.name, s))
}

func (l *Logger) isNeedTraceMore(level Priority) bool {
	return level <= LevelError
}

func (l *Logger) isNeedLog(level Priority) bool {
	return level <= l.level
}

func buildMsg(calldepth int, loop bool, v ...interface{}) (msg string) {
	s := fmtSprint(v...)
	msg = doBuildMsg(calldepth+1, loop, s)
	return
}

func buildFormatMsg(calldepth int, loop bool, format string, v ...interface{}) (msg string) {
	s := fmt.Sprintf(format, v...)
	msg = doBuildMsg(calldepth+1, loop, s)
	return
}

func fmtSprint(v ...interface{}) (s string) {
	s = fmt.Sprintln(v...)
	s = strings.TrimSuffix(s, "\n")
	return
}

func doBuildMsg(calldepth int, loop bool, s string) (msg string) {
	var file, lastFile string
	var line, lastLine int
	var ok bool
	_, file, line, ok = runtime.Caller(calldepth)
	lastFile, lastLine = file, line
	msg = fmt.Sprintf("%s:%d: %s", filepath.Base(file), line, s)
	if loop && ok {
		for {
			calldepth++
			_, file, line, ok = runtime.Caller(calldepth)
			if file == lastFile && line == lastLine {
				// prevent infinite loop for that some platforms not
				// works well, e.g. mips
				break
			}
			if ok {
				msg = fmt.Sprintf("%s\n  ->  %s:%d", msg, filepath.Base(file), line)
			}
			lastFile, lastLine = file, line
		}
	}
	return
}

func combineMsg(level Priority, module, msg string) (combineMsg string) {
	var leve string
	switch level {
	case LevelDebug:
		leve = " [Debug]"
	case LevelInfo:
		leve = " [Info]"
	case LevelWarning:
		leve = " [Warning]"
	case LevelError:
		leve = " [Error]"
	case LevelFatal:
		leve = " [Fatal]"
	}
	if logger.isInStdout {
		return module + leve + msg
	} else {
		return leve + msg
	}
}

func Debug(v ...interface{}) {
	logger.doLog(LevelDebug, v...)
}

func Warning(v ...interface{}) {
	logger.doLog(LevelWarning, v...)
}

func Info(v ...interface{}) {
	logger.doLog(LevelInfo, v...)
}

func Error(v ...interface{}) {
	logger.doLog(LevelError, v...)
}

func Fatal(v ...interface{}) {
	logger.doLog(LevelFatal, v...)
}

func Debugf(format string, v ...interface{}) {
	logger.doLogf(LevelDebug, format, v...)
}

func Warningf(format string, v ...interface{}) {
	logger.doLogf(LevelWarning, format, v...)
}

func Infof(format string, v ...interface{}) {
	logger.doLogf(LevelInfo, format, v...)
}

func Errorf(format string, v ...interface{}) {
	logger.doLogf(LevelError, format, v...)
}

func Fatalf(format string, v ...interface{}) {
	logger.doLogf(LevelFatal, format, v...)
}
