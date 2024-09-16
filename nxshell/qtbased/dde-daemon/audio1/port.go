// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"fmt"

	"github.com/linuxdeepin/go-lib/pulse"
)

type Port struct {
	Name        string
	Description string
	Available   byte // Unknow:0, No:1, Yes:2
}

func portAvailToString(v int) string {
	switch v {
	case pulse.AvailableTypeUnknow:
		return "Unknown"
	case pulse.AvailableTypeNo:
		return "No"
	case pulse.AvailableTypeYes:
		return "Yes"
	default:
		return fmt.Sprintf("<invalid available %d>", v)
	}
}

func (p *Port) String() string {
	if p == nil {
		return "<nil>"
	}

	availableStr := portAvailToString(int(p.Available))
	return fmt.Sprintf("<Port name=%q desc=%q available=%s>", p.Name, p.Description, availableStr)
}

func toPort(v pulse.PortInfo) Port {
	return Port{
		Name:        v.Name,
		Description: v.Description,
		Available:   byte(v.Available),
	}
}

func toPorts(portInfoList []pulse.PortInfo) (result []Port) {
	for _, p := range portInfoList {
		result = append(result, toPort(p))
	}
	return
}

// return port and whether found
func getPortByName(ports []Port, name string) (Port, bool) {
	if name == "" {
		return Port{}, false
	}
	for _, port := range ports {
		if port.Name == name {
			return port, true
		}
	}
	return Port{}, false
}

func portsEqual(a, b []Port) bool {
	if a == nil && b == nil {
		return true
	}

	if a == nil || b == nil {
		return false
	}

	if len(a) != len(b) {
		return false
	}

	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}

	return true
}
