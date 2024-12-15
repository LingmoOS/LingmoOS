// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"fmt"
	"strings"
	"sync"
	"unicode"
)

type searchTask struct {
	mu    sync.RWMutex
	chars []rune
	stack *searchTaskStack

	result MatchResults

	isCanceled bool
	isFinished bool
}

func (t *searchTask) IsCanceled() bool {
	t.mu.RLock()
	val := t.isCanceled
	t.mu.RUnlock()
	return val
}

func (t *searchTask) Cancel() {
	t.mu.Lock()
	t.isCanceled = true
	t.mu.Unlock()
}

func (t *searchTask) IsFinished() bool {
	t.mu.RLock()
	val := t.isFinished
	t.mu.RUnlock()
	return val
}

func (t *searchTask) Finish() {
	t.mu.Lock()
	t.isFinished = true
	t.mu.Unlock()
	logger.Debug("finish", t)
}

func (t *searchTask) String() string {
	if t == nil {
		return "<nil>"
	}
	canceled := t.IsCanceled()
	finished := t.IsFinished()
	return fmt.Sprintf("<Task %s count=%v canceled=%v finished=%v>", string(t.chars), len(t.result), canceled, finished)
}

func newSearchTask(c rune, stack *searchTaskStack, prev *searchTask) *searchTask {
	t := &searchTask{
		stack: stack,
	}

	if prev != nil {
		// copy chars
		t.chars = prev.chars[:]
	}
	t.chars = append(t.chars, c)

	return t
}

func (t *searchTask) search(prev *searchTask) {
	if prev == nil {
		go t.searchWithoutBase()
	} else {
		if prev.IsFinished() {
			logger.Debug("start", t, "doSearch prev finished")
			go t.searchWithBase(prev.result)
		}
	}
}

func (t *searchTask) searchWithoutBase() {
	logger.Debug("search without base", t)
	for _, item := range t.stack.items {
		t.matchItem(item)
		if t.IsCanceled() {
			logger.Debug("matchItem stop canceled", t)
			return
		}
	}
	t.done()
}

func (st *searchTask) searchWithBase(result MatchResults) {
	for _, mResult := range result {
		st.matchItem(mResult.item)
		if st.IsCanceled() {
			logger.Debug("matchItem stop canceled", st)
			return
		}
	}
	st.done()
}

const (
	Poor         = 50
	BelowAverage = 60
	Average      = 70
	AboveAverage = 75
	Good         = 80
	VeryGood     = 85
	Excellent    = 90
	Highest      = 100
)

func (st *searchTask) match(item *Item) *MatchResult {
	var score SearchScore
	for v, vScore := range item.searchTargets {
		key := string(st.chars)
		index := strings.Index(v, key)
		if index != -1 {
			// key is substr of v
			score += 2 * vScore
			if len(key) == len(v) {
				// ^query$
				score += Highest
			} else if index == 0 {
				// ^query
				score += Excellent
			} else {
				prev := v[:index]
				var prevChar rune
				for _, r := range prev {
					prevChar = r
				}
				//logger.Debugf("prevChar %c", prevChar)
				if prevChar != 0 && !unicode.IsLetter(prevChar) {
					// \bquery
					score += AboveAverage
				} else {
					// xqueryx
					score += BelowAverage
				}
			}
		}
	}

	if score == 0 {
		return nil
	}
	mResult := &MatchResult{
		item:  item,
		score: score,
	}
	return mResult
}

func (st *searchTask) matchItem(item *Item) {
	mResult := st.match(item)
	if mResult != nil {
		logger.Debugf("searchTask %s match item score: %d, item: %v",
			string(st.chars), mResult.score, mResult.item)
		st.result = append(st.result, mResult)
	}
}

func (st *searchTask) emitResult() {
	st.stack.manager.emitSearchDone(st.result)
}

func (st *searchTask) done() {
	if st.IsCanceled() {
		logger.Debug("no done canceled", st)
		return
	}
	next := st.stack.GetNext(st)
	if next != nil {
		// notify next task
		logger.Debug("start", next, "next")
		go next.searchWithBase(st.result)
		st.Finish()
	} else {
		// if no next task, emit SearchDone signal
		st.Finish()
		st.emitResult()
	}
}
