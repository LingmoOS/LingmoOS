// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

// EntryType is used to define entry's type in '/boot/grub/grub.cfg'.
type EntryType int

const (
	MENUENTRY EntryType = iota
	SUBMENU
)

// Entry is a struct to store each entry's data/
type Entry struct {
	entryType     EntryType
	title         string
	num           int
	parentSubMenu *Entry
}

func (entry *Entry) getFullTitle() string {
	if entry.parentSubMenu != nil {
		return entry.parentSubMenu.getFullTitle() + ">" + entry.title
	}
	return entry.title
}
