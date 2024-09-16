// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"fmt"
	"sort"
)

type MatchResult struct {
	score SearchScore
	item  *Item
}

func (r *MatchResult) String() string {
	if r == nil {
		return "<nil>"
	}
	return fmt.Sprintf("<MatchResult item=%v score=%v>", r.item.ID, r.score)
}

type MatchResults []*MatchResult

// impl sort interface
func (p MatchResults) Len() int { return len(p) }
func (p MatchResults) Less(i, j int) bool {
	if p[i].score == p[j].score {
		return p[i].item.ID < p[j].item.ID
	}
	return p[i].score < p[j].score
}
func (p MatchResults) Swap(i, j int) { p[i], p[j] = p[j], p[i] }

func (results MatchResults) GetTruncatedOrderedIDs() []string {
	sort.Sort(sort.Reverse(results))
	const idsMaxLen = 42

	idsLen := len(results)
	if idsLen > idsMaxLen {
		idsLen = idsMaxLen
	}
	ids := make([]string, idsLen)

	for i := 0; i < idsLen; i++ {
		ids[i] = results[i].item.ID
	}
	return ids
}

func (results MatchResults) Copy() MatchResults {
	resultsCopy := make(MatchResults, len(results))
	copy(resultsCopy, results)
	return resultsCopy
}
