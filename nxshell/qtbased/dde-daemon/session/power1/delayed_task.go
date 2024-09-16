// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"sync"
	"time"
)

type delayedTaskState uint

const (
	delayedTaskStateReady delayedTaskState = iota
	delayedTaskStateRunning
	delayedTaskStateDone
)

type delayedTask struct {
	state      delayedTaskState
	mu         sync.Mutex
	cancelable bool
	timer      *time.Timer
	name       string
}

func newDelayedTask(name string, delay time.Duration, fn func()) *delayedTask {
	t := &delayedTask{
		state:      delayedTaskStateReady,
		cancelable: true,
		name:       name,
	}
	t.timer = time.AfterFunc(delay, func() {
		t.mu.Lock()
		t.cancelable = false
		t.state = delayedTaskStateRunning
		t.mu.Unlock()

		fn()

		t.mu.Lock()
		t.state = delayedTaskStateDone
		t.mu.Unlock()
	})
	return t
}

func (t *delayedTask) Cancel() {
	t.mu.Lock()
	if t.cancelable {
		t.timer.Stop()
		t.state = delayedTaskStateDone
		logger.Debugf("delayedTask %s cancelled", t.name)
	}
	t.mu.Unlock()
}

type delayedTasks []*delayedTask

func (tasks delayedTasks) Wait(delay time.Duration, countMax int) {
	count := 0
	for {
		allDone := true
		for _, task := range tasks {
			task.mu.Lock()
			state := task.state
			task.mu.Unlock()

			if state != delayedTaskStateDone {
				allDone = false
				break
			}
		}
		if allDone || count >= countMax {
			break
		}
		time.Sleep(delay)
		count++
	}
}

func (tasks delayedTasks) CancelAll() {
	for _, task := range tasks {
		task.Cancel()
	}
}
