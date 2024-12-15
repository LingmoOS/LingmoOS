// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

func isItemInList(item string, list []string) bool {
	for _, v := range list {
		if v == item {
			return true
		}
	}

	return false
}

func addItemToList(item string, list []string) ([]string, bool) {
	if isItemInList(item, list) {
		return list, false
	}

	list = append(list, item)
	return list, true
}

func deleteItemFromList(item string, list []string) ([]string, bool) {
	var (
		ret   []string
		found bool = false
	)
	for _, v := range list {
		if v == item {
			found = true
			continue
		}

		ret = append(ret, v)
	}

	return ret, found
}

func filterNilString(list []string) ([]string, bool) {
	var (
		ret    []string
		hasNil bool = false
	)
	for _, v := range list {
		if len(v) == 0 {
			hasNil = true
			continue
		}
		ret = append(ret, v)
	}

	return ret, hasNil
}
