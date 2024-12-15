// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package log

import (
	"fmt"
	"io"
	"log"
	"log/syslog"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

type Logger struct {
	name  string
	log   *log.Logger
	level Priority
}

type Priority int

var logger Logger

var _stdLog = log.New(os.Stderr, "", log.Lshortfile)

const (
	LevelDisable Priority = iota
	LevelFatal
	LevelPanic
	LevelError
	LevelWarning
	LevelInfo
	LevelDebug
)

func NewLogger(name string) *Logger {
	var writer io.Writer
	syslogger, err := syslog.New(syslog.LOG_DAEMON, name)
	if err != nil {
		_stdLog.Printf("failed to open log file: %v", err)
		writer = io.MultiWriter(os.Stderr)
	} else {
		writer = io.MultiWriter(os.Stderr, syslogger)
	}
	return &Logger{
		name:  name,
		log:   log.New(writer, "", log.Ldate|log.Ltime),
		level: LevelDebug,
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
	switch level {
	case LevelDebug:
		module += " [Debug]"
	case LevelInfo:
		module += " [Info]"
	case LevelWarning:
		module += " [Warning]"
	case LevelError:
		module += " [Error]"
	case LevelFatal:
		module += " [Fatal]"
	}
	return module + msg
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
