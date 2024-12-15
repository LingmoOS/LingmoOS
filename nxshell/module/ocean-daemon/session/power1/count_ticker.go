// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"time"
)

type countTicker struct {
	action   func(int)
	interval time.Duration
	count    int
	ticker   *time.Ticker
	exit     chan struct{}
}

func newCountTicker(interval time.Duration, action func(int)) *countTicker {
	t := &countTicker{
		interval: interval,
		action:   action,
	}
	t.Reset()
	return t
}

func (t *countTicker) Reset() {
	t.ticker = time.NewTicker(t.interval)
	t.count = 0
	t.action(0)
	t.exit = make(chan struct{})
	go func() {
		for {
			select {
			case _, ok := <-t.ticker.C:
				if !ok {
					logger.Error("Invalid ticker event")
					return
				}

				t.count++
				logger.Debug("tick", t.count)
				t.action(t.count)
			case <-t.exit:
				t.exit = nil
				return
			}
		}
	}()
}

func (t *countTicker) Stop() {
	if t.ticker != nil {
		logger.Debug("Stop")
		t.ticker.Stop()
	}
	if t.exit != nil {
		logger.Debug("exit")
		close(t.exit)
	}
}
