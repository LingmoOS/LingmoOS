// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package service_trigger

func Tr(in string) string {
	return in
}

var _ = Tr("\"%s\" did not pass the system security verification, and cannot run now")

var _ = Tr("You cannot run the unverified \"%s\", but you can change the settings in Security Center.")

var _ = Tr("Proceed")
