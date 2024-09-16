// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"sort"

	"github.com/linuxdeepin/go-lib/pulse"
)

type Profile struct {
	Name        string
	Description string

	// The higher this value is, the more useful this profile is as a default.
	Priority uint32

	// 如果值是 0, 表示这个配置不可用，无法被激活
	// 如果值不为 0, 也不能保证此配置是可用的，它仅仅意味着不能肯定它是不可用的
	Available int
}

func newProfile(info pulse.ProfileInfo2) *Profile {
	return &Profile{
		Name:        info.Name,
		Description: info.Description,
		Priority:    info.Priority,
		Available:   info.Available,
	}
}

type ProfileList []*Profile

func newProfileList(src []pulse.ProfileInfo2) ProfileList {
	var result ProfileList
	for _, v := range src {
		result = append(result, newProfile(v))
	}
	return result
}

func getCommonProfiles(info1, info2 pulse.CardPortInfo) pulse.ProfileInfos2 {
	var commons pulse.ProfileInfos2
	if len(info1.Profiles) == 0 || len(info2.Profiles) == 0 {
		return commons
	}
	for _, profile := range info1.Profiles {
		if !info2.Profiles.Exists(profile.Name) {
			continue
		}
		commons = append(commons, profile)
	}
	if len(commons) != 0 {
		sort.Sort(commons)
	}
	return commons
}
