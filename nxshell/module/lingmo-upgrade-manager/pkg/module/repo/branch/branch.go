// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// branch format: <os distribution>.<major>.<minor>.<date>
// if date equal, minor increment, else major increment and reset minor.
// such as: v23.0.0.20220215, v23.0.1.20220215, v23.1.0.20220220
package branch

import (
	"fmt"
	"strconv"
	"strings"
	"time"
)

const (
	BRANCH_DELIM = "."

	BRANCH_ITEM_LEN = 4
)

type BranchList []string

func (list BranchList) Len() int {
	return len(list)
}

func (list BranchList) Swap(i, j int) {
	list[i], list[j] = list[j], list[i]
}

func (list BranchList) Less(i, j int) bool {
	iList := strings.Split(list[i], BRANCH_DELIM)
	jList := strings.Split(list[j], BRANCH_DELIM)

	iMajor, _ := strconv.ParseInt(iList[1], 10, 64)
	jMajor, _ := strconv.ParseInt(jList[1], 10, 64)
	if iMajor > jMajor {
		return true
	} else if iMajor < jMajor {
		return false
	}
	iMinor, _ := strconv.ParseInt(iList[2], 10, 64)
	jMinor, _ := strconv.ParseInt(jList[2], 10, 64)
	return iMinor > jMinor
}

func Increment(branchName string) (string, error) {
	if !IsValid(branchName) {
		return "", fmt.Errorf("invalid branch name: %s", branchName)
	}

	items := strings.Split(branchName, BRANCH_DELIM)
	major, _ := strconv.ParseInt(items[1], 10, 64)
	minor, _ := strconv.ParseInt(items[2], 10, 64)
	t := time.Now()
	month := strconv.FormatInt(int64(t.Month()), 10)
	if len(month) == 1 {
		month = "0" + month
	}
	day := strconv.FormatInt(int64(t.Day()), 10)
	if len(day) == 1 {
		day = "0" + day
	}
	date := strconv.FormatInt(int64(t.Year()), 10) + month + day
	if date == items[3] {
		minor++
	} else {
		major++
		minor = 0
	}
	return items[0] + BRANCH_DELIM + strconv.FormatInt(major, 10) +
		BRANCH_DELIM + strconv.FormatInt(minor, 10) + BRANCH_DELIM + date, nil
}

func IsValid(branchName string) bool {
	if len(branchName) == 0 {
		return false
	}
	items := strings.Split(branchName, BRANCH_DELIM)
	if len(items) != BRANCH_ITEM_LEN {
		return false
	}
	for i := 1; i < 3; i++ {
		if _, err := strconv.ParseInt(items[i], 10, 64); err != nil {
			return false
		}
	}
	return true
}

func GenInitName(distribution string) string {
	t := time.Now()
	month := strconv.FormatInt(int64(t.Month()), 10)
	if len(month) == 1 {
		month = "0" + month
	}
	day := strconv.FormatInt(int64(t.Day()), 10)
	if len(day) == 1 {
		day = "0" + day
	}
	date := strconv.FormatInt(int64(t.Year()), 10) + month + day
	return distribution + BRANCH_DELIM + "0" + BRANCH_DELIM + "0" + BRANCH_DELIM + date
}
