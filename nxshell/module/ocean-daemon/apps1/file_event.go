// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package apps1

import (
	"os"

	"github.com/fsnotify/fsnotify"
)

type FileEvent struct {
	fsnotify.Event
	NotExist bool
	IsDir    bool
	IsFound  bool
}

func NewFileFoundEvent(name string) *FileEvent {
	return &FileEvent{
		Event: fsnotify.Event{
			Name: name,
		},
		IsFound: true,
	}
}

func NewFileEvent(ev fsnotify.Event) *FileEvent {
	var notExist bool
	var isDir bool
	if stat, err := os.Stat(ev.Name); os.IsNotExist(err) {
		notExist = true
	} else if err == nil {
		isDir = stat.IsDir()
	}
	return &FileEvent{
		Event:    ev,
		NotExist: notExist,
		IsDir:    isDir,
	}
}
