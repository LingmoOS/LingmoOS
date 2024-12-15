// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

type ItemInfo struct {
	Path          string
	Name          string // display name
	ID            string
	Icon          string
	CategoryID    CategoryID
	TimeInstalled int64
}

func (item *Item) newItemInfo() ItemInfo {
	iInfo := ItemInfo{
		Path:          item.Path,
		Name:          item.Name,
		ID:            item.ID,
		Icon:          item.Icon,
		CategoryID:    item.CategoryID,
		TimeInstalled: item.TimeInstalled,
	}
	return iInfo
}
